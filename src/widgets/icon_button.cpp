#include "enki/widgets/icon_button.hpp"

namespace enki {

WidgetPtr IconButton::build(BuildContext& context) {
    ButtonOptions btn_options;
    btn_options.normal_color = options.normal_color;
    btn_options.hover_color = options.hover_color;
    btn_options.pressed_color = options.pressed_color;
    btn_options.disabled_color = options.disabled_color;
    
    // Fully circular
    btn_options.border_radius = options.size / 2.0f;
    btn_options.padding = options.padding;
    
    // No shadows for IconButton by default
    btn_options.shadow_blur = 0.0f;
    btn_options.shadow_offset_dy = 0.0f;
    
    btn_options.enable_ripple = options.enable_ripple;
    btn_options.ripple_color = options.ripple_color;
    
    btn_options.min_width = options.size;
    btn_options.min_height = options.size;
    
    return button(icon, on_pressed, btn_options);
}

} // namespace enki
