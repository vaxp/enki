/// @file x11_platform.cpp
/// @brief X11 + EGL native platform backend implementation with multi-MIME Clipboard and XDnD.

#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/window.hpp"

#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <GL/gl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>

namespace enki::x11 {

// ── X11 Data Offer Wrapper (implements DataOffer for XDnD and Clipboard) ─

class X11DataOffer : public DataOffer {
public:
    X11DataOffer(::Display* display, ::Window target_window, ::Window source_window,
                 Atom sel_atom, Atom prop_atom, std::vector<std::string> formats)
        : display_(display), target_window_(target_window), source_window_(source_window),
          sel_atom_(sel_atom), prop_atom_(prop_atom), formats_(std::move(formats)) {}

    [[nodiscard]] bool hasFormat(std::string_view mime_type) const override {
        for (const auto& f : formats_) {
            if (f == mime_type) return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<std::string> formats() const override {
        return formats_;
    }

    [[nodiscard]] std::string readText() override {
        for (const auto& m : {mime::TextPlainUtf8, mime::TextPlain, mime::TextUtf8, mime::TextString}) {
            if (hasFormat(m)) {
                auto data = readData(m);
                return std::string(data.begin(), data.end());
            }
        }
        return {};
    }

    [[nodiscard]] std::vector<std::string> readUris() override {
        auto raw = readData(mime::TextUriList);
        if (raw.empty()) return {};
        std::string content(raw.begin(), raw.end());
        std::istringstream stream(content);
        std::string line;
        std::vector<std::string> uris;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty() && line.front() != '#') {
                uris.push_back(line);
            }
        }
        return uris;
    }

    [[nodiscard]] std::vector<uint8_t> readData(std::string_view mime_type) override {
        if (!display_ || target_window_ == None) return {};
        Atom target_atom = XInternAtom(display_, std::string(mime_type).c_str(), False);
        if (target_atom == None) return {};

        XConvertSelection(display_, sel_atom_, target_atom, prop_atom_, target_window_, CurrentTime);
        XFlush(display_);

        // Wait for SelectionNotify with timeout
        XEvent ev;
        bool received = false;
        for (int i = 0; i < 50; ++i) {
            if (XCheckTypedWindowEvent(display_, target_window_, SelectionNotify, &ev)) {
                received = true;
                break;
            }
            usleep(2000); // 2ms
        }

        if (!received || ev.xselection.property == None) return {};

        Atom actual_type;
        int actual_format;
        unsigned long nitems = 0, bytes_after = 0;
        unsigned char* prop_data = nullptr;

        int status = XGetWindowProperty(display_, target_window_, prop_atom_,
                                        0, 1024 * 1024 / 4, True, AnyPropertyType,
                                        &actual_type, &actual_format, &nitems, &bytes_after,
                                        &prop_data);

        std::vector<uint8_t> result;
        if (status == Success && prop_data && nitems > 0) {
            size_t num_bytes = nitems * (actual_format / 8);
            result.assign(prop_data, prop_data + num_bytes);
            XFree(prop_data);
        }
        return result;
    }

private:
    ::Display* display_ = nullptr;
    ::Window target_window_ = None;
    ::Window source_window_ = None;
    Atom sel_atom_ = None;
    Atom prop_atom_ = None;
    std::vector<std::string> formats_;
};

// ════════════════════════════════════════════════════════════════
// X11PlatformBackend Implementation
// ════════════════════════════════════════════════════════════════

X11PlatformBackend::X11PlatformBackend(Platform* owner)
    : owner_(owner) {}

X11PlatformBackend::~X11PlatformBackend() {
    shutdown();
}

bool X11PlatformBackend::init() {
    // 1. Connect to X server
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        std::cerr << "[ENKI X11] Failed to open X11 display\n";
        return false;
    }
    default_screen_ = DefaultScreen(display_);
    std::cout << "[ENKI Platform] X11 backend active\n";

    // 2. Intern atoms
    atom_wm_protocols_     = XInternAtom(display_, "WM_PROTOCOLS",     False);
    atom_wm_delete_window_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    atom_utf8_string_      = XInternAtom(display_, "UTF8_STRING",      False);
    atom_string_           = XInternAtom(display_, "STRING",           False);
    atom_text_             = XInternAtom(display_, "TEXT",             False);
    atom_targets_          = XInternAtom(display_, "TARGETS",          False);
    atom_clipboard_        = XInternAtom(display_, "CLIPBOARD",        False);
    atom_primary_          = XInternAtom(display_, "PRIMARY",          False);
    atom_enki_sel_prop_    = XInternAtom(display_, "ENKI_SELECTION",   False);

