/// @file x11_window.cpp
/// @brief X11 + EGL window backend implementation.

#include "enki/platform/x11/x11_window.hpp"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <dlfcn.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

namespace enki::x11 {

X11Window::X11Window(X11PlatformBackend& backend)
    : backend_(backend) {}

X11Window::~X11Window() {
    destroy();
}

bool X11Window::init(const WindowConfig& cfg) {
    config_     = cfg;
    display_    = backend_.getDisplay();
    egl_display_ = backend_.getEGLDisplay();
    EGLConfig egl_cfg = backend_.getEGLConfig();

    if (!display_ || egl_display_ == EGL_NO_DISPLAY || !egl_cfg) {
        std::cerr << "[ENKI X11Window] Backend display/EGL invalid\n";
        return false;
    }

    int screen     = backend_.getDefaultScreen();
    ::Window root  = RootWindow(display_, screen);

    current_width_  = cfg.width;
    current_height_ = cfg.height;

    int screen_w = DisplayWidth(display_,  screen);
    int screen_h = DisplayHeight(display_, screen);
    int pos_x = (cfg.x >= 0) ? cfg.x : (screen_w - current_width_)  / 2;
    int pos_y = (cfg.y >= 0) ? cfg.y : (screen_h - current_height_) / 2;
    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;

    // Match X11 visual to EGL native visual id
    EGLint visual_id = 0;
    eglGetConfigAttrib(egl_display_, egl_cfg, EGL_NATIVE_VISUAL_ID, &visual_id);

    Visual* visual = nullptr;
    int depth = 0;
    if (visual_id != 0) {
        XVisualInfo tmpl; tmpl.visualid = visual_id;
        int n = 0;
        XVisualInfo* list = XGetVisualInfo(display_, VisualIDMask, &tmpl, &n);
        if (list && n > 0) { visual = list[0].visual; depth = list[0].depth; }
        if (list) XFree(list);
    }
    if (!visual) {
        visual = DefaultVisual(display_, screen);
        depth  = DefaultDepth(display_, screen);
    }

    colormap_ = XCreateColormap(display_, root, visual, AllocNone);

    XSetWindowAttributes swa{};
    swa.colormap     = colormap_;
    swa.border_pixel = 0;
    swa.bit_gravity  = NorthWestGravity;
    swa.event_mask   = ExposureMask | StructureNotifyMask | ButtonPressMask |
                       ButtonReleaseMask | PointerMotionMask | KeyPressMask |
                       KeyReleaseMask | FocusChangeMask | PropertyChangeMask;
    unsigned long valuemask = CWColormap | CWBorderPixel | CWBitGravity | CWEventMask;

    if (cfg.override_redirect) {
        swa.override_redirect = True;
        valuemask |= CWOverrideRedirect;
    }

    x11_window_ = XCreateWindow(display_, root, pos_x, pos_y,
                                 current_width_, current_height_,
                                 0, depth, InputOutput, visual,
                                 valuemask, &swa);
    if (!x11_window_) {
        std::cerr << "[ENKI X11Window] XCreateWindow failed\n";
        return false;
    }

    // Never paint or clear any background by the X server (pure client rendering)
    XSetWindowBackgroundPixmap(display_, x11_window_, None);

    setTitle(cfg.title);

    // Set _NET_WM_WINDOW_TYPE to _NET_WM_WINDOW_TYPE_NORMAL
    Atom net_wm_window_type = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_window_type_normal = XInternAtom(display_, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    XChangeProperty(display_, x11_window_, net_wm_window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&net_wm_window_type_normal, 1);

    // WM_DELETE_WINDOW protocol
    Atom wm_proto  = backend_.getAtomWmProtocols();
    Atom wm_delete = backend_.getAtomWmDeleteWindow();
    XSetWMProtocols(display_, x11_window_, &wm_delete, 1);

    if (cfg.borderless || cfg.csd) {
        setBorderless(true);
        // Inform window manager that client provides its own frame
        Atom frame_extents = backend_.getAtomGtkFrameExtents();
        if (frame_extents) {
            unsigned long extents[4] = {0, 0, 0, 0};
            XChangeProperty(display_, x11_window_, frame_extents, XA_CARDINAL, 32,
                            PropModeReplace, (unsigned char*)extents, 4);
        }
    }
    if (cfg.always_on_top)         setAlwaysOnTop(true);

    // Set _NET_WM_PID
    Atom net_pid = XInternAtom(display_, "_NET_WM_PID", False);
    long pid     = (long)getpid();
    XChangeProperty(display_, x11_window_, net_pid, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&pid, 1);

    // Size hints
    XSizeHints sh{};
    sh.flags     = PMinSize;
    sh.min_width  = cfg.min_width;
    sh.min_height = cfg.min_height;
    if (!cfg.resizable) {
        sh.flags |= PMaxSize;
        sh.max_width = sh.max_height = cfg.width;
    }
    XSetWMNormalHints(display_, x11_window_, &sh);

    // EGL surface
    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_cfg,
                                          (EGLNativeWindowType)x11_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[ENKI X11Window] Failed to create EGL surface\n";
        return false;
    }

    // Use shared master EGL context from backend
    egl_context_ = backend_.getEGLContext();
    if (egl_context_ == EGL_NO_CONTEXT) {
        const EGLint core_attrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE };
        const EGLint gles_attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

        egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, core_attrs);
        if (egl_context_ == EGL_NO_CONTEXT)
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, gles_attrs);
        if (egl_context_ == EGL_NO_CONTEXT)
            egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, nullptr);
    }
    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[ENKI X11Window] Failed to create EGL context\n";
        return false;
    }

    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    eglSwapInterval(egl_display_, cfg.vsync ? 1 : 0);

    XMapWindow(display_, x11_window_);
    XFlush(display_);
    return true;
}

