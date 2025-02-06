#pragma once
/// @file x11_platform.hpp
/// @brief X11 + EGL native platform backend.
/// Internal backend used by Platform when WAYLAND_DISPLAY is not available.

#include "enki/platform/platform.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <unordered_set>
#include <string>

namespace enki {
class Window;
}

namespace enki::x11 {

class X11PlatformBackend {
public:
    explicit X11PlatformBackend(Platform* owner);
    ~X11PlatformBackend();

    bool init();
    void shutdown();
    bool pollEvents();

    // ── Display / EGL accessors ──────────────────────────────────
    [[nodiscard]] ::Display*  getDisplay()    const { return display_; }
    [[nodiscard]] EGLDisplay  getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig   getEGLConfig()  const { return egl_config_; }
    [[nodiscard]] int         getDefaultScreen() const { return default_screen_; }

    // ── Atoms ────────────────────────────────────────────────────
    [[nodiscard]] Atom getAtomWmProtocols()    const { return atom_wm_protocols_; }
    [[nodiscard]] Atom getAtomWmDeleteWindow() const { return atom_wm_delete_window_; }
    [[nodiscard]] Atom getAtomUtf8String()     const { return atom_utf8_string_; }
    [[nodiscard]] Atom getAtomClipboard()      const { return atom_clipboard_; }

    void registerWindow(Window* w);
    void unregisterWindow(Window* w);
    [[nodiscard]] const std::unordered_set<Window*>& windows() const { return windows_; }

    void setClipboardText(const std::string& text);

private:
    Platform*   owner_          = nullptr;
    ::Display*  display_        = nullptr;
    int         default_screen_ = 0;
    EGLDisplay egl_display_    = EGL_NO_DISPLAY;
    EGLConfig  egl_config_     = nullptr;
    int        egl_major_      = 0;
    int        egl_minor_      = 0;

    Atom atom_wm_protocols_     = None;
    Atom atom_wm_delete_window_ = None;
    Atom atom_utf8_string_      = None;
    Atom atom_clipboard_        = None;

    std::unordered_set<Window*> windows_;
    std::string clipboard_buffer_;
};

} // namespace enki::x11
