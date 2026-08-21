#pragma once
/// @file dialog.hpp
/// @brief Advanced In-Window Overlay Modal Dialog widget for ENKI Framework.
/// Follows the robust container-wrapping architecture (like Drawer & BottomSheet),
/// with AnimationController, Ticker, scale-and-fade animations, scrim backdrops,
/// Escape-key dismissal, and Positioned stack layout.
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

namespace enki {

/// Semantic dialog type
enum class DialogType {
    Standard,   ///< Regular neutral modal card
    Info,       ///< Informational notification with sky/blue accents
    Success,    ///< Positive confirmation with emerald/green accents
    Warning,    ///< Precautionary notice with amber/yellow accents
    Danger      ///< Destructive action with crimson/red accents
};

/// Animation curve preset
enum class DialogAnimation {
    ScaleAndFade,   ///< Scale from 0.88 -> 1.0 with opacity fade (Default)
    SlideAndFade,   ///< Slide down from top with opacity fade
    FadeOnly        ///< Clean opacity fade
};

/// ════════════════════════════════════════════════════════════════
/// Dialog Action Button Definition
/// ════════════════════════════════════════════════════════════════

struct DialogAction {
    std::string id = "";
    std::string label = "";
    bool is_primary = false;
    bool is_danger = false;
    bool is_cancel = false;
    std::function<void()> on_click;

    static DialogAction primary(std::string label, std::function<void()> cb = nullptr) {
        DialogAction a;
        a.id = "primary";
        a.label = std::move(label);
        a.is_primary = true;
        a.on_click = std::move(cb);
        return a;
    }

    static DialogAction cancel(std::string label = "Cancel", std::function<void()> cb = nullptr) {
        DialogAction a;
        a.id = "cancel";
        a.label = std::move(label);
        a.is_cancel = true;
        a.on_click = std::move(cb);
        return a;
    }

    static DialogAction danger(std::string label, std::function<void()> cb = nullptr) {
        DialogAction a;
        a.id = "danger";
        a.label = std::move(label);
        a.is_primary = true;
        a.is_danger = true;
        a.on_click = std::move(cb);
        return a;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Dialog Options
/// ════════════════════════════════════════════════════════════════

struct DialogOptions {
    DialogType type = DialogType::Standard;
    DialogAnimation animation = DialogAnimation::ScaleAndFade;

    float width = 480.0f;
    float max_height = 600.0f;
    float border_radius = 14.0f;

    bool barrier_dismissible = true;  ///< Tap backdrop scrim to dismiss
    bool escape_to_close = true;       ///< Press Escape key to dismiss
    bool show_close_button = true;     ///< Show ✕ icon in top-right

    std::string icon = "";            ///< Leading emoji/icon (e.g. ⚠️, 🗑️, ✅, ℹ️, 🔒)
    std::string title = "";
    std::string subtitle = "";

    // Action buttons (if using structured templates)
    std::vector<DialogAction> actions;

    // Theme Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color overlay_color    = 0x99000000; // Semi-transparent black backdrop
    Color title_color      = 0xFFFFFFFF; // White
    Color subtitle_color   = 0xFF94A3B8; // Slate 400
    Color icon_badge_bg    = 0x2E38BDF8; // Sky badge background
    Color icon_badge_fg    = 0xFF38BDF8; // Sky badge foreground

    // Callbacks
    std::function<void()> on_opened;
    std::function<void()> on_closed;
};

/// ════════════════════════════════════════════════════════════════
/// Dialog Controller
/// ════════════════════════════════════════════════════════════════

class DialogController {
public:
    std::function<void()> show_fn;
    std::function<void()> hide_fn;
    std::function<void()> toggle_fn;
    std::function<bool()> is_open_fn;

    void show()   { if (show_fn) show_fn(); }
    void hide()   { if (hide_fn) hide_fn(); }
    void toggle() { if (toggle_fn) toggle_fn(); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }
};

struct DialogProps {
    Key key = Key::none();
    WidgetPtr dialog_content;
    WidgetPtr child;
    bool initial_open = false;
    DialogOptions options;
    std::shared_ptr<DialogController> controller;
};

/// ════════════════════════════════════════════════════════════════
/// Dialog Widget
/// ════════════════════════════════════════════════════════════════

class Dialog : public StatefulWidget {
public:
    WidgetPtr dialog_content;                    ///< Content inside the modal dialog
    WidgetPtr body;                              ///< Main page body content to wrap
    bool initial_open = false;
    DialogOptions options;
    std::shared_ptr<DialogController> controller;

    Dialog() = default;

    Dialog(WidgetPtr content, WidgetPtr body_, DialogOptions opts = {})
        : dialog_content(std::move(content)), body(std::move(body_)),
          options(std::move(opts)) {}

    // Fluent Builder API
    Dialog& width(float w) { options.width = w; return *this; }
    Dialog& borderRadius(float r) { options.border_radius = r; return *this; }
    Dialog& barrierDismissible(bool d) { options.barrier_dismissible = d; return *this; }
    Dialog& type(DialogType t) { options.type = t; return *this; }
    Dialog& title(std::string t) { options.title = std::move(t); return *this; }
    Dialog& subtitle(std::string s) { options.subtitle = std::move(s); return *this; }
    Dialog& icon(std::string i) { options.icon = std::move(i); return *this; }
    Dialog& addAction(DialogAction a) { options.actions.push_back(std::move(a)); return *this; }
    Dialog& setController(std::shared_ptr<DialogController> c) {
        controller = std::move(c);
        return *this;
    }
    Dialog& onOpened(std::function<void()> fn) { options.on_opened = std::move(fn); return *this; }
    Dialog& onClosed(std::function<void()> fn) { options.on_closed = std::move(fn); return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Dialog"; }
};

/// Factory function to wrap body with modal dialog overlay
inline std::shared_ptr<Dialog> dialog(
    WidgetPtr dialog_content,
    WidgetPtr body,
    DialogOptions options = {}) {
    return std::make_shared<Dialog>(std::move(dialog_content), std::move(body), std::move(options));
}

inline std::shared_ptr<Dialog> dialog(DialogProps props) {
    auto d = std::make_shared<Dialog>(std::move(props.dialog_content), std::move(props.child), std::move(props.options));
    d->key = props.key;
    d->initial_open = props.initial_open;
    d->controller = std::move(props.controller);
    return d;
}

/// Convenience factory for confirmation/alert dialog
inline std::shared_ptr<Dialog> confirmDialog(
    std::string title,
    std::string message,
    WidgetPtr body,
    std::function<void()> on_confirm,
    std::function<void()> on_cancel = nullptr,
    bool is_danger = false) {
    DialogOptions opts;
    opts.title = std::move(title);
    opts.subtitle = std::move(message);
    opts.type = is_danger ? DialogType::Danger : DialogType::Standard;
    opts.icon = is_danger ? "⚠️" : "ℹ️";

    if (is_danger) {
        opts.actions.push_back(DialogAction::cancel("Cancel", std::move(on_cancel)));
        opts.actions.push_back(DialogAction::danger("Delete", std::move(on_confirm)));
    } else {
        opts.actions.push_back(DialogAction::cancel("Cancel", std::move(on_cancel)));
        opts.actions.push_back(DialogAction::primary("Confirm", std::move(on_confirm)));
    }

    return std::make_shared<Dialog>(nullptr, std::move(body), opts);
}

} // namespace enki
