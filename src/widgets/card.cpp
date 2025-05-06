#include "enki/widgets/card.hpp"

namespace enki {

WidgetPtr Card::build(BuildContext&) {
    auto c = container(child);
    c->color(options.color);
    c->borderRadius(options.border_radius);
    
    if (options.border.has_value()) {
        c->border(options.border.value());
    }
    
    c->margin(options.margin);
    c->padding(options.padding);
    
    if (options.elevation > 0.0f) {
        c->shadow(options.shadow_color, {0.0f, options.elevation / 2.0f}, options.elevation);
    }
    
    return c;
}

} // namespace enki
