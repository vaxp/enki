#include "enki/widgets/badge.hpp"

namespace enki {

WidgetPtr Badge::build(BuildContext&) {
    auto badge_box = container(label);
    badge_box->color(options.bg_color);
    badge_box->borderRadius(options.border_radius);
    
    // If it has a label, add padding, else set fixed size for a dot
    if (label) {
        badge_box->padding(options.padding);
    } else {
        badge_box->size(options.size, options.size);
    }
    
    // Position the badge using Positioned relative to the stack
    auto p = positioned(badge_box);
    
    float ox = options.offset.x;
    float oy = options.offset.y;
    
    if (options.alignment == Alignment::TopRight) {
        p->top(oy).right(-ox);
    } else if (options.alignment == Alignment::TopLeft) {
        p->top(oy).left(ox);
    } else if (options.alignment == Alignment::BottomRight) {
        p->bottom(-oy).right(-ox);
    } else if (options.alignment == Alignment::BottomLeft) {
        p->bottom(-oy).left(ox);
    } else {
        // Fallback for center, etc.
        p->top(oy).right(-ox);
    }
    
    // Create stack to overlay badge on top of child
    // Clip::None allows the badge to render outside the child's bounds if offset pushes it
    auto s = stack({child, p});
    s->clip(Clip::None);
    
    return s;
}

} // namespace enki