    // XDnD atoms
    atom_xdnd_aware_       = XInternAtom(display_, "XdndAware",        False);
    atom_xdnd_enter_       = XInternAtom(display_, "XdndEnter",        False);
    atom_xdnd_position_    = XInternAtom(display_, "XdndPosition",     False);
    atom_xdnd_status_      = XInternAtom(display_, "XdndStatus",       False);
    atom_xdnd_leave_       = XInternAtom(display_, "XdndLeave",        False);
    atom_xdnd_drop_        = XInternAtom(display_, "XdndDrop",         False);
    atom_xdnd_finished_    = XInternAtom(display_, "XdndFinished",     False);
    atom_xdnd_selection_   = XInternAtom(display_, "XdndSelection",    False);
    atom_xdnd_type_list_   = XInternAtom(display_, "XdndTypeList",     False);
    atom_xdnd_action_copy_ = XInternAtom(display_, "XdndActionCopy",   False);
    atom_xdnd_action_move_ = XInternAtom(display_, "XdndActionMove",   False);
    atom_xdnd_action_link_ = XInternAtom(display_, "XdndActionLink",   False);
    atom_xdnd_action_ask_  = XInternAtom(display_, "XdndActionAsk",    False);
    atom_xdnd_action_priv_ = XInternAtom(display_, "XdndActionPrivate",False);

    // 3. Init EGL
    egl_display_ = eglGetDisplay((EGLNativeDisplayType)display_);
    if (egl_display_ == EGL_NO_DISPLAY) {
        std::cerr << "[ENKI X11] eglGetDisplay failed\n";
        return false;
    }
    if (!eglInitialize(egl_display_, &egl_major_, &egl_minor_)) {
        std::cerr << "[ENKI X11] eglInitialize failed\n";
        return false;
    }
    if (!eglBindAPI(EGL_OPENGL_API)) {
        eglBindAPI(EGL_OPENGL_ES_API);
    }

    // 4. Choose EGL config (RGBA8 + stencil8)
    const EGLint attrs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE,   8, EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8, EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLint n = 0;
    if (!eglChooseConfig(egl_display_, attrs, &egl_config_, 1, &n) || n == 0) {
        const EGLint fallback[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
        if (!eglChooseConfig(egl_display_, fallback, &egl_config_, 1, &n) || n == 0) {
            std::cerr << "[ENKI X11] No suitable EGL config\n";
            return false;
        }
    }
    return true;
}

void X11PlatformBackend::shutdown() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
    }
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
}

bool X11PlatformBackend::pollEvents() {
    if (!display_) return false;

    while (XPending(display_) > 0) {
        XEvent xev;
        XNextEvent(display_, &xev);

        switch (xev.type) {

        case ClientMessage:
            if (xev.xclient.message_type == atom_wm_protocols_ &&
                (Atom)xev.xclient.data.l[0] == atom_wm_delete_window_) {
                owner_->onQuit().emit();
                return false;
            } else if (xev.xclient.message_type == atom_xdnd_enter_) {
                handleXdndEnter(xev.xclient);
            } else if (xev.xclient.message_type == atom_xdnd_position_) {
                handleXdndPosition(xev.xclient);
            } else if (xev.xclient.message_type == atom_xdnd_leave_) {
                handleXdndLeave(xev.xclient);
            } else if (xev.xclient.message_type == atom_xdnd_drop_) {
                handleXdndDrop(xev.xclient);
            }
            break;

        case SelectionRequest:
            handleSelectionRequest(xev.xselectionrequest);
            break;

        case SelectionClear:
            handleSelectionClear(xev.xselectionclear);
            break;

        case MotionNotify:
            owner_->onMouseMove().emit(
                (float)xev.xmotion.x, (float)xev.xmotion.y);
            break;

        case ButtonPress: {
            float x = (float)xev.xbutton.x, y = (float)xev.xbutton.y;
            int   b = xev.xbutton.button;
            if      (b == 4) owner_->onScroll().emit( 0.0f,  1.0f);
            else if (b == 5) owner_->onScroll().emit( 0.0f, -1.0f);
            else             owner_->onMouseDown().emit(x, y, b == 1 ? 1 : (b == 3 ? 3 : 2));
            break;
        }

        case ButtonRelease: {
            float x = (float)xev.xbutton.x, y = (float)xev.xbutton.y;
            int   b = xev.xbutton.button;
            if (b != 4 && b != 5)
                owner_->onMouseUp().emit(x, y, b == 1 ? 1 : (b == 3 ? 3 : 2));
            break;
        }

        case KeyPress: {
            KeySym sym = XLookupKeysym(&xev.xkey, 0);
            int mods = 0;
            if (xev.xkey.state & ShiftMask)   mods |= 1;
            if (xev.xkey.state & ControlMask) mods |= 2;
            if (xev.xkey.state & Mod1Mask)    mods |= 4;
            owner_->onKeyDown().emit((int)sym, mods);
            char buf[32];
            int len = XLookupString(&xev.xkey, buf, 31, nullptr, nullptr);
            if (len > 0) { buf[len] = '\0'; owner_->onTextInput().emit(std::string_view(buf, len)); }
            break;
        }

        case KeyRelease: {
            KeySym sym = XLookupKeysym(&xev.xkey, 0);
            int mods = 0;
            if (xev.xkey.state & ShiftMask)   mods |= 1;
            if (xev.xkey.state & ControlMask) mods |= 2;
            if (xev.xkey.state & Mod1Mask)    mods |= 4;
            owner_->onKeyUp().emit((int)sym, mods);
            break;
        }

        case ConfigureNotify: {
            for (Window* w : windows_) {
                if ((void*)(uintptr_t)xev.xconfigure.window == w->getNativeHandle()) {
                    w->onResize().emit(xev.xconfigure.width, xev.xconfigure.height);
                    break;
                }
            }
            break;
        }

        default: break;
        }
    }
    return true;
}

