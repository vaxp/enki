/// @file wayland_window.cpp
/// @brief Standard Desktop Window implementation using xdg_shell protocol and EGL.

#include "enki/platform/wayland/wayland_window.hpp"
#include <iostream>

namespace enki::wayland {

// ── XDG Protocol Listeners ───────────────────────────────────────

static void xdg_surface_configure_handler(void* data, xdg_surface* xdg_surf, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surf, serial);
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleSurfaceConfigure(serial);
}

static const struct xdg_surface_listener xdg_surface_listener_impl = {
    .configure = xdg_surface_configure_handler,
};

static void xdg_toplevel_configure_handler(void* data, xdg_toplevel* /*toplevel*/,
                                           int32_t width, int32_t height, wl_array* states) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleToplevelConfigure(width, height, states);
}

static void xdg_toplevel_close_handler(void* data, xdg_toplevel* /*toplevel*/) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->handleClose();
}

static void xdg_toplevel_configure_bounds_handler(void* /*data*/, xdg_toplevel* /*toplevel*/,
                                                  int32_t /*width*/, int32_t /*height*/) {
    // Optional bounds recommendation from compositor
}

static void xdg_toplevel_wm_capabilities_handler(void* /*data*/, xdg_toplevel* /*toplevel*/,
                                                 wl_array* /*capabilities*/) {
    // Optional WM capabilities advertised by compositor
}

static const struct xdg_toplevel_listener xdg_toplevel_listener_impl = {
    .configure        = xdg_toplevel_configure_handler,
    .close            = xdg_toplevel_close_handler,
    .configure_bounds = xdg_toplevel_configure_bounds_handler,
    .wm_capabilities  = xdg_toplevel_wm_capabilities_handler,
};

// ════════════════════════════════════════════════════════════════
// WaylandWindow Implementation
// ════════════════════════════════════════════════════════════════

WaylandWindow::WaylandWindow(WaylandPlatformBackend& backend)
    : backend_(backend) {}

WaylandWindow::~WaylandWindow() {
    destroy();
}

bool WaylandWindow::init(const WindowConfig& config) {
    config_         = config;
    current_width_  = config_.width > 0 ? config_.width : 1280;
    current_height_ = config_.height > 0 ? config_.height : 800;

    auto* compositor = backend_.getCompositor();
    auto* xdg_wm_base = backend_.getXdgWmBase();
    egl_display_ = backend_.getEGLDisplay();
    auto egl_cfg = backend_.getEGLConfig();

    if (!compositor) {
        std::cerr << "[ENKI WaylandWindow] wl_compositor unavailable\n";
        return false;
    }
    if (!xdg_wm_base) {
        std::cerr << "[ENKI WaylandWindow] xdg_wm_base unavailable on compositor\n";
        return false;
    }
    if (egl_display_ == EGL_NO_DISPLAY || !egl_cfg) {
        std::cerr << "[ENKI WaylandWindow] EGL display/config invalid\n";
        return false;
    }

    // 1. Create base Wayland surface
    wl_surface_ = wl_compositor_create_surface(compositor);
    if (!wl_surface_) {
        std::cerr << "[ENKI WaylandWindow] Failed to create wl_surface\n";
        return false;
    }

    // 2. Create XDG Surface & Toplevel
    xdg_surface_ = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface_);
    if (!xdg_surface_) {
        std::cerr << "[ENKI WaylandWindow] Failed to get xdg_surface\n";
        return false;
    }
    xdg_surface_add_listener(xdg_surface_, &xdg_surface_listener_impl, this);

    xdg_toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
    if (!xdg_toplevel_) {
        std::cerr << "[ENKI WaylandWindow] Failed to get xdg_toplevel\n";
        return false;
    }
    xdg_toplevel_add_listener(xdg_toplevel_, &xdg_toplevel_listener_impl, this);

    // Apply configuration metadata
    if (!config_.title.empty()) {
        xdg_toplevel_set_title(xdg_toplevel_, config_.title.c_str());
    }
    xdg_toplevel_set_app_id(xdg_toplevel_, "enki.app");

    if (config_.min_width > 0 && config_.min_height > 0) {
        xdg_toplevel_set_min_size(xdg_toplevel_, config_.min_width, config_.min_height);
    }

    // Commit initial state and wait for compositor's first configure sequence
    wl_surface_commit(wl_surface_);
    wl_display_roundtrip(backend_.getDisplay());

    // 3. Create Wayland EGL Window & Surface
    egl_window_ = wl_egl_window_create(wl_surface_, current_width_, current_height_);
    if (!egl_window_) {
        std::cerr << "[ENKI WaylandWindow] Failed to create wl_egl_window\n";
        return false;
    }

    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_cfg, (EGLNativeWindowType)egl_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[ENKI WaylandWindow] Failed to create EGL surface\n";
        return false;
    }

    // 4. Use shared EGL Context from backend
    egl_context_ = backend_.getEGLContext();
    if (egl_context_ == EGL_NO_CONTEXT) {
        const EGLint ctx_attribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE
        };
        egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, ctx_attribs);
        if (egl_context_ == EGL_NO_CONTEXT) {
            const EGLint gles_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, gles_attribs);
        }
        if (egl_context_ == EGL_NO_CONTEXT) {
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, nullptr);
        }
    }

    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[ENKI WaylandWindow] Failed to create EGL context\n";
        return false;
    }

    // Make current & configure VSync
    makeCurrent();
    eglSwapInterval(egl_display_, config_.vsync ? 1 : 0);

    return true;
}

