/// @file x11_window.cpp
/// @brief X11 + EGL window backend implementation.

#include "enki/platform/x11/x11_window.hpp"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
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
    swa.background_pixel = 0;
    swa.event_mask   = ExposureMask | StructureNotifyMask | ButtonPressMask |
                       ButtonReleaseMask | PointerMotionMask | KeyPressMask |
                       KeyReleaseMask | FocusChangeMask;

    x11_window_ = XCreateWindow(display_, root, pos_x, pos_y,
                                 current_width_, current_height_,
                                 0, depth, InputOutput, visual,
                                 CWColormap | CWBorderPixel | CWBackPixel | CWEventMask, &swa);
    if (!x11_window_) {
        std::cerr << "[ENKI X11Window] XCreateWindow failed\n";
        return false;
    }

    setTitle(cfg.title);

    // WM_DELETE_WINDOW protocol
    Atom wm_proto  = backend_.getAtomWmProtocols();
    Atom wm_delete = backend_.getAtomWmDeleteWindow();
    XSetWMProtocols(display_, x11_window_, &wm_delete, 1);

    if (cfg.borderless)    setBorderless(true);
    if (cfg.always_on_top) setAlwaysOnTop(true);

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

    // EGL context — prefer OpenGL 3.3 core, fallback GLES2, then default
    {
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
        if (egl_context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(egl_display_, egl_context_);
            egl_context_ = EGL_NO_CONTEXT;
        }
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
    h.flags = 2; h.decorations = borderless ? 0 : 1;
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