void X11PlatformBackend::registerWindow(Window* w) {
    if (!w) return;
    windows_.insert(w);
    if (display_) {
        ::Window xwin = (::Window)(uintptr_t)w->getNativeHandle();
        Atom xdnd_version = 5;
        XChangeProperty(display_, xwin, atom_xdnd_aware_, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)&xdnd_version, 1);
        XFlush(display_);
    }
}

void X11PlatformBackend::unregisterWindow(Window* w) {
    if (w) windows_.erase(w);
}

// ── Clipboard Subsystem ──────────────────────────────────────────

void X11PlatformBackend::setClipboardData(const ClipboardData& data, ClipboardType type) {
    if (type == ClipboardType::Primary) {
        local_primary_ = data;
    } else {
        local_clipboard_ = data;
    }

    if (windows_.empty() || !display_) return;

    ::Window xwin = (::Window)(uintptr_t)(*windows_.begin())->getNativeHandle();
    Atom sel = (type == ClipboardType::Primary) ? atom_primary_ : atom_clipboard_;
    XSetSelectionOwner(display_, sel, xwin, CurrentTime);
    XFlush(display_);
}

void X11PlatformBackend::setClipboardText(std::string_view text, ClipboardType type) {
    ClipboardData data;
    data.setText(text);
    setClipboardData(data, type);
}

std::string X11PlatformBackend::getClipboardText(ClipboardType type) const {
    const ClipboardData& local = (type == ClipboardType::Primary) ? local_primary_ : local_clipboard_;
    if (local.hasText()) {
        return local.getText();
    }

    auto bytes = getClipboardDataForMime(mime::TextPlainUtf8, type);
    if (bytes.empty()) {
        bytes = getClipboardDataForMime(mime::TextPlain, type);
    }
    if (bytes.empty()) {
        bytes = getClipboardDataForMime(mime::TextString, type);
    }
    if (!bytes.empty()) {
        return std::string(bytes.begin(), bytes.end());
    }
    return {};
}

ClipboardData X11PlatformBackend::getClipboardData(ClipboardType type) const {
    const ClipboardData& local = (type == ClipboardType::Primary) ? local_primary_ : local_clipboard_;
    if (!local.empty()) return local;

    ClipboardData result;
    auto formats = getClipboardFormats(type);
    for (const auto& fmt : formats) {
        auto bytes = getClipboardDataForMime(fmt, type);
        if (!bytes.empty()) {
            result.setRaw(fmt, bytes);
        }
    }
    return result;
}

