/// @file window_native.cpp
/// @brief Window abstraction layer — delegates to X11 or Wayland backend.
/// The Window class is backend-agnostic; it holds an X11Window, WaylandWindow, or WaylandLayerSurface
/// depending on which Platform backend and WindowMode is active.

#include "enki/platform/window.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/x11/x11_window.hpp"

#if defined(ENKI_HAS_WAYLAND)
#include "enki/platform/wayland/wayland_platform.hpp"
#include "enki/platform/wayland/wayland_surface.hpp"
#include "enki/platform/wayland/wayland_window.hpp"
#endif

#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Window::Impl  — owned backend handle (X11 or Wayland)
// ════════════════════════════════════════════════════════════════
struct Window::Impl {
    Platform* platform = nullptr;
    Window*   window   = nullptr;

    // Active backend
    std::unique_ptr<x11::X11Window>               x11;
#if defined(ENKI_HAS_WAYLAND)
    std::unique_ptr<wayland::WaylandWindow>       wayland_window;
    std::unique_ptr<wayland::WaylandLayerSurface> wayland_layer;
#endif

    int current_width  = 0;
    int current_height = 0;

    // ── Factory ─────────────────────────────────────────────────
    bool init(Window* win, Platform& plat, const WindowConfig& cfg) {
        platform = &plat;
        window   = win;

#if defined(ENKI_HAS_WAYLAND)
        if (plat.isWayland()) {
            auto* wb = static_cast<wayland::WaylandPlatformBackend*>(plat.getWaylandBackend());
            if (!wb) {
                std::cerr << "[ENKI Window] Wayland backend unavailable\n";
                return false;
            }

            if (cfg.mode == WindowMode::LayerShell) {
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

                wayland_layer = std::make_unique<wayland::WaylandLayerSurface>(*wb, lsc);
                if (!wayland_layer->init()) {
                    std::cerr << "[ENKI Window] Failed to create Wayland layer surface\n";
                    wayland_layer.reset();
                    return false;
                }
                wayland_layer->onClose().connect([this]() {
                    if (window) window->onClose().emit();
                });
                wayland_layer->onResize().connect([this](int w, int h) {
                    if (window) window->onResize().emit(w, h);
                });
            } else {
                wayland_window = std::make_unique<wayland::WaylandWindow>(*wb);
                if (!wayland_window->init(cfg)) {
                    std::cerr << "[ENKI Window] Failed to create Wayland XDG window\n";
                    wayland_window.reset();
                    return false;
                }
                wayland_window->onClose().connect([this]() {
                    if (window) window->onClose().emit();
                });
                wayland_window->onResize().connect([this](int w, int h) {
                    if (window) window->onResize().emit(w, h);
                });
                wayland_window->onFocus().connect([this](bool f) {
                    if (window) window->onFocus().emit(f);
                });
                wayland_window->onMaximized().connect([this](bool m) {
                    if (window) window->onMaximized().emit(m);
                });
                wayland_window->onStateChanged().connect([this](WindowState s) {
                    if (window) window->onStateChanged().emit(s);
                });
            }
            current_width  = cfg.width;
            current_height = cfg.height;
            return true;
        }
#endif

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
        x11->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        x11->onMaximized().connect([this](bool m) {
            if (window) window->onMaximized().emit(m);
        });
        x11->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        x11->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
    }

