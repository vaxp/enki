#include "enki/widgets/floating_action_button.hpp"

namespace enki {

WidgetPtr FloatingActionButton::build(BuildContext& context) {
    ButtonProps btn_options;
    btn_options.normal_color = options.normal_color;
    btn_options.hover_color = options.hover_color;
    btn_options.pressed_color = options.pressed_color;
    btn_options.disabled_color = options.disabled_color;
    
    btn_options.border_radius = options.border_radius;
    btn_options.padding = options.padding;
    
    btn_options.shadow_color = options.shadow_color;
    btn_options.shadow_blur = options.shadow_blur;
    btn_options.shadow_offset_dy = options.shadow_offset_dy;
    
    btn_options.enable_ripple = options.enable_ripple;
    btn_options.ripple_color = options.ripple_color;
    
    btn_options.min_width = options.size;
    btn_options.min_height = options.size;
    
    return button(child, on_pressed, btn_options);
}

} // namespace enki
