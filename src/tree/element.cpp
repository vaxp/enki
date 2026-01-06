/// @file element.cpp
/// @brief Element lifecycle and reconciliation implementations.
///
/// This file contains the core of ENKI's tree management:
///   - Element lifecycle (mount, unmount, activate, deactivate)
///   - The reconciliation algorithm (updateChild, inflateWidget)
///   - BuildOwner for managing the dirty element queue
///   - All Element subclass implementations

#include "enki/tree/element.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/tree/build_context.hpp"
#include <algorithm>
#include <sstream>
#include <cassert>
#include <iostream>
#include <chrono>

#include "enki/state/state.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// Element — Base Implementation
// ════════════════════════════════════════════════════════════════

Element::Element(WidgetPtr widget)
    : widget_(std::move(widget)) {
    assert(widget_ && "Element created with null widget");
}

Element::~Element() = default;

void Element::mount(Element* parent, size_t slot) {
    assert(lifecycle_ == ElementLifecycle::Initial ||
           lifecycle_ == ElementLifecycle::Inactive);

    parent_ = parent;
    slot_ = slot;
    depth_ = parent ? parent->depth_ + 1 : 0;
    lifecycle_ = ElementLifecycle::Active;

    // Inherit the BuildOwner from parent
    if (parent && parent->owner_) {
        owner_ = parent->owner_;
    }

    // If already dirty, schedule a build
    if (dirty_) {
        if (owner_) {
            owner_->scheduleBuild(this);
        }
    }
}

void Element::unmount() {
    assert(lifecycle_ == ElementLifecycle::Active ||
           lifecycle_ == ElementLifecycle::Inactive);
    lifecycle_ = ElementLifecycle::Defunct;
    widget_ = nullptr;
    parent_ = nullptr;
}

void Element::activate() {
    assert(lifecycle_ == ElementLifecycle::Inactive);
    lifecycle_ = ElementLifecycle::Active;

    // Re-inherit owner from parent
    if (parent_ && parent_->owner_) {
        owner_ = parent_->owner_;
    }

    // Mark for rebuild since we may have missed updates
    markNeedsBuild();
}

void Element::deactivate() {
    assert(lifecycle_ == ElementLifecycle::Active);
    lifecycle_ = ElementLifecycle::Inactive;
}

void Element::update(WidgetPtr newWidget) {
    assert(lifecycle_ == ElementLifecycle::Active);
    assert(newWidget);
    assert(widget_->canUpdate(*newWidget) &&
           "Cannot update element with incompatible widget type");
    widget_ = std::move(newWidget);
}

void Element::markNeedsBuild() {
    if (dirty_) return;
    dirty_ = true;
    if (owner_ && lifecycle_ == ElementLifecycle::Active) {
        owner_->scheduleBuild(this);
    }
}

void Element::rebuild(bool force) {
    if (lifecycle_ != ElementLifecycle::Active) return;
    if (!dirty_ && !force) return;

    performRebuild();
    dirty_ = false;
}

Element* Element::updateChild(Element* child, WidgetPtr newWidget, size_t newSlot) {
    // Case 1: Remove child (new widget is null)
    if (!newWidget) {
        if (child) {
            deactivateChild(child);
        }
        return nullptr;
    }

    // Case 2: Create new child (no existing child)
    if (!child) {
        return inflateWidget(std::move(newWidget), newSlot);
    }

    // Case 3: Update existing child (same type + key)
    if (child->widget()->canUpdate(*newWidget)) {
        if (child->slot_ != newSlot) {
            child->slot_ = newSlot;
        }
        child->update(std::move(newWidget));
        return child;
    }

    // Case 4: Replace child (different type or key)
    deactivateChild(child);
    return inflateWidget(std::move(newWidget), newSlot);
}

Element* Element::inflateWidget(WidgetPtr widget, size_t slot) {
    assert(widget);
    auto element = widget->createElement();
    assert(element);

    element->mount(this, slot);
    return element.release();  // Caller takes ownership
}

void Element::deactivateChild(Element* child) {
    assert(child);
    assert(child->parent_ == this);
    child->deactivate();
    child->unmount();
}

RenderObject* Element::findRenderObject() {
    // Default: walk down to find the first render object
    RenderObject* result = nullptr;
    visitChildren([&](Element& child) {
        if (!result) {
            result = child.findRenderObject();
        }
    });
    return result;
}

void Element::visitAncestors(const std::function<bool(Element&)>& visitor) {
    Element* ancestor = parent_;
    while (ancestor) {
        if (!visitor(*ancestor)) break;
        ancestor = ancestor->parent_;
    }
}

