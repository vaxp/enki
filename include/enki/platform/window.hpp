#pragma once
/// @file window.hpp
/// @brief Native OS Window management with direct EGL + OpenGL + Skia context.
/// Zero SDL dependency — pure native platform window.

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/core/signal.hpp"
#include "enki/platform/platform.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace enki {

/// Window display and role mode.
enum class WindowMode {
    Normal,      ///< Standard application window (XDG Toplevel on Wayland, Normal window on X11).
    LayerShell,  ///< Desktop overlay surface (zwlr_layer_shell_v1).
    Popup,       ///< Child popup window (xdg_popup on Wayland, transient override-redirect on X11).
};

/// Window creation configuration.
struct WindowConfig {
    std::string title        = "ENKI App";
    int         width        = 1280;
    int         height       = 800;
    int         x            = -1; // -1 for centered
    int         y            = -1;
    bool        resizable    = true;
    bool        fullscreen   = false;
    bool        borderless   = false;
    bool        transparent  = false; // 32-bit ARGB with compositing
    bool        always_on_top = false;
    bool        override_redirect = false; ///< Bypass window manager (for popups, tooltips, panels)
    int         min_width    = 2;
    int         min_height   = 2;
    bool        vsync        = true;
    WindowMode  mode         = WindowMode::Normal; ///< Standard window or layer surface overlay.
    class Window* parent_window = nullptr; ///< Parent window if mode == Popup
    class LayerSurface* parent_layer = nullptr; ///< Parent layer surface if mode == Popup
};

/// Represents a native window backed by EGL and OpenGL for Skia GPU rendering.
class Window {
public:
    /// Create a new native window.
    static Result<std::unique_ptr<Window>> create(Platform& platform, WindowConfig config);

    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// Set the window title.
    void setTitle(std::string_view title);

    /// Set window size.
    void setSize(int width, int height);

    /// Set window position on screen.
    void setPosition(int x, int y);

    /// Set whether window is borderless / frameless.
    void setBorderless(bool borderless);

    /// Set whether window stays on top of other windows.
    void setAlwaysOnTop(bool on_top);

    /// Get current window size in logical/pixel coordinates.
    [[nodiscard]] Size getSize() const;

    /// Get drawable framebuffer size in pixels.
    [[nodiscard]] Size getDrawableSize() const;

    /// Get the DPI scale factor (default 1.0f on standard displays).
    [[nodiscard]] float getDpiScale() const;

    /// Make the OpenGL context of this window current on the active thread.
    void makeCurrent();

    /// Swap front/back buffers to present frame on screen.
    void swapBuffers();

    /// Get native window handle (e.g. X11 Window uint64_t or wl_surface*).
    void* getNativeHandle() const;

    /// Get the EGL Surface handle.
    void* getEGLSurface() const;

    /// Get the EGL Context handle.
    void* getEGLContext() const;

    /// Get the internal backend window object (e.g. WaylandWindow*).
    void* getBackendWindow() const;

    /// Get the internal backend layer object (e.g. WaylandLayerSurface*).
    void* getBackendLayer() const;

    // --- Signals ---
    Signal<int, int>& onResize() { return on_resize_; }
    Signal<>&         onClose()  { return on_close_; }
    Signal<bool>&     onFocus()  { return on_focus_; }

    struct Impl;
    Impl* impl() { return impl_.get(); }

private:
    Window();
    std::unique_ptr<Impl> impl_;
    Signal<int, int> on_resize_;
    Signal<>         on_close_;
    Signal<bool>     on_focus_;
};

}  // namespace enki
