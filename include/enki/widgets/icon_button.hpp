#pragma once
/// @file icon_button.hpp
/// @brief IconButton widget
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/button.hpp"
#include "enki/widgets/icon.hpp"

namespace enki {

struct IconButtonProps {
    Key key = Key::none();
    WidgetPtr icon = nullptr;
    ButtonCallback on_pressed = nullptr;
    bool disabled = false;

    Color normal_color = 0x00000000; // Transparent by default
    Color hover_color = 0x1A000000;  // Light gray on hover
    Color pressed_color = 0x33000000; // Darker gray on press
    Color disabled_color = 0x00000000;
    
    float size = 48.0f; // Size of the bounding box
    EdgeInsets padding = EdgeInsets::all(8.0f);
    
    bool enable_ripple = true;
    Color ripple_color = 0x40000000; // Dark ripple
};

class IconButton : public StatelessWidget {
public:
    WidgetPtr icon;
    ButtonCallback on_pressed;
    IconButtonProps options;
    bool disabled;

    IconButton(WidgetPtr icon, ButtonCallback on_pressed = nullptr, IconButtonProps options = IconButtonProps(), bool disabled = false)
        : icon(std::move(icon)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}
        
    IconButton(Key key, WidgetPtr icon, ButtonCallback on_pressed, IconButtonProps options, bool disabled)
        : StatelessWidget(std::move(key)), icon(std::move(icon)), on_pressed(std::move(on_pressed)), options(std::move(options)), disabled(disabled) {}

    [[nodiscard]] WidgetPtr build(BuildContext& context) override;
    [[nodiscard]] std::string_view typeName() const override { return "IconButton"; }
    
    // Fluent API
    IconButton* bgColor(Color c) { options.normal_color = c; return this; }
    IconButton* hoverColor(Color c) { options.hover_color = c; return this; }
    IconButton* size(float s) { options.size = s; return this; }
    IconButton* paddingAll(float p) { options.padding = EdgeInsets::all(p); return this; }
};

inline std::shared_ptr<IconButton> iconButton(WidgetPtr icon, ButtonCallback on_pressed = nullptr, IconButtonProps options = IconButtonProps()) {
    return std::make_shared<IconButton>(std::move(icon), std::move(on_pressed), std::move(options), on_pressed == nullptr);
}

inline std::shared_ptr<IconButton> iconButton(IconButtonProps props) {
    bool is_disabled = props.disabled || props.on_pressed == nullptr;
    return std::make_shared<IconButton>(std::move(props.key), std::move(props.icon), std::move(props.on_pressed), std::move(props), is_disabled);
}

inline std::shared_ptr<IconButton> iconButton(IconButtonProps props, WidgetPtr icon) {
    props.icon = std::move(icon);
    return iconButton(std::move(props));
}

} // namespace enki
