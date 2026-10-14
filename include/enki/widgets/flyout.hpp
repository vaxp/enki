#pragma once
/// @file flyout.hpp
/// @brief Shell Flyout / Drawer widget subsystem for Desktop Shells & Panels.
/// Provides native Control Center, Notification Center, and System Tray flyouts
/// with smart panel-relative anchoring, multi-surface isolation, and auto-dismiss.
///
/// @copyright ENKI Framework — Apache 2.0 License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/platform/window.hpp"
#include "enki/widgets/container.hpp"

#include <memory>
#include <functional>
#include <string>
#include <optional>
#include <vector>

namespace enki {

class SurfaceHost;

/// Placement strategy for Flyout relative to the trigger button / panel or mouse cursor
enum class FlyoutPlacement {
    Auto,         ///< Automatically determine anchor & margins based on screen quadrant
    BelowAnchor,  ///< Force flyout below the trigger button
    AboveAnchor,  ///< Force flyout above the trigger button
    LeftAnchor,   ///< Force flyout to the left of the trigger button
    RightAnchor,  ///< Force flyout to the right of the trigger button
    AtCursor,     ///< Spawn exactly at mouse click coordinates (like a context popup menu)
    Manual,       ///< Spawn at custom_position coordinates
};

/// Trigger interaction for opening the Flyout
enum class FlyoutTrigger {
    Click,
    SecondaryClick,
    Manual
};

/// @brief Programmatic controller for Flyout state.
class FlyoutController {
private:
    std::function<void(bool)> toggle_callback_;
    bool is_open_ = false;

public:
    void setToggleCallback(std::function<void(bool)> cb) {
        toggle_callback_ = std::move(cb);
    }

    void show() {
        is_open_ = true;
        if (toggle_callback_) toggle_callback_(true);
    }

    void hide() {
        is_open_ = false;
        if (toggle_callback_) toggle_callback_(false);
    }

    void toggle() {
        if (is_open_) hide();
        else show();
    }

    [[nodiscard]] bool isOpen() const { return is_open_; }
};

/// Configuration options for Flyout styling, positioning, and lifecycle
struct FlyoutOptions {
    int32_t         width            = 360;          ///< Desired flyout width
    int32_t         height           = 480;          ///< Desired flyout height
    FlyoutPlacement placement        = FlyoutPlacement::Auto;
    FlyoutTrigger   trigger          = FlyoutTrigger::Click;
    Point           offset           = {0.0f, 6.0f}; ///< Gap from the trigger widget or cursor
    std::optional<Point> custom_position = std::nullopt; ///< Explicit screen position when using Manual placement or overriding

    bool            auto_dismiss     = true;         ///< Automatically close when clicking outside
    KeyboardMode    keyboard_mode    = KeyboardMode::OnDemand; ///< Focus mode for keyboard navigation/search
    ShellLayer      layer            = ShellLayer::Top;        ///< System surface stacking layer

    // ── Full BoxDecoration / Custom Window Frame ───────────────
    std::optional<BoxDecoration>  decoration = std::nullopt; ///< Full container decoration override

    // Visual styling shortcuts (applied when decoration is not explicitly set)
    Color           background_color = 0xF5181C24;   ///< Modern translucent dark background
    Color           border_color     = 0xFF333A46;   ///< Subtle border stroke
    float           border_width     = 1.0f;
    float           border_radius    = 16.0f;        ///< Rounded corners
    float           elevation        = 24.0f;        ///< Drop shadow radius
    Color           shadow_color     = 0x70000000;

    // Advanced procedural shaders & SVG vector frame injection (matching container_demo)
    std::string     border_shader    = "";           ///< SkSL procedural border shader (e.g. Daggers & Blood, Cyber Neon)
    std::string     background_shader= "";           ///< SkSL procedural background shader (e.g. Live Plasma, Matrix)
    std::string     border_svg       = "";           ///< Direct SVG vector border (e.g. Sci-Fi HUD chamfers)
    std::string     background_svg   = "";           ///< Direct SVG vector background
    std::optional<GradientConfig> gradient = std::nullopt;
    std::vector<BoxShadow> box_shadow = {};

    EdgeInsets      padding          = EdgeInsets::all(16.0f);
    std::string     custom_shader    = "";           ///< Legacy alias for background_shader
    std::function<void()> on_close   = nullptr;      ///< Invoked when flyout is closed
};

/// @brief Handle representing an open Flyout session
class FlyoutHandle {
public:
    explicit FlyoutHandle(SurfaceHost* host) : host_(host) {}
    ~FlyoutHandle() = default;

    void close();
    void setSurfaceHost(SurfaceHost* host) { host_ = host; }
    [[nodiscard]] bool isOpen() const { return host_ != nullptr; }
    [[nodiscard]] SurfaceHost* surfaceHost() const { return host_; }

private:
    friend class FlyoutState;
    friend class FlyoutWidget;
    SurfaceHost* host_ = nullptr;
};

/// @brief High-level Stateful widget wrapping an anchor child and a shell flyout drawer.
class FlyoutWidget : public StatefulWidget {
public:
    WidgetPtr child;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder;
    FlyoutOptions options;
    std::shared_ptr<FlyoutController> controller;

    FlyoutWidget(WidgetPtr child,
                 std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder,
                 FlyoutOptions options = FlyoutOptions(),
                 std::shared_ptr<FlyoutController> controller = nullptr)
        : child(std::move(child)),
          builder(std::move(builder)),
          options(std::move(options)),
          controller(std::move(controller)) {}

    FlyoutWidget(WidgetPtr child,
                 std::function<WidgetPtr(BuildContext&)> simple_builder,
                 FlyoutOptions options = FlyoutOptions(),
                 std::shared_ptr<FlyoutController> controller = nullptr)
        : child(std::move(child)),
          builder([b = std::move(simple_builder)](BuildContext& ctx, std::shared_ptr<FlyoutHandle>) {
              return b ? b(ctx) : nullptr;
          }),
          options(std::move(options)),
          controller(std::move(controller)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FlyoutWidget"; }

    /// Imperative launcher to open a Flyout from any BuildContext
    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder,
        FlyoutOptions options = FlyoutOptions()
    );

    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&)> builder,
        FlyoutOptions options = FlyoutOptions()
    ) {
        return show(context, [b = std::move(builder)](BuildContext& ctx, std::shared_ptr<FlyoutHandle>) {
            return b ? b(ctx) : nullptr;
        }, options);
    }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Flyout {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder = nullptr;
    std::function<WidgetPtr(BuildContext&)> simple_builder = nullptr;
    FlyoutOptions options = {};
    std::shared_ptr<FlyoutController> controller = nullptr;

    operator WidgetPtr() const {
        if (simple_builder) {
            return std::make_shared<FlyoutWidget>(child, simple_builder, options, controller);
        }
        return std::make_shared<FlyoutWidget>(child, builder, options, controller);
    }

    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder,
        FlyoutOptions options = FlyoutOptions()
    ) {
        return FlyoutWidget::show(context, std::move(builder), options);
    }

    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&)> builder,
        FlyoutOptions options = FlyoutOptions()
    ) {
        return FlyoutWidget::show(context, std::move(builder), options);
    }
};

inline std::shared_ptr<FlyoutWidget> flyout(Flyout f) {
    return std::static_pointer_cast<FlyoutWidget>(static_cast<WidgetPtr>(f));
}

} // namespace enki
