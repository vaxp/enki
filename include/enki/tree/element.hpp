#pragma once
/// @file element.hpp
/// @brief Element classes — the lifecycle and state management layer.
///
/// Elements are the second tree in the Three-Tree Architecture. They are
/// long-lived objects that bridge the gap between immutable Widgets
/// (rebuilt every frame) and RenderObjects (expensive to create).
///
/// Element responsibilities:
///   - Manage the lifecycle of widgets (mount → build → update → unmount).
///   - Hold references to parent/child elements (forming the element tree).
///   - Track whether a rebuild is needed (dirty flag).
///   - Orchestrate the reconciliation of old and new widget trees.
///   - Create and maintain RenderObjects (for RenderObjectElements).
///
/// Element lifecycle:
///   Initial → Active (mounted) → [rebuilds...] → Inactive (deactivated) → Defunct (unmounted)
///
/// Element subclasses:
///   - StatelessElement:                  Manages a StatelessWidget.
///   - StatefulElement:                   Manages a StatefulWidget + State.
///   - SingleChildRenderObjectElement:    Manages a SingleChildRenderObjectWidget.
///   - MultiChildRenderObjectElement:     Manages a MultiChildRenderObjectWidget.
///   - ProxyElement:                      Manages a ProxyWidget.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/key.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <cassert>
#include <algorithm>
#include <unordered_map>

namespace enki {

// Forward declarations
class Widget;
class StatelessWidget;
class StatefulWidget;
class RenderObjectWidget;
class SingleChildRenderObjectWidget;
class MultiChildRenderObjectWidget;
class ProxyWidget;
class State;
class RenderObject;
class BuildContext;
class BuildOwner;

using WidgetPtr = std::shared_ptr<Widget>;

/// Element lifecycle states — tracks where an element is in its lifetime.
enum class ElementLifecycle : uint8_t {
    Initial,    ///< Just created, not yet mounted.
    Active,     ///< Mounted and participating in the tree.
    Inactive,   ///< Deactivated — removed from tree but may be reactivated.
    Defunct,     ///< Permanently removed, resources released.
};

// ════════════════════════════════════════════════════════════════
// Element — Abstract Base
// ════════════════════════════════════════════════════════════════

/// @brief The core element in the element tree — manages widget lifecycle.
///
/// Element is the bridge between the declarative Widget tree and the
/// mutable RenderObject tree. Each Element holds a reference to its
/// current Widget and is responsible for creating, updating, and
/// destroying the corresponding RenderObjects.
///
/// Elements are created by `Widget::createElement()` and are managed
/// by the framework's build pipeline.
class Element {
public:
    explicit Element(WidgetPtr widget);
    virtual ~Element();

    // Non-copyable, non-movable (identity-bound)
    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;
    Element(Element&&) = delete;
    Element& operator=(Element&&) = delete;

    // ── Lifecycle Methods ──────────────────────────────────────

    /// Mount this element into the tree.
    /// Called when the element is first inserted into the element tree.
    ///
    /// @param parent The parent element (nullptr for root).
    /// @param slot   The position index within the parent's children.
    virtual void mount(Element* parent, size_t slot);

    /// Unmount this element from the tree permanently.
    /// Called when the element is removed and will not be reinserted.
    /// After this call, the element is in the Defunct state.
    virtual void unmount();

    /// Activate a previously deactivated element.
    /// Called when the element is reinserted into the tree after deactivation.
    virtual void activate();

    /// Deactivate this element (remove from tree, may be reactivated).
    /// Called when the element's widget is removed from the tree but the
    /// element might be reinserted at a different position (e.g., due to
    /// key-based reordering).
    virtual void deactivate();

    // ── Update / Rebuild ───────────────────────────────────────

    /// Update this element with a new widget configuration.
    /// Called during reconciliation when canUpdate() returns true.
    ///
    /// @param newWidget The new widget to adopt.
    virtual void update(WidgetPtr newWidget);

    /// Mark this element as needing a rebuild.
    /// The rebuild will happen before the next frame is painted.
    void markNeedsBuild();

    /// Perform the rebuild. Called by the framework's build pipeline.
    /// Subclasses must override `performRebuild()` to implement their
    /// specific rebuild logic.
    virtual void rebuild(bool force = false);

    // ── Reconciliation ─────────────────────────────────────────

    /// The core reconciliation method. Subclasses implement this to
    /// diff their old and new child widgets and update the element tree.
    virtual void performRebuild() = 0;

    /// Update a single child element with a new widget.
    ///
    /// This is the heart of the reconciliation algorithm. It decides
    /// whether to reuse, replace, or remove an existing child element.
    ///
    /// @param child     The existing child element (may be nullptr).
    /// @param newWidget The new widget for this slot (may be nullptr to remove).
    /// @param newSlot   The new position index.
    /// @return The updated child element (may be the same or a new one).
    Element* updateChild(Element* child, WidgetPtr newWidget, size_t newSlot);