std::vector<uint8_t> X11PlatformBackend::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
    const ClipboardData& local = (type == ClipboardType::Primary) ? local_primary_ : local_clipboard_;
    if (local.hasFormat(mime_type)) {
        return local.getRaw(mime_type);
    }

    if (windows_.empty() || !display_) return {};
    ::Window xwin = (::Window)(uintptr_t)(*windows_.begin())->getNativeHandle();
    Atom sel = (type == ClipboardType::Primary) ? atom_primary_ : atom_clipboard_;
    Atom target = XInternAtom(display_, std::string(mime_type).c_str(), False);
    if (target == None) return {};

    XConvertSelection(display_, sel, target, atom_enki_sel_prop_, xwin, CurrentTime);
    XFlush(display_);

    XEvent ev;
    bool received = false;
    for (int i = 0; i < 50; ++i) {
        if (XCheckTypedWindowEvent(display_, xwin, SelectionNotify, &ev)) {
            received = true;
            break;
        }
        usleep(2000); // 2ms
    }

    if (!received || ev.xselection.property == None) return {};

    Atom actual_type;
    int actual_format;
    unsigned long nitems = 0, bytes_after = 0;
    unsigned char* prop_data = nullptr;

    int status = XGetWindowProperty(display_, xwin, atom_enki_sel_prop_,
                                    0, 1024 * 1024 / 4, True, AnyPropertyType,
                                    &actual_type, &actual_format, &nitems, &bytes_after,
                                    &prop_data);

    std::vector<uint8_t> result;
    if (status == Success && prop_data && nitems > 0) {
        size_t num_bytes = nitems * (actual_format / 8);
        result.assign(prop_data, prop_data + num_bytes);
        XFree(prop_data);
    }
    return result;
}

std::vector<std::string> X11PlatformBackend::getClipboardFormats(ClipboardType type) const {
    const ClipboardData& local = (type == ClipboardType::Primary) ? local_primary_ : local_clipboard_;
    if (!local.empty()) {
        return local.formats();
    }

    if (windows_.empty() || !display_) return {};
    ::Window xwin = (::Window)(uintptr_t)(*windows_.begin())->getNativeHandle();
    Atom sel = (type == ClipboardType::Primary) ? atom_primary_ : atom_clipboard_;

    XConvertSelection(display_, sel, atom_targets_, atom_enki_sel_prop_, xwin, CurrentTime);
    XFlush(display_);

    XEvent ev;
    bool received = false;
    for (int i = 0; i < 50; ++i) {
        if (XCheckTypedWindowEvent(display_, xwin, SelectionNotify, &ev)) {
            received = true;
            break;
        }
        usleep(2000);
    }

    if (!received || ev.xselection.property == None) return {};

    Atom actual_type;
    int actual_format;
    unsigned long nitems = 0, bytes_after = 0;
    unsigned char* prop_data = nullptr;

    int status = XGetWindowProperty(display_, xwin, atom_enki_sel_prop_,
                                    0, 4096, True, XA_ATOM,
                                    &actual_type, &actual_format, &nitems, &bytes_after,
                                    &prop_data);

    std::vector<std::string> formats;
    if (status == Success && prop_data && nitems > 0) {
        auto* atoms = reinterpret_cast<Atom*>(prop_data);
        for (unsigned long i = 0; i < nitems; ++i) {
            char* name = XGetAtomName(display_, atoms[i]);
            if (name) {
                formats.emplace_back(name);
                XFree(name);
            }
        }
        XFree(prop_data);
    }
    return formats;
}

bool X11PlatformBackend::hasClipboardFormat(std::string_view mime_type, ClipboardType type) const {
    const ClipboardData& local = (type == ClipboardType::Primary) ? local_primary_ : local_clipboard_;
    if (local.hasFormat(mime_type)) return true;

    auto formats = getClipboardFormats(type);
    for (const auto& fmt : formats) {
        if (fmt == mime_type) return true;
    }
    return false;
}

