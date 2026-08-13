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

    // 1. Calculate screen coordinates directly from provided absolute position
    int32_t popup_x = static_cast<int32_t>(options.position.x);
    int32_t popup_y = static_cast<int32_t>(options.position.y);

    if (popup_x < 0) popup_x = 0;
    if (popup_y < 0) popup_y = 0;

    // 2. Build the popup content widget
    WidgetPtr content = builder(context, popup);

    // 3. Create independent native surface
    SurfaceHost* parent_host = options.parent_host;
    if (!parent_host && context.element()) {
        parent_host = app->findSurfaceByOwner(context.element()->owner());
    }

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
    win_cfg.transparent       = true;

    SurfaceHost* host = app->addPopup(parent_host, win_cfg, content);

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
