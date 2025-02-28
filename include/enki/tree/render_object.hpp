#pragma once
/// @file render_object.hpp
/// @brief RenderObject classes — the layout and painting layer.
///
/// RenderObjects are the third tree in the Three-Tree Architecture.
/// They are the most expensive to create and are therefore carefully
/// managed by Elements to minimize creation/destruction.
///
/// RenderObject responsibilities:
///   - Compute layout (size and position) using Anu.
///   - Paint the visual representation using Canvas.
///   - Perform hit testing for pointer events.
///   - Manage a Anu LayoutNode for Flexbox-based layout.
///
/// RenderObject subclasses:
///   - RenderBox:            Standard box-model render object (most common).
///   - RenderFlex:           Flexbox container (Row, Column).
///   - RenderDecoratedBox:   Renders decorations (background, border, shadow).
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/platform/input.hpp"
#include <layout_engine/Anu.h>
#include <memory>
#include <cmath>
#include <vector>
#include <functional>
#include <string>

namespace enki {

// Forward declarations
class Canvas;
class Element;
class BuildContext;

// ════════════════════════════════════════════════════════════════
// PaintContext
// ════════════════════════════════════════════════════════════════

/// @brief Context passed during the paint phase, providing canvas
///        and coordinate transformation utilities.
struct PaintContext {
    Canvas& canvas;                ///< The canvas to paint on.
    Point offset;                  ///< Accumulated offset from root.
    Rect clip_rect;                ///< Current clip rectangle.
    float opacity = 1.0f;          ///< Accumulated opacity.

    /// Create a child context with an additional offset.
    [[nodiscard]] PaintContext withOffset(Point additional) const {
        return {canvas, offset + additional, clip_rect, opacity};
    }

    /// Create a child context with a modified opacity.
    [[nodiscard]] PaintContext withOpacity(float child_opacity) const {
        return {canvas, offset, clip_rect, opacity * child_opacity};
    }
};

// ════════════════════════════════════════════════════════════════
// HitTestResult
// ════════════════════════════════════════════════════════════════

/// @brief Collects the hit test path — the list of render objects
///        under a pointer position, from innermost to outermost.
class HitTestResult {
public:
    /// An entry in the hit test path.
    struct Entry {
        RenderObject* target = nullptr;  ///< The render object that was hit.
        Point localPosition;             ///< The point in the target's local coordinates.
    };

    /// Add a hit test entry.
    void add(Entry entry) { path_.push_back(std::move(entry)); }

    /// @return The full hit path (innermost first).
    [[nodiscard]] const std::vector<Entry>& path() const { return path_; }

    /// @return true if the hit test found at least one target.
    [[nodiscard]] bool isHit() const { return !path_.empty(); }

    /// Clear all entries.
    void clear() { path_.clear(); }

private:
    std::vector<Entry> path_;
};

// ════════════════════════════════════════════════════════════════
// RenderObject — Abstract Base
// ════════════════════════════════════════════════════════════════

/// @brief Abstract base for all render objects in the render tree.
///
/// A RenderObject handles layout (computing its size given constraints)
/// and painting (drawing itself onto a canvas). It also participates in
/// hit testing to determine which object is under the pointer.
///
/// Each RenderObject owns a Anu LayoutNode for Flexbox-based layout
/// computation. The layout phase uses Anu to calculate positions and
/// sizes, while the paint phase uses the Skia-based Canvas to render.
class RenderObject {
public:
    RenderObject();
    virtual ~RenderObject();

