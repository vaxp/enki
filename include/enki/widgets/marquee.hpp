#pragma once
/// @file marquee.hpp
/// @brief Marquee Widget for ENKI Framework.
/// Smooth auto-scrolling single-line text (ticker tape) with configurable velocity,
/// direction (LTR/RTL), loop blank space, pause on hover, and smooth edge fade masks.
///
/// 100% C++20 Declarative Syntax.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/text.hpp"
#include <string>
#include <memory>
#include <optional>

namespace enki {

enum class MarqueeDirection {
    RightToLeft,
    LeftToRight,
};

class MarqueeWidget : public SingleChildRenderObjectWidget {
public:
    std::string      text;
    TextStyle        style;
    float            velocity = 50.0f; // px/sec
    float            blank_space = 60.0f;
    MarqueeDirection direction = MarqueeDirection::RightToLeft;
    bool             pause_on_hover = true;
    float            fading_edge_length = 24.0f;

    MarqueeWidget() = default;
    explicit MarqueeWidget(std::string t, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), text(std::move(t)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Marquee"; }
};

struct MarqueeProps {
    std::string               text;
    std::optional<TextStyle>  style;
    std::optional<Color>      color;
    std::optional<float>      font_size;
    std::optional<FontWeight> font_weight;
    float                     velocity = 50.0f;
    float                     blank_space = 60.0f;
    MarqueeDirection          direction = MarqueeDirection::RightToLeft;
    bool                      pause_on_hover = true;
    float                     fading_edge_length = 24.0f;
    Key                       key = Key::none();
};

struct Marquee {
    std::string               text = "";
    std::optional<TextStyle>  style = std::nullopt;
    std::optional<Color>      color = std::nullopt;
    std::optional<float>      font_size = std::nullopt;
    std::optional<FontWeight> font_weight = std::nullopt;
    float                     velocity = 50.0f;
    float                     blank_space = 60.0f;
    MarqueeDirection          direction = MarqueeDirection::RightToLeft;
    bool                      pause_on_hover = true;
    float                     fading_edge_length = 24.0f;
    Key                       key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<MarqueeWidget>(text, key);
        if (style) w->style = *style;
        if (color) w->style.color = *color;
        if (font_size) w->style.font_size = *font_size;
        if (font_weight) w->style.font_weight = *font_weight;
        w->velocity = velocity;
        w->blank_space = blank_space;
        w->direction = direction;
        w->pause_on_hover = pause_on_hover;
        w->fading_edge_length = fading_edge_length;
        return w;
    }
};

inline std::shared_ptr<MarqueeWidget> marquee(std::string text) {
    return std::make_shared<MarqueeWidget>(std::move(text));
}

inline std::shared_ptr<MarqueeWidget> marquee(const MarqueeProps& props) {
    auto w = std::make_shared<MarqueeWidget>(props.text, props.key);
    if (props.style) w->style = *props.style;
    if (props.color) w->style.color = *props.color;
    if (props.font_size) w->style.font_size = *props.font_size;
    if (props.font_weight) w->style.font_weight = *props.font_weight;
    w->velocity = props.velocity;
    w->blank_space = props.blank_space;
    w->direction = props.direction;
    w->pause_on_hover = props.pause_on_hover;
    w->fading_edge_length = props.fading_edge_length;
    return w;
}

} // namespace enki
