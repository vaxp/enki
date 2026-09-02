#pragma once
/// @file titlebar.hpp
/// @brief Modern Client-Side Decoration (CSD) TitleBar component.
///
/// Provides a sleek, interactive window header with:
///   - Native window dragging via platform compositor (Wayland/X11)
///   - Double-click to toggle maximize/restore
///   - Right-click window menu
///   - Minimize, Maximize / Restore, and Close buttons with responsive hover effects
///   - Automatic state synchronization with window focus and maximize events
///   - Slots for leading icon/menu, custom title widget, and trailing actions
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/platform/window.hpp"
#include <string>
#include <functional>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// TitleBar Props
// ════════════════════════════════════════════════════════════════

struct TitleBarProps {
    Key key = Key::none();
    Window* window = nullptr;                   ///< Target window for native CSD actions
    std::string title = "ENKI Application";     ///< Window title text
    WidgetPtr leading = nullptr;                ///< Optional leading widget (e.g. app icon)
    WidgetPtr title_widget = nullptr;           ///< Optional custom title widget
    WidgetPtr trailing = nullptr;               ///< Optional trailing widget (before buttons)

    float height = 38.0f;                       ///< Titlebar height (default 38px)
    Color background_color = 0xFF181B22;        ///< Active titlebar background
    Color inactive_background_color = 0xFF12141A; ///< Inactive titlebar background
    Color title_color = 0xFFE6EDF3;             ///< Active title text color
    Color inactive_title_color = 0xFF7D8590;    ///< Inactive title text color
    float font_size = 13.0f;                    ///< Title font size
    
    // Window control buttons
    bool show_minimize = true;                  ///< Show minimize button
    bool show_maximize = true;                  ///< Show maximize / restore button
    bool show_close = true;                     ///< Show close button

    // Button colors
    Color button_fg = 0xFF9DA7B3;               ///< Icon normal color
    Color button_hover_bg = 0x22FFFFFF;         ///< Hover background for min/max
    Color button_hover_fg = 0xFFFFFFFF;         ///< Hover icon color for min/max
    Color close_hover_bg = 0xFFE81123;          ///< Prominent close button red hover
    Color close_hover_fg = 0xFFFFFFFF;          ///< Close button white icon on hover

    // Interactivity
    bool double_click_maximize = true;          ///< Double click to maximize/restore
    bool right_click_menu = true;               ///< Right click for native window menu

    // Custom callbacks (optional overrides)
    std::function<void()> on_minimize = nullptr;
    std::function<void()> on_maximize = nullptr;
    std::function<void()> on_close = nullptr;
};

// ════════════════════════════════════════════════════════════════
// TitleBar Widget
// ════════════════════════════════════════════════════════════════

class TitleBar : public StatefulWidget {
public:
    TitleBarProps props;

    explicit TitleBar(TitleBarProps p = {})
        : StatefulWidget(p.key), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TitleBar"; }
};

/// Helper to create a TitleBar widget
inline std::shared_ptr<TitleBar> titleBar(TitleBarProps props = {}) {
    return std::make_shared<TitleBar>(std::move(props));
}

inline std::shared_ptr<TitleBar> titleBar(Window* window, std::string title = "") {
    TitleBarProps p;
    p.window = window;
    if (!title.empty()) p.title = std::move(title);
    return std::make_shared<TitleBar>(std::move(p));
}

} // namespace enki
