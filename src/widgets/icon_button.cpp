#include "enki/widgets/icon_button.hpp"

namespace enki {

WidgetPtr IconButtonWidget::build(BuildContext& context) {
    return button(icon, on_pressed, ButtonProps{
        .normal_color = normal_color,
        .hover_color = hover_color,
        .pressed_color = pressed_color,
        .disabled_color = disabled_color,
        .border_radius = size / 2.0f,
        .padding = padding,
        .shadow_blur = 0.0f,
        .shadow_offset_dy = 0.0f,
        .enable_ripple = enable_ripple,
        .ripple_color = ripple_color,
        .min_width = size,
        .min_height = size
    });
}

} // namespace enki
