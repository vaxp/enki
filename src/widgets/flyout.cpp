/// @file flyout.cpp
/// @brief Shell Flyout / Drawer implementation for Desktop Shells.
/// @copyright ENKI Framework — Apache 2.0 License

#include "enki/widgets/flyout.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/app/app.hpp"
#include "enki/shell/surface_host.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"


#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace enki {

void FlyoutHandle::close() {
    if (!host_) return;
    App* app = App::instance();
    if (app) {
        app->removeSurface(host_);
    }
    host_ = nullptr;
}

// ════════════════════════════════════════════════════════════════
// Flyout Positioning & Anchor Calculation
// ════════════════════════════════════════════════════════════════

static WindowConfig computeFlyoutConfig(const Rect& anchor_rect, const Size& screen_sz, const FlyoutOptions& opt) {
    WindowConfig cfg;
    cfg.title             = "enki-flyout";
    cfg.width             = opt.width;
    cfg.height            = opt.height;
    cfg.mode              = WindowMode::Flyout;
    cfg.layer             = opt.layer;
    cfg.keyboard_mode     = opt.keyboard_mode;
    cfg.transparent       = true;
    cfg.borderless        = true;
    cfg.always_on_top     = true;
    cfg.override_redirect = true;
    cfg.resizable         = false;
    cfg.exclusive_zone    = -1;

    // 1. Direct or Cursor positioning
    if (opt.placement == FlyoutPlacement::AtCursor || opt.placement == FlyoutPlacement::Manual || opt.custom_position.has_value()) {
        Point pt = opt.custom_position.value_or(Point{anchor_rect.x, anchor_rect.y});
        float px = pt.x + opt.offset.x;
        float py = pt.y + opt.offset.y;

        // Keep inside screen bounds
        if (px + opt.width > screen_sz.width - 8.0f) {
            px = std::max(8.0f, pt.x - opt.width - opt.offset.x);
        }
        if (py + opt.height > screen_sz.height - 8.0f) {
            py = std::max(8.0f, pt.y - opt.height - opt.offset.y);
        }
        px = std::max(8.0f, px);
        py = std::max(8.0f, py);

        cfg.anchor = ShellAnchor::TopLeft;
        cfg.margin.left = static_cast<int32_t>(px);
        cfg.margin.top = static_cast<int32_t>(py);
        cfg.x = static_cast<int>(px);
        cfg.y = static_cast<int>(py);
        return cfg;
    }

    float btn_cx = anchor_rect.x + anchor_rect.width * 0.5f;
    float btn_cy = anchor_rect.y + anchor_rect.height * 0.5f;

    bool is_top_panel = (btn_cy <= screen_sz.height * 0.5f);
    bool is_right_half = (btn_cx > screen_sz.width * 0.5f);

    if (opt.placement == FlyoutPlacement::AboveAnchor) is_top_panel = false;
    else if (opt.placement == FlyoutPlacement::BelowAnchor) is_top_panel = true;

    if (is_top_panel) {
        if (is_right_half) {
            cfg.anchor = ShellAnchor::TopRight;
            int32_t mr = static_cast<int32_t>(screen_sz.width - (btn_cx + opt.width * 0.5f));
            cfg.margin.right = std::max(8, mr);
            if (cfg.margin.right + opt.width > screen_sz.width - 8) {
                cfg.margin.right = std::max(8, static_cast<int32_t>(screen_sz.width - opt.width - 8));
            }
        } else {
            cfg.anchor = ShellAnchor::TopLeft;
            int32_t ml = static_cast<int32_t>(btn_cx - opt.width * 0.5f);
            cfg.margin.left = std::max(8, ml);
            if (cfg.margin.left + opt.width > screen_sz.width - 8) {
                cfg.margin.left = std::max(8, static_cast<int32_t>(screen_sz.width - opt.width - 8));
            }
        }
        cfg.margin.top = std::max(0, static_cast<int32_t>(anchor_rect.y + anchor_rect.height + opt.offset.y));
    } else {
        // Bottom panel
        if (is_right_half) {
            cfg.anchor = ShellAnchor::BottomRight;
            int32_t mr = static_cast<int32_t>(screen_sz.width - (btn_cx + opt.width * 0.5f));
            cfg.margin.right = std::max(8, mr);
            if (cfg.margin.right + opt.width > screen_sz.width - 8) {
                cfg.margin.right = std::max(8, static_cast<int32_t>(screen_sz.width - opt.width - 8));
            }
        } else {
            cfg.anchor = ShellAnchor::BottomLeft;
            int32_t ml = static_cast<int32_t>(btn_cx - opt.width * 0.5f);
            cfg.margin.left = std::max(8, ml);
            if (cfg.margin.left + opt.width > screen_sz.width - 8) {
                cfg.margin.left = std::max(8, static_cast<int32_t>(screen_sz.width - opt.width - 8));
            }
        }
        cfg.margin.bottom = std::max(0, static_cast<int32_t>((screen_sz.height - anchor_rect.y) + opt.offset.y));
    }

    // Absolute screen pixel coordinates for X11 / Win32
    if (hasAnchor(cfg.anchor, ShellAnchor::Right)) {
        cfg.x = static_cast<int>(screen_sz.width - cfg.margin.right - opt.width);
    } else {
        cfg.x = cfg.margin.left;
    }

    if (hasAnchor(cfg.anchor, ShellAnchor::Bottom)) {
        cfg.y = static_cast<int>(screen_sz.height - cfg.margin.bottom - opt.height);
    } else {
        cfg.y = cfg.margin.top;
    }

    return cfg;
}

