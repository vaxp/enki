#pragma once
/// @file spinner.hpp
/// @brief Advanced Spinner widget supporting multiple visual styles (Spokes, OrbitDots, DualArc, CustomShader).
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

/// Visual style variants for the Spinner widget
enum class SpinnerStyle {
    Spokes,       ///< style radiating ticks that fade in sequence
    OrbitDots,    ///< Material / Fluent style orbiting dots
    DualArc,      ///< Dual counter-rotating glowing arcs
    CustomShader  ///< Direct developer SkSL procedural shader injection
};

/// Options for configuring a Spinner
struct SpinnerOptions {
    SpinnerStyle style = SpinnerStyle::Spokes;
    
    float size = 36.0f;
    Color color = 0xFF3B82F6; // Primary blue
    std::vector<Color> gradient_colors = {}; // Optional multi-color sequence
    
    // Spokes parameters (for SpinnerStyle::Spokes)
    int spoke_count = 12;
    float spoke_width = 3.0f;
    float spoke_length = 8.0f;
    
    // OrbitDots parameters (for SpinnerStyle::OrbitDots)
    int dot_count = 5;
    float dot_size = 6.0f;
    
    // Animation & Effects
    float rotation_speed = 1.0f;
    Color glow_color = 0x00000000;
    float glow_blur = 0.0f;
    
    // Custom SkSL Shader
    std::string custom_shader = "";
};

/// An advanced loading spinner indicator widget with optional center child
class Spinner : public StatefulWidget {
public:
    WidgetPtr child;
    SpinnerOptions options;

    explicit Spinner(SpinnerOptions options = SpinnerOptions(), WidgetPtr child = nullptr)
        : child(std::move(child)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Spinner"; }
};

/// Helper function to construct a Spinner widget
inline WidgetPtr spinner(SpinnerOptions options = SpinnerOptions(), WidgetPtr child = nullptr) {
    return std::make_shared<Spinner>(std::move(options), std::move(child));
}

} // namespace enki
