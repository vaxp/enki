/// @file window_native.cpp
/// @brief Window abstraction layer — delegates to X11 or Wayland backend.
/// The Window class is backend-agnostic; it holds an X11Window or a WaylandLayerSurface
/// depending on which Platform backend is active.

#include "enki/platform/window.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/x11/x11_window.hpp"
#include "enki/platform/wayland/wayland_platform.hpp"
#include "enki/platform/wayland/wayland_surface.hpp"

#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Window::Impl  — owned backend handle (X11 or Wayland)
// ════════════════════════════════════════════════════════════════
struct Window::Impl {
    Platform* platform = nullptr;

    // Active backend (exactly one is non-null)
    std::unique_ptr<x11::X11Window>                     x11;
    std::unique_ptr<wayland::WaylandLayerSurface>        wayland;

    int current_width  = 0;
    int current_height = 0;

    // ── Factory ─────────────────────────────────────────────────
    bool init(Platform& plat, const WindowConfig& cfg) {
        platform = &plat;

        if (plat.isWayland()) {
            // Wayland path — create a standard "application" layer surface (Top layer, no anchor)
            auto* wb = static_cast<wayland::WaylandPlatformBackend*>(plat.getWaylandBackend());
            if (!wb) {
                std::cerr << "[ENKI Window] Wayland backend unavailable\n";
                return false;
            }

            LayerSurfaceConfig lsc;
            lsc.namespace_id   = "enki-window";
            lsc.layer          = ShellLayer::Top;
            lsc.anchor         = ShellAnchor::None;     // Floating window
            lsc.width          = cfg.width;
            lsc.height         = cfg.height;
            lsc.exclusive_zone = 0;
            lsc.keyboard_mode  = KeyboardMode::OnDemand;
            lsc.transparent    = cfg.transparent;
            lsc.vsync          = cfg.vsync;

            wayland = std::make_unique<wayland::WaylandLayerSurface>(*wb, lsc);
            if (!wayland->init()) {
                std::cerr << "[ENKI Window] Failed to create Wayland surface\n";
                wayland.reset();
                return false;
            }
            current_width  = cfg.width;
            current_height = cfg.height;
            return true;
        }

        // X11 path
        auto* xb = static_cast<x11::X11PlatformBackend*>(plat.getX11Backend());
        if (!xb) {
            std::cerr << "[ENKI Window] X11 backend unavailable\n";
            return false;
        }

        x11 = std::make_unique<x11::X11Window>(*xb);
        if (!x11->init(cfg)) {
            x11.reset();
            return false;
        }
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
    }

    void destroy() {
        if (x11)     { x11.reset(); }
        if (wayland) { wayland.reset(); }
    }
};

// ════════════════════════════════════════════════════════════════
// Window — Public API
// ════════════════════════════════════════════════════════════════

Window::Window() : impl_(std::make_unique<Impl>()) {}

Window::~Window() {
    if (impl_) {
        if (impl_->platform) impl_->platform->unregisterWindow(this);
        impl_->destroy();
    }
}

Result<std::unique_ptr<Window>> Window::create(Platform& platform, WindowConfig config) {
    auto window = std::unique_ptr<Window>(new Window());
    if (!window->impl_->init(platform, config)) {
        return Result<std::unique_ptr<Window>>::err(
            ErrorCode::WindowError, "Failed to initialize Window");
    }
    platform.registerWindow(window.get());

    window->onResize().connect([w = window.get()](int nw, int nh) {
        w->impl_->current_width  = nw;
        w->impl_->current_height = nh;
    });

    return Result<std::unique_ptr<Window>>::ok(std::move(window));
}

// ── Mutators ────────────────────────────────────────────────────
void Window::setTitle(std::string_view title) {
    if (impl_->x11)     impl_->x11->setTitle(title);
    // Wayland layer surfaces don't have traditional title bars
}

void Window::setSize(int w, int h) {
    if (impl_->x11)     impl_->x11->setSize(w, h);
    if (impl_->wayland) impl_->wayland->setSize(w, h);
    impl_->current_width  = w;
    impl_->current_height = h;
}

void Window::setPosition(int x, int y) {
    if (impl_->x11) impl_->x11->setPosition(x, y);
    // Layer surfaces are positioned by the compositor
}

void Window::setBorderless(bool b) {
    if (impl_->x11) impl_->x11->setBorderless(b);
}

void Window::setAlwaysOnTop(bool t) {
    if (impl_->x11) impl_->x11->setAlwaysOnTop(t);
}

// ── Accessors ───────────────────────────────────────────────────
Size Window::getSize() const {
    if (impl_->x11)     return impl_->x11->getSize();
    if (impl_->wayland) return impl_->wayland->getSize();
    return {(float)impl_->current_width, (float)impl_->current_height};
}

Size Window::getDrawableSize() const {
    if (impl_->x11)     return impl_->x11->getDrawableSize();
    if (impl_->wayland) return impl_->wayland->getDrawableSize();
    return getSize();
}

float Window::getDpiScale() const {
    if (impl_->x11)     return impl_->x11->getDpiScale();
    if (impl_->wayland) return impl_->wayland->getDpiScale();
    return 1.0f;
}

void Window::makeCurrent() {
    if (impl_->x11)     impl_->x11->makeCurrent();
    if (impl_->wayland) impl_->wayland->makeCurrent();
}

void Window::swapBuffers() {
    if (impl_->x11)     impl_->x11->swapBuffers();
    if (impl_->wayland) impl_->wayland->swapBuffers();
}

void* Window::getNativeHandle() const {
    if (impl_->x11)     return impl_->x11->getNativeHandle();
    if (impl_->wayland) return impl_->wayland->getWlSurface();
    return nullptr;
}

void* Window::getEGLSurface() const {
    if (impl_->x11)     return impl_->x11->getEGLSurface();
    if (impl_->wayland) return impl_->wayland->getEGLSurface();
    return nullptr;
}

void* Window::getEGLContext() const {
    if (impl_->x11)     return impl_->x11->getEGLContext();
    if (impl_->wayland) return impl_->wayland->getEGLContext();
    return nullptr;
}

}  // namespace enki