// ════════════════════════════════════════════════════════════════
// FlyoutState Implementation
// ════════════════════════════════════════════════════════════════

class FlyoutState : public State {
private:
    std::shared_ptr<FlyoutHandle> active_handle_;
    Point last_cursor_pos_ = {0.0f, 0.0f};
    std::chrono::steady_clock::time_point last_dismiss_time_{};

public:
    void initState() override {
        State::initState();
        auto* widget_ptr = static_cast<const FlyoutWidget*>(widget());
        if (widget_ptr && widget_ptr->controller) {
            widget_ptr->controller->setToggleCallback([this](bool show) {
                if (show) showFlyoutNow();
                else hideFlyoutNow();
            });
        }
    }

    void dispose() override {
        hideFlyoutNow();
        State::dispose();
    }

    void showFlyoutNow() {
        if (active_handle_ && active_handle_->isOpen()) return;

        // Auto-dismiss debounce: if this click was the one that closed the open flyout,
        // do not reopen it on tap-up. This provides natural toggle-to-close behavior.
        auto now = std::chrono::steady_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(now - last_dismiss_time_).count();
        if (elapsed_ms < 350.0) {
            return;
        }

        auto* widget_ptr = static_cast<const FlyoutWidget*>(widget());
        if (!widget_ptr || !widget_ptr->builder) return;

        Element* elem = element();
        if (!elem) return;

        BuildContext ctx(elem);
        FlyoutOptions opts = widget_ptr->options;
        if (opts.placement == FlyoutPlacement::AtCursor && !opts.custom_position.has_value()) {
            opts.custom_position = last_cursor_pos_;
        }

        auto orig_on_close = opts.on_close;
        opts.on_close = [this, orig_on_close]() {
            last_dismiss_time_ = std::chrono::steady_clock::now();
            active_handle_ = nullptr;
            if (orig_on_close) orig_on_close();
        };

        active_handle_ = FlyoutWidget::show(ctx, widget_ptr->builder, opts);
    }

    void hideFlyoutNow() {
        if (active_handle_) {
            last_dismiss_time_ = std::chrono::steady_clock::now();
            active_handle_->close();
            active_handle_ = nullptr;
        }
    }

    void toggleFlyout() {
        if (active_handle_ && active_handle_->isOpen()) {
            hideFlyoutNow();
        } else {
            showFlyoutNow();
        }
    }

    WidgetPtr build(BuildContext& context) override {
        auto* widget_ptr = static_cast<const FlyoutWidget*>(widget());
        if (!widget_ptr) return nullptr;

        GestureDetectorProps props;
        props.child = widget_ptr->child;
        props.hit_test_behavior = HitTestBehavior::Translucent;

        if (widget_ptr->options.trigger == FlyoutTrigger::Click) {
            props.on_tap_up = [this](const TapUpDetails& details) {
                last_cursor_pos_ = {details.global_position.x, details.global_position.y};
                toggleFlyout();
            };
        } else if (widget_ptr->options.trigger == FlyoutTrigger::SecondaryClick) {
            props.on_secondary_tap_up = [this](const TapUpDetails& details) {
                last_cursor_pos_ = {details.global_position.x, details.global_position.y};
                toggleFlyout();
            };
        }

        return std::make_shared<GestureDetector>(props);
    }
};

