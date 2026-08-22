#pragma once
/// @file icon_button.hpp
/// @brief IconButton widget
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/button.hpp"
#include "enki/widgets/icon.hpp"

namespace enki {

class IconButtonWidget : public StatelessWidget {
public:
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

    explicit IconButtonWidget(Key key) : StatelessWidget(std::move(key)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& context) override;
    [[nodiscard]] std::string_view typeName() const override { return "IconButton"; }
};

struct IconButton {
    Key key = Key::none();
    WidgetPtr icon = nullptr;
    ButtonCallback on_pressed = nullptr;
    bool disabled = false;

    Color normal_color = 0x00000000;
    Color hover_color = 0x1A000000;
    Color pressed_color = 0x33000000;
    Color disabled_color = 0x00000000;
    
    float size = 48.0f;
    EdgeInsets padding = EdgeInsets::all(8.0f);
    
    bool enable_ripple = true;
    Color ripple_color = 0x40000000;

    operator WidgetPtr() const {
        auto w = std::make_shared<IconButtonWidget>(key);
        w->icon = icon;
        w->on_pressed = on_pressed;
        w->disabled = disabled || (on_pressed == nullptr);
        w->normal_color = normal_color;
        w->hover_color = hover_color;
        w->pressed_color = pressed_color;
        w->disabled_color = disabled_color;
        w->size = size;
        w->padding = padding;
        w->enable_ripple = enable_ripple;
        w->ripple_color = ripple_color;
        return w;
    }
};

} // namespace enki
