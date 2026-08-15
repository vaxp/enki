#pragma once
/// @file container.hpp
/// @brief Comprehensive Container widget with advanced Skia BoxDecoration, BoxShadow, Gradient, and Flexbox styling.
///
/// Features:
///   - Rich visual decoration (Background color, Linear/Radial Gradients, Multi-BoxShadow, BorderRadius, Borders).
///   - Full integration with Anu Flexbox engine via FlexboxStyle (Padding, Margin, Width, Height, Constraints, AspectRatio).
///   - Child alignment, clipping, and fluent chainable builder API.
///   - Semantic helpers: container(), sizedBox(), paddingBox(), centerBox().
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include <vector>
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Box Shadow Specification
// ════════════════════════════════════════════════════════════════

struct BoxShadow {
    Color color          = 0x40000000;  // 25% black
    Point offset         = {0.0f, 4.0f}; // dx, dy
    float blur_radius    = 8.0f;
    float spread_radius  = 0.0f;
    bool  inset          = false;

    constexpr BoxShadow() = default;
    constexpr BoxShadow(Color c, Point off, float blur, float spread = 0.0f, bool in = false)
        : color(c), offset(off), blur_radius(blur), spread_radius(spread), inset(in) {}

    static constexpr BoxShadow standard(Color c = 0x40000000, float blur = 8.0f, float dy = 4.0f) {
        return BoxShadow(c, {0.0f, dy}, blur, 0.0f, false);
    }

    static constexpr BoxShadow glow(Color c, float radius, float spread = 0.0f) {
        return BoxShadow(c, {0.0f, 0.0f}, radius, spread, false);
    }

    constexpr bool operator==(const BoxShadow&) const = default;
};

// ════════════════════════════════════════════════════════════════
// Gradient Configuration
// ════════════════════════════════════════════════════════════════

enum class GradientType {
    Linear,
    Radial,
    Sweep
};

struct GradientConfig {
    GradientType        type   = GradientType::Linear;
    std::vector<Color>  colors;
    std::vector<float>  stops;
    
    // Linear coordinates (relative 0.0 .. 1.0)
    Point start_point = {0.0f, 0.0f};
    Point end_point   = {1.0f, 1.0f};

    // Radial / Sweep
    Point center = {0.5f, 0.5f};
    float radius = 0.5f;
    float start_angle = 0.0f;
    float end_angle   = 360.0f;

    static GradientConfig linear(const std::vector<Color>& colors,
                                 Point start = {0.0f, 0.0f},
                                 Point end   = {0.0f, 1.0f},
                                 const std::vector<float>& stops = {}) {
        GradientConfig g;
        g.type = GradientType::Linear;
        g.colors = colors;
        g.start_point = start;
        g.end_point = end;
        g.stops = stops;
        return g;
    }

    static GradientConfig horizontal(const std::vector<Color>& colors, const std::vector<float>& stops = {}) {
        return linear(colors, {0.0f, 0.5f}, {1.0f, 0.5f}, stops);
    }

    static GradientConfig vertical(const std::vector<Color>& colors, const std::vector<float>& stops = {}) {
        return linear(colors, {0.5f, 0.0f}, {0.5f, 1.0f}, stops);
    }

    static GradientConfig radial(const std::vector<Color>& colors,
                                Point center = {0.5f, 0.5f},
                                float radius = 0.5f,
                                const std::vector<float>& stops = {}) {
        GradientConfig g;
        g.type = GradientType::Radial;
        g.colors = colors;
        g.center = center;
        g.radius = radius;
        g.stops = stops;
        return g;
    }
    bool operator==(const GradientConfig&) const = default;
};

// ════════════════════════════════════════════════════════════════
// Box Decoration
// ════════════════════════════════════════════════════════════════

struct BoxDecoration {
    Color                        color          = Colors::Transparent;
    std::optional<GradientConfig> gradient;
    BorderRadius                 border_radius  = BorderRadius::zero();
    std::optional<Border>        border;
    std::vector<BoxShadow>       box_shadow;
    BoxShape                     shape          = BoxShape::Rectangle;
    bool                         clip_content   = false;