    // Non-copyable
    RenderObject(const RenderObject&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;

    // ── Layout ─────────────────────────────────────────────────

    /// Perform layout calculation starting from this node.
    /// In native Anu philosophy, this is usually only called on the root node
    /// once per frame. Anu computes the sizes for the entire tree.
    void layout(float available_width = NAN, float available_height = NAN);

    /// Sync the computed layout (size and offset) from Anu back to this C++ object.
    /// This is called recursively.
    virtual void syncLayout();

    /// Mark this render object as needing layout recalculation.
    /// The layout will be recomputed before the next paint.
    void markNeedsLayout();

    /// @return true if this render object needs layout.
    [[nodiscard]] bool needsLayout() const { return needs_layout_; }

    // ── Painting ───────────────────────────────────────────────

    /// Paint this render object and its children.
    /// Override to implement custom painting logic.
    ///
    /// @param context The paint context with canvas and offset.
    virtual void paint(PaintContext& context) = 0;

    virtual void tick(double /*now*/) {}
    virtual void handlePointerDown(const PointerEvent& e) {}
    virtual void handlePointerUp(const PointerEvent& e) {}
    virtual void handlePointerEnter(const PointerEvent& e) {}
    virtual void handlePointerExit(const PointerEvent& e) {}
    virtual void handlePointerMove(const PointerEvent& e) {}
    virtual void handlePointerScroll(float dx, float dy) {}
    virtual bool handlesScroll() const { return false; }
    virtual SystemCursor cursor() const { return SystemCursor::Default; }

    /// Check if a RenderObject pointer is still allocated and alive in the tree.
    static bool isAlive(const RenderObject* ro);

    /// Mark this render object as needing repaint.
    void markNeedsPaint();

    /// @return true if this render object needs repaint.
    [[nodiscard]] bool needsPaint() const { return needs_paint_; }

    // ── Hit Testing ────────────────────────────────────────────

    /// Test whether a point hits this render object.
    ///
    /// @param result      The hit test result to populate.
    /// @param localPoint  The point in this object's local coordinate space.
    /// @return true if the point hits this object or any of its children.
    virtual bool hitTest(HitTestResult& result, Point localPoint);

    /// Test whether a point is within this render object's bounds.
    /// The default implementation checks against the object's size.
    ///
    /// @param localPoint The point in local coordinates.
    /// @return true if the point is within bounds.
    [[nodiscard]] virtual bool hitTestSelf(Point localPoint) const;

    /// Test children for hit (in reverse paint order, i.e., topmost first).
    ///
    /// @param result      The hit test result to populate.
    /// @param localPoint  The point in this object's local coordinates.
    /// @return true if any child was hit.
    virtual bool hitTestChildren(HitTestResult& result, Point localPoint);

    // ── Geometry ───────────────────────────────────────────────

    /// @return The computed size after layout.
    [[nodiscard]] Size size() const { return size_; }

    /// Set the size directly (use during performLayout).
    void setSize(Size size) { size_ = size; }

    /// @return The computed offset relative to parent.
    [[nodiscard]] Point offset() const { return offset_; }

    /// Set the offset relative to parent.
    void setOffset(Point offset) { offset_ = offset; }

    /// @return The paint bounds in local coordinates (origin at 0,0).
    [[nodiscard]] Rect paintBounds() const {
        return Rect::fromPointSize({0, 0}, size_);
    }

    /// @return The global bounds (accumulated from root).
    [[nodiscard]] Rect globalBounds() const;

    // ── Anu Layout Node ───────────────────────────────────────

    /// @return The Anu layout node for this render object.
    [[nodiscard]] ANUNodeRef anuNode() { return anu_node_; }
    [[nodiscard]] ANUNodeRef anuNode() const { return anu_node_; }
    [[nodiscard]] ANUNodeRef getAnuNode() { return anu_node_; }
    [[nodiscard]] ANUNodeRef getAnuNode() const { return anu_node_; }

    // ── Tree Structure ─────────────────────────────────────────

    /// @return The parent render object.
    [[nodiscard]] RenderObject* parentRenderObject() const { return parent_; }

    /// Set the parent render object.
    void setParent(RenderObject* parent) { parent_ = parent; }

    /// @return The child render objects.
    [[nodiscard]] const std::vector<RenderObject*>& children() const { return children_; }

    /// Add a child render object.
    void addChild(RenderObject* child);

    /// Insert a child at a specific index.
    void insertChild(RenderObject* child, size_t index);

    /// Remove a child render object.
    void removeChild(RenderObject* child);

    /// Remove all children.
    void removeAllChildren();

    /// @return The number of child render objects.
    [[nodiscard]] size_t childCount() const { return children_.size(); }

    // ── Ownership ──────────────────────────────────────────────

    /// @return The element that owns this render object.
    [[nodiscard]] Element* ownerElement() const { return owner_element_; }

    /// Set the owning element.
    void setOwnerElement(Element* element) { owner_element_ = element; }

    // ── Debug ──────────────────────────────────────────────────

    /// @return A human-readable debug description.
    [[nodiscard]] virtual std::string debugDescription() const;

    /// @return The depth of this render object in the render tree.
    [[nodiscard]] size_t depth() const;

protected:
    Size size_;                            ///< Computed size after layout.
    Point offset_;                         ///< Offset relative to parent.
    ANUNodeRef anu_node_;                  ///< Anu layout node.
    RenderObject* parent_ = nullptr;       ///< Parent in render tree.
    Element* owner_element_ = nullptr;     ///< Owning element.
    std::vector<RenderObject*> children_;  ///< Children in render tree.
    bool needs_layout_ = true;             ///< Dirty flag for layout.
    bool needs_paint_ = true;              ///< Dirty flag for paint.
};

// ════════════════════════════════════════════════════════════════
// RenderBox — standard box-model render object
// ════════════════════════════════════════════════════════════════

/// @brief A render object that uses the standard box model.
///
/// RenderBox is the most common base class for render objects. It
/// computes its size using Anu's Flexbox algorithm and provides
/// standard hit testing against its rectangular bounds.
///
/// Subclasses should override:
///   - `paint()`:        To draw the visual content.
///   - `computeSize()`:  To customize intrinsic size computation.
class RenderBox : public RenderObject {
public:
    RenderBox() = default;
    ~RenderBox() override = default;

    /// Sync the computed layout from Anu.
    void syncLayout() override;

    /// Hit test against this box's rectangular bounds.
    [[nodiscard]] bool hitTestSelf(Point localPoint) const override;
};




}  // namespace enki

