#include "enki/widgets/badge.hpp"

namespace enki {

WidgetPtr BadgeWidget::build(BuildContext&) {
    ContainerProps c_props = {
        .color = bg_color,
        .border_radius = border_radius,
        .child = label
    };
    
    // If it has a label, add padding, else set fixed size for a dot
    if (label) {
        c_props.padding = padding;
    } else {
        c_props.width = StyleValue::point(size);
        c_props.height = StyleValue::point(size);
    }
    
    auto badge_box = container(c_props);
    
    PositionedProps p_props = {
        .child = badge_box
    };
    
    float ox = offset.x;
    float oy = offset.y;
    
    if (alignment == Alignment::TopRight) {
        p_props.top = StyleValue::point(oy);
        p_props.right = StyleValue::point(-ox);
    } else if (alignment == Alignment::TopLeft) {
        p_props.top = StyleValue::point(oy);
        p_props.left = StyleValue::point(ox);
    } else if (alignment == Alignment::BottomRight) {
        p_props.bottom = StyleValue::point(-oy);
        p_props.right = StyleValue::point(-ox);
    } else if (alignment == Alignment::BottomLeft) {
        p_props.bottom = StyleValue::point(-oy);
        p_props.left = StyleValue::point(ox);
    } else {
        // Fallback for center, etc.
        p_props.top = StyleValue::point(oy);
        p_props.right = StyleValue::point(-ox);
    }
    
    auto p = positioned(p_props);
    
    // Create stack to overlay badge on top of child
    // Clip::None allows the badge to render outside the child's bounds if offset pushes it
    return stack({
        .children = {child, p},
        .clip_behavior = Clip::None
    });
}

} // namespace enki
