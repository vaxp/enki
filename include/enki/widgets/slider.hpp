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
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

struct SliderOptions {
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

using SliderCallback = std::function<void(float)>;

class Slider : public SingleChildRenderObjectWidget {
public:
    float value;
    SliderCallback on_change;
    SliderOptions options;

    Slider(float value, SliderCallback on_change, SliderOptions options = SliderOptions())
        : SingleChildRenderObjectWidget(Key::none(), nullptr), value(value), on_change(std::move(on_change)), options(std::move(options)) {}

    // Fluent API
    Slider* activeColor(Color c) { options.active_color = c; return this; }
    Slider* inactiveColor(Color c) { options.inactive_color = c; return this; }
    Slider* thumbColor(Color c) { options.thumb_color = c; return this; }
    Slider* trackHeight(float h) { options.track_height = h; return this; }
    Slider* thumbRadius(float r) { options.thumb_radius = r; return this; }
    Slider* min(float m) { options.min_value = m; return this; }
    Slider* max(float m) { options.max_value = m; return this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Slider"; }
};

inline std::shared_ptr<Slider> slider(float value, SliderCallback on_change) {
    return std::make_shared<Slider>(value, std::move(on_change));
}

} // namespace enki