    /// Inflate a widget into a new element and mount it.
    ///
    /// @param widget The widget to inflate.
    /// @param slot   The position index.
    /// @return The newly created and mounted element.
    Element* inflateWidget(WidgetPtr widget, size_t slot);

    /// Deactivate and unmount a child element.
    ///
    /// @param child The child element to remove.
    void deactivateChild(Element* child);

    // ── RenderObject Access ────────────────────────────────────

    /// Find the nearest RenderObject in or below this element.
    /// For RenderObjectElements, returns their own render object.
    /// For other elements, walks down the child tree.
    [[nodiscard]] virtual RenderObject* findRenderObject();

    // ── Ancestor Lookup ────────────────────────────────────────

    /// Walk up the element tree to find an ancestor of a specific type.
    ///
    /// @tparam T The element type to search for.
    /// @return Pointer to the ancestor, or nullptr if not found.
    template<typename T>
    [[nodiscard]] T* findAncestorElementOfType() const {
        Element* ancestor = parent_;
        while (ancestor != nullptr) {
            if (auto* result = dynamic_cast<T*>(ancestor)) {
                return result;
            }
            ancestor = ancestor->parent_;
        }
        return nullptr;
    }

    /// Walk up to find an ancestor widget of a specific type.
    template<typename T>
    [[nodiscard]] T* findAncestorWidgetOfType() const {
        Element* ancestor = parent_;
        while (ancestor != nullptr) {
            if (auto* result = dynamic_cast<T*>(ancestor->widget_.get())) {
                return result;
            }
            ancestor = ancestor->parent_;
        }
        return nullptr;
    }

    // ── Visitor / Tree Traversal ───────────────────────────────

    /// Visit this element and all its descendants.
    /// @param visitor The callback to invoke for each element.
    virtual void visitChildren(const std::function<void(Element&)>& visitor) = 0;

    /// Visit only the ancestor chain (upward traversal).
    void visitAncestors(const std::function<bool(Element&)>& visitor);

    // ── Accessors ──────────────────────────────────────────────

    /// @return The current widget.
    [[nodiscard]] WidgetPtr widget() const { return widget_; }

    /// @return The parent element (nullptr for root).
    [[nodiscard]] Element* parent() const { return parent_; }

    /// @return The depth of this element in the tree (root = 0).
    [[nodiscard]] size_t depth() const { return depth_; }

    /// @return The current lifecycle state.
    [[nodiscard]] ElementLifecycle lifecycle() const { return lifecycle_; }

    /// @return true if this element needs rebuilding.
    [[nodiscard]] bool dirty() const { return dirty_; }

    /// @return The slot index within the parent.
    [[nodiscard]] size_t slot() const { return slot_; }

    /// Update the slot index.
    void updateSlot(size_t new_slot) { slot_ = new_slot; }

    /// @return The BuildOwner that manages this element's build pipeline.
    [[nodiscard]] BuildOwner* owner() const { return owner_; }

    /// Set the BuildOwner for this element.
    void setOwner(BuildOwner* owner) { owner_ = owner; }

    // ── Debug ──────────────────────────────────────────────────

    /// @return A human-readable debug description.
    [[nodiscard]] virtual std::string debugDescription() const;

    /// @return The total number of elements in this subtree (including self).
    [[nodiscard]] size_t subtreeSize() const;

protected:
    WidgetPtr widget_;                                   ///< Current widget.
    Element* parent_ = nullptr;                          ///< Parent element.
    BuildOwner* owner_ = nullptr;                        ///< Build pipeline owner.
    size_t depth_ = 0;                                   ///< Tree depth.
    size_t slot_ = 0;                                    ///< Position in parent.
    bool dirty_ = true;                                  ///< Needs rebuild?
    ElementLifecycle lifecycle_ = ElementLifecycle::Initial;
};

// ════════════════════════════════════════════════════════════════
// BuildOwner — manages the build pipeline
// ════════════════════════════════════════════════════════════════

/// @brief Manages the list of dirty elements and orchestrates rebuilds.
///
/// BuildOwner is the central coordinator for the build phase. When an
/// element marks itself dirty (via markNeedsBuild), it registers with
/// the BuildOwner. Before each frame, the BuildOwner rebuilds all dirty
/// elements in depth order (parents before children).
class BuildOwner {
public:
    BuildOwner() = default;
    ~BuildOwner() = default;

    /// Schedule an element for rebuilding.
    void scheduleBuild(Element* element);

    /// Rebuild all dirty elements in depth order.
    /// Called once per frame before layout and paint.
    void buildScope(Element* root);

    /// @return The number of elements pending rebuild.
    [[nodiscard]] size_t dirtyElementCount() const { return dirty_elements_.size(); }

    /// @return true if there are elements pending rebuild.
    [[nodiscard]] bool hasDirtyElements() const { return !dirty_elements_.empty(); }

private:
    std::vector<Element*> dirty_elements_;
    bool building_ = false;
};

// ════════════════════════════════════════════════════════════════
// StatelessElement
// ════════════════════════════════════════════════════════════════

/// @brief Element for StatelessWidget — simply calls build() and manages one child.
class StatelessElement : public Element {
public:
    explicit StatelessElement(WidgetPtr widget);