    void destroy() {
        if (x11) { x11.reset(); }
#if defined(ENKI_HAS_WAYLAND)
        if (wayland_window) { wayland_window.reset(); }
        if (wayland_layer)  { wayland_layer.reset(); }
#endif
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
    if (!window->impl_->init(window.get(), platform, config)) {
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
    if (impl_->x11) impl_->x11->setTitle(title);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setTitle(title);
#endif
}

void Window::setSize(int w, int h) {
    if (impl_->x11) impl_->x11->setSize(w, h);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setSize(w, h);
    if (impl_->wayland_layer)  impl_->wayland_layer->setSize(w, h);
#endif
    impl_->current_width  = w;
    impl_->current_height = h;
}

void Window::setPosition(int x, int y) {
    if (impl_->x11) impl_->x11->setPosition(x, y);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setPosition(x, y);
#endif
}

void Window::setBorderless(bool b) {
    if (impl_->x11) impl_->x11->setBorderless(b);
}

void Window::setAlwaysOnTop(bool t) {
    if (impl_->x11) impl_->x11->setAlwaysOnTop(t);
}

// ── Accessors ───────────────────────────────────────────────────
Size Window::getSize() const {
    if (impl_->x11) return impl_->x11->getSize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getSize();
#endif
    return {(float)impl_->current_width, (float)impl_->current_height};
}

Size Window::getDrawableSize() const {
    if (impl_->x11) return impl_->x11->getDrawableSize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDrawableSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDrawableSize();
#endif
    return getSize();
}

float Window::getDpiScale() const {
    if (impl_->x11) return impl_->x11->getDpiScale();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDpiScale();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDpiScale();
#endif
    return 1.0f;
}

void Window::makeCurrent() {
    if (impl_->x11) impl_->x11->makeCurrent();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->makeCurrent();
    if (impl_->wayland_layer)  impl_->wayland_layer->makeCurrent();
#endif
}

void Window::swapBuffers() {
    if (impl_->x11) impl_->x11->swapBuffers();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->swapBuffers();
    if (impl_->wayland_layer)  impl_->wayland_layer->swapBuffers();
#endif
}

void* Window::getNativeHandle() const {
    if (impl_->x11) return impl_->x11->getNativeHandle();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getNativeHandle();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getWlSurface();
#endif
    return nullptr;
}

void* Window::getEGLSurface() const {
    if (impl_->x11) return impl_->x11->getEGLSurface();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLSurface();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLSurface();
#endif
    return nullptr;
}

void* Window::getEGLContext() const {
    if (impl_->x11) return impl_->x11->getEGLContext();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLContext();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLContext();
#endif
    return nullptr;
}

void* Window::getBackendWindow() const {
    if (impl_->x11) return impl_->x11.get();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window.get();
#endif
    return nullptr;
}

void* Window::getBackendLayer() const {
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_layer) return impl_->wayland_layer.get();
#endif
    return nullptr;
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────

void Window::beginMove(float local_x, float local_y, int button) {
    if (impl_->x11) impl_->x11->beginMove(local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginMove(local_x, local_y, button);
#endif
}

void Window::beginResize(WindowEdge edge, float local_x, float local_y, int button) {
    if (impl_->x11) impl_->x11->beginResize(edge, local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginResize(edge, local_x, local_y, button);
#endif
}

void Window::setMaximized(bool max) {
    if (impl_->x11) impl_->x11->setMaximized(max);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMaximized(max);
#endif
}

void Window::setMinimized(bool min) {
    if (impl_->x11) impl_->x11->setMinimized(min);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMinimized(min);
#endif
}

void Window::setFullscreen(bool full) {
    if (impl_->x11) impl_->x11->setFullscreen(full);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setFullscreen(full);
#endif
}

void Window::toggleMaximize() {
    if (impl_->x11) impl_->x11->toggleMaximize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->toggleMaximize();
#endif
}

void Window::showWindowMenu(float local_x, float local_y, int button) {
    if (impl_->x11) impl_->x11->showWindowMenu(local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->showWindowMenu(local_x, local_y, button);
#endif
}

void Window::setDecorated(bool decorated) {
    if (impl_->x11) impl_->x11->setDecorated(decorated);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setDecorated(decorated);
#endif
}

void Window::setWindowGeometry(int x, int y, int width, int height) {
    if (impl_->x11) impl_->x11->setWindowGeometry(x, y, width, height);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setWindowGeometry(x, y, width, height);
#endif
}

bool Window::isMaximized() const {
    if (impl_->x11) return impl_->x11->isMaximized();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMaximized();
#endif
    return false;
}

bool Window::isMinimized() const {
    if (impl_->x11) return impl_->x11->isMinimized();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMinimized();
#endif
    return false;
}

bool Window::isFullscreen() const {
    if (impl_->x11) return impl_->x11->isFullscreen();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isFullscreen();
#endif
    return false;
}

bool Window::isActivated() const {
    if (impl_->x11) return impl_->x11->isActivated();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isActivated();
#endif
    return true;
}

WindowState Window::getWindowState() const {
    if (impl_->x11) return impl_->x11->getWindowState();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getWindowState();
#endif
    return WindowState::Normal;
}

}  // namespace enki
