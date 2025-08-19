#include "enki/widgets/card.hpp"

namespace enki {

WidgetPtr CardWidget::build(BuildContext&) {
    std::vector<BoxShadow> shadows;
    if (elevation > 0.0f) {
        shadows.push_back(BoxShadow(shadow_color, {0.0f, elevation / 2.0f}, elevation));
    }
    
    return container({
        .color = color,
        .border_radius = border_radius,
        .border = border,
        .box_shadow = std::move(shadows),
        .padding = padding,
        .margin = margin,
        .child = child
    });
}

} // namespace enki