std::string Element::debugDescription() const {
    std::ostringstream oss;
    oss << (widget_ ? widget_->debugDescription() : "<defunct>")
        << " (depth=" << depth_ << ", slot=" << slot_ << ")";
    return oss.str();
}

size_t Element::subtreeSize() const {
    size_t count = 1;
    const_cast<Element*>(this)->visitChildren([&](Element& child) {
        count += child.subtreeSize();
    });
    return count;
}

// ════════════════════════════════════════════════════════════════
// BuildOwner
// ════════════════════════════════════════════════════════════════

void BuildOwner::scheduleBuild(Element* element) {
    assert(element);
    // Avoid duplicates
    if (std::find(dirty_elements_.begin(), dirty_elements_.end(), element)
        == dirty_elements_.end()) {
        dirty_elements_.push_back(element);
    }
}

void BuildOwner::buildScope(Element* root) {
    if (building_) return;  // Prevent re-entrant builds
    building_ = true;

    // Sort by depth (parents before children)
    std::sort(dirty_elements_.begin(), dirty_elements_.end(),
              [](const Element* a, const Element* b) {
                  return a->depth() < b->depth();
              });

    // Rebuild all dirty elements
    for (size_t i = 0; i < dirty_elements_.size(); ++i) {
        Element* element = dirty_elements_[i];
        // Element may have been unmounted during a parent rebuild
        if (element->lifecycle() == ElementLifecycle::Active) {
            element->rebuild();
        }
    }

    dirty_elements_.clear();
    building_ = false;
}

// ════════════════════════════════════════════════════════════════
// StatelessElement
// ════════════════════════════════════════════════════════════════

StatelessElement::StatelessElement(WidgetPtr widget)
    : Element(std::move(widget)) {}

void StatelessElement::update(WidgetPtr newWidget) {
    Element::update(std::move(newWidget));
    rebuild(true);
}

void StatelessElement::unmount() {
    if (child_) {
        deactivateChild(child_.get());
        child_.reset();
    }
    Element::unmount();
}

void StatelessElement::performRebuild() {
    auto* slw = dynamic_cast<StatelessWidget*>(widget_.get());
    assert(slw && "StatelessElement must be backed by a StatelessWidget");

    BuildContext ctx(this);
    WidgetPtr built = slw->build(ctx);

    // Reconcile the single child
    Element* old_child = child_.get();
    Element* new_child = updateChild(old_child, std::move(built), 0);

    if (new_child != old_child) {
        child_.reset(new_child);
    }
}

void StatelessElement::visitChildren(const std::function<void(Element&)>& visitor) {
    if (child_) {
        visitor(*child_);
    }
}

RenderObject* StatelessElement::findRenderObject() {
    if (child_) return child_->findRenderObject();
    return nullptr;
}

// ════════════════════════════════════════════════════════════════
// StatefulElement
// ════════════════════════════════════════════════════════════════

StatefulElement::StatefulElement(WidgetPtr widget)
    : Element(std::move(widget)) {
    // Create the State object immediately
    auto* sfw = dynamic_cast<StatefulWidget*>(widget_.get());
    assert(sfw && "StatefulElement must be backed by a StatefulWidget");
    state_ = sfw->createState();
    assert(state_ && "createState() must return a non-null State");
    state_->element_ = this;
}

void StatefulElement::mount(Element* parent, size_t slot) {
    Element::mount(parent, slot);

    // Initialize the state
    state_->initState();
    state_->didChangeDependencies();
}

void StatefulElement::unmount() {
    // Dispose the state before unmounting
    state_->dispose();
    state_->mounted_ = false;
    state_->element_ = nullptr;

    if (child_) {
        deactivateChild(child_.get());
        child_.reset();
    }

    Element::unmount();
}

void StatefulElement::update(WidgetPtr newWidget) {
    auto oldWidget = widget_;
    Element::update(std::move(newWidget));

    // Notify state that the widget config changed
    state_->didUpdateWidget(*oldWidget);
    rebuild(true);
}

void StatefulElement::performRebuild() {
    assert(state_);
    BuildContext ctx(this);
    WidgetPtr built = state_->build(ctx);

    Element* old_child = child_.get();
    Element* new_child = updateChild(old_child, std::move(built), 0);

    if (new_child != old_child) {
        child_.reset(new_child);
    }
}

void StatefulElement::visitChildren(const std::function<void(Element&)>& visitor) {
    if (child_) {
        visitor(*child_);
    }
}

RenderObject* StatefulElement::findRenderObject() {
    if (child_) return child_->findRenderObject();
    return nullptr;
}

