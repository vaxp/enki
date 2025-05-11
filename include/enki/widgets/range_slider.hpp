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

struct RangeSliderOptions {
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

using RangeSliderCallback = std::function<void(float start, float end)>;

class RangeSlider : public SingleChildRenderObjectWidget {
public:
    float start_value;
    float end_value;
    RangeSliderCallback on_change;
    RangeSliderOptions options;

    RangeSlider(float start, float end, RangeSliderCallback on_change, RangeSliderOptions options = RangeSliderOptions())
        : SingleChildRenderObjectWidget(Key::none(), nullptr), start_value(start), end_value(end), on_change(std::move(on_change)), options(std::move(options)) {}

    // Fluent API
    RangeSlider* activeColor(Color c) { options.active_color = c; return this; }
    RangeSlider* inactiveColor(Color c) { options.inactive_color = c; return this; }
    RangeSlider* thumbColor(Color c) { options.thumb_color = c; return this; }
    RangeSlider* trackHeight(float h) { options.track_height = h; return this; }
    RangeSlider* thumbRadius(float r) { options.thumb_radius = r; return this; }
    RangeSlider* min(float m) { options.min_value = m; return this; }
    RangeSlider* max(float m) { options.max_value = m; return this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "RangeSlider"; }
};

inline std::shared_ptr<RangeSlider> rangeSlider(float start, float end, RangeSliderCallback on_change) {
    return std::make_shared<RangeSlider>(start, end, std::move(on_change));
}

} // namespace enki
