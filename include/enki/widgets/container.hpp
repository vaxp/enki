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
#include "enki/rendering/svg.hpp"
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
    std::string                  background_shader = "";
    std::string                  border_shader     = "";
    std::string                  background_svg    = "";
    std::string                  border_svg        = "";
    SvgFit                       svg_fit           = SvgFit::Stretch;
    std::optional<SvgSlice>      svg_slice         = std::nullopt;

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
    ~RenderDecoratedBox() override;

    void setDecoration(const BoxDecoration& decoration);
    [[nodiscard]] const BoxDecoration& decoration() const { return decoration_; }

    void setStyle(const FlexboxStyle& style);
    [[nodiscard]] const FlexboxStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    [[nodiscard]] bool hitTestSelf(Point localPoint) const override;

private:
    BoxDecoration decoration_;
    FlexboxStyle  style_;

    struct ShaderData;
    std::unique_ptr<ShaderData> shader_data_;

    void updateShaders();
    void paintShadows(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
    void paintBackground(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
    void paintBorder(PaintContext& context, const Rect& bounds, const BorderRadius& radius);
};

// ════════════════════════════════════════════════════════════════
// Container Widget Implementation
// ════════════════════════════════════════════════════════════════

class ContainerWidget : public SingleChildRenderObjectWidget {
public:
    BoxDecoration            decoration;
    FlexboxStyle             style;
    std::optional<Alignment> alignment;

    ContainerWidget() = default;
    explicit ContainerWidget(WidgetPtr c) : SingleChildRenderObjectWidget(Key::none(), std::move(c)) {}
    ContainerWidget(Key k, WidgetPtr c)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)) {}
    ContainerWidget(BoxDecoration dec, WidgetPtr c)
        : SingleChildRenderObjectWidget(Key::none(), std::move(c)), decoration(std::move(dec)) {}
    ContainerWidget(Key k, BoxDecoration dec, WidgetPtr c = nullptr)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)), decoration(std::move(dec)) {}
    ContainerWidget(Key k, BoxDecoration dec, FlexboxStyle s, WidgetPtr c)
        : SingleChildRenderObjectWidget(std::move(k), std::move(c)), decoration(std::move(dec)), style(std::move(s)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Container"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Container Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Container {
    // ── Visual Decoration ──────────────────────────────────────
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;
    std::string                   background_shader = "";
    std::string                   border_shader     = "";
    std::string                   background_svg    = "";
    std::string                   border_svg        = "";
    SvgFit                        svg_fit           = SvgFit::Stretch;
    std::optional<SvgSlice>       svg_slice         = std::nullopt;

    // ── Child Alignment ────────────────────────────────────────
    std::optional<Alignment>      align;

    // ── Dimensions & Constraints ───────────────────────────────
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // ── Insets ─────────────────────────────────────────────────
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;
    std::optional<StyleInsets>    position;

    // ── Flexbox Item Properties ────────────────────────────────
    std::optional<float>          flex;
    std::optional<float>          flex_grow;
    std::optional<float>          flex_shrink;
    std::optional<StyleValue>     flex_basis;
    std::optional<Align>          align_self;
    std::optional<PositionType>   position_type;

    WidgetPtr child = nullptr;
    Key key = Key::none();

    operator WidgetPtr() const;
};

