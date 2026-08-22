#pragma once
/// @file floating_action_button.hpp
/// @brief FloatingActionButton (FAB) widget
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/button.hpp"

namespace enki {

struct FloatingActionButtonProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    ButtonCallback on_pressed = nullptr;
    
    Color normal_color = 0xFF2563EB; // Primary Blue
    Color hover_color = 0xFF3B82F6;
    Color pressed_color = 0xFF1D4ED8;
    Color disabled_color = 0xFF475569;
    
    float size = 56.0f; // Default FAB size
    float border_radius = 28.0f; // Fully circular for standard FAB
    EdgeInsets padding = EdgeInsets::all(16.0f);
    
    // Strong shadow for elevation
    Color shadow_color = 0x60000000;
    float shadow_blur = 12.0f;
    float shadow_offset_dy = 6.0f;
    
    bool enable_ripple = true;
    Color ripple_color = 0x40FFFFFF; // Light ripple on primary background
    bool disabled = false;
};

class FloatingActionButtonWidget : public StatelessWidget {
public:
    WidgetPtr child;
    ButtonCallback on_pressed;
    FloatingActionButtonProps options;
    bool disabled;

    FloatingActionButtonWidget(WidgetPtr child, ButtonCallback on_pressed = nullptr, FloatingActionButtonProps options = FloatingActionButtonProps(), bool disabled = false)
        : child(std::move(child)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}
    
    FloatingActionButtonWidget(Key key, WidgetPtr child, ButtonCallback on_pressed, FloatingActionButtonProps options, bool disabled)
        : StatelessWidget(std::move(key)), child(std::move(child)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}

    [[nodiscard]] WidgetPtr build(BuildContext& context) override;
    [[nodiscard]] std::string_view typeName() const override { return "FloatingActionButton"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct FloatingActionButton {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    ButtonCallback on_pressed = nullptr;

    Color normal_color = 0xFF2563EB;
    Color hover_color = 0xFF3B82F6;
    Color pressed_color = 0xFF1D4ED8;
    Color disabled_color = 0xFF475569;

    float size = 56.0f;
    float border_radius = 28.0f;
    EdgeInsets padding = EdgeInsets::all(16.0f);

    Color shadow_color = 0x60000000;
    float shadow_blur = 12.0f;
    float shadow_offset_dy = 6.0f;

    bool enable_ripple = true;
    Color ripple_color = 0x40FFFFFF;
    bool disabled = false;

    operator WidgetPtr() const {
        FloatingActionButtonProps opts;
        opts.key = key;
        opts.child = child;
        opts.on_pressed = on_pressed;
        opts.normal_color = normal_color;
        opts.hover_color = hover_color;
        opts.pressed_color = pressed_color;
        opts.disabled_color = disabled_color;
        opts.size = size;
        opts.border_radius = border_radius;
        opts.padding = padding;
        opts.shadow_color = shadow_color;
        opts.shadow_blur = shadow_blur;
        opts.shadow_offset_dy = shadow_offset_dy;
        opts.enable_ripple = enable_ripple;
        opts.ripple_color = ripple_color;
        opts.disabled = disabled || (on_pressed == nullptr);

        bool is_dis = opts.disabled;
        return std::make_shared<FloatingActionButtonWidget>(key, child, on_pressed, opts, is_dis);
    }
};

} // namespace enki
