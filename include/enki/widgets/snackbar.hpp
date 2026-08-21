#pragma once
/// @file snackbar.hpp
/// @brief Advanced In-Window Overlay Snackbar / Toast widget for ENKI Framework.
/// Follows the robust container-wrapping architecture (like Drawer, BottomSheet & Dialog),
/// with AnimationController, Ticker, auto-dismiss timers, countdown progress bar,
/// pause-on-hover, 6-way multi-placement, and Positioned stack layout.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>

namespace enki {

/// Semantic snackbar type
enum class SnackbarType {
    Standard,   ///< Regular dark slate card
    Success,    ///< Positive confirmation with emerald accents
    Error,      ///< Failure notification with crimson accents
    Warning,    ///< Cautionary alert with amber accents
    Info,       ///< Informational notice with sky accents
    Loading     ///< Async loading operation with spinner
};

/// 6-Way Placement relative to the window viewport
enum class SnackbarPlacement {
    BottomCenter,   ///< Centered along bottom edge (Standard)
    BottomRight,    ///< Bottom-right corner (Desktop/IDE default)
    BottomLeft,     ///< Bottom-left corner
    TopCenter,      ///< Centered along top edge (Alert banner)
    TopRight,       ///< Top-right corner (Toast notification)
    TopLeft         ///< Top-left corner
};

/// ════════════════════════════════════════════════════════════════
/// Snackbar Action Button Definition
/// ════════════════════════════════════════════════════════════════

struct SnackbarAction {
    std::string label = "";
    bool is_danger = false;
    std::function<void()> on_click;

    SnackbarAction() = default;
    SnackbarAction(std::string lbl, std::function<void()> cb = nullptr, bool danger = false)
        : label(std::move(lbl)), is_danger(danger), on_click(std::move(cb)) {}
};

/// ════════════════════════════════════════════════════════════════
/// Snackbar Options
/// ════════════════════════════════════════════════════════════════

struct SnackbarOptions {
    SnackbarType type = SnackbarType::Standard;
    SnackbarPlacement placement = SnackbarPlacement::BottomCenter;

    int duration_ms = 4000;              ///< Auto-dismiss duration in milliseconds (0 = persistent)
    bool show_progress_bar = true;       ///< Show animated timer progress bar along bottom edge
    bool pause_on_hover = true;          ///< Freeze timer countdown when cursor hovers over card
    bool show_close_button = true;       ///< Show ✕ dismiss button

    float width = 420.0f;
    float border_radius = 10.0f;
    float margin = 24.0f;                ///< Viewport edge margin

    std::string icon = "";               ///< Leading emoji/icon (e.g. ✅, ❌, ⚠️, ℹ️, ⚡)
    std::string title = "";
    std::string message = "";
    std::optional<SnackbarAction> action;

    // Styling Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color title_color      = 0xFFFFFFFF; // White
    Color message_color    = 0xFFCBD5E1; // Slate 300
    Color accent_color     = 0xFF38BDF8; // Sky 400
    Color progress_bar_col = 0xFF38BDF8; // Progress bar accent

    // Callbacks
    std::function<void()> on_shown;
    std::function<void()> on_dismissed;
};

/// ════════════════════════════════════════════════════════════════
/// Snackbar Controller
/// ════════════════════════════════════════════════════════════════

class SnackbarController {
public:
    std::function<void(const SnackbarOptions&)> show_fn;
    std::function<void()> hide_fn;
    std::function<bool()> is_open_fn;

    void show(const SnackbarOptions& opts) { if (show_fn) show_fn(opts); }
    void hide() { if (hide_fn) hide_fn(); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }

    // Semantic Convenience Helpers
    void showSuccess(std::string msg, std::string title = "Success",
                     std::optional<SnackbarAction> action = std::nullopt,
                     SnackbarPlacement placement = SnackbarPlacement::BottomCenter) {
        SnackbarOptions o;
        o.type = SnackbarType::Success;
        o.icon = "✅";
        o.title = std::move(title);
        o.message = std::move(msg);
        o.action = std::move(action);
        o.placement = placement;
        o.accent_color = 0xFF10B981;
        o.progress_bar_col = 0xFF10B981;
        o.border_color = 0xFF059669;
        show(o);
    }

    void showError(std::string msg, std::string title = "Error Occurred",
                   std::optional<SnackbarAction> action = std::nullopt,
                   SnackbarPlacement placement = SnackbarPlacement::BottomCenter) {
        SnackbarOptions o;
        o.type = SnackbarType::Error;
        o.icon = "❌";
        o.title = std::move(title);
        o.message = std::move(msg);
        o.action = std::move(action);
        o.placement = placement;
        o.accent_color = 0xFFEF4444;
        o.progress_bar_col = 0xFFEF4444;
        o.border_color = 0xFFDC2626;
        show(o);
    }

    void showWarning(std::string msg, std::string title = "Warning",
                     std::optional<SnackbarAction> action = std::nullopt,
                     SnackbarPlacement placement = SnackbarPlacement::BottomCenter) {
        SnackbarOptions o;
        o.type = SnackbarType::Warning;
        o.icon = "⚠️";
        o.title = std::move(title);
        o.message = std::move(msg);
        o.action = std::move(action);
        o.placement = placement;
        o.accent_color = 0xFFF59E0B;
        o.progress_bar_col = 0xFFF59E0B;
        o.border_color = 0xFFD97706;
        show(o);
    }

    void showInfo(std::string msg, std::string title = "Information",
                  std::optional<SnackbarAction> action = std::nullopt,
                  SnackbarPlacement placement = SnackbarPlacement::BottomCenter) {
        SnackbarOptions o;
        o.type = SnackbarType::Info;
        o.icon = "ℹ️";
        o.title = std::move(title);
        o.message = std::move(msg);
        o.action = std::move(action);
        o.placement = placement;
        o.accent_color = 0xFF38BDF8;
        o.progress_bar_col = 0xFF38BDF8;
        o.border_color = 0xFF0284C7;
        show(o);
    }
};

/// ════════════════════════════════════════════════════════════════
/// Snackbar Widget
/// ════════════════════════════════════════════════════════════════

struct SnackbarProps {
    Key key = Key::none();
    WidgetPtr body;
    std::shared_ptr<SnackbarController> controller;
    SnackbarOptions initial_options;
};

class Snackbar : public StatefulWidget {
public:
    WidgetPtr body;                              ///< Main page body content to wrap
    std::shared_ptr<SnackbarController> controller;
    SnackbarOptions initial_options;

    Snackbar() = default;
    Snackbar(WidgetPtr body_, std::shared_ptr<SnackbarController> ctrl, SnackbarOptions init_opts = {})
        : body(std::move(body_)), controller(std::move(ctrl)), initial_options(std::move(init_opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Snackbar"; }
};

/// Factory function to wrap body with Snackbar overlay layer
inline std::shared_ptr<Snackbar> snackbar(
    WidgetPtr body,
    std::shared_ptr<SnackbarController> controller,
    SnackbarOptions options = {}) {
    return std::make_shared<Snackbar>(std::move(body), std::move(controller), std::move(options));
}

inline std::shared_ptr<Snackbar> snackbar(SnackbarProps props) {
    auto sb = std::make_shared<Snackbar>(std::move(props.body), std::move(props.controller), std::move(props.initial_options));
    sb->key = props.key;
    return sb;
}

} // namespace enki
