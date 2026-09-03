#include "enki/widgets/toggle_button.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"

namespace enki {

ToggleButtonProps::operator WidgetPtr() const {
    return std::make_shared<ToggleButtonWidget>(*this);
}

WidgetPtr ToggleButtonWidget::build(BuildContext&) {
    WidgetPtr content = props.child;

    if (!content) {
        std::vector<WidgetPtr> row_children;
        if (!props.icon.empty()) {
            row_children.push_back(text(props.icon, {
                .color = props.is_toggled ? props.active_color : props.inactive_color,
                .font_size = 14.0f,
            }));
        }
        if (!props.label.empty()) {
            row_children.push_back(text(props.label, {
                .color = props.is_toggled ? props.active_color : props.inactive_color,
                .font_size = 13.0f,
                .font_weight = props.is_toggled ? FontWeight::Bold : FontWeight::Medium,
            }));
        }
        if (row_children.size() == 1) {
            content = row_children[0];
        } else if (!row_children.empty()) {
            content = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = std::move(row_children),
            });
        } else {
            content = text("Toggle", {.color = props.inactive_color});
        }
    }

    Color bg_color;
    Color hover_color;
    Color border_color = 0x00000000;
    float border_w = 0.0f;
    Color shadow_col = 0x00000000;
    float shadow_blur = 0.0f;

    switch (props.style) {
        case ToggleButtonStyle::Filled:
            bg_color = props.is_toggled ? props.active_background : props.inactive_background;
            hover_color = props.is_toggled ? (props.active_background + 0x1A1A1A) : (props.inactive_background + 0x1A1A1A);
            if (props.is_toggled) {
                border_color = props.active_color;
                border_w = props.border_width;
            }
            break;
        case ToggleButtonStyle::Outlined:
            bg_color = props.is_toggled ? (props.active_color & 0x26FFFFFF) : 0x00000000;
            hover_color = props.is_toggled ? (props.active_color & 0x40FFFFFF) : 0x1AFFFFFF;
            border_color = props.is_toggled ? props.active_color : props.border_color;
            border_w = props.border_width > 0.0f ? props.border_width : 1.0f;
            break;
        case ToggleButtonStyle::Ghost:
            bg_color = props.is_toggled ? (props.active_color & 0x1AFFFFFF) : 0x00000000;
            hover_color = props.is_toggled ? (props.active_color & 0x33FFFFFF) : 0x14FFFFFF;
            break;
        case ToggleButtonStyle::Glow:
            bg_color = props.is_toggled ? props.active_background : props.inactive_background;
            hover_color = props.is_toggled ? (props.active_background + 0x151515) : 0x22FFFFFF;
            if (props.is_toggled) {
                border_color = props.active_color;
                border_w = 1.2f;
                shadow_col = props.active_color & 0x66FFFFFF;
                shadow_blur = 8.0f;
            }
            break;
    }

    auto btn = button(
        content,
        [on_toggle = props.on_toggle, enabled = props.enabled, is_toggled = props.is_toggled]() {
            if (enabled && on_toggle) {
                on_toggle(!is_toggled);
            }
        },
        ButtonProps{
            .disabled = !props.enabled,
            .normal_color = bg_color,
            .hover_color = hover_color,
            .pressed_color = props.is_toggled ? (bg_color - 0x101010) : (bg_color + 0x202020),
            .border_radius = props.border_radius,
            .padding = props.padding,
            .shadow_color = shadow_col,
            .shadow_blur = shadow_blur,
        }
    );

    if (border_w > 0.0f && ((border_color >> 24) > 0)) {
        return container(Container{
            .border_radius = BorderRadius::circular(props.border_radius),
            .border = Border(border_color, border_w),
            .child = btn,
        });
    }

    return btn;
}

} // namespace enki