void X11Window::destroy() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        if (egl_surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }
    }
    if (display_ && x11_window_) {
        XDestroyWindow(display_, x11_window_);
        x11_window_ = 0;
    }
    if (display_ && colormap_) {
        XFreeColormap(display_, colormap_);
        colormap_ = 0;
    }
}

void X11Window::setTitle(std::string_view title) {
    if (!display_ || !x11_window_) return;
    std::string s(title);
    XStoreName(display_, x11_window_, s.c_str());
    Atom net_name = XInternAtom(display_, "_NET_WM_NAME", False);
    Atom utf8     = backend_.getAtomUtf8String();
    XChangeProperty(display_, x11_window_, net_name, utf8, 8,
                    PropModeReplace, (unsigned char*)s.c_str(), (int)s.size());
    XFlush(display_);
}

void X11Window::setSize(int w, int h) {
    current_width_ = w; current_height_ = h;
    if (display_ && x11_window_) {
        XResizeWindow(display_, x11_window_, w, h);
        XFlush(display_);
    }
}

void X11Window::setPosition(int x, int y) {
    if (display_ && x11_window_) {
        XMoveWindow(display_, x11_window_, x, y);
        XFlush(display_);
    }
}

void X11Window::setBorderless(bool borderless) {
    if (!display_ || !x11_window_) return;
    struct { unsigned long flags, functions, decorations; long inputMode; unsigned long status; } h{};
    h.flags = 2; // MWM_HINTS_DECORATIONS
    h.decorations = borderless ? 0 : 1;
    Atom atom = XInternAtom(display_, "_MOTIF_WM_HINTS", False);
    XChangeProperty(display_, x11_window_, atom, atom, 32, PropModeReplace, (unsigned char*)&h, 5);
    XFlush(display_);
}

