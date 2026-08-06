/// @file x11_platform.cpp
/// @brief X11 + EGL native platform backend implementation.

#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/window.hpp"

#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <GL/gl.h>
#include <iostream>
#include <cstring>

namespace enki::x11 {

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
    atom_clipboard_        = XInternAtom(display_, "CLIPBOARD",        False);

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
            }
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
            // Forward resize to the matching Window if registered
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
    if (w) windows_.insert(w);
}

void X11PlatformBackend::unregisterWindow(Window* w) {
    if (w) windows_.erase(w);
}

void X11PlatformBackend::setClipboardText(const std::string& text) {
    clipboard_buffer_ = text;
    if (windows_.empty() || !display_) return;
    ::Window xwin = (::Window)(uintptr_t)(*windows_.begin())->getNativeHandle();
    XSetSelectionOwner(display_, atom_clipboard_, xwin, CurrentTime);
    XFlush(display_);
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
