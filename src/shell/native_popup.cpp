/// @file native_popup.cpp
/// @brief Native Compositor-level Popup surface implementation.

#include "enki/shell/native_popup.hpp"
#include "enki/shell/shell_app.hpp"
#include <iostream>

namespace enki {

NativePopup::NativePopup(const PopupOptions& options)
    : options_(options) {}

NativePopup::~NativePopup() {
    close();
}

std::shared_ptr<NativePopup> NativePopup::show(
    BuildContext& context,
    const PopupOptions& options,
    std::function<WidgetPtr(BuildContext&)> builder) {
    return show(context, options, [builder = std::move(builder)](BuildContext& ctx, std::shared_ptr<NativePopup>) {
        return builder(ctx);
    });
}

std::shared_ptr<NativePopup> NativePopup::show(
    BuildContext& context,
    const PopupOptions& options,
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> builder) {

    ShellApp* app = ShellApp::instance();
    if (!app) {
        std::cerr << "[ENKI NativePopup] No active ShellApp instance found\n";
        return nullptr;
    }

    auto popup = std::shared_ptr<NativePopup>(new NativePopup(options));

    // 1. Calculate screen coordinates based on anchor and placement
    int32_t popup_x = static_cast<int32_t>(options.anchor_rect.x);
    int32_t popup_y = static_cast<int32_t>(options.anchor_rect.y + options.anchor_rect.height + options.offset_gap);

    switch (options.placement) {
        case PopupPlacement::BottomStart:
            popup_x = static_cast<int32_t>(options.anchor_rect.x);
            popup_y = static_cast<int32_t>(options.anchor_rect.y + options.anchor_rect.height + options.offset_gap);
            break;
        case PopupPlacement::BottomEnd:
            popup_x = static_cast<int32_t>(options.anchor_rect.x + options.anchor_rect.width - options.width);
            popup_y = static_cast<int32_t>(options.anchor_rect.y + options.anchor_rect.height + options.offset_gap);
            break;
        case PopupPlacement::BottomCenter:
            popup_x = static_cast<int32_t>(options.anchor_rect.x + (options.anchor_rect.width - options.width) / 2.0f);
            popup_y = static_cast<int32_t>(options.anchor_rect.y + options.anchor_rect.height + options.offset_gap);
            break;
        case PopupPlacement::TopStart:
            popup_x = static_cast<int32_t>(options.anchor_rect.x);
            popup_y = static_cast<int32_t>(options.anchor_rect.y - options.height - options.offset_gap);
            break;
        case PopupPlacement::TopEnd:
            popup_x = static_cast<int32_t>(options.anchor_rect.x + options.anchor_rect.width - options.width);
            popup_y = static_cast<int32_t>(options.anchor_rect.y - options.height - options.offset_gap);
            break;
        case PopupPlacement::TopCenter:
            popup_x = static_cast<int32_t>(options.anchor_rect.x + (options.anchor_rect.width - options.width) / 2.0f);
            popup_y = static_cast<int32_t>(options.anchor_rect.y - options.height - options.offset_gap);
            break;
        default:
            break;
    }

    if (popup_x < 0) popup_x = 0;
    if (popup_y < 0) popup_y = 0;

    // 2. Build the popup content widget
    WidgetPtr content = builder(context, popup);

    // 3. Create independent native surface
    // Try Wayland Layer Shell first
    LayerSurfaceConfig layer_cfg;
    layer_cfg.namespace_id   = "enki-popup";
    layer_cfg.layer          = ShellLayer::Overlay;
    layer_cfg.anchor         = ShellAnchor::TopLeft;
    layer_cfg.width          = options.width;
    layer_cfg.height         = options.height;
    layer_cfg.exclusive_zone = -1; // Never reserve space
    layer_cfg.margin.left    = popup_x;
    layer_cfg.margin.top     = popup_y;
    layer_cfg.keyboard_mode  = options.keyboard_mode;
    layer_cfg.target_output  = options.target_output;
    layer_cfg.transparent    = true;

    SurfaceHost* host = app->addLayerSurface(layer_cfg, content);

    if (!host) {
        // Fallback to X11 override-redirect / popup window
        WindowConfig win_cfg;
        win_cfg.title             = "enki-popup";
        win_cfg.x                 = popup_x;
        win_cfg.y                 = popup_y;
        win_cfg.width             = options.width;
        win_cfg.height            = options.height;
        win_cfg.borderless        = true;
        win_cfg.always_on_top     = true;
        win_cfg.override_redirect = true;
        win_cfg.resizable         = false;

        host = app->addWindow(win_cfg, content);
    }

    if (!host) {
        std::cerr << "[ENKI NativePopup] Failed to create native popup surface\n";
        return nullptr;
    }

    popup->host_ = host;
    return popup;
}

void NativePopup::close() {
    if (!host_) return;

    ShellApp* app = ShellApp::instance();
    if (app) {
        app->removeSurface(host_);
    }
    host_ = nullptr;

    if (options_.on_close) {
        options_.on_close();
    }
}

} // namespace enki