// ════════════════════════════════════════════════════════════════
// RenderObjectElement
// ════════════════════════════════════════════════════════════════

RenderObjectElement::RenderObjectElement(WidgetPtr widget)
    : Element(std::move(widget)) {}

void RenderObjectElement::mount(Element* parent, size_t slot) {
    Element::mount(parent, slot);

    // Create the render object
    auto* row_widget = dynamic_cast<RenderObjectWidget*>(widget_.get());
    assert(row_widget && "RenderObjectElement must be backed by a RenderObjectWidget");

    BuildContext ctx(this);
    render_object_ = row_widget->createRenderObject(ctx);
    assert(render_object_ && "createRenderObject() must return a non-null RenderObject");
    render_object_->setOwnerElement(this);

    // Attach to the render tree
    attachRenderObject();
}

void RenderObjectElement::unmount() {
    auto* row_widget = dynamic_cast<RenderObjectWidget*>(widget_.get());
    if (row_widget && render_object_) {
        row_widget->didUnmountRenderObject(*render_object_);
    }

    detachRenderObject();
    render_object_.reset();

    Element::unmount();
}

void RenderObjectElement::update(WidgetPtr newWidget) {
    Element::update(std::move(newWidget));

    // Update the render object with the new widget config
    auto* row_widget = dynamic_cast<RenderObjectWidget*>(widget_.get());
    assert(row_widget);

    BuildContext ctx(this);
    row_widget->updateRenderObject(ctx, *render_object_);
}

RenderObject* RenderObjectElement::findRenderObject() {
    return render_object_.get();
}

void RenderObjectElement::attachRenderObject() {
    if (!render_object_) return;

    RenderObject* parent_ro = findAncestorRenderObject();
    if (parent_ro) {
        parent_ro->addChild(render_object_.get());
    }
}

void RenderObjectElement::detachRenderObject() {
    if (!render_object_) return;

    RenderObject* parent_ro = render_object_->parentRenderObject();
    if (parent_ro) {
        parent_ro->removeChild(render_object_.get());
    }
}

RenderObject* RenderObjectElement::findAncestorRenderObject() const {
    Element* ancestor = parent_;
    while (ancestor) {
        if (auto* ro_element = dynamic_cast<RenderObjectElement*>(ancestor)) {
            return ro_element->render_object_.get();
        }
        ancestor = ancestor->parent();
    }
    return nullptr;
}

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectElement
// ════════════════════════════════════════════════════════════════

SingleChildRenderObjectElement::SingleChildRenderObjectElement(WidgetPtr widget)
    : RenderObjectElement(std::move(widget)) {}

void SingleChildRenderObjectElement::mount(Element* parent, size_t slot) {
    RenderObjectElement::mount(parent, slot);

    // Inflate the single child if present
    auto* sc_widget = dynamic_cast<SingleChildRenderObjectWidget*>(widget_.get());
    if (sc_widget && sc_widget->child) {
        Element* child = inflateWidget(sc_widget->child, 0);
        child_.reset(child);
    }
}

void SingleChildRenderObjectElement::unmount() {
    if (child_) {
        deactivateChild(child_.get());
        child_.reset();
    }
    RenderObjectElement::unmount();
}

void SingleChildRenderObjectElement::update(WidgetPtr newWidget) {
    RenderObjectElement::update(std::move(newWidget));

    auto* sc_widget = dynamic_cast<SingleChildRenderObjectWidget*>(widget_.get());
    assert(sc_widget);

    WidgetPtr new_child_widget = sc_widget->child;
    Element* old_child = child_.get();
    Element* new_child = updateChild(old_child, std::move(new_child_widget), 0);

    if (new_child != old_child) {
        child_.reset(new_child);
    }
}

void SingleChildRenderObjectElement::performRebuild() {
    auto* sc_widget = dynamic_cast<SingleChildRenderObjectWidget*>(widget_.get());
    assert(sc_widget);

    // Update the render object
    BuildContext ctx(this);
    sc_widget->updateRenderObject(ctx, *render_object_);

    // Reconcile the single child
    WidgetPtr new_child_widget = sc_widget->child;
    Element* old_child = child_.get();
    Element* new_child = updateChild(old_child, std::move(new_child_widget), 0);

    if (new_child != old_child) {
        child_.reset(new_child);
    }
}

void SingleChildRenderObjectElement::visitChildren(
    const std::function<void(Element&)>& visitor) {
    if (child_) {
        visitor(*child_);
    }
}

// ════════════════════════════════════════════════════════════════
// MultiChildRenderObjectElement
// ════════════════════════════════════════════════════════════════

