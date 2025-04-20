#include "enki/widgets/avatar.hpp"
#include "enki/widgets/stack.hpp"

namespace enki {

WidgetPtr Avatar::build(BuildContext& ctx) {
    float diameter = options.radius * 2.0f;
    WidgetPtr content;

    // 1. Content (Image or Initials)
    if (options.image_data) {
        auto img_widget = image(options.image_data);
        img_widget->fit(BoxFit::Cover).size(diameter, diameter);
        content = img_widget;
    } else if (!options.image_path.empty()) {
        auto img_widget = imageAsset(options.image_path);
        img_widget->fit(BoxFit::Cover).size(diameter, diameter);
        content = img_widget;
    } else {
        auto text_widget = text(options.initials);
        // Calculate font size based on radius (rough heuristic: 40% of diameter)
        text_widget->fontSize(diameter * 0.4f)
                   .color(options.text_color)
                   .bold();
        
        auto center_box = container(text_widget);
        center_box->align(Alignment::Center)
                  .width(diameter).height(diameter);
        content = center_box;
    }

    // 2. Wrap in clipped container (the circle)
    auto bg_container = container(content);
    bg_container->width(diameter).height(diameter)
                .borderRadius(options.radius)
                .color(options.background_color)
                .align(Alignment::Center)
                .clip(true); // Ensure content doesn't bleed out of border radius

    if (options.border_width > 0.0f) {
        bg_container->border(options.border_color, options.border_width);
    }
    
    if (options.shadow_blur > 0.0f) {
        bg_container->shadow(options.shadow_color, {0.0f, options.shadow_blur / 2.0f}, options.shadow_blur);
    }

    // 3. Add Status Badge if enabled
    if (options.show_badge) {
        auto badge = container(nullptr);
        badge->width(options.badge_size).height(options.badge_size)
             .borderRadius(options.badge_size / 2.0f)
             .color(options.badge_color)
             .border(options.badge_border_color, options.badge_border_width);
             
        // Position badge at bottom right. The angle is 45 degrees from bottom right.
        // For a circle, bottom right offset is radius * (1 - sqrt(2)/2) from the corner.
        // We'll just position it approximately.
        float offset = options.radius * 0.146f; // (1 - sin(45)) ~ 0.29, half is 0.146
        auto pos_badge = positioned(badge);
        pos_badge->bottom(offset).right(offset);
        
        std::vector<WidgetPtr> stack_children = { bg_container, pos_badge };
        return stack(stack_children);
    }

    return bg_container;
}

WidgetPtr AvatarGroup::build(BuildContext& ctx) {
    if (avatars.empty()) {
        auto empty_box = container(nullptr);
        empty_box->width(0).height(0);
        return empty_box;
    }

    std::vector<WidgetPtr> stack_children;
    size_t count = std::min(avatars.size(), max_avatars);
    float current_left = 0.0f;
    
    // We want the first avatar to be on top? 
    // In Stack, last child is painted on top. 
    // Standard avatar groups usually have first avatar on bottom, or first avatar on top.
    // If first avatar is on top, we must render it last.
    // Wait, let's just reverse the iteration so the first one (index 0) is added last.
    
    float total_width = 0.0f;
    
    for (size_t i = 0; i < count; ++i) {
        // Find radius of this avatar
        float diameter = 48.0f; // Default diameter
        if (auto av = std::dynamic_pointer_cast<Avatar>(avatars[i])) {
            diameter = av->options.radius * 2.0f;
        }
        
        if (i == 0) total_width = diameter;
        else total_width += (diameter + spacing);
    }
    
    // Add "+X" if there are more avatars
    size_t extra = avatars.size() - count;
    WidgetPtr extra_widget = nullptr;
    if (extra > 0) {
        AvatarOptions opt;
        opt.background_color = 0xFF1E293B; // Dark gray
        opt.initials = "+" + std::to_string(extra);
        opt.border_width = 2.0f;
        
        if (auto av = std::dynamic_pointer_cast<Avatar>(avatars[0])) {
            opt.radius = av->options.radius;
        }
        auto extra_av = avatar(opt);
        extra_widget = extra_av;
        total_width += (opt.radius * 2.0f + spacing);
    }

    // Build the stack, adding items in reverse so the first one is painted on top (added last)
    // Actually, usually the first avatar is on the left.
    // Left offset: i * (diameter + spacing)
    // If we add it last, it's drawn on top.
    
    std::vector<std::pair<float, WidgetPtr>> positioned_items;
    
    current_left = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        positioned_items.push_back({current_left, avatars[i]});
        float diameter = 48.0f;
        if (auto av = std::dynamic_pointer_cast<Avatar>(avatars[i])) {
            diameter = av->options.radius * 2.0f;
        }
        current_left += (diameter + spacing);
    }
    
    if (extra_widget) {
        positioned_items.push_back({current_left, extra_widget});
    }
    
    // Now push to stack in reverse
    for (auto it = positioned_items.rbegin(); it != positioned_items.rend(); ++it) {
        auto pos = positioned(it->second);
        pos->left(it->first).top(0.0f);
        stack_children.push_back(pos);
    }

    auto s = stack(stack_children);
    // The stack size depends on its contents, but absolute positioning inside stack doesn't size the stack inherently in ENKI unless configured.
    // We wrap it in a container with exact size.
    float max_d = 48.0f;
    if (auto av = std::dynamic_pointer_cast<Avatar>(avatars[0])) {
        max_d = av->options.radius * 2.0f;
    }
    
    auto box = container(s);
    box->width(total_width).height(max_d);
    
    return box;
}

} // namespace enki
