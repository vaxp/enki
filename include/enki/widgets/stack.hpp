#pragma once
/// @file stack.hpp
/// @brief Stack and Positioned layout widgets for multi-layered 2.5D layouts.
///
/// Features:
///   - Stacking widgets along the Z-axis (children painted in order, hit-tested in reverse).
///   - Absolute positioning with Positioned (top, right, bottom, left, width, height).
///   - Full alignment support for non-positioned children (TopLeft, Center, BottomRight, etc.).
///   - Sizing strategies via StackFit (Loose, Expand, Passthrough).
///   - Clipping support via Clip (None, HardEdge, AntiAlias).
///   - Direct 1:1 integration with Anu Layout Engine.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include <layout_engine/Anu.h>
#include <optional>
#include <vector>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// PositionedStyle — Configuration for Positioned items
// ════════════════════════════════════════════════════════════════

struct PositionedStyle {
    std::optional<StyleValue> top;
    std::optional<StyleValue> right;
    std::optional<StyleValue> bottom;
    std::optional<StyleValue> left;
    std::optional<StyleValue> start;
    std::optional<StyleValue> end;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;

    constexpr bool operator==(const PositionedStyle&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderPositioned — Render object for absolutely positioned items
// ════════════════════════════════════════════════════════════════

class RenderPositioned : public RenderBox {
public:
    RenderPositioned();
    explicit RenderPositioned(PositionedStyle style);
    ~RenderPositioned() override = default;

    void setStyle(const PositionedStyle& style);
    [[nodiscard]] const PositionedStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;

private:
    void applyStyleToNode();
    PositionedStyle style_;
};

// ════════════════════════════════════════════════════════════════
// Positioned Widget
// ════════════════════════════════════════════════════════════════

class Positioned : public SingleChildRenderObjectWidget {
public:
    PositionedStyle style;

    Positioned() = default;
    explicit Positioned(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    Positioned(Key key, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Positioned"; }

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;

    // ── Static Factories ───────────────────────────────────────

    /// Creates a Positioned widget that fills the entire stack with optional insets.
    static std::shared_ptr<Positioned> fill(WidgetPtr child,
                                            float left = 0.0f, float top = 0.0f,
                                            float right = 0.0f, float bottom = 0.0f) {
        auto p = std::make_shared<Positioned>(std::move(child));
        p->style.left = StyleValue::point(left);
        p->style.top = StyleValue::point(top);
        p->style.right = StyleValue::point(right);
        p->style.bottom = StyleValue::point(bottom);
        return p;
    }

    /// Creates a Positioned widget from a Rect.
    static std::shared_ptr<Positioned> fromRect(WidgetPtr child, const Rect& rect) {
        auto p = std::make_shared<Positioned>(std::move(child));
        p->style.left = StyleValue::point(rect.x);
        p->style.top = StyleValue::point(rect.y);
        p->style.width = StyleValue::point(rect.width);
        p->style.height = StyleValue::point(rect.height);
        return p;
    }

    /// Creates a directional Positioned widget.
    static std::shared_ptr<Positioned> directional(WidgetPtr child,
                                                   float top = 0.0f, float end = 0.0f,
                                                   float bottom = 0.0f, float start = 0.0f) {
        auto p = std::make_shared<Positioned>(std::move(child));
        p->style.top = StyleValue::point(top);
        p->style.end = StyleValue::point(end);
        p->style.bottom = StyleValue::point(bottom);
        p->style.start = StyleValue::point(start);
        return p;
    }

    // ── Fluent Builder API ─────────────────────────────────────

    Positioned& top(float v) { style.top = StyleValue::point(v); return *this; }
    Positioned& right(float v) { style.right = StyleValue::point(v); return *this; }
    Positioned& bottom(float v) { style.bottom = StyleValue::point(v); return *this; }
    Positioned& left(float v) { style.left = StyleValue::point(v); return *this; }
    Positioned& start(float v) { style.start = StyleValue::point(v); return *this; }
    Positioned& end(float v) { style.end = StyleValue::point(v); return *this; }
    Positioned& width(float v) { style.width = StyleValue::point(v); return *this; }
    Positioned& height(float v) { style.height = StyleValue::point(v); return *this; }

    Positioned& top(StyleValue v) { style.top = v; return *this; }
    Positioned& right(StyleValue v) { style.right = v; return *this; }
    Positioned& bottom(StyleValue v) { style.bottom = v; return *this; }
    Positioned& left(StyleValue v) { style.left = v; return *this; }
    Positioned& width(StyleValue v) { style.width = v; return *this; }
    Positioned& height(StyleValue v) { style.height = v; return *this; }
};

/// Global factory helper for Positioned
inline std::shared_ptr<Positioned> positioned(WidgetPtr child) {
    return std::make_shared<Positioned>(std::move(child));
}

inline std::shared_ptr<Positioned> positioned(float top, float right, float bottom, float left, WidgetPtr child) {
    auto p = std::make_shared<Positioned>(std::move(child));
    p->top(top).right(right).bottom(bottom).left(left);
    return p;
}

// ════════════════════════════════════════════════════════════════
// StackStyle — Configuration for Stack container
// ════════════════════════════════════════════════════════════════

struct StackStyle {
    Alignment                 alignment      = Alignment::TopLeft;
    StackFit                  fit            = StackFit::Loose;
    Clip                      clip_behavior  = Clip::HardEdge;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    constexpr bool operator==(const StackStyle&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderStack — Stack Container Render Object
// ════════════════════════════════════════════════════════════════

class RenderStack : public RenderBox {
public:
    RenderStack();
    explicit RenderStack(StackStyle style);
    ~RenderStack() override = default;

    void setStyle(const StackStyle& style);
    [[nodiscard]] const StackStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;

private:
    void applyStyleToNode();
    StackStyle style_;
};

// ════════════════════════════════════════════════════════════════
// Stack Widget
// ════════════════════════════════════════════════════════════════

class Stack : public MultiChildRenderObjectWidget {
public:
    StackStyle style;

    Stack() = default;
    explicit Stack(std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(Key::none(), std::move(children)) {}
    Stack(Key key, std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(std::move(key), std::move(children)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Stack"; }

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;

    // ── Fluent Builder API ─────────────────────────────────────

    Stack& alignment(Alignment a) { style.alignment = a; return *this; }
    Stack& fit(StackFit f) { style.fit = f; return *this; }
    Stack& clip(Clip c) { style.clip_behavior = c; return *this; }
    Stack& clipBehavior(Clip c) { style.clip_behavior = c; return *this; }

    Stack& width(float w) { style.width = StyleValue::point(w); return *this; }
    Stack& height(float h) { style.height = StyleValue::point(h); return *this; }
    Stack& width(StyleValue w) { style.width = w; return *this; }
    Stack& height(StyleValue h) { style.height = h; return *this; }

    Stack& minWidth(float w) { style.min_width = StyleValue::point(w); return *this; }
    Stack& minHeight(float h) { style.min_height = StyleValue::point(h); return *this; }
    Stack& maxWidth(float w) { style.max_width = StyleValue::point(w); return *this; }
    Stack& maxHeight(float h) { style.max_height = StyleValue::point(h); return *this; }
};

// ════════════════════════════════════════════════════════════════
// Global Helper Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Stack> stack(std::vector<WidgetPtr> children) {
    return std::make_shared<Stack>(std::move(children));
}

inline std::shared_ptr<Stack> stack(Alignment alignment, std::vector<WidgetPtr> children) {
    auto s = std::make_shared<Stack>(std::move(children));
    s->alignment(alignment);
    return s;
}

inline std::shared_ptr<Stack> stack(Alignment alignment, StackFit fit, std::vector<WidgetPtr> children) {
    auto s = std::make_shared<Stack>(std::move(children));
    s->alignment(alignment).fit(fit);
    return s;
}

} // namespace enki
