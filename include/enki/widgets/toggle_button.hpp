#pragma once
/// @file toggle_button.hpp
/// @brief ToggleButton widget for ENKI Framework.
/// An atomic pressable button switching between active (ON) and inactive (OFF) visual states.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <functional>
#include <memory>

namespace enki {

enum class ToggleButtonStyle {
    Filled,    ///< Solid background with high contrast when active
    Outlined,  ///< Glowing border and subtle background tint
    Ghost,     ///< Transparent background, accent color on text/icon only
    Glow,      ///< Futuristic neon outer shadow/glow when active
};

struct ToggleButtonProps {
    bool                                is_toggled = false;
    std::function<void(bool)>           on_toggle;
    std::string                         label = "";
    std::string                         icon = "";
    WidgetPtr                           child = nullptr;

    ToggleButtonStyle                   style = ToggleButtonStyle::Filled;
    Color                               active_color = 0xFF00E5FF;
    Color                               inactive_color = 0xFF94A3B8;
    Color                               active_background = 0xFF0C3559;
    Color                               inactive_background = 0x33000000;
    Color                               border_color = 0x4D00E5FF;
    float                               border_width = 1.0f;
    float                               border_radius = 8.0f;
    EdgeInsets                          padding = EdgeInsets::symmetric(8.0f, 16.0f);
    bool                                enabled = true;

    operator WidgetPtr() const;
};

class ToggleButtonWidget : public StatelessWidget {
public:
    ToggleButtonProps props;

    explicit ToggleButtonWidget(ToggleButtonProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "ToggleButton"; }
    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
};

inline WidgetPtr toggleButton(ToggleButtonProps props) {
    return std::make_shared<ToggleButtonWidget>(std::move(props));
}

} // namespace enki
