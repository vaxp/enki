#pragma once
/// @file slider.hpp
/// @brief Advanced Slider widget for ENKI Framework
///
/// Features:
///   - Customizable track and thumb colors
///   - Drop shadow on thumb for elevation
///   - Hover and drag states
///   - Smooth dragging support
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

using SliderCallback = std::function<void(float)>;

class SliderWidget : public SingleChildRenderObjectWidget {
public:
    float value;
    SliderCallback on_change;
    
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

    SliderWidget(Key key) : SingleChildRenderObjectWidget(std::move(key), nullptr) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Slider"; }
};

struct Slider {
    float value = 0.0f;
    SliderCallback on_change = nullptr;

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
    
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<SliderWidget>(key);
        w->value = value;
        w->on_change = on_change;
        w->active_color = active_color;
        w->inactive_color = inactive_color;
        w->thumb_color = thumb_color;
        w->track_height = track_height;
        w->thumb_radius = thumb_radius;
        w->min_value = min_value;
        w->max_value = max_value;
        w->shadow_color = shadow_color;
        w->shadow_blur = shadow_blur;
        w->shadow_offset_dy = shadow_offset_dy;
        return w;
    }
};

} // namespace enki
