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
// PositionedWidget Engine Implementation
// ════════════════════════════════════════════════════════════════

class PositionedWidget : public SingleChildRenderObjectWidget {
public:
    PositionedStyle style;

    PositionedWidget() = default;
    explicit PositionedWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    PositionedWidget(Key key, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Positioned"; }

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

// ════════════════════════════════════════════════════════════════
// Declarative Positioned Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Positioned {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    std::optional<StyleValue> top;
    std::optional<StyleValue> right;
    std::optional<StyleValue> bottom;
    std::optional<StyleValue> left;
    std::optional<StyleValue> start;
    std::optional<StyleValue> end;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;

    operator WidgetPtr() const {
        PositionedStyle st;
        st.top = top;
        st.right = right;
        st.bottom = bottom;
        st.left = left;
        st.start = start;
        st.end = end;
        st.width = width;
        st.height = height;
        auto p = std::make_shared<PositionedWidget>(key, child);
        p->style = st;
        return p;
    }

    /// Creates a Positioned widget that fills the entire stack with optional insets.
    static std::shared_ptr<PositionedWidget> fill(WidgetPtr child,
                                                  float left = 0.0f, float top = 0.0f,
                                                  float right = 0.0f, float bottom = 0.0f) {
        auto p = std::make_shared<PositionedWidget>(std::move(child));
        p->style.left = StyleValue::point(left);
        p->style.top = StyleValue::point(top);
        p->style.right = StyleValue::point(right);
        p->style.bottom = StyleValue::point(bottom);
        return p;
    }

    /// Creates a Positioned widget from a Rect.
    static std::shared_ptr<PositionedWidget> fromRect(WidgetPtr child, const Rect& rect) {
        auto p = std::make_shared<PositionedWidget>(std::move(child));
        p->style.left = StyleValue::point(rect.x);
        p->style.top = StyleValue::point(rect.y);
        p->style.width = StyleValue::point(rect.width);
        p->style.height = StyleValue::point(rect.height);
        return p;
    }

    /// Creates a directional Positioned widget.
    static std::shared_ptr<PositionedWidget> directional(WidgetPtr child,
                                                         float top = 0.0f, float end = 0.0f,
                                                         float bottom = 0.0f, float start = 0.0f) {
        auto p = std::make_shared<PositionedWidget>(std::move(child));
        p->style.top = StyleValue::point(top);
        p->style.end = StyleValue::point(end);
        p->style.bottom = StyleValue::point(bottom);
        p->style.start = StyleValue::point(start);
        return p;
    }
};

struct PositionedProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    std::optional<StyleValue> top;
    std::optional<StyleValue> right;
    std::optional<StyleValue> bottom;
    std::optional<StyleValue> left;
    std::optional<StyleValue> start;
    std::optional<StyleValue> end;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
};

inline std::shared_ptr<PositionedWidget> positioned(WidgetPtr child) {
    return std::make_shared<PositionedWidget>(std::move(child));
}

inline std::shared_ptr<PositionedWidget> positioned(float top, float right, float bottom, float left, WidgetPtr child) {
    auto p = std::make_shared<PositionedWidget>(std::move(child));
    p->style.top = StyleValue::point(top);
    p->style.right = StyleValue::point(right);
    p->style.bottom = StyleValue::point(bottom);
    p->style.left = StyleValue::point(left);
    return p;
}

inline std::shared_ptr<PositionedWidget> positioned(PositionedProps props) {
    auto p = std::make_shared<PositionedWidget>(std::move(props.key), std::move(props.child));
    if (props.top) p->style.top = props.top;
    if (props.right) p->style.right = props.right;
    if (props.bottom) p->style.bottom = props.bottom;
    if (props.left) p->style.left = props.left;
    if (props.start) p->style.start = props.start;
    if (props.end) p->style.end = props.end;
    if (props.width) p->style.width = props.width;
    if (props.height) p->style.height = props.height;
    return p;
}

inline std::shared_ptr<PositionedWidget> positioned(PositionedProps props, WidgetPtr child) {
    props.child = std::move(child);
    return positioned(std::move(props));
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
// StackWidget Engine Implementation
// ════════════════════════════════════════════════════════════════

class StackWidget : public MultiChildRenderObjectWidget {
public:
    StackStyle style;

    StackWidget() = default;
    explicit StackWidget(std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(Key::none(), std::move(children)) {}
    StackWidget(Key key, std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(std::move(key), std::move(children)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Stack"; }

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

// ════════════════════════════════════════════════════════════════
// Declarative Stack Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Stack {
    Key key = Key::none();
    Alignment alignment = Alignment::TopLeft;
    StackFit fit = StackFit::Loose;
    Clip clip_behavior = Clip::HardEdge;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    std::vector<WidgetPtr> children;

    operator WidgetPtr() const {
        StackStyle st;
        st.alignment = alignment;
        st.fit = fit;
        st.clip_behavior = clip_behavior;
        st.width = width;
        st.height = height;
        st.min_width = min_width;
        st.min_height = min_height;
        st.max_width = max_width;
        st.max_height = max_height;
        auto s = std::make_shared<StackWidget>(key, children);
        s->style = st;
        return s;
    }
};

struct StackProps {
    Key key = Key::none();
    std::vector<WidgetPtr> children;

    Alignment alignment = Alignment::TopLeft;
    StackFit fit = StackFit::Loose;
    Clip clip_behavior = Clip::HardEdge;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;
};

// ════════════════════════════════════════════════════════════════
// Global Helper Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<StackWidget> stack(std::vector<WidgetPtr> children) {
    return std::make_shared<StackWidget>(std::move(children));
}

inline std::shared_ptr<StackWidget> stack(Alignment alignment, std::vector<WidgetPtr> children) {
    auto s = std::make_shared<StackWidget>(std::move(children));
    s->style.alignment = alignment;
    return s;
}

inline std::shared_ptr<StackWidget> stack(Alignment alignment, StackFit fit, std::vector<WidgetPtr> children) {
    auto s = std::make_shared<StackWidget>(std::move(children));
    s->style.alignment = alignment;
    s->style.fit = fit;
    return s;
}

inline std::shared_ptr<StackWidget> stack(StackProps props) {
    auto s = std::make_shared<StackWidget>(std::move(props.key), std::move(props.children));
    s->style.alignment = props.alignment;
    s->style.fit = props.fit;
    s->style.clip_behavior = props.clip_behavior;
    if (props.width) s->style.width = props.width;
    if (props.height) s->style.height = props.height;
    if (props.min_width) s->style.min_width = props.min_width;
    if (props.min_height) s->style.min_height = props.min_height;
    if (props.max_width) s->style.max_width = props.max_width;
    if (props.max_height) s->style.max_height = props.max_height;
    return s;
}

inline std::shared_ptr<StackWidget> stack(StackProps props, std::vector<WidgetPtr> children) {
    props.children = std::move(children);
    return stack(std::move(props));
}

} // namespace enki