void X11PlatformBackend::handleSelectionRequest(const XSelectionRequestEvent& req) {
    if (!display_) return;

    XSelectionEvent reply;
    reply.type = SelectionNotify;
    reply.display = req.display;
    reply.requestor = req.requestor;
    reply.selection = req.selection;
    reply.target = req.target;
    reply.property = None;
    reply.time = req.time;

    const ClipboardData& data = (req.selection == atom_primary_) ? local_primary_ :
                                (req.selection == atom_xdnd_selection_) ? outgoing_drag_data_.payload :
                                local_clipboard_;

    if (req.target == atom_targets_) {
        // Return supported target atoms
        std::vector<Atom> targets = { atom_targets_ };
        for (const auto& fmt : data.formats()) {
            Atom a = XInternAtom(display_, fmt.c_str(), False);
            if (a != None) targets.push_back(a);
        }
        if (data.hasText()) {
            targets.push_back(atom_utf8_string_);
            targets.push_back(atom_string_);
            targets.push_back(atom_text_);
        }

        XChangeProperty(display_, req.requestor, req.property, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<unsigned char*>(targets.data()), static_cast<int>(targets.size()));
        reply.property = req.property;
    } else if (req.target == atom_utf8_string_ || req.target == atom_string_ || req.target == atom_text_) {
        std::string text = data.getText();
        XChangeProperty(display_, req.requestor, req.property, req.target, 8, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(text.data()), static_cast<int>(text.size()));
        reply.property = req.property;
    } else {
        char* target_name = XGetAtomName(display_, req.target);
        if (target_name) {
            std::string mime(target_name);
            XFree(target_name);
            if (data.hasFormat(mime)) {
                auto raw = data.getRaw(mime);
                XChangeProperty(display_, req.requestor, req.property, req.target, 8, PropModeReplace,
                                raw.data(), static_cast<int>(raw.size()));
                reply.property = req.property;
            }
        }
    }

    XSendEvent(display_, req.requestor, False, 0, reinterpret_cast<XEvent*>(&reply));
    XFlush(display_);
}

void X11PlatformBackend::handleSelectionClear(const XSelectionClearEvent& clr) {
    if (owner_) {
        ClipboardType type = (clr.selection == atom_primary_) ? ClipboardType::Primary : ClipboardType::Clipboard;
        owner_->onClipboardChanged().emit(type);
    }
}

// ── XDnD Handling ────────────────────────────────────────────────

void X11PlatformBackend::handleXdndEnter(const XClientMessageEvent& cme) {
    xdnd_source_window_ = static_cast<::Window>(cme.data.l[0]);
    bool more_than_3 = (cme.data.l[1] & 1) != 0;

    xdnd_source_types_.clear();
    if (!more_than_3) {
        for (int i = 2; i <= 4; ++i) {
            Atom a = static_cast<Atom>(cme.data.l[i]);
            if (a != None) {
                char* name = XGetAtomName(display_, a);
                if (name) {
                    xdnd_source_types_.emplace_back(name);
                    XFree(name);
                }
            }
        }
    } else {
        Atom actual_type;
        int actual_format;
        unsigned long nitems = 0, bytes_after = 0;
        unsigned char* prop_data = nullptr;

        int status = XGetWindowProperty(display_, xdnd_source_window_, atom_xdnd_type_list_,
                                        0, 1024, False, XA_ATOM,
                                        &actual_type, &actual_format, &nitems, &bytes_after,
                                        &prop_data);
        if (status == Success && prop_data && nitems > 0) {
            auto* atoms = reinterpret_cast<Atom*>(prop_data);
            for (unsigned long i = 0; i < nitems; ++i) {
                char* name = XGetAtomName(display_, atoms[i]);
                if (name) {
                    xdnd_source_types_.emplace_back(name);
                    XFree(name);
                }
            }
            XFree(prop_data);
        }
    }

    if (owner_) {
        DragEnterEvent ev;
        ev.position = xdnd_last_pos_;
        ev.mime_types = xdnd_source_types_;
        ev.suggested_action = DragAction::Copy;
        owner_->onDragEnter().emit(ev);
        xdnd_accepted_action_ = ev.accepted_action;
    }
}

void X11PlatformBackend::handleXdndPosition(const XClientMessageEvent& cme) {
    xdnd_source_window_ = static_cast<::Window>(cme.data.l[0]);
    float x = static_cast<float>((cme.data.l[2] >> 16) & 0xFFFF);
    float y = static_cast<float>(cme.data.l[2] & 0xFFFF);
    xdnd_last_pos_ = Point{x, y};

    if (owner_) {
        DragMotionEvent ev;
        ev.position = xdnd_last_pos_;
        ev.suggested_action = DragAction::Copy;
        owner_->onDragMotion().emit(ev);
        xdnd_accepted_action_ = ev.accepted_action;
    }

    // Send XdndStatus back to source window
    XClientMessageEvent status_ev;
    std::memset(&status_ev, 0, sizeof(status_ev));
    status_ev.type = ClientMessage;
    status_ev.display = display_;
    status_ev.window = xdnd_source_window_;
    status_ev.message_type = atom_xdnd_status_;
    status_ev.format = 32;
    status_ev.data.l[0] = cme.window; // Target window
    status_ev.data.l[1] = (xdnd_accepted_action_ != DragAction::NoAction) ? (1 | 2) : 0; // bit 0: accept, bit 1: want position
    status_ev.data.l[2] = 0; // Empty rectangle
    status_ev.data.l[3] = 0;
    status_ev.data.l[4] = (xdnd_accepted_action_ == DragAction::Move) ? atom_xdnd_action_move_ : atom_xdnd_action_copy_;

    XSendEvent(display_, xdnd_source_window_, False, NoEventMask, reinterpret_cast<XEvent*>(&status_ev));
    XFlush(display_);
}

