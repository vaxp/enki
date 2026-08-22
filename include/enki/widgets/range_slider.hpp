#pragma once
/// @file range_slider.hpp
/// @brief Advanced RangeSlider widget for ENKI Framework
///
/// Features:
///   - Two interactive thumbs
///   - Customizable track and thumb colors
///   - Drop shadow on thumbs
///   - Smooth dragging support without crossing
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

using RangeSliderCallback = std::function<void(float start, float end)>;

struct RangeSliderProps {
    Key key = Key::none();
    float start_value = 0.0f;
    float end_value = 1.0f;
    RangeSliderCallback on_change = nullptr;

    Color active_color = 0xFF3B82F6; // Blue 500
    Color inactive_color = 0xFFCBD5E1; // Slate 300
    Color thumb_color = 0xFFFFFFFF; // White
    float track_height = 4.0f;
    float thumb_radius = 10.0f;
    float min_value = 0.0f;
    float max_value = 1.0f;
    
    // Shadow properties for thumb
    Color shadow_color = 0x40000000;
    float shadow_blur = 4.0f;
    float shadow_offset_dy = 2.0f;
};

class RangeSliderWidget : public SingleChildRenderObjectWidget {
public:
    float start_value;
    float end_value;
    RangeSliderCallback on_change;
    RangeSliderProps options;

    RangeSliderWidget(float start, float end, RangeSliderCallback on_change, RangeSliderProps options = RangeSliderProps())
        : SingleChildRenderObjectWidget(Key::none(), nullptr), start_value(start), end_value(end), on_change(std::move(on_change)), options(std::move(options)) {}

    RangeSliderWidget(Key key, float start, float end, RangeSliderCallback on_change, RangeSliderProps options)
        : SingleChildRenderObjectWidget(std::move(key), nullptr), start_value(start), end_value(end), on_change(std::move(on_change)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "RangeSlider"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct RangeSlider {
    Key key = Key::none();
    float start_value = 0.0f;
    float end_value = 1.0f;
    RangeSliderCallback on_change = nullptr;

    Color active_color = 0xFF3B82F6;
    Color inactive_color = 0xFFCBD5E1;
    Color thumb_color = 0xFFFFFFFF;
    float track_height = 4.0f;
    float thumb_radius = 10.0f;
    float min_value = 0.0f;
    float max_value = 1.0f;
    
    Color shadow_color = 0x40000000;
    float shadow_blur = 4.0f;
    float shadow_offset_dy = 2.0f;

    operator WidgetPtr() const {
        RangeSliderProps props;
        props.key = key;
        props.start_value = start_value;
        props.end_value = end_value;
        props.on_change = on_change;
        props.active_color = active_color;
        props.inactive_color = inactive_color;
        props.thumb_color = thumb_color;
        props.track_height = track_height;
        props.thumb_radius = thumb_radius;
        props.min_value = min_value;
        props.max_value = max_value;
        props.shadow_color = shadow_color;
        props.shadow_blur = shadow_blur;
        props.shadow_offset_dy = shadow_offset_dy;

        return std::make_shared<RangeSliderWidget>(key, start_value, end_value, on_change, std::move(props));
    }
};

} // namespace enki