    constexpr BoxDecoration() = default;
    explicit BoxDecoration(Color c) : color(c) {}
    BoxDecoration(Color c, BorderRadius r) : color(c), border_radius(r) {}
    BoxDecoration(Color c, BorderRadius r, Border b) : color(c), border_radius(r), border(b) {}
    BoxDecoration(GradientConfig grad, BorderRadius r = BorderRadius::zero())
        : gradient(std::move(grad)), border_radius(r) {}

    static BoxDecoration rounded(Color color, float radius) {
        return BoxDecoration(color, BorderRadius::circular(radius));
    }

    static BoxDecoration circle(Color color, std::optional<Border> border = std::nullopt) {
        BoxDecoration d;
        d.color = color;
        d.shape = BoxShape::Circle;
        d.border = border;
        return d;
    }

    bool operator==(const BoxDecoration&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderDecoratedBox — Skia Render Object for Container
// ════════════════════════════════════════════════════════════════

class RenderDecoratedBox : public RenderBox {
public:
    RenderDecoratedBox();
    RenderDecoratedBox(BoxDecoration decoration, FlexboxStyle style);
    ~RenderDecoratedBox() override = default;

    void setDecoration(const BoxDecoration& decoration);
    [[nodiscard]] const BoxDecoration& decoration() const { return decoration_; }

    void setStyle(const FlexboxStyle& style);
    [[nodiscard]] const FlexboxStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    [[nodiscard]] bool hitTestSelf(Point localPoint) const override;

private:
    BoxDecoration decoration_;
    FlexboxStyle  style_;

    void paintShadows(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
    void paintBackground(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
    void paintBorder(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
};

// ════════════════════════════════════════════════════════════════
// Container Widget
// ════════════════════════════════════════════════════════════════

class Container : public SingleChildRenderObjectWidget {
public:
    BoxDecoration            decoration;
    FlexboxStyle             style;
    std::optional<Alignment> alignment;

    Container() = default;
    explicit Container(WidgetPtr c) : SingleChildRenderObjectWidget(Key::none(), std::move(c)) {}
    Container(Key k, WidgetPtr c)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)) {}
    Container(BoxDecoration dec, WidgetPtr c)
        : SingleChildRenderObjectWidget(Key::none(), std::move(c)), decoration(std::move(dec)) {}
    Container(Key k, BoxDecoration dec, WidgetPtr c = nullptr)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)), decoration(std::move(dec)) {}
    Container(Key k, BoxDecoration dec, FlexboxStyle s, WidgetPtr c)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)), decoration(std::move(dec)), style(std::move(s)) {}

    // ── Fluent Visual Styling ──────────────────────────────────
    Container& color(Color c) {
        decoration.color = c;
        return *this;
    }

    Container& gradient(GradientConfig grad) {
        decoration.gradient = std::move(grad);
        return *this;
    }

    Container& borderRadius(BorderRadius r) {
        decoration.border_radius = r;
        return *this;
    }

    Container& borderRadius(float r) {
        decoration.border_radius = BorderRadius::circular(r);
        return *this;
    }

    Container& border(Border b) {
        decoration.border = b;
        style.border = StyleBorders::uniform(b.width);
        return *this;
    }

    Container& border(Color c, float width) {
        decoration.border = Border(c, width);
        style.border = StyleBorders::uniform(width);
        return *this;
    }

    Container& shadow(BoxShadow s) {
        decoration.box_shadow.push_back(s);
        return *this;
    }

    Container& shadow(Color color, Point offset = {0, 4}, float blur = 8, float spread = 0) {
        decoration.box_shadow.emplace_back(color, offset, blur, spread);
        return *this;
    }

    Container& shape(BoxShape s) {
        decoration.shape = s;
        return *this;
    }

    Container& clip(bool clip_content = true) {
        decoration.clip_content = clip_content;
        return *this;
    }

    // ── Fluent Dimensions & Flexbox Constraints ────────────────
    Container& width(StyleValue w) { style.width = w; return *this; }
    Container& width(float w) { style.width = StyleValue::point(w); return *this; }
    Container& height(StyleValue h) { style.height = h; return *this; }
    Container& height(float h) { style.height = StyleValue::point(h); return *this; }
    Container& size(float w, float h) {
        style.width = StyleValue::point(w);
        style.height = StyleValue::point(h);
        return *this;
    }

    Container& minWidth(StyleValue mw) { style.min_width = mw; return *this; }
    Container& minHeight(StyleValue mh) { style.min_height = mh; return *this; }
    Container& maxWidth(StyleValue mw) { style.max_width = mw; return *this; }
    Container& maxHeight(StyleValue mh) { style.max_height = mh; return *this; }
    Container& aspectRatio(float ar) { style.aspect_ratio = ar; return *this; }

    // ── Insets & Margins ───────────────────────────────────────
    Container& padding(StyleInsets p) { style.padding = p; return *this; }
    Container& padding(EdgeInsets p) { style.padding = StyleInsets::only(p.top, p.right, p.bottom, p.left); return *this; }
    Container& paddingAll(float p) { style.padding = StyleInsets::all(p); return *this; }
    Container& paddingSymmetric(float v, float h) { style.padding = StyleInsets::symmetric(v, h); return *this; }

    Container& margin(StyleInsets m) { style.margin = m; return *this; }
    Container& margin(EdgeInsets m) { style.margin = StyleInsets::only(m.top, m.right, m.bottom, m.left); return *this; }
    Container& marginAll(float m) { style.margin = StyleInsets::all(m); return *this; }
    Container& marginSymmetric(float v, float h) { style.margin = StyleInsets::symmetric(v, h); return *this; }

    // ── Flex Factors ───────────────────────────────────────────
    Container& flex(float f) { style.flex = f; return *this; }
    Container& flexGrow(float fg) { style.flex_grow = fg; return *this; }
    Container& flexShrink(float fs) { style.flex_shrink = fs; return *this; }
    Container& flexBasis(StyleValue fb) { style.flex_basis = fb; return *this; }
    Container& alignSelf(Align as) { style.align_self = as; return *this; }
    Container& positionType(PositionType pt) { style.position_type = pt; return *this; }
    Container& position(StyleInsets p) { style.position = p; return *this; }

    // ── Alignment ──────────────────────────────────────────────
    Container& align(Alignment a) {
        alignment = a;
        switch (a) {
            case Alignment::TopLeft:
                style.justify_content = Justify::Start;
                style.align_items = Align::Start;
                break;
            case Alignment::TopCenter:
                style.justify_content = Justify::Start;
                style.align_items = Align::Center;
                break;
            case Alignment::TopRight:
                style.justify_content = Justify::Start;
                style.align_items = Align::End;
                break;
            case Alignment::CenterLeft:
                style.justify_content = Justify::Center;
                style.align_items = Align::Start;
                break;
            case Alignment::Center:
                style.justify_content = Justify::Center;
                style.align_items = Align::Center;
                break;
            case Alignment::CenterRight:
                style.justify_content = Justify::Center;
                style.align_items = Align::End;
                break;
            case Alignment::BottomLeft:
                style.justify_content = Justify::End;
                style.align_items = Align::Start;
                break;
            case Alignment::BottomCenter:
                style.justify_content = Justify::End;
                style.align_items = Align::Center;
                break;
            case Alignment::BottomRight:
                style.justify_content = Justify::End;
                style.align_items = Align::End;
                break;
        }
        return *this;
    }

    Container& setChild(WidgetPtr c) {
        this->child = std::move(c);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Container"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Container> container(WidgetPtr child = nullptr) {
    return std::make_shared<Container>(std::move(child));
}

inline std::shared_ptr<Container> container(Key key, WidgetPtr child = nullptr) {
    return std::make_shared<Container>(std::move(key), std::move(child));
}

inline std::shared_ptr<Container> container(BoxDecoration decoration, WidgetPtr child = nullptr) {
    return std::make_shared<Container>(std::move(decoration), std::move(child));
}

inline std::shared_ptr<Container> container(Key key, BoxDecoration decoration, WidgetPtr child = nullptr) {
    return std::make_shared<Container>(std::move(key), std::move(decoration), std::move(child));
}

inline std::shared_ptr<Container> sizedBox(float width, float height, WidgetPtr child = nullptr) {
    auto c = std::make_shared<Container>(std::move(child));
    c->size(width, height);
    return c;
}

inline std::shared_ptr<Container> paddingBox(EdgeInsets insets, WidgetPtr child) {
    auto c = std::make_shared<Container>(std::move(child));
    c->padding(insets);
    return c;
}

inline std::shared_ptr<Container> centerBox(WidgetPtr child) {
    auto c = std::make_shared<Container>(std::move(child));
    c->align(Alignment::Center);
    return c;
}

} // namespace enki
