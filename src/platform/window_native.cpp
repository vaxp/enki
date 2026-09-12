/// @file window_native.cpp
/// @brief Window abstraction layer — delegates to X11 or Wayland backend.
/// The Window class is backend-agnostic; it holds an X11Window, WaylandWindow, or WaylandLayerSurface
/// depending on which Platform backend and WindowMode is active.

#include "enki/platform/window.hpp"
#include "enki/platform/platform.hpp"

#if defined(__ANDROID__)
#include "enki/platform/android/android_platform.hpp"
#include "enki/platform/android/android_surface.hpp"
#elif defined(_WIN32)
#include "enki/platform/windows/win32_window.hpp"
#include "enki/platform/windows/win32_platform.hpp"
#else
#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/x11/x11_window.hpp"

#if defined(ENKI_HAS_WAYLAND)
#include "enki/platform/wayland/wayland_platform.hpp"
#include "enki/platform/wayland/wayland_surface.hpp"
#include "enki/platform/wayland/wayland_window.hpp"
#endif
#endif

#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Window::Impl  — owned backend handle (Android / Win32 / X11 / Wayland)
// ════════════════════════════════════════════════════════════════
struct Window::Impl {
    Platform* platform = nullptr;
    Window*   window   = nullptr;

    // Active backend
#if defined(__ANDROID__)
    std::unique_ptr<android::AndroidSurface> android_surface;
#elif defined(_WIN32)
    std::unique_ptr<win32::Win32Window>           win32_window;
#else
    std::unique_ptr<x11::X11Window>               x11;
#if defined(ENKI_HAS_WAYLAND)
    std::unique_ptr<wayland::WaylandWindow>       wayland_window;
    std::unique_ptr<wayland::WaylandLayerSurface> wayland_layer;
#endif
#endif

    int current_width  = 0;
    int current_height = 0;

    // ── Factory ─────────────────────────────────────────────────
    bool init(Window* win, Platform& plat, const WindowConfig& cfg) {
        platform = &plat;
        window   = win;

#if defined(__ANDROID__)
        auto* ab = static_cast<android::AndroidPlatformBackend*>(plat.getAndroidBackend());
        if (!ab) {
            std::cerr << "[ENKI Window] Android backend unavailable\n";
            return false;
        }

        android_surface = std::make_unique<android::AndroidSurface>(*ab);
        if (!android_surface->init()) {
            std::cerr << "[ENKI Window] Failed to create AndroidSurface\n";
            android_surface.reset();
            return false;
        }
        android_surface->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        android_surface->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        android_surface->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        android_surface->onClose().connect([this]() {
            if (window) window->onClose().emit();
        });
        android_surface->onSurfaceRecreated().connect([this]() {
            if (window) window->onSurfaceRecreated().emit();
        });
        android_surface->onSurfaceDestroyed().connect([this]() {
            if (window) window->onSurfaceDestroyed().emit();
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
#elif defined(_WIN32)
        auto* wb = static_cast<win32::Win32PlatformBackend*>(plat.getWin32Backend());
        if (!wb) {
            std::cerr << "[ENKI Window] Win32 backend unavailable\n";
            return false;
        }

        win32_window = std::make_unique<win32::Win32Window>(*wb);
        if (!win32_window->init(cfg)) {
            win32_window.reset();
            return false;
        }
        win32_window->onClose().connect([this]() {
            if (window) window->onClose().emit();
        });
        win32_window->onFocus().connect([this](bool f) {
            if (window) window->onFocus().emit(f);
        });
        win32_window->onMaximized().connect([this](bool m) {
            if (window) window->onMaximized().emit(m);
        });
        win32_window->onStateChanged().connect([this](WindowState s) {
            if (window) window->onStateChanged().emit(s);
        });
        win32_window->onResize().connect([this](int w, int h) {
            current_width  = w;
            current_height = h;
            if (window) window->onResize().emit(w, h);
        });
        current_width  = cfg.width;
        current_height = cfg.height;
        return true;
#else
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
#endif
    }

    void destroy() {
#if defined(__ANDROID__)
        if (android_surface) { android_surface.reset(); }
#elif defined(_WIN32)
        if (win32_window) { win32_window.reset(); }
#else
        if (x11) { x11.reset(); }
#if defined(ENKI_HAS_WAYLAND)
        if (wayland_window) { wayland_window.reset(); }
        if (wayland_layer)  { wayland_layer.reset(); }
#endif
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
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setTitle(title);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setTitle(title);
#else
    if (impl_->x11) impl_->x11->setTitle(title);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setTitle(title);
#endif
#endif
}

void Window::setSize(int w, int h) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setSize(w, h);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setSize(w, h);
#else
    if (impl_->x11) impl_->x11->setSize(w, h);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setSize(w, h);
    if (impl_->wayland_layer)  impl_->wayland_layer->setSize(w, h);
#endif
#endif
    impl_->current_width  = w;
    impl_->current_height = h;
}

void Window::setPosition(int x, int y) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setPosition(x, y);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setPosition(x, y);
#else
    if (impl_->x11) impl_->x11->setPosition(x, y);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setPosition(x, y);
#endif
#endif
}

void Window::setBorderless(bool b) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setBorderless(b);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setBorderless(b);
#else
    if (impl_->x11) impl_->x11->setBorderless(b);
#endif
}

