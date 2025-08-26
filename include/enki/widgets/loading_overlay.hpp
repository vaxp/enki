#pragma once
/// @file loading_overlay.hpp
/// @brief Advanced LoadingOverlay widget for ENKI Framework (Category 8. Feedback).
/// Provides full-screen or scoped loading/busy overlays with 4 indicator styles,
/// determinate/indeterminate progress tracking, cancel actions, and 60fps animations.
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

/// Visual indicator style
enum class LoadingIndicatorStyle {
    Spinner,        ///< Continuous rotating arc spinner
    ProgressRing,   ///< Circular progress ring (with % label in center)
    ProgressBar,    ///< Linear progress bar with track
    DotsPulse,      ///< 3 rhythmic pulsing/bouncing dots
    Custom          ///< Custom child widget
};

/// ════════════════════════════════════════════════════════════════
/// LoadingOverlay Options
/// ════════════════════════════════════════════════════════════════

struct LoadingOverlayProps {
    Key key = Key::none();
    LoadingIndicatorStyle indicator_style = LoadingIndicatorStyle::Spinner;

    std::string title = "Processing...";
    std::string message = "Please wait while operation completes.";
    float progress = 0.0f;               ///< 0.0f to 1.0f for determinate progress
    bool is_determinate = false;

    bool allow_cancel = false;           ///< Show interactive cancel button
    std::string cancel_label = "Cancel Operation";

    float width = 360.0f;
    float border_radius = 12.0f;

    // Theme Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color overlay_color    = 0xB30B1120; // 70% dark slate backdrop
    Color title_color      = 0xFFFFFFFF; // White
    Color message_color    = 0xFF94A3B8; // Slate 400
    Color accent_color     = 0xFF38BDF8; // Sky 400

    // Custom loader widget (if indicator_style == Custom)
    WidgetPtr custom_indicator;

    // Callbacks
    std::function<void()> on_cancel;
    std::function<void()> on_shown;
    std::function<void()> on_hidden;
};

/// ════════════════════════════════════════════════════════════════
/// LoadingOverlay Controller
/// ════════════════════════════════════════════════════════════════

class LoadingOverlayController {
public:
    std::function<void(const LoadingOverlayProps&)> show_fn;
    std::function<void()> hide_fn;
    std::function<void(float, const std::string&)> set_progress_fn;
    std::function<void(const std::string&)> set_message_fn;
    std::function<bool()> is_loading_fn;

    void show(const LoadingOverlayProps& opts) { if (show_fn) show_fn(opts); }
    void hide() { if (hide_fn) hide_fn(); }
    void setProgress(float p, const std::string& msg = "") { if (set_progress_fn) set_progress_fn(p, msg); }
    void setMessage(const std::string& msg) { if (set_message_fn) set_message_fn(msg); }
    [[nodiscard]] bool isLoading() const { return is_loading_fn ? is_loading_fn() : false; }

    // Semantic Convenience Helpers
    void showSpinner(std::string title = "Loading...", std::string msg = "Please wait...", bool cancelable = false, std::function<void()> on_cancel = nullptr) {
        LoadingOverlayProps o;
        o.indicator_style = LoadingIndicatorStyle::Spinner;
        o.title = std::move(title);
        o.message = std::move(msg);
        o.allow_cancel = cancelable;
        o.on_cancel = std::move(on_cancel);
        show(o);
    }

    void showProgressRing(float p, std::string title = "Uploading...", std::string msg = "", bool cancelable = false, std::function<void()> on_cancel = nullptr) {
        LoadingOverlayProps o;
        o.indicator_style = LoadingIndicatorStyle::ProgressRing;
        o.is_determinate = true;
        o.progress = p;
        o.title = std::move(title);
        o.message = std::move(msg);
        o.allow_cancel = cancelable;
        o.on_cancel = std::move(on_cancel);
        show(o);
    }

    void showProgressBar(float p, std::string title = "Compiling...", std::string msg = "", bool cancelable = false, std::function<void()> on_cancel = nullptr) {
        LoadingOverlayProps o;
        o.indicator_style = LoadingIndicatorStyle::ProgressBar;
        o.is_determinate = true;
        o.progress = p;
        o.title = std::move(title);
        o.message = std::move(msg);
        o.allow_cancel = cancelable;
        o.on_cancel = std::move(on_cancel);
        show(o);
    }

    void showDots(std::string title = "Syncing...", std::string msg = "Connecting to peer nodes...") {
        LoadingOverlayProps o;
        o.indicator_style = LoadingIndicatorStyle::DotsPulse;
        o.title = std::move(title);
        o.message = std::move(msg);
        show(o);
    }
};

/// ════════════════════════════════════════════════════════════════
/// LoadingOverlay Widget Implementation
/// ════════════════════════════════════════════════════════════════

class LoadingOverlayWidget : public StatefulWidget {
public:
    WidgetPtr body;                                      ///< Background content to wrap
    std::shared_ptr<LoadingOverlayController> controller;
    bool initial_loading = false;
    LoadingOverlayProps initial_options;

    LoadingOverlayWidget() = default;
    LoadingOverlayWidget(WidgetPtr body_, std::shared_ptr<LoadingOverlayController> ctrl,
                         bool init_loading = false, LoadingOverlayProps init_opts = {})
        : body(std::move(body_)), controller(std::move(ctrl)),
          initial_loading(init_loading), initial_options(std::move(init_opts)) {}
          
    LoadingOverlayWidget(Key k, WidgetPtr body_, std::shared_ptr<LoadingOverlayController> ctrl, bool init_loading, LoadingOverlayProps init_opts)
        : StatefulWidget(std::move(k)), body(std::move(body_)), controller(std::move(ctrl)), initial_loading(init_loading), initial_options(std::move(init_opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "LoadingOverlay"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct LoadingOverlay {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    std::shared_ptr<LoadingOverlayController> controller = nullptr;
    bool initial_loading = false;
    LoadingOverlayProps initial_options;

    operator WidgetPtr() const {
        return std::make_shared<LoadingOverlayWidget>(key, body, controller, initial_loading, initial_options);
    }
};

} // namespace enki
