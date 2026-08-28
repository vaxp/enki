#include "enki/widgets/avatar.hpp"
#include "enki/widgets/stack.hpp"

namespace enki {

WidgetPtr AvatarWidget::build(BuildContext& ctx) {
    float diameter = options.radius * 2.0f;
    WidgetPtr content;

    // 1. Content (Image or Initials)
    if (options.image_data) {
        content = image({
            .image = options.image_data,
            .width = StyleValue::point(diameter),
            .height = StyleValue::point(diameter),
            .fit = BoxFit::Cover,
        });
    } else if (!options.image_path.empty()) {
        content = image({
            .source_path = options.image_path,
            .width = StyleValue::point(diameter),
            .height = StyleValue::point(diameter),
            .fit = BoxFit::Cover,
        });
    } else {
        auto text_widget = text({
            .text = options.initials,
            .color = options.text_color,
            .font_size = diameter * 0.4f,
            .font_weight = FontWeight::Bold,
        });
        
        content = container({
            .align = Alignment::Center,
            .width = StyleValue::point(diameter),
            .height = StyleValue::point(diameter),
            .child = text_widget,
        });
    }

    // 2. Wrap in clipped container (the circle)
    std::optional<Border> avatar_border;
    if (options.border_width > 0.0f) {
        avatar_border = Border(options.border_color, options.border_width);
    }

    std::vector<BoxShadow> avatar_shadows;
    if (options.shadow_blur > 0.0f) {
        avatar_shadows.emplace_back(options.shadow_color, Point{0.0f, options.shadow_blur / 2.0f}, options.shadow_blur);
    }

    auto bg_container = container({
        .color = options.background_color,
        .border_radius = BorderRadius::circular(options.radius),
        .border = avatar_border,
        .box_shadow = std::move(avatar_shadows),
        .clip_content = true,
        .align = Alignment::Center,
        .width = StyleValue::point(diameter),
        .height = StyleValue::point(diameter),
        .child = content,
    });

    // 3. Add Status Badge if enabled
    if (options.show_badge) {
        auto badge = container({
            .color = options.badge_color,
            .border_radius = BorderRadius::circular(options.badge_size / 2.0f),
            .border = Border(options.badge_border_color, options.badge_border_width),
            .width = StyleValue::point(options.badge_size),
            .height = StyleValue::point(options.badge_size),
        });
             
        float offset = options.radius * 0.146f;
        auto pos_badge = Positioned {
            .child = badge,
            .right = StyleValue::point(offset),
            .bottom = StyleValue::point(offset),
        };
        
        return Stack {
            .children = { bg_container, pos_badge }
        };
    }

    return bg_container;
}

WidgetPtr AvatarGroupWidget::build(BuildContext& ctx) {
    if (avatars.empty()) {
        return container({
            .width = StyleValue::point(0.0f),
            .height = StyleValue::point(0.0f),
        });
    }

    std::vector<WidgetPtr> stack_children;
    size_t count = std::min(avatars.size(), max_avatars);
    float current_left = 0.0f;
    float total_width = 0.0f;
    
    for (size_t i = 0; i < count; ++i) {
        float diameter = 48.0f; // Default diameter
        if (auto av = std::dynamic_pointer_cast<AvatarWidget>(avatars[i])) {
            diameter = av->options.radius * 2.0f;
        }
        
        if (i == 0) total_width = diameter;
        else total_width += (diameter + spacing);
    }
    
    size_t extra = avatars.size() - count;
    WidgetPtr extra_widget = nullptr;
    if (extra > 0) {
        float rad = 24.0f;
        if (auto av = std::dynamic_pointer_cast<AvatarWidget>(avatars[0])) {
            rad = av->options.radius;
        }
        extra_widget = Avatar {
            .radius = rad,
            .background_color = 0xFF1E293B,
            .initials = "+" + std::to_string(extra),
            .border_width = 2.0f,
        };
        total_width += (rad * 2.0f + spacing);
    }
    
    std::vector<std::pair<float, WidgetPtr>> positioned_items;
    
    current_left = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        positioned_items.push_back({current_left, avatars[i]});
        float diameter = 48.0f;
        if (auto av = std::dynamic_pointer_cast<AvatarWidget>(avatars[i])) {
            diameter = av->options.radius * 2.0f;
        }
        current_left += (diameter + spacing);
    }
    
    if (extra_widget) {
        positioned_items.push_back({current_left, extra_widget});
    }
    
    // Now push to stack in reverse
    for (auto it = positioned_items.rbegin(); it != positioned_items.rend(); ++it) {
        stack_children.push_back(Positioned {
            .child = it->second,
            .top = StyleValue::point(0.0f),
            .left = StyleValue::point(it->first),
        });
    }

    float max_d = 48.0f;
    if (auto av = std::dynamic_pointer_cast<AvatarWidget>(avatars[0])) {
        max_d = av->options.radius * 2.0f;
    }
    
    return container({
        .width = StyleValue::point(total_width),
        .height = StyleValue::point(max_d),
        .child = Stack {
            .children = std::move(stack_children),
        }
    });
}

} // namespace enki