void X11Window::setAlwaysOnTop(bool on_top) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = XInternAtom(display_, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(display_, "_NET_WM_STATE_ABOVE", False);
    XEvent xev{};
    xev.type = ClientMessage;
    xev.xclient.window       = x11_window_;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = on_top ? 1 : 0;
    xev.xclient.data.l[1]    = wm_above;
    XSendEvent(display_, DefaultRootWindow(display_), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XFlush(display_);
}

// ── Client-Side Decoration (CSD) Operations ─────────────────────

void X11Window::beginMove(float local_x, float local_y, int button) {
    if (!display_ || !x11_window_) return;
    if (state_ == WindowState::Maximized || state_ == WindowState::Fullscreen) return;

    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    ::Window root_ret = 0, child_ret = 0;
    int win_x = 0, win_y = 0;
    unsigned int mask = 0;
    XQueryPointer(display_, root, &root_ret, &child_ret,
                  &drag_start_root_x_, &drag_start_root_y_,
                  &win_x, &win_y, &mask);

    ::Window child = 0;
    XTranslateCoordinates(display_, x11_window_, root, 0, 0,
                          &drag_orig_win_x_, &drag_orig_win_y_, &child);

    is_moving_   = true;
    is_resizing_ = false;

    // Grab pointer so we continue receiving motion events reliably during drag
    XGrabPointer(display_, x11_window_, False,
                 ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 None, None, CurrentTime);
    XFlush(display_);
}

void X11Window::beginResize(WindowEdge edge, float local_x, float local_y, int button) {
    if (!display_ || !x11_window_ || edge == WindowEdge::NoneEdge) return;
    if (state_ == WindowState::Maximized || state_ == WindowState::Fullscreen) return;

    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    ::Window root_ret = 0, child_ret = 0;
    int win_x = 0, win_y = 0;
    unsigned int mask = 0;
    XQueryPointer(display_, root, &root_ret, &child_ret,
                  &drag_start_root_x_, &drag_start_root_y_,
                  &win_x, &win_y, &mask);

    ::Window child = 0;
    XTranslateCoordinates(display_, x11_window_, root, 0, 0,
                          &drag_orig_win_x_, &drag_orig_win_y_, &child);

    drag_orig_win_w_ = current_width_;
    drag_orig_win_h_ = current_height_;

    is_resizing_ = true;
    is_moving_   = false;
    resize_edge_ = edge;

    XGrabPointer(display_, x11_window_, False,
                 ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 None, None, CurrentTime);
    XFlush(display_);
}

bool X11Window::handleDragMotion(int root_x, int root_y) {
    if (!display_ || !x11_window_) return false;

    if (is_moving_) {
        int dx = root_x - drag_start_root_x_;
        int dy = root_y - drag_start_root_y_;
        int new_x = drag_orig_win_x_ + dx;
        int new_y = drag_orig_win_y_ + dy;

        XMoveWindow(display_, x11_window_, new_x, new_y);
        return true;
    }

    if (is_resizing_) {
        int dx = root_x - drag_start_root_x_;
        int dy = root_y - drag_start_root_y_;
        int new_x = drag_orig_win_x_;
        int new_y = drag_orig_win_y_;
        int new_w = drag_orig_win_w_;
        int new_h = drag_orig_win_h_;

        switch (resize_edge_) {
            case WindowEdge::Left:
                new_x += dx; new_w -= dx; break;
            case WindowEdge::Right:
                new_w += dx; break;
            case WindowEdge::Top:
                new_y += dy; new_h -= dy; break;
            case WindowEdge::Bottom:
                new_h += dy; break;
            case WindowEdge::TopLeft:
                new_x += dx; new_w -= dx;
                new_y += dy; new_h -= dy; break;
            case WindowEdge::TopRight:
                new_w += dx;
                new_y += dy; new_h -= dy; break;
            case WindowEdge::BottomLeft:
                new_x += dx; new_w -= dx;
                new_h += dy; break;
            case WindowEdge::BottomRight:
                new_w += dx;
                new_h += dy; break;
            default: break;
        }

        int min_w = config_.min_width > 0 ? config_.min_width : 300;
        int min_h = config_.min_height > 0 ? config_.min_height : 200;
        if (new_w < min_w) {
            if (new_x != drag_orig_win_x_) new_x = drag_orig_win_x_ + (drag_orig_win_w_ - min_w);
            new_w = min_w;
        }
        if (new_h < min_h) {
            if (new_y != drag_orig_win_y_) new_y = drag_orig_win_y_ + (drag_orig_win_h_ - min_h);
            new_h = min_h;
        }

        if (new_x != drag_orig_win_x_ || new_y != drag_orig_win_y_) {
            XMoveResizeWindow(display_, x11_window_, new_x, new_y, new_w, new_h);
        } else {
            XResizeWindow(display_, x11_window_, new_w, new_h);
        }

        if (new_w != current_width_ || new_h != current_height_) {
            current_width_  = new_w;
            current_height_ = new_h;
            on_resize_.emit(new_w, new_h);
        }
        return true;
    }

    return false;
}

void X11Window::endDrag() {
    if (is_moving_ || is_resizing_) {
        is_moving_   = false;
        is_resizing_ = false;
        resize_edge_ = WindowEdge::NoneEdge;
        if (display_) {
            XUngrabPointer(display_, CurrentTime);
            XFlush(display_);
        }
    }
}

void X11Window::setMaximized(bool max) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = backend_.getAtomNetWmState();
    Atom max_vert = backend_.getAtomNetWmStateMaxVert();
    Atom max_horz = backend_.getAtomNetWmStateMaxHorz();
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());

    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = wm_state;
    xclient.format = 32;
    xclient.data.l[0] = max ? 1 : 0; // 1 = _NET_WM_STATE_ADD, 0 = _NET_WM_STATE_REMOVE
    xclient.data.l[1] = max_vert;
    xclient.data.l[2] = max_horz;
    xclient.data.l[3] = 1;

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::setMinimized(bool min) {
    if (!display_ || !x11_window_) return;
    if (min) {
        XIconifyWindow(display_, x11_window_, backend_.getDefaultScreen());
        XFlush(display_);
    }
}

void X11Window::setFullscreen(bool full) {
    if (!display_ || !x11_window_) return;
    Atom wm_state = backend_.getAtomNetWmState();
    Atom fullscreen = backend_.getAtomNetWmStateFullscreen();
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());

    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = wm_state;
    xclient.format = 32;
    xclient.data.l[0] = full ? 1 : 0;
    xclient.data.l[1] = fullscreen;
    xclient.data.l[2] = 0;
    xclient.data.l[3] = 1;

    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::toggleMaximize() {
    setMaximized(!isMaximized());
}

