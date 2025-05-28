#pragma once
/// @file progress_ring.hpp
/// @brief Advanced ProgressRing widget with Determinate/Indeterminate modes, Gradients, and SkSL Shader Injection.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/widgets/flexbox.hpp"
#include <string>
#include <vector>
#include <memory>

namespace enki {

/// Options for configuring a ProgressRing
struct ProgressRingOptions {
    float size = 48.0f;
    float stroke_width = 6.0f;
    
    Color background_color = 0xFF1E293B; // Dark slate
    Color progress_color   = 0xFF3B82F6; // Blue 500
    std::vector<Color> gradient_colors = {}; // Optional sweep gradient
    
    Color glow_color = 0x00000000;
    float glow_blur  = 0.0f;
    
    bool round_cap   = true;
    float start_angle = -90.0f; // 12 o'clock
    
    bool indeterminate = false;
    std::string custom_shader = ""; // SkSL Runtime Shader
};

/// A circular progress ring widget with optional center child
class ProgressRing : public StatefulWidget {
public:
    float value; // 0.0f to 1.0f
    WidgetPtr child;
    ProgressRingOptions options;

    explicit ProgressRing(float value = 0.0f, WidgetPtr child = nullptr, ProgressRingOptions options = ProgressRingOptions())
        : value(value), child(std::move(child)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ProgressRing"; }
};

/// Helper function to construct a ProgressRing widget
inline WidgetPtr progressRing(float value = 0.0f, WidgetPtr child = nullptr, ProgressRingOptions options = ProgressRingOptions()) {
    return std::make_shared<ProgressRing>(value, std::move(child), std::move(options));
}

} // namespace enki
