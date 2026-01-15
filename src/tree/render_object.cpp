/// @file render_object.cpp
/// @brief RenderObject implementations — layout, painting, and hit testing.

#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include <algorithm>
#include <sstream>
#include <cassert>
#include <cmath>

#include <unordered_set>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderObject — Base Implementation
// ════════════════════════════════════════════════════════════════

static std::unordered_set<const RenderObject*> s_alive_objects;

RenderObject::RenderObject() {
    s_alive_objects.insert(this);
    anu_node_ = ANUNodeNew();
}

RenderObject::~RenderObject() {
    s_alive_objects.erase(this);
    if (anu_node_) {
        ANUNodeFree(anu_node_);
    }
}

bool RenderObject::isAlive(const RenderObject* ro) {
    if (!ro) return false;
    return s_alive_objects.find(ro) != s_alive_objects.end();
}

void RenderObject::layout(float available_width, float available_height) {
    // Trigger Anu layout on this node (typically root)
    ANUNodeCalculateLayout(anu_node_, available_width, available_height, ANUDirectionLTR);

    // Sync computed layouts from Anu into C++ objects
    syncLayout();
}

void RenderObject::syncLayout() {
    // Read size and position from Anu's computed layout
    size_.width = ANUNodeLayoutGetWidth(anu_node_);
    size_.height = ANUNodeLayoutGetHeight(anu_node_);
    offset_.x = ANUNodeLayoutGetLeft(anu_node_);
    offset_.y = ANUNodeLayoutGetTop(anu_node_);
    
    needs_layout_ = false;
    
    // Pass sync signal down to children natively
    for (auto* child : children_) {
        child->syncLayout();
    }
}

void RenderObject::markNeedsLayout() {
    needs_layout_ = true;
    // Propagate up — parent also needs relayout to account for
    // changed child sizes.
    if (parent_) {
        parent_->markNeedsLayout();
    }
}

void RenderObject::markNeedsPaint() {
    needs_paint_ = true;
    // Propagate up so ancestors know their subtree needs repaint
    if (parent_) {
        parent_->markNeedsPaint();
    }
}

bool RenderObject::subtreeNeedsPaint() const {
    return needs_paint_;
}

void RenderObject::clearPaintFlag() {
    if (!needs_paint_) return;
    needs_paint_ = false;
    for (auto* child : children_) {
        child->clearPaintFlag();
    }
}

bool RenderObject::hitTest(HitTestResult& result, Point localPoint) {
    // First, test children (in reverse order — topmost child first)
    if (hitTestChildren(result, localPoint)) {
        // A child was hit; add ourselves too for event bubbling
        result.add({this, localPoint});
        return true;
    }

    // Then test self
    if (hitTestSelf(localPoint)) {
        result.add({this, localPoint});
        return true;
    }

    return false;
}

bool RenderObject::hitTestSelf(Point localPoint) const {
    return paintBounds().contains(localPoint);
}

bool RenderObject::hitTestChildren(HitTestResult& result, Point localPoint) {
    // Test children in reverse order (last painted = topmost = tested first)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        RenderObject* child = *it;
        if (!child) continue;

        // Transform the point into the child's local coordinate space
        Point childLocalPoint = {
            localPoint.x - child->offset_.x,
            localPoint.y - child->offset_.y
        };

        if (child->hitTest(result, childLocalPoint)) {
            return true;
        }
    }
    return false;
}

Rect RenderObject::globalBounds() const {
    Point global_offset = offset_;
    const RenderObject* current = parent_;
    while (current) {
        global_offset = global_offset + current->offset_;
        current = current->parent_;
    }
    return Rect::fromPointSize(global_offset, size_);
}

void RenderObject::addChild(RenderObject* child) {
    assert(child);
    assert(child->parent_ == nullptr && "Child already has a parent");
    child->parent_ = this;
    children_.push_back(child);

    // Integrate into Anu tree
    ANUNodeInsertChild(anu_node_, child->anu_node_, ANUNodeGetChildCount(anu_node_));

    markNeedsLayout();
}

void RenderObject::insertChild(RenderObject* child, size_t index) {
    assert(child);
    assert(child->parent_ == nullptr && "Child already has a parent");
    assert(index <= children_.size());

    child->parent_ = this;
    children_.insert(children_.begin() + static_cast<ptrdiff_t>(index), child);

    // Integrate into Anu tree
    ANUNodeInsertChild(anu_node_, child->anu_node_, index);

    markNeedsLayout();
}

void RenderObject::removeChild(RenderObject* child) {
    assert(child);
    assert(child->parent_ == this);

    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
    }

    // Remove from Anu tree
    ANUNodeRemoveChild(anu_node_, child->anu_node_);

    child->parent_ = nullptr;
    markNeedsLayout();
}

void RenderObject::removeAllChildren() {
    for (auto* child : children_) {
        ANUNodeRemoveChild(anu_node_, child->anu_node_);
        child->parent_ = nullptr;
    }
    children_.clear();
    markNeedsLayout();
}

std::string RenderObject::debugDescription() const {
    std::ostringstream oss;
    oss << "RenderObject("
        << "size=" << size_.width << "x" << size_.height
        << ", offset=" << offset_.x << "," << offset_.y
        << ", children=" << children_.size()
        << ")";
    return oss.str();
}

size_t RenderObject::depth() const {
    size_t d = 0;
    const RenderObject* current = parent_;
    while (current) {
        ++d;
        current = current->parent_;
    }
    return d;
}

// ════════════════════════════════════════════════════════════════
// RenderBox
// ════════════════════════════════════════════════════════════════

void RenderBox::syncLayout() {
    // Read new size and position from Anu
    float new_w = ANUNodeLayoutGetWidth(anu_node_);
    float new_h = ANUNodeLayoutGetHeight(anu_node_);
    float new_x = ANUNodeLayoutGetLeft(anu_node_);
    float new_y = ANUNodeLayoutGetTop(anu_node_);

    // Only mark paint dirty when geometry actually changed
    bool geometry_changed = (new_w != size_.width  || new_h != size_.height ||
                             new_x != offset_.x    || new_y != offset_.y);

    size_.width  = new_w;  size_.height = new_h;
    offset_.x    = new_x;  offset_.y    = new_y;
    needs_layout_ = false;

    if (geometry_changed) {
        markNeedsPaint();
    }

    for (auto* child : children_) {
        child->syncLayout();
    }
}

bool RenderBox::hitTestSelf(Point localPoint) const {
    return Rect::fromPointSize({0, 0}, size_).contains(localPoint);
}

}  // namespace enki