void X11Window::showWindowMenu(float local_x, float local_y, int /*button*/) {
    if (!display_ || !x11_window_) return;
    int root_x = 0, root_y = 0;
    ::Window child = 0;
    ::Window root = RootWindow(display_, backend_.getDefaultScreen());
    XTranslateCoordinates(display_, x11_window_, root,
                          static_cast<int>(local_x), static_cast<int>(local_y),
                          &root_x, &root_y, &child);

    Atom show_menu = XInternAtom(display_, "_GTK_SHOW_WINDOW_MENU", False);
    XClientMessageEvent xclient{};
    xclient.type = ClientMessage;
    xclient.window = x11_window_;
    xclient.message_type = show_menu;
    xclient.format = 32;
    xclient.data.l[0] = 0; // default device
    xclient.data.l[1] = root_x;
    xclient.data.l[2] = root_y;

    XUngrabPointer(display_, CurrentTime);
    XSendEvent(display_, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent*)&xclient);
    XFlush(display_);
}

void X11Window::setDecorated(bool decorated) {
    setBorderless(!decorated);
}

void X11Window::setWindowGeometry(int x, int y, int width, int height) {
    if (!display_ || !x11_window_) return;
    long left = x > 0 ? x : 0;
    long top = y > 0 ? y : 0;
    long right = (current_width_ > (x + width)) ? (current_width_ - (x + width)) : 0;
    long bottom = (current_height_ > (y + height)) ? (current_height_ - (y + height)) : 0;

    unsigned long extents[4] = {
        static_cast<unsigned long>(left),
        static_cast<unsigned long>(right),
        static_cast<unsigned long>(top),
        static_cast<unsigned long>(bottom)
    };
    Atom frame_extents = backend_.getAtomGtkFrameExtents();
    if (frame_extents) {
        XChangeProperty(display_, x11_window_, frame_extents, XA_CARDINAL, 32,
                        PropModeReplace, reinterpret_cast<unsigned char*>(extents), 4);
        XFlush(display_);
    }
}

void X11Window::updateState() {
    if (!display_ || !x11_window_) return;
    Atom net_wm_state = backend_.getAtomNetWmState();
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long nitems = 0, bytes_after = 0;
    unsigned char* prop_data = nullptr;

    WindowState new_state = state_ & WindowState::Activated;
    int status = XGetWindowProperty(display_, x11_window_, net_wm_state,
                                    0, 64, False, XA_ATOM,
                                    &actual_type, &actual_format, &nitems, &bytes_after,
                                    &prop_data);
    if (status == Success && prop_data && nitems > 0) {
        Atom* atoms = reinterpret_cast<Atom*>(prop_data);
        Atom max_vert = backend_.getAtomNetWmStateMaxVert();
        Atom max_horz = backend_.getAtomNetWmStateMaxHorz();
        Atom fullscreen = backend_.getAtomNetWmStateFullscreen();
        Atom hidden = backend_.getAtomNetWmStateHidden();

        bool has_vert = false, has_horz = false;
        for (unsigned long i = 0; i < nitems; ++i) {
            if (atoms[i] == max_vert) has_vert = true;
            if (atoms[i] == max_horz) has_horz = true;
            if (atoms[i] == fullscreen) new_state |= WindowState::Fullscreen;
            if (atoms[i] == hidden) new_state |= WindowState::Minimized;
        }
        if (has_vert && has_horz) {
            new_state |= WindowState::Maximized;
        }
        XFree(prop_data);
    }

    bool max_changed = (hasWindowState(state_, WindowState::Maximized) != hasWindowState(new_state, WindowState::Maximized));
    bool state_changed = (state_ != new_state);
    state_ = new_state;

    if (max_changed) {
        on_maximized_.emit(hasWindowState(state_, WindowState::Maximized));
    }
    if (state_changed) {
        on_state_changed_.emit(state_);
    }
}

void X11Window::handleFocus(bool focused) {
    bool prev_focused = hasWindowState(state_, WindowState::Activated);
    if (focused) {
        state_ |= WindowState::Activated;
    } else {
        state_ = static_cast<WindowState>(static_cast<uint32_t>(state_) & ~static_cast<uint32_t>(WindowState::Activated));
    }
    if (prev_focused != focused) {
        on_focus_.emit(focused);
        on_state_changed_.emit(state_);
    }
}

void X11Window::handleConfigure(int nw, int nh) {
    current_width_ = nw;
    current_height_ = nh;
}

Size X11Window::getSize() const {
    return {(float)current_width_, (float)current_height_};
}
Size X11Window::getDrawableSize() const { return getSize(); }
float X11Window::getDpiScale() const { return 1.0f; }

void X11Window::makeCurrent() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE)
        eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
}

void X11Window::swapBuffers() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE)
        eglSwapBuffers(egl_display_, egl_surface_);
}

} // namespace enki::x11
