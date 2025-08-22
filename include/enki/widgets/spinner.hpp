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

/// An advanced loading spinner indicator widget with optional center child
class SpinnerWidget : public StatefulWidget {
public:
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

    WidgetPtr child = nullptr;

    explicit SpinnerWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Spinner"; }
};

struct Spinner {
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

    WidgetPtr child = nullptr;
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<SpinnerWidget>(key);
        w->style = style;
        w->size = size;
        w->color = color;
        w->gradient_colors = gradient_colors;
        w->spoke_count = spoke_count;
        w->spoke_width = spoke_width;
        w->spoke_length = spoke_length;
        w->dot_count = dot_count;
        w->dot_size = dot_size;
        w->rotation_speed = rotation_speed;
        w->glow_color = glow_color;
        w->glow_blur = glow_blur;
        w->custom_shader = custom_shader;
        w->child = child;
        return w;
    }
};

} // namespace enki
