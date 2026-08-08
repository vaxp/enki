#pragma once
/// @file platform.hpp
/// @brief Native Linux Platform subsystem (X11 / Wayland / EGL / Linux Native).
/// Zero SDL dependency — pure native platform layer.

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/core/signal.hpp"
#include "enki/platform/input.hpp"
#include "enki/platform/clipboard.hpp"
#include "enki/platform/dnd.hpp"
#include "enki/platform/toplevel.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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

    // ── Clipboard Subsystem ──────────────────────────────────────

    /// Get the clipboard text (UTF-8).
    [[nodiscard]] std::string getClipboardText(ClipboardType type = ClipboardType::Clipboard) const;

    /// Set the clipboard text (UTF-8).
    void setClipboardText(std::string_view text, ClipboardType type = ClipboardType::Clipboard);

    /// Set multi-format clipboard data.
    void setClipboardData(const ClipboardData& data, ClipboardType type = ClipboardType::Clipboard);

    /// Get full multi-format clipboard data.
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type = ClipboardType::Clipboard) const;

    /// Get raw clipboard data for a specific MIME format.
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    /// List all available MIME formats currently on the clipboard.
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type = ClipboardType::Clipboard) const;

    /// Check if the clipboard contains data in the specified MIME format.
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    // ── Drag & Drop Subsystem ────────────────────────────────────

    /// Start a Drag and Drop operation from Enki to external windows.
    bool startDrag(const DragData& data, DragAction actions = DragAction::Copy);

    /// Set the platform mouse cursor shape.
    void setCursor(SystemCursor cursor);

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

    /// Signal emitted when clipboard selection changes.
    Signal<ClipboardType>& onClipboardChanged() { return on_clipboard_changed_; }

    /// Signal emitted when a drag operation enters a window.
    Signal<DragEnterEvent&>& onDragEnter() { return on_drag_enter_; }

    /// Signal emitted as a drag moves across a window.
    Signal<DragMotionEvent&>& onDragMotion() { return on_drag_motion_; }

    /// Signal emitted when a drag leaves a window.
    Signal<DragLeaveEvent&>& onDragLeave() { return on_drag_leave_; }

    /// Signal emitted when data is dropped on a window.
    Signal<DropEvent&>& onDrop() { return on_drop_; }

    // ── Foreign Toplevel / Window Management ─────────────────────
    /// List all currently opened external application windows.
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const;

    /// Get the currently focused / active external window (if any).
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const;

    /// Emitted when a new application window is opened.
    Signal<std::shared_ptr<ToplevelWindow>>& onToplevelCreated() { return on_toplevel_created_; }

    /// Emitted when an application window is closed.
    Signal<std::shared_ptr<ToplevelWindow>>& onToplevelClosed() { return on_toplevel_closed_; }

    /// Emitted when an application window's title changes.
    Signal<std::shared_ptr<ToplevelWindow>, std::string>& onToplevelTitleChanged() { return on_toplevel_title_changed_; }

    /// Emitted when an application window's app-id / class changes.
    Signal<std::shared_ptr<ToplevelWindow>, std::string>& onToplevelAppIdChanged() { return on_toplevel_app_id_changed_; }

    /// Emitted when an application window's state changes (maximized, minimized, etc.).
    Signal<std::shared_ptr<ToplevelWindow>, WindowState>& onToplevelStateChanged() { return on_toplevel_state_changed_; }

    /// Emitted when keyboard / active focus shifts to a new window.
    Signal<std::shared_ptr<ToplevelWindow>>& onActiveToplevelChanged() { return on_active_toplevel_changed_; }

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

    Signal<ClipboardType>     on_clipboard_changed_;
    Signal<DragEnterEvent&>   on_drag_enter_;
    Signal<DragMotionEvent&>  on_drag_motion_;
    Signal<DragLeaveEvent&>   on_drag_leave_;
    Signal<DropEvent&>        on_drop_;

    Signal<std::shared_ptr<ToplevelWindow>>               on_toplevel_created_;
    Signal<std::shared_ptr<ToplevelWindow>>               on_toplevel_closed_;
    Signal<std::shared_ptr<ToplevelWindow>, std::string>  on_toplevel_title_changed_;
    Signal<std::shared_ptr<ToplevelWindow>, std::string>  on_toplevel_app_id_changed_;
    Signal<std::shared_ptr<ToplevelWindow>, WindowState>  on_toplevel_state_changed_;
    Signal<std::shared_ptr<ToplevelWindow>>               on_active_toplevel_changed_;
};

}  // namespace enki