    void update(WidgetPtr newWidget) override;
    void unmount() override;
    void performRebuild() override;
    void visitChildren(const std::function<void(Element&)>& visitor) override;

    [[nodiscard]] RenderObject* findRenderObject() override;

private:
    std::unique_ptr<Element> child_;
};

// ════════════════════════════════════════════════════════════════
// StatefulElement
// ════════════════════════════════════════════════════════════════

/// @brief Element for StatefulWidget — manages the State object and its lifecycle.
class StatefulElement : public Element {
public:
    explicit StatefulElement(WidgetPtr widget);

    void mount(Element* parent, size_t slot) override;
    void unmount() override;
    void update(WidgetPtr newWidget) override;
    void performRebuild() override;
    void visitChildren(const std::function<void(Element&)>& visitor) override;

    [[nodiscard]] RenderObject* findRenderObject() override;

    /// @return The State object managed by this element.
    [[nodiscard]] State* state() const { return state_.get(); }

private:
    std::unique_ptr<State> state_;
    std::unique_ptr<Element> child_;
};

// ════════════════════════════════════════════════════════════════
// RenderObjectElement (abstract base for render elements)
// ════════════════════════════════════════════════════════════════

/// @brief Base element for widgets that create RenderObjects.
///
/// Handles creating, updating, and destroying the render object,
/// as well as integrating it into the render object tree.
class RenderObjectElement : public Element {
public:
    explicit RenderObjectElement(WidgetPtr widget);

    void mount(Element* parent, size_t slot) override;
    void unmount() override;
    void update(WidgetPtr newWidget) override;

    [[nodiscard]] RenderObject* findRenderObject() override;

    /// @return The RenderObject owned by this element.
    [[nodiscard]] RenderObject* renderObject() const { return render_object_.get(); }

protected:
    /// Insert the render object into the render tree.
    /// Finds the nearest ancestor RenderObject and attaches as child.
    void attachRenderObject();

    /// Remove the render object from the render tree.
    void detachRenderObject();

    /// Find the nearest ancestor render object (walks up the element tree).
    [[nodiscard]] RenderObject* findAncestorRenderObject() const;

    std::unique_ptr<RenderObject> render_object_;
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectElement
// ════════════════════════════════════════════════════════════════

/// @brief Element for SingleChildRenderObjectWidget — has at most one child.
class SingleChildRenderObjectElement : public RenderObjectElement {
public:
    explicit SingleChildRenderObjectElement(WidgetPtr widget);

    void mount(Element* parent, size_t slot) override;
    void unmount() override;
    void update(WidgetPtr newWidget) override;
    void performRebuild() override;
    void visitChildren(const std::function<void(Element&)>& visitor) override;

private:
    std::unique_ptr<Element> child_;
};

// ════════════════════════════════════════════════════════════════
// MultiChildRenderObjectElement
// ════════════════════════════════════════════════════════════════

/// @brief Element for MultiChildRenderObjectWidget — manages multiple children.
///
/// Implements the full reconciliation algorithm with key-based matching:
///   1. Walk both old and new child lists from the start, matching by canUpdate().
///   2. Walk both from the end, matching backwards.
///   3. Remaining old children are keyed into a map.
///   4. Remaining new children look up reusable elements by key.
///   5. Unmatched old children are deactivated and unmounted.
class MultiChildRenderObjectElement : public RenderObjectElement {
public:
    explicit MultiChildRenderObjectElement(WidgetPtr widget);

    void mount(Element* parent, size_t slot) override;
    void unmount() override;
    void update(WidgetPtr newWidget) override;
    void performRebuild() override;
    void visitChildren(const std::function<void(Element&)>& visitor) override;

    /// @return The list of child elements.
    [[nodiscard]] const std::vector<std::unique_ptr<Element>>& childElements() const {
        return children_;
    }

private:
    /// Reconcile old children with new widgets using the O(n) algorithm.
    void updateChildren(const std::vector<WidgetPtr>& newWidgets);

    std::vector<std::unique_ptr<Element>> children_;
};

// ════════════════════════════════════════════════════════════════
// ProxyElement — for InheritedWidget / Provider pattern
// ════════════════════════════════════════════════════════════════

/// @brief Element for ProxyWidget — propagates data to descendants.
class ProxyElement : public Element {
public:
    explicit ProxyElement(WidgetPtr widget);

    void mount(Element* parent, size_t slot) override;
    void unmount() override;
    void update(WidgetPtr newWidget) override;
    void performRebuild() override;
    void visitChildren(const std::function<void(Element&)>& visitor) override;

    [[nodiscard]] RenderObject* findRenderObject() override;

    /// Register a dependent element.
    void addDependent(Element* dependent);

    /// Remove a dependent element.
    void removeDependent(Element* dependent);

    /// Notify all dependents that the provided data has changed.
    void notifyDependents();

private:
    std::unique_ptr<Element> child_;
    std::vector<Element*> dependents_;
};

}  // namespace enki