void X11PlatformBackend::handleXdndLeave(const XClientMessageEvent& /*cme*/) {
    xdnd_source_window_ = None;
    xdnd_source_types_.clear();
    xdnd_accepted_action_ = DragAction::NoAction;
    if (owner_) {
        DragLeaveEvent ev;
        owner_->onDragLeave().emit(ev);
    }
}

void X11PlatformBackend::handleXdndDrop(const XClientMessageEvent& cme) {
    xdnd_source_window_ = static_cast<::Window>(cme.data.l[0]);
    bool handled = false;

    if (owner_) {
        DropEvent ev;
        ev.position = xdnd_last_pos_;
        ev.action = xdnd_accepted_action_;
        ev.data = std::make_shared<X11DataOffer>(display_, cme.window, xdnd_source_window_,
                                                atom_xdnd_selection_, atom_enki_sel_prop_, xdnd_source_types_);
        owner_->onDrop().emit(ev);
        handled = ev.handled;
    }

    // Send XdndFinished back to source window
    XClientMessageEvent finish_ev;
    std::memset(&finish_ev, 0, sizeof(finish_ev));
    finish_ev.type = ClientMessage;
    finish_ev.display = display_;
    finish_ev.window = xdnd_source_window_;
    finish_ev.message_type = atom_xdnd_finished_;
    finish_ev.format = 32;
    finish_ev.data.l[0] = cme.window;
    finish_ev.data.l[1] = handled ? 1 : 0;
    finish_ev.data.l[2] = (xdnd_accepted_action_ == DragAction::Move) ? atom_xdnd_action_move_ : atom_xdnd_action_copy_;

    XSendEvent(display_, xdnd_source_window_, False, NoEventMask, reinterpret_cast<XEvent*>(&finish_ev));
    XFlush(display_);

    xdnd_source_window_ = None;
    xdnd_source_types_.clear();
    xdnd_accepted_action_ = DragAction::NoAction;
}

bool X11PlatformBackend::startDrag(const DragData& data, DragAction /*actions*/) {
    if (windows_.empty() || !display_) return false;

    outgoing_drag_data_ = data;
    ::Window drag_win = (::Window)(uintptr_t)(*windows_.begin())->getNativeHandle();
    XSetSelectionOwner(display_, atom_xdnd_selection_, drag_win, CurrentTime);
    XFlush(display_);

    // Set XdndTypeList property on drag window
    std::vector<Atom> type_atoms;
    for (const auto& fmt : data.payload.formats()) {
        Atom a = XInternAtom(display_, fmt.c_str(), False);
        if (a != None) type_atoms.push_back(a);
    }
    if (data.payload.hasText()) {
        type_atoms.push_back(atom_utf8_string_);
    }

    XChangeProperty(display_, drag_win, atom_xdnd_type_list_, XA_ATOM, 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(type_atoms.data()), static_cast<int>(type_atoms.size()));
    XFlush(display_);
    return true;
}

void X11PlatformBackend::setCursor(SystemCursor cursor) {
    if (!display_ || windows_.empty()) return;

    unsigned int shape = XC_left_ptr;
    switch (cursor) {
        case SystemCursor::Pointer: shape = XC_hand2; break;
        case SystemCursor::Text: shape = XC_xterm; break;
        case SystemCursor::Crosshair: shape = XC_crosshair; break;
        case SystemCursor::Move: shape = XC_fleur; break;
        case SystemCursor::NotAllowed: shape = XC_circle; break;
        case SystemCursor::ResizeHorizontal: shape = XC_sb_h_double_arrow; break;
        case SystemCursor::ResizeVertical: shape = XC_sb_v_double_arrow; break;
        case SystemCursor::Wait: shape = XC_watch; break;
        default: shape = XC_left_ptr; break;
    }

    ::Cursor xcursor = XCreateFontCursor(display_, shape);
    for (Window* w : windows_) {
        ::Window xwin = (::Window)(uintptr_t)w->getNativeHandle();
        XDefineCursor(display_, xwin, xcursor);
    }
    XFreeCursor(display_, xcursor);
    XFlush(display_);
}

} // namespace enki::x11
