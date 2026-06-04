#pragma once
/// @file helpers.hpp
/// @brief UI widget construction helpers with reliable hit testing.

#include "enki/widgets/container.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include <memory>
#include <functional>
#include <optional>

namespace enki::gallery {

inline std::shared_ptr<GestureDetector> makeClickable(WidgetPtr child, std::function<void()> on_tap) {
    GestureDetectorProps p;
    p.child = std::move(child);
    p.on_tap = std::move(on_tap);
    // Crucial: Opaque hit testing ensures taps on container, images, and text are captured
    p.hit_test_behavior = HitTestBehavior::Opaque;
    return gestureDetector(p);
}

inline std::shared_ptr<ContainerWidget> makeBox(
    WidgetPtr child,
    std::optional<Color> color = std::nullopt,
    std::optional<BorderRadius> radius = std::nullopt,
    std::optional<Border> border = std::nullopt,
    std::optional<StyleInsets> padding = std::nullopt,
    std::optional<StyleValue> width = std::nullopt,
    std::optional<StyleValue> height = std::nullopt,
    std::optional<float> flex = std::nullopt,
    std::optional<Alignment> align = std::nullopt)
{
    ContainerProps p;
    p.child = std::move(child);
    p.color = color;
    p.border_radius = radius;
    p.border = border;
    p.padding = padding;
    p.width = width;
    p.height = height;
    p.flex = flex;
    p.align = align;
    return container(p);
}

inline std::string truncateString(const std::string& str, size_t max_len) {
    if (str.length() <= max_len) return str;
    if (max_len <= 3) return str.substr(0, max_len);
    return str.substr(0, max_len - 3) + "...";
}

} // namespace enki::gallery
