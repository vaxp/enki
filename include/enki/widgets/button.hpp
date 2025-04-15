#pragma once
/// @file button.hpp
/// @brief Advanced Button widget with Hover, Press, Ripple, and Shader Injection support.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/widgets/flexbox.hpp"
#include <string>
#include <functional>
#include <memory>

namespace enki {

struct ButtonOptions {
    Color normal_color = 0xFF2563EB; // Default primary blue
    Color hover_color = 0xFF3B82F6;
    Color pressed_color = 0xFF1D4ED8;
    Color disabled_color = 0xFF475569;
    
    float border_radius = 8.0f;
    EdgeInsets padding = EdgeInsets::symmetric(10.0f, 20.0f);
    
    // Shadow properties
    Color shadow_color = 0x40000000;
    float shadow_blur = 4.0f;
    float shadow_offset_dy = 2.0f;
    
    // Advanced Effects
    bool enable_ripple = true;
    Color ripple_color = 0x40FFFFFF;
    
    // Shader Injection (SkSL)
    std::string custom_shader = ""; 
    
    float min_width = 64.0f;
    float min_height = 36.0f;
};

using ButtonCallback = std::function<void()>;

class Button : public StatefulWidget {
public:
    WidgetPtr child;
    ButtonCallback on_pressed;
    ButtonOptions options;
    bool disabled;

    Button(WidgetPtr child, ButtonCallback on_pressed = nullptr, ButtonOptions options = ButtonOptions(), bool disabled = false)
        : child(std::move(child)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Button"; }
};

inline WidgetPtr button(WidgetPtr child, ButtonCallback on_pressed = nullptr, ButtonOptions options = ButtonOptions()) {
    return std::make_shared<Button>(std::move(child), std::move(on_pressed), std::move(options), on_pressed == nullptr);
}

} // namespace enki