void Window::setAlwaysOnTop(bool t) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setAlwaysOnTop(t);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setAlwaysOnTop(t);
#else
    if (impl_->x11) impl_->x11->setAlwaysOnTop(t);
#endif
}

void Window::setBlurBehind(bool enable) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setBlurBehind(enable);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setBlurBehind(enable);
#else
    if (impl_->x11) impl_->x11->setBlurBehind(enable);
#endif
}

// ── Accessors ───────────────────────────────────────────────────
Size Window::getSize() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getSize();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getSize();
#else
    if (impl_->x11) return impl_->x11->getSize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getSize();
#endif
#endif
    return {(float)impl_->current_width, (float)impl_->current_height};
}

Size Window::getDrawableSize() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getDrawableSize();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getDrawableSize();
#else
    if (impl_->x11) return impl_->x11->getDrawableSize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDrawableSize();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDrawableSize();
#endif
#endif
    return getSize();
}

float Window::getDpiScale() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getDpiScale();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getDpiScale();
#else
    if (impl_->x11) return impl_->x11->getDpiScale();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getDpiScale();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getDpiScale();
#endif
#endif
    return 1.0f;
}

void Window::makeCurrent() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->makeCurrent();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->makeCurrent();
#else
    if (impl_->x11) impl_->x11->makeCurrent();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->makeCurrent();
    if (impl_->wayland_layer)  impl_->wayland_layer->makeCurrent();
#endif
#endif
}

void Window::swapBuffers() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->swapBuffers();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->swapBuffers();
#else
    if (impl_->x11) impl_->x11->swapBuffers();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->swapBuffers();
    if (impl_->wayland_layer)  impl_->wayland_layer->swapBuffers();
#endif
#endif
}

void* Window::getNativeHandle() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getNativeHandle();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getNativeHandle();
#else
    if (impl_->x11) return impl_->x11->getNativeHandle();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getNativeHandle();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getWlSurface();
#endif
#endif
    return nullptr;
}

void* Window::getEGLSurface() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getEGLSurface();
    return nullptr;
#elif defined(_WIN32)
    return nullptr;
#else
    if (impl_->x11) return impl_->x11->getEGLSurface();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLSurface();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLSurface();
#endif
    return nullptr;
#endif
}

void* Window::getEGLContext() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getEGLContext();
    return nullptr;
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getEGLContext();
    return nullptr;
#else
    if (impl_->x11) return impl_->x11->getEGLContext();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getEGLContext();
    if (impl_->wayland_layer)  return impl_->wayland_layer->getEGLContext();
#endif
    return nullptr;
#endif
}

void* Window::getBackendWindow() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface.get();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window.get();
#else
    if (impl_->x11) return impl_->x11.get();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window.get();
