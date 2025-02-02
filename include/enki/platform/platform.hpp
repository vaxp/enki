#pragma once
/// @file platform.hpp
/// @brief Native Linux Platform subsystem (X11 / Wayland / EGL / Linux Native).
/// Zero SDL dependency — pure native platform layer.

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/core/signal.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace enki {

class Window;

/// Native Platform subsystem — manages display server connection and native event loop.
class Platform {
public:
    /// Create and initialize the native platform subsystem.
    static Result<std::unique_ptr<Platform>> create();

    /// Get the active Platform instance (singleton).
    static Platform* instance();

    ~Platform();

    // Non-copyable
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    /// Poll and dispatch all pending native OS events.
    /// @return false if the application should quit.
    bool pollEvents();

    /// Get high-resolution time in seconds since initialization.
    [[nodiscard]] double getTime() const;

    /// Get the clipboard text (UTF-8).
    [[nodiscard]] std::string getClipboardText() const;

    /// Set the clipboard text (UTF-8).
    void setClipboardText(std::string_view text);

    // --- Signals ---
    /// Signal emitted when the platform requests quit.
    Signal<>& onQuit() { return on_quit_; }

    /// Signal emitted on mouse button press. Args: x, y, button (1=left, 2=middle, 3=right).
    Signal<float, float, int>& onMouseDown() { return on_mouse_down_; }

    /// Signal emitted on mouse button release. Args: x, y, button.
    Signal<float, float, int>& onMouseUp() { return on_mouse_up_; }

    /// Signal emitted on mouse move. Args: x, y.
    Signal<float, float>& onMouseMove() { return on_mouse_move_; }

    /// Signal emitted on mouse scroll. Args: dx, dy.
    Signal<float, float>& onScroll() { return on_scroll_; }

    /// Signal emitted on text input (UTF-8 string).
    Signal<std::string_view>& onTextInput() { return on_text_input_; }

    /// Signal emitted on key down. Args: keycode, modifiers.
    Signal<int, int>& onKeyDown() { return on_key_down_; }

    /// Signal emitted on key up. Args: keycode, modifiers.
    Signal<int, int>& onKeyUp() { return on_key_up_; }

    /// Access native display handles
    void* getNativeDisplay() const;
    void* getEGLDisplay() const;
    void* getEGLConfig() const;
    void* getWaylandBackend() const;
    void* getX11Backend() const;
    [[nodiscard]] bool isWayland() const;

    /// Register/unregister active window for event dispatch
    void registerWindow(Window* win);
    void unregisterWindow(Window* win);

    struct Impl;
    Impl* impl() { return impl_.get(); }

private:
    Platform();
    std::unique_ptr<Impl> impl_;

    Signal<>                  on_quit_;
    Signal<float, float, int> on_mouse_down_;
    Signal<float, float, int> on_mouse_up_;
    Signal<float, float>      on_mouse_move_;
    Signal<float, float>      on_scroll_;
    Signal<std::string_view>  on_text_input_;
    Signal<int, int>          on_key_down_;
    Signal<int, int>          on_key_up_;
};

}  // namespace enki
