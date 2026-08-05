#pragma once
/// @file x11_window.hpp
/// @brief X11 + EGL native window backend.
/// Implements the Window abstraction for X11 sessions.

#include "enki/platform/window.hpp"
#include "enki/platform/x11/x11_platform.hpp"

#include <X11/Xlib.h>
#include <EGL/egl.h>

namespace enki::x11 {

class X11Window {
public:
    explicit X11Window(X11PlatformBackend& backend);
    ~X11Window();

    bool init(const WindowConfig& config);
    void destroy();

    void setTitle(std::string_view title);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setBorderless(bool borderless);
    void setAlwaysOnTop(bool on_top);

    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] Size  getDrawableSize() const;
    [[nodiscard]] float getDpiScale() const;

    void makeCurrent();
    void swapBuffers();

    [[nodiscard]] void* getNativeHandle() const { return (void*)(uintptr_t)x11_window_; }
    [[nodiscard]] void* getEGLSurface()   const { return (void*)egl_surface_; }
    [[nodiscard]] void* getEGLContext()   const { return (void*)egl_context_; }
    [[nodiscard]] ::Window getX11Window() const { return x11_window_; }

private:
    X11PlatformBackend& backend_;
    WindowConfig config_;

    Display*   display_     = nullptr;
    ::Window   x11_window_  = 0;
    Colormap   colormap_    = 0;
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLSurface egl_surface_ = EGL_NO_SURFACE;
    EGLContext egl_context_ = EGL_NO_CONTEXT;

    int current_width_  = 0;
    int current_height_ = 0;
};

} // namespace enki::x11