MultiChildRenderObjectElement::MultiChildRenderObjectElement(WidgetPtr widget)
    : RenderObjectElement(std::move(widget)) {}

void MultiChildRenderObjectElement::mount(Element* parent, size_t slot) {
    RenderObjectElement::mount(parent, slot);

    // Inflate all children
    auto* mc_widget = dynamic_cast<MultiChildRenderObjectWidget*>(widget_.get());
    assert(mc_widget);

    children_.reserve(mc_widget->children.size());
    for (size_t i = 0; i < mc_widget->children.size(); ++i) {
        if (mc_widget->children[i]) {
            Element* child = inflateWidget(mc_widget->children[i], i);
            children_.emplace_back(child);
        }
    }
}

void MultiChildRenderObjectElement::unmount() {
    for (auto& child : children_) {
        if (child) {
            deactivateChild(child.get());
        }
    }
    children_.clear();
    RenderObjectElement::unmount();
}

void MultiChildRenderObjectElement::update(WidgetPtr newWidget) {
    RenderObjectElement::update(std::move(newWidget));

    auto* mc_widget = dynamic_cast<MultiChildRenderObjectWidget*>(widget_.get());
    assert(mc_widget);

    updateChildren(mc_widget->children);
}

void MultiChildRenderObjectElement::performRebuild() {
    auto* mc_widget = dynamic_cast<MultiChildRenderObjectWidget*>(widget_.get());
    assert(mc_widget);

    // Update the render object
    BuildContext ctx(this);
    mc_widget->updateRenderObject(ctx, *render_object_);

    // Reconcile children
    updateChildren(mc_widget->children);
}

void MultiChildRenderObjectElement::updateChildren(
    const std::vector<WidgetPtr>& newWidgets) {

    // ═══════════════════════════════════════════════════════════
    // O(n) reconciliation algorithm ( -inspired)
    // ═══════════════════════════════════════════════════════════
    //
    // The algorithm works in three passes:
    //   1. Forward scan: match from the beginning.
    //   2. Backward scan: match from the end.
    //   3. Key-based matching: match remaining by key.
    //
    // This handles common cases efficiently:
    //   - Appending/prepending items: O(n)
    //   - Removing items: O(n)
    //   - Reordering by key: O(n)

    const size_t old_count = children_.size();
    const size_t new_count = newWidgets.size();

    if (new_count == 0) {
        // Remove all children
        for (auto& child : children_) {
            if (child) deactivateChild(child.get());
        }
        children_.clear();
        return;
    }

    if (old_count == 0) {
        // Create all new children
        children_.reserve(new_count);
        for (size_t i = 0; i < new_count; ++i) {
            if (newWidgets[i]) {
                children_.emplace_back(inflateWidget(newWidgets[i], i));
            } else {
                children_.emplace_back(nullptr);
            }
        }
        return;
    }

    // ── Pass 1: Forward scan ───────────────────────────────────
    size_t new_top = 0;
    size_t old_top = 0;

    while (old_top < old_count && new_top < new_count) {
        Element* old_child = children_[old_top].get();
        if (!old_child || !newWidgets[new_top]) break;
        if (!old_child->widget()->canUpdate(*newWidgets[new_top])) break;
        ++old_top;
        ++new_top;
    }

    // ── Pass 2: Backward scan ──────────────────────────────────
    size_t old_bottom = old_count - 1;
    size_t new_bottom = new_count - 1;

    while (old_bottom >= old_top && new_bottom >= new_top && old_bottom < old_count) {
        Element* old_child = children_[old_bottom].get();
        if (!old_child || !newWidgets[new_bottom]) break;
        if (!old_child->widget()->canUpdate(*newWidgets[new_bottom])) break;
        if (old_bottom == 0) break; // Prevent underflow
        --old_bottom;
        --new_bottom;
    }

    // ── Pass 3: Key-based matching for the middle section ──────
    // Collect keyed old children in the unmatched middle range
    std::unordered_map<Key, size_t> old_keyed;
    for (size_t i = old_top; i <= old_bottom && i < old_count; ++i) {
        Element* child = children_[i].get();
        if (child && child->widget()->key.hasValue()) {
            old_keyed[child->widget()->key] = i;
        }
    }

    // Build the new children list
    std::vector<std::unique_ptr<Element>> new_children(new_count);

    // Copy forward-matched elements (update them)
    for (size_t i = 0; i < old_top; ++i) {
        Element* old_child = children_[i].get();
        old_child->update(newWidgets[i]);
        old_child->rebuild(true);
        old_child->updateSlot(i);
        new_children[i] = std::move(children_[i]);
    }

    // Handle the middle section (new_top..new_bottom)
    for (size_t i = new_top; i <= new_bottom && i < new_count; ++i) {
        Element* reused = nullptr;

        // Try to find a keyed match
        if (newWidgets[i] && newWidgets[i]->key.hasValue()) {
            auto it = old_keyed.find(newWidgets[i]->key);
            if (it != old_keyed.end()) {
                size_t old_idx = it->second;
                Element* old_child = children_[old_idx].get();
                if (old_child && old_child->widget()->canUpdate(*newWidgets[i])) {
                    old_child->update(newWidgets[i]);
                    old_child->rebuild(true);
                    old_child->updateSlot(i);
                    new_children[i] = std::move(children_[old_idx]);
                    old_keyed.erase(it);
                    reused = old_child;
                }
            }
        }

        if (!reused && newWidgets[i]) {
            // Create a new element
            new_children[i].reset(inflateWidget(newWidgets[i], i));
        }
    }

    // Copy backward-matched elements (update them)
    {
        size_t old_i = old_bottom + 1;
        size_t new_i = new_bottom + 1;
        while (old_i < old_count && new_i < new_count) {
            Element* old_child = children_[old_i].get();
            if (old_child) {
                old_child->update(newWidgets[new_i]);
                old_child->rebuild(true);
                old_child->updateSlot(new_i);
            }
            new_children[new_i] = std::move(children_[old_i]);
            ++old_i;
            ++new_i;
        }
    }

    // Deactivate unmatched old children
    for (size_t i = old_top; i <= old_bottom && i < old_count; ++i) {
        if (children_[i]) {
            deactivateChild(children_[i].get());
            children_[i].reset();
        }
    }

    // Deactivate any remaining keyed old children that weren't reused
    // (already handled above since we moved them out)

    children_ = std::move(new_children);
}

