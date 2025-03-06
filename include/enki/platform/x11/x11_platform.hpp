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

    // ── Clipboard Subsystem ──────────────────────────────────────
    void setClipboardData(const ClipboardData& data, ClipboardType type);
    void setClipboardText(std::string_view text, ClipboardType type = ClipboardType::Clipboard);
    [[nodiscard]] std::string getClipboardText(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    // ── Drag and Drop Subsystem ──────────────────────────────────
    bool startDrag(const DragData& data, DragAction actions);

    void setCursor(SystemCursor cursor);

    // Internal XDnD handlers
    void handleSelectionRequest(const XSelectionRequestEvent& req);
    void handleSelectionClear(const XSelectionClearEvent& clr);
    void handleXdndEnter(const XClientMessageEvent& cme);
    void handleXdndPosition(const XClientMessageEvent& cme);
    void handleXdndLeave(const XClientMessageEvent& cme);
    void handleXdndDrop(const XClientMessageEvent& cme);

private:
    Platform*   owner_          = nullptr;
    ::Display*  display_        = nullptr;
    int         default_screen_ = 0;
    EGLDisplay egl_display_    = EGL_NO_DISPLAY;
    EGLConfig  egl_config_     = nullptr;
    int        egl_major_      = 0;
    int        egl_minor_      = 0;

    // Standard Atoms
    Atom atom_wm_protocols_     = None;
    Atom atom_wm_delete_window_ = None;
    Atom atom_utf8_string_      = None;
    Atom atom_string_           = None;
    Atom atom_text_             = None;
    Atom atom_targets_          = None;
    Atom atom_clipboard_        = None;
    Atom atom_primary_          = None;
    Atom atom_enki_sel_prop_    = None;

    // XDnD Atoms
    Atom atom_xdnd_aware_       = None;
    Atom atom_xdnd_enter_       = None;
    Atom atom_xdnd_position_    = None;
    Atom atom_xdnd_status_      = None;
    Atom atom_xdnd_leave_       = None;
    Atom atom_xdnd_drop_        = None;
    Atom atom_xdnd_finished_    = None;
    Atom atom_xdnd_selection_   = None;
    Atom atom_xdnd_type_list_   = None;
    Atom atom_xdnd_action_copy_ = None;
    Atom atom_xdnd_action_move_ = None;
    Atom atom_xdnd_action_link_ = None;
    Atom atom_xdnd_action_ask_  = None;
    Atom atom_xdnd_action_priv_ = None;

    std::unordered_set<Window*> windows_;

    // Local Clipboard buffers
    ClipboardData local_clipboard_;
    ClipboardData local_primary_;

    // XDnD state
    ::Window                 xdnd_source_window_ = None;
    std::vector<std::string> xdnd_source_types_;
    DragAction               xdnd_accepted_action_ = DragAction::NoAction;
    Point                    xdnd_last_pos_;

    // Outgoing Drag state
    DragData outgoing_drag_data_;
};

} // namespace enki::x11