#endif
#endif
    return nullptr;
}

void* Window::getBackendLayer() const {
#if defined(ENKI_HAS_WAYLAND) && !defined(_WIN32) && !defined(__ANDROID__)
    if (impl_->wayland_layer) return impl_->wayland_layer.get();
#endif
    return nullptr;
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────

void Window::beginMove(float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->beginMove(local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->beginMove(local_x, local_y, button);
#else
    if (impl_->x11) impl_->x11->beginMove(local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginMove(local_x, local_y, button);
#endif
#endif
}

void Window::beginResize(WindowEdge edge, float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->beginResize(edge, local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->beginResize(edge, local_x, local_y, button);
#else
    if (impl_->x11) impl_->x11->beginResize(edge, local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->beginResize(edge, local_x, local_y, button);
#endif
#endif
}

void Window::setMaximized(bool max) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setMaximized(max);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setMaximized(max);
#else
    if (impl_->x11) impl_->x11->setMaximized(max);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMaximized(max);
#endif
#endif
}

void Window::setMinimized(bool min) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setMinimized(min);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setMinimized(min);
#else
    if (impl_->x11) impl_->x11->setMinimized(min);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setMinimized(min);
#endif
#endif
}

void Window::setFullscreen(bool full) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setFullscreen(full);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setFullscreen(full);
#else
    if (impl_->x11) impl_->x11->setFullscreen(full);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setFullscreen(full);
#endif
#endif
}

void Window::toggleMaximize() {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->toggleMaximize();
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->toggleMaximize();
#else
    if (impl_->x11) impl_->x11->toggleMaximize();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->toggleMaximize();
#endif
#endif
}

void Window::showWindowMenu(float local_x, float local_y, int button) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->showWindowMenu(local_x, local_y, button);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->showWindowMenu(local_x, local_y, button);
#else
    if (impl_->x11) impl_->x11->showWindowMenu(local_x, local_y, button);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->showWindowMenu(local_x, local_y, button);
#endif
#endif
}

void Window::setDecorated(bool decorated) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setDecorated(decorated);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setDecorated(decorated);
#else
    if (impl_->x11) impl_->x11->setDecorated(decorated);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setDecorated(decorated);
#endif
#endif
}

void Window::setWindowGeometry(int x, int y, int width, int height) {
#if defined(__ANDROID__)
    if (impl_->android_surface) impl_->android_surface->setWindowGeometry(x, y, width, height);
#elif defined(_WIN32)
    if (impl_->win32_window) impl_->win32_window->setWindowGeometry(x, y, width, height);
#else
    if (impl_->x11) impl_->x11->setWindowGeometry(x, y, width, height);
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) impl_->wayland_window->setWindowGeometry(x, y, width, height);
#endif
#endif
}

bool Window::isMaximized() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isMaximized();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isMaximized();
#else
    if (impl_->x11) return impl_->x11->isMaximized();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMaximized();
#endif
#endif
    return false;
}

bool Window::isMinimized() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isMinimized();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isMinimized();
#else
    if (impl_->x11) return impl_->x11->isMinimized();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isMinimized();
#endif
#endif
    return false;
}

bool Window::isFullscreen() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isFullscreen();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isFullscreen();
#else
    if (impl_->x11) return impl_->x11->isFullscreen();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isFullscreen();
#endif
#endif
    return false;
}

bool Window::isActivated() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->isActivated();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->isActivated();
#else
    if (impl_->x11) return impl_->x11->isActivated();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->isActivated();
#endif
#endif
    return true;
}

WindowState Window::getWindowState() const {
#if defined(__ANDROID__)
    if (impl_->android_surface) return impl_->android_surface->getWindowState();
#elif defined(_WIN32)
    if (impl_->win32_window) return impl_->win32_window->getWindowState();
#else
    if (impl_->x11) return impl_->x11->getWindowState();
#if defined(ENKI_HAS_WAYLAND)
    if (impl_->wayland_window) return impl_->wayland_window->getWindowState();
#endif
#endif
    return WindowState::Normal;
}

}  // namespace enki