void WaylandWindow::destroy() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        if (egl_surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }
    }

    if (egl_window_) {
        wl_egl_window_destroy(egl_window_);
        egl_window_ = nullptr;
    }
    if (xdg_toplevel_) {
        xdg_toplevel_destroy(xdg_toplevel_);
        xdg_toplevel_ = nullptr;
    }
    if (xdg_surface_) {
        xdg_surface_destroy(xdg_surface_);
        xdg_surface_ = nullptr;
    }
    if (wl_surface_) {
        wl_surface_destroy(wl_surface_);
        wl_surface_ = nullptr;
    }
}

void WaylandWindow::handleSurfaceConfigure(uint32_t /*serial*/) {
    configured_ = true;
}

void WaylandWindow::handleToplevelConfigure(int32_t width, int32_t height, wl_array* /*states*/) {
    if (width > 0 && height > 0) {
        if (current_width_ != width || current_height_ != height) {
            current_width_  = width;
            current_height_ = height;
            if (egl_window_) {
                wl_egl_window_resize(egl_window_, current_width_, current_height_, 0, 0);
            }
            on_resize_.emit(current_width_, current_height_);
        }
    }
}

void WaylandWindow::handleClose() {
    on_close_.emit();
}

void WaylandWindow::setTitle(std::string_view title) {
    config_.title = std::string(title);
    if (xdg_toplevel_) {
        xdg_toplevel_set_title(xdg_toplevel_, config_.title.c_str());
    }
}

void WaylandWindow::setSize(int width, int height) {
    current_width_  = width;
    current_height_ = height;
    if (egl_window_) {
        wl_egl_window_resize(egl_window_, current_width_, current_height_, 0, 0);
    }
    if (wl_surface_) {
        wl_surface_commit(wl_surface_);
    }
}

void WaylandWindow::setPosition(int /*x*/, int /*y*/) {
    // Window position on Wayland is managed by the compositor
}

Size WaylandWindow::getSize() const {
    return Size{static_cast<float>(current_width_), static_cast<float>(current_height_)};
}

Size WaylandWindow::getDrawableSize() const {
    return Size{static_cast<float>(current_width_), static_cast<float>(current_height_)};
}

float WaylandWindow::getDpiScale() const {
    return scale_factor_;
}

void WaylandWindow::makeCurrent() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE && egl_context_ != EGL_NO_CONTEXT) {
        eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    }
}

void WaylandWindow::swapBuffers() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE) {
        eglSwapBuffers(egl_display_, egl_surface_);
    }
}

} // namespace enki::wayland