std::unique_ptr<State> FlyoutWidget::createState() {
    return std::make_unique<FlyoutState>();
}

std::shared_ptr<FlyoutHandle> FlyoutWidget::show(
    BuildContext& context,
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder,
    FlyoutOptions options
) {
    App* app = App::instance();
    if (!app) {
        std::cerr << "[ENKI Flyout] No active App instance found\n";
        return nullptr;
    }

    Element* elem = context.element();
    RenderObject* ro = elem ? elem->findRenderObject() : nullptr;
    Rect target_bounds = ro ? ro->globalBounds() : Rect{0, 0, 0, 0};
    Size screen_sz = context.mediaSize();
    if (screen_sz.width <= 0.0f || screen_sz.height <= 0.0f) {
        screen_sz = Size{1920.0f, 1080.0f};
    }

    WindowConfig cfg = computeFlyoutConfig(target_bounds, screen_sz, options);

    SurfaceHost* parent_host = nullptr;
    if (elem) {
        parent_host = app->findSurfaceByOwner(elem->owner());
    }

    auto handle = std::make_shared<FlyoutHandle>(nullptr);

    WidgetPtr inner_content = builder ? builder(context, handle) : nullptr;

    BoxDecoration dec;
    if (options.decoration) {
        dec = *options.decoration;
    } else {
        dec.color = options.background_color;
        dec.border_radius = BorderRadius::circular(options.border_radius);
        if (options.border_width > 0.0f) {
            dec.border = Border(options.border_color, options.border_width);
        }
        if (!options.border_shader.empty()) {
            dec.border_shader = options.border_shader;
            if (!dec.border) dec.border = Border(Colors::Transparent, options.border_width > 0.0f ? options.border_width : 2.0f);
        }
        if (!options.background_shader.empty()) {
            dec.background_shader = options.background_shader;
        } else if (!options.custom_shader.empty()) {
            dec.background_shader = options.custom_shader;
        }
        if (!options.border_svg.empty()) {
            dec.border_svg = options.border_svg;
            if (!dec.border) dec.border = Border(Colors::Transparent, options.border_width > 0.0f ? options.border_width : 2.0f);
        }
        if (!options.background_svg.empty()) {
            dec.background_svg = options.background_svg;
        }
        if (options.gradient) {
            dec.gradient = options.gradient;
        }
        if (!options.box_shadow.empty()) {
            dec.box_shadow = options.box_shadow;
        } else if (options.elevation > 0.0f) {
            dec.box_shadow = { BoxShadow::standard(options.shadow_color, options.elevation, options.elevation * 0.25f) };
        }
    }

    ContainerProps c_props;
    c_props.padding = StyleInsets::only(options.padding.top, options.padding.right, options.padding.bottom, options.padding.left);
    c_props.color = dec.color;
    c_props.border = dec.border;
    c_props.border_radius = dec.border_radius;
    c_props.box_shadow = dec.box_shadow;
    c_props.gradient = dec.gradient;
    c_props.border_shader = dec.border_shader;
    c_props.background_shader = dec.background_shader;
    c_props.border_svg = dec.border_svg;
    c_props.background_svg = dec.background_svg;
    c_props.shape = dec.shape;
    c_props.clip_content = dec.clip_content;
    c_props.child = inner_content;

    auto root_widget = container(std::move(c_props));

    SurfaceHost* host = app->addFlyout(parent_host, cfg, root_widget);
    if (!host) {
        std::cerr << "[ENKI Flyout] Failed to create native flyout surface\n";
        return nullptr;
    }

    host->setAutoDismiss(options.auto_dismiss);
    handle->host_ = host;

    std::weak_ptr<FlyoutHandle> weak_handle = handle;
    host->onClose().connect([weak_handle, on_close = options.on_close]() {
        if (auto h = weak_handle.lock()) {
            h->host_ = nullptr;
        }
        if (on_close) on_close();
    });

    return handle;
}

} // namespace enki