void MultiChildRenderObjectElement::visitChildren(
    const std::function<void(Element&)>& visitor) {
    for (auto& child : children_) {
        if (child) {
            visitor(*child);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// ProxyElement
// ════════════════════════════════════════════════════════════════

ProxyElement::ProxyElement(WidgetPtr widget)
    : Element(std::move(widget)) {}

void ProxyElement::mount(Element* parent, size_t slot) {
    Element::mount(parent, slot);

    auto* proxy_widget = dynamic_cast<ProxyWidget*>(widget_.get());
    assert(proxy_widget);

    if (proxy_widget->child) {
        Element* child = inflateWidget(proxy_widget->child, 0);
        child_.reset(child);
    }
}

void ProxyElement::unmount() {
    if (child_) {
        deactivateChild(child_.get());
        child_.reset();
    }
    dependents_.clear();
    Element::unmount();
}

void ProxyElement::update(WidgetPtr newWidget) {
    auto oldWidget = widget_;
    Element::update(std::move(newWidget));

    auto* old_proxy = dynamic_cast<ProxyWidget*>(oldWidget.get());
    auto* new_proxy = dynamic_cast<ProxyWidget*>(widget_.get());

    if (new_proxy && old_proxy) {
        if (new_proxy->updateShouldNotify(*old_proxy)) {
            notifyDependents();
        }
    }

    markNeedsBuild();
}

void ProxyElement::performRebuild() {
    auto* proxy_widget = dynamic_cast<ProxyWidget*>(widget_.get());
    assert(proxy_widget);

    WidgetPtr new_child_widget = proxy_widget->child;
    Element* old_child = child_.get();
    Element* new_child = updateChild(old_child, std::move(new_child_widget), 0);

    if (new_child != old_child) {
        child_.reset(new_child);
    }
}

void ProxyElement::visitChildren(const std::function<void(Element&)>& visitor) {
    if (child_) {
        visitor(*child_);
    }
}

RenderObject* ProxyElement::findRenderObject() {
    if (child_) return child_->findRenderObject();
    return nullptr;
}

void ProxyElement::addDependent(Element* dependent) {
    assert(dependent);
    if (std::find(dependents_.begin(), dependents_.end(), dependent)
        == dependents_.end()) {
        dependents_.push_back(dependent);
    }
}

void ProxyElement::removeDependent(Element* dependent) {
    dependents_.erase(
        std::remove(dependents_.begin(), dependents_.end(), dependent),
        dependents_.end());
}

void ProxyElement::notifyDependents() {
    for (Element* dep : dependents_) {
        if (dep->lifecycle() == ElementLifecycle::Active) {
            dep->markNeedsBuild();
        }
    }
}

}  // namespace enki