using ContainerProps = Container;

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<ContainerWidget> container(ContainerProps props = {}) {
    auto c = std::make_shared<ContainerWidget>(props.key, std::move(props.child));
    
    // Apply Decoration
    if (props.color) c->decoration.color = *props.color;
    if (props.gradient) c->decoration.gradient = std::move(*props.gradient);
    if (props.border_radius) c->decoration.border_radius = *props.border_radius;
    if (props.border) {
        c->decoration.border = *props.border;
        c->style.border = StyleBorders::uniform(props.border->width);
    }
    c->decoration.box_shadow = std::move(props.box_shadow);
    if (props.shape) c->decoration.shape = *props.shape;
    if (props.clip_content) c->decoration.clip_content = *props.clip_content;
    if (!props.background_shader.empty()) c->decoration.background_shader = std::move(props.background_shader);
    if (!props.border_shader.empty()) {
        c->decoration.border_shader = std::move(props.border_shader);
        if (!props.border) {
            c->decoration.border = Border(Colors::Transparent, 1.0f);
            c->style.border = StyleBorders::uniform(1.0f);
        }
    }
    if (!props.background_svg.empty()) c->decoration.background_svg = std::move(props.background_svg);
    if (!props.border_svg.empty()) {
        c->decoration.border_svg = std::move(props.border_svg);
        if (!props.border) {
            c->decoration.border = Border(Colors::Transparent, 1.0f);
            c->style.border = StyleBorders::uniform(1.0f);
        }
    }
    c->decoration.svg_fit = props.svg_fit;
    c->decoration.svg_slice = props.svg_slice;
    
    // Apply Alignment
    if (props.align) {
        c->alignment = props.align;
        switch (*props.align) {
            case Alignment::TopLeft:
                c->style.justify_content = Justify::Start;
                c->style.align_items = Align::Start;
                break;
            case Alignment::TopCenter:
                c->style.justify_content = Justify::Start;
                c->style.align_items = Align::Center;
                break;
            case Alignment::TopRight:
                c->style.justify_content = Justify::Start;
                c->style.align_items = Align::End;
                break;
            case Alignment::CenterLeft:
                c->style.justify_content = Justify::Center;
                c->style.align_items = Align::Start;
                break;
            case Alignment::Center:
                c->style.justify_content = Justify::Center;
                c->style.align_items = Align::Center;
                break;
            case Alignment::CenterRight:
                c->style.justify_content = Justify::Center;
                c->style.align_items = Align::End;
                break;
            case Alignment::BottomLeft:
                c->style.justify_content = Justify::End;
                c->style.align_items = Align::Start;
                break;
            case Alignment::BottomCenter:
                c->style.justify_content = Justify::End;
                c->style.align_items = Align::Center;
                break;
            case Alignment::BottomRight:
                c->style.justify_content = Justify::End;
                c->style.align_items = Align::End;
                break;
        }
    }
    
    // Apply Constraints & Dimensions
    if (props.width) c->style.width = *props.width;
    if (props.height) c->style.height = *props.height;
    if (props.min_width) c->style.min_width = *props.min_width;
    if (props.min_height) c->style.min_height = *props.min_height;
    if (props.max_width) c->style.max_width = *props.max_width;
    if (props.max_height) c->style.max_height = *props.max_height;
    if (props.aspect_ratio) c->style.aspect_ratio = props.aspect_ratio;
    
    // Apply Insets
    if (props.padding) c->style.padding = *props.padding;
    if (props.margin) c->style.margin = *props.margin;
    if (props.position) c->style.position = *props.position;
    
    // Apply Flex Factors
    if (props.flex) c->style.flex = props.flex;
    if (props.flex_grow) c->style.flex_grow = props.flex_grow;
    if (props.flex_shrink) c->style.flex_shrink = props.flex_shrink;
    if (props.flex_basis) c->style.flex_basis = *props.flex_basis;
    if (props.align_self) c->style.align_self = props.align_self;
    if (props.position_type) c->style.position_type = props.position_type;
    
    return c;
}

inline Container::operator WidgetPtr() const {
    return container(*this);
}

inline std::shared_ptr<ContainerWidget> sizedBox(float width, float height, WidgetPtr child = nullptr) {
    return container({
        .width = StyleValue::point(width),
        .height = StyleValue::point(height),
        .child = std::move(child)
    });
}

inline std::shared_ptr<ContainerWidget> paddingBox(EdgeInsets insets, WidgetPtr child) {
    return container({
        .padding = StyleInsets::only(insets.top, insets.right, insets.bottom, insets.left),
        .child = std::move(child)
    });
}

inline std::shared_ptr<ContainerWidget> centerBox(WidgetPtr child) {
    return container({
        .align = Alignment::Center,
        .child = std::move(child)
    });
}

} // namespace enki
