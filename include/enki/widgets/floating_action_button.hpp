#pragma once
/// @file floating_action_button.hpp
/// @brief FloatingActionButton (FAB) widget
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/button.hpp"

namespace enki {

struct FloatingActionButtonProps {
    Key key = Key::none();
    WidgetPtr child;
    ButtonCallback on_pressed;
    
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
};

class FloatingActionButton : public StatelessWidget {
public:
    WidgetPtr child;
    ButtonCallback on_pressed;
    FloatingActionButtonProps options;
    bool disabled;

    FloatingActionButton(WidgetPtr child, ButtonCallback on_pressed = nullptr, FloatingActionButtonProps options = FloatingActionButtonProps(), bool disabled = false)
        : child(std::move(child)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}
    
    FloatingActionButton(Key key, WidgetPtr child, ButtonCallback on_pressed, FloatingActionButtonProps options, bool disabled)
        : StatelessWidget(std::move(key)), child(std::move(child)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}

    [[nodiscard]] WidgetPtr build(BuildContext& context) override;
    [[nodiscard]] std::string_view typeName() const override { return "FloatingActionButton"; }
    
    // Fluent API
    FloatingActionButton* bgColor(Color c) { options.normal_color = c; return this; }
    FloatingActionButton* hoverColor(Color c) { options.hover_color = c; return this; }
    FloatingActionButton* size(float s) { options.size = s; return this; }
    FloatingActionButton* borderRadius(float r) { options.border_radius = r; return this; }
    FloatingActionButton* elevation(float shadow_blur, float offset_dy) { 
        options.shadow_blur = shadow_blur; 
        options.shadow_offset_dy = offset_dy; 
        return this; 
    }
};

inline std::shared_ptr<FloatingActionButton> floatingActionButton(WidgetPtr child, ButtonCallback on_pressed = nullptr, FloatingActionButtonProps options = FloatingActionButtonProps()) {
    return std::make_shared<FloatingActionButton>(std::move(child), std::move(on_pressed), std::move(options), on_pressed == nullptr);
}

inline std::shared_ptr<FloatingActionButton> floatingActionButton(FloatingActionButtonProps props) {
    bool is_disabled = props.on_pressed == nullptr;
    auto key = std::move(props.key);
    auto child = std::move(props.child);
    auto on_pressed = std::move(props.on_pressed);
    return std::make_shared<FloatingActionButton>(std::move(key), std::move(child), std::move(on_pressed), std::move(props), is_disabled);
}

inline std::shared_ptr<FloatingActionButton> floatingActionButton(FloatingActionButtonProps props, WidgetPtr child) {
    props.child = std::move(child);
    return floatingActionButton(std::move(props));
}

} // namespace enki
