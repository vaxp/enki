#pragma once
/// @file window_frame.hpp
/// @brief Comprehensive Client-Side Decoration (CSD) WindowFrame container widget.
///
/// Features:
///   - Integrates TitleBar at top and user application content below.
///   - 8-direction interactive native resize handles (Top, Bottom, Left, Right, 4 corners)
///   - Automatic dynamic cursor switching (ResizeHorizontal, ResizeVertical, ResizeTopLeft, etc.)
///   - State-aware styling: rounded corners when floating, square when maximized
///   - Seamless compositor resize initiation via window->beginResize()
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/platform/window.hpp"
#include "enki/widgets/titlebar.hpp"
#include <string>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// WindowFrame Props
// ════════════════════════════════════════════════════════════════

struct WindowFrameProps {
    Key key = Key::none();
    Window* window = nullptr;                   ///< Target window
    WidgetPtr content = nullptr;                ///< Application root content widget
    WidgetPtr titlebar = nullptr;               ///< Custom TitleBar widget (auto-created if null)
    std::string title = "ENKI Application";     ///< Title text (used if titlebar is null)

    float border_radius = 10.0f;                ///< Floating window corner radius (0 when maximized)
    Color border_color = 0x26FFFFFF;            ///< Subtle outer window border
    float border_width = 1.0f;                  ///< Border stroke width
    Color background_color = 0xFF0F1117;        ///< Window content background color
    std::optional<Color> titlebar_background_color = std::nullopt;          ///< Custom active titlebar background color
    std::optional<Color> titlebar_inactive_background_color = std::nullopt; ///< Custom inactive titlebar background color
    TitleBarStyle titlebar_style = TitleBarStyle::VAXPOS;                  ///< Titlebar style (Default or VAXPOS)

    float resize_thickness = 6.0f;              ///< Grab thickness of edge resize handles
    float corner_size = 14.0f;                  ///< Grab dimension of corner resize handles
    bool enable_resize = true;                  ///< Enable edge resizing
};

// ════════════════════════════════════════════════════════════════
// WindowFrame Widget
// ════════════════════════════════════════════════════════════════

class WindowFrame : public StatefulWidget {
public:
    WindowFrameProps props;

    explicit WindowFrame(WindowFrameProps p = {})
        : StatefulWidget(p.key), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "WindowFrame"; }
};

/// Helper to create a WindowFrame widget
inline std::shared_ptr<WindowFrame> windowFrame(WindowFrameProps props = {}) {
    return std::make_shared<WindowFrame>(std::move(props));
}

inline std::shared_ptr<WindowFrame> windowFrame(Window* window, WidgetPtr content, std::string title = "") {
    WindowFrameProps p;
    p.window = window;
    p.content = std::move(content);
    if (!title.empty()) p.title = std::move(title);
    return std::make_shared<WindowFrame>(std::move(p));
}

} // namespace enki
