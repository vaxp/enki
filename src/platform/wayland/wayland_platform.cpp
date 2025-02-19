/// @file wayland_platform.cpp
/// @brief Native Wayland Client Platform backend implementation with wlr-layer-shell & EGL.

#include "enki/platform/wayland/wayland_platform.hpp"
#include "enki/platform/wayland/wayland_surface.hpp"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include <cstring>

namespace enki::wayland {

// ── Wayland Registry Listeners ───────────────────────────────────

static void registry_global_handler(void* data, wl_registry* registry, uint32_t name,
                                   const char* interface, uint32_t version) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleGlobal(name, interface, version);
}

static void registry_global_remove_handler(void* data, wl_registry* registry, uint32_t name) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleGlobalRemove(name);
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler,
};

// ── Wayland Pointer Listeners ────────────────────────────────────

static void pointer_enter_handler(void* data, wl_pointer* pointer, uint32_t serial,
                                  wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handlePointerEnter(serial, surface, sx, sy);
}

static void pointer_leave_handler(void* data, wl_pointer* pointer, uint32_t serial,
                                 wl_surface* surface) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handlePointerLeave(surface);
}

static void pointer_motion_handler(void* data, wl_pointer* pointer, uint32_t time,
                                  wl_fixed_t sx, wl_fixed_t sy) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handlePointerMotion(time, sx, sy);
}

static void pointer_button_handler(void* data, wl_pointer* pointer, uint32_t serial,
                                  uint32_t time, uint32_t button, uint32_t state) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handlePointerButton(serial, time, button, state);
}

static void pointer_axis_handler(void* data, wl_pointer* pointer, uint32_t time,
                                uint32_t axis, wl_fixed_t value) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handlePointerAxis(time, axis, value);
}

static void pointer_frame_handler(void* data, wl_pointer* pointer) {}
static void pointer_axis_source_handler(void* data, wl_pointer* pointer, uint32_t axis_source) {}
static void pointer_axis_stop_handler(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis) {}
static void pointer_axis_discrete_handler(void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete) {}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter_handler,
    .leave = pointer_leave_handler,
    .motion = pointer_motion_handler,
    .button = pointer_button_handler,
    .axis = pointer_axis_handler,
    .frame = pointer_frame_handler,
    .axis_source = pointer_axis_source_handler,
    .axis_stop = pointer_axis_stop_handler,
    .axis_discrete = pointer_axis_discrete_handler,
};

// ── Wayland Keyboard Listeners ───────────────────────────────────

static void keyboard_keymap_handler(void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleKeymap(format, fd, size);
    close(fd);
}

static void keyboard_enter_handler(void* data, wl_keyboard* keyboard, uint32_t serial,
                                  wl_surface* surface, wl_array* keys) {}
static void keyboard_leave_handler(void* data, wl_keyboard* keyboard, uint32_t serial,
                                  wl_surface* surface) {}

static void keyboard_key_handler(void* data, wl_keyboard* keyboard, uint32_t serial,
                                uint32_t time, uint32_t key, uint32_t state) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleKey(serial, time, key, state);
}

static void keyboard_modifiers_handler(void* data, wl_keyboard* keyboard, uint32_t serial,
                                      uint32_t mods_depressed, uint32_t mods_latched,
                                      uint32_t mods_locked, uint32_t group) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleModifiers(mods_depressed, mods_latched, mods_locked, group);
}

static void keyboard_repeat_info_handler(void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay) {}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap_handler,
    .enter = keyboard_enter_handler,
    .leave = keyboard_leave_handler,
    .key = keyboard_key_handler,
    .modifiers = keyboard_modifiers_handler,
    .repeat_info = keyboard_repeat_info_handler,
};

// ── Wayland Seat Listeners ───────────────────────────────────────

static void seat_capabilities_handler(void* data, wl_seat* seat, uint32_t caps) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleSeatCapabilities(seat, caps);
}

static void seat_name_handler(void* data, wl_seat* seat, const char* name) {}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities_handler,
    .name = seat_name_handler,
};

void WaylandPlatformBackend::handleSeatCapabilities(wl_seat* seat, uint32_t caps) {
    if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
        if (!pointer_) {
            pointer_ = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(pointer_, &pointer_listener, this);
        }
    } else if (pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
        if (!keyboard_) {
            keyboard_ = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(keyboard_, &keyboard_listener, this);
        }
    } else if (keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
    }
}

// ════════════════════════════════════════════════════════════════
// WaylandPlatformBackend Implementation
// ════════════════════════════════════════════════════════════════

WaylandPlatformBackend::WaylandPlatformBackend(Platform* owner)
    : owner_(owner) {
    xkb_context_ = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

WaylandPlatformBackend::~WaylandPlatformBackend() {
    shutdown();
    if (xkb_state_)   xkb_state_unref(xkb_state_);
    if (xkb_keymap_)  xkb_keymap_unref(xkb_keymap_);
    if (xkb_context_) xkb_context_unref(xkb_context_);
}

bool WaylandPlatformBackend::init() {
    // 1. Connect to Wayland display socket
    display_ = wl_display_connect(nullptr);
    if (!display_) {
        std::cerr << "[ENKI Wayland] Could not connect to Wayland display server\n";
        return false;
    }

    // 2. Fetch registry globals
    registry_ = wl_display_get_registry(display_);
    wl_registry_add_listener(registry_, &registry_listener, this);

    // Initial roundtrip to populate registry globals
    wl_display_roundtrip(display_);

    if (!compositor_) {
        std::cerr << "[ENKI Wayland] Missing required wl_compositor interface\n";
        return false;
    }

    if (shm_) {
        const char* theme_name = std::getenv("XCURSOR_THEME");
        int size = 24;
        if (const char* size_env = std::getenv("XCURSOR_SIZE")) {
            int s = std::atoi(size_env);
            if (s > 0) size = s;
        }
        cursor_theme_ = wl_cursor_theme_load(theme_name, size, shm_);
    }

    if (compositor_) {
        cursor_surface_ = wl_compositor_create_surface(compositor_);
    }

    if (!layer_shell_) {
        std::cerr << "[ENKI Wayland] zwlr_layer_shell_v1 not available on this compositor\n";
    }

    // 3. Initialize EGL for Wayland
    egl_display_ = eglGetDisplay((EGLNativeDisplayType)display_);
    if (egl_display_ == EGL_NO_DISPLAY) {
        std::cerr << "[ENKI Wayland] Failed to get EGL display for Wayland\n";
        return false;
    }

    if (!eglInitialize(egl_display_, &egl_major_, &egl_minor_)) {
        std::cerr << "[ENKI Wayland] Failed to initialize EGL on Wayland\n";
        return false;
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        eglBindAPI(EGL_OPENGL_ES_API);
    }

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_STENCIL_SIZE,    8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;
    if (!eglChooseConfig(egl_display_, config_attribs, &egl_config_, 1, &num_configs) || num_configs == 0) {
        const EGLint fallback_attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE,     8,
            EGL_GREEN_SIZE,   8,
            EGL_BLUE_SIZE,    8,
            EGL_ALPHA_SIZE,   8,
            EGL_NONE
        };
        eglChooseConfig(egl_display_, fallback_attribs, &egl_config_, 1, &num_configs);
    }

    return true;
}

void WaylandPlatformBackend::shutdown() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
    }

    if (cursor_surface_) {
        wl_surface_destroy(cursor_surface_);
        cursor_surface_ = nullptr;
    }
    if (cursor_theme_) {
        wl_cursor_theme_destroy(cursor_theme_);
        cursor_theme_ = nullptr;
    }
    if (pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }
    if (keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
    }

    if (layer_shell_) {
        zwlr_layer_shell_v1_destroy(layer_shell_);
        layer_shell_ = nullptr;
    }
    if (xdg_wm_base_) {
        xdg_wm_base_destroy(xdg_wm_base_);
        xdg_wm_base_ = nullptr;
    }
    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }
    if (registry_) {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }
    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }
}

// ── XDG Shell Base Listener (Ping/Pong) ───────────────────────────

static void xdg_wm_base_ping_handler(void* /*data*/, xdg_wm_base* wm_base, uint32_t serial) {
    xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
    .ping = xdg_wm_base_ping_handler,
};

void WaylandPlatformBackend::handleGlobal(uint32_t name, const char* interface, uint32_t version) {
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry_, name, &wl_compositor_interface, std::min<uint32_t>(version, 4)));
    } else if (std::strcmp(interface, wl_subcompositor_interface.name) == 0) {
        subcompositor_ = static_cast<wl_subcompositor*>(
            wl_registry_bind(registry_, name, &wl_subcompositor_interface, 1));
    } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        shm_ = static_cast<wl_shm*>(
            wl_registry_bind(registry_, name, &wl_shm_interface, 1));
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        seat_ = static_cast<wl_seat*>(
            wl_registry_bind(registry_, name, &wl_seat_interface, std::min<uint32_t>(version, 5)));
        wl_seat_add_listener(seat_, &seat_listener, this);
    } else if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        layer_shell_ = static_cast<zwlr_layer_shell_v1*>(
            wl_registry_bind(registry_, name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(version, 4)));
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_wm_base_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry_, name, &xdg_wm_base_interface, std::min<uint32_t>(version, 3)));
        xdg_wm_base_add_listener(xdg_wm_base_, &wm_base_listener, this);
    } else if (std::strcmp(interface, wl_output_interface.name) == 0) {
        auto* output = static_cast<wl_output*>(
            wl_registry_bind(registry_, name, &wl_output_interface, std::min<uint32_t>(version, 3)));
        outputs_.push_back(WaylandOutput{output, name, 0, 0, 0, 0, 1, ""});
    }
}

void WaylandPlatformBackend::handleGlobalRemove(uint32_t name) {
    for (auto it = outputs_.begin(); it != outputs_.end(); ++it) {
        if (it->id == name) {
            wl_output_destroy(it->output);
            outputs_.erase(it);
            break;
        }
    }
}

bool WaylandPlatformBackend::pollEvents() {
    if (!display_) return false;

    // 1. Process pending incoming events
    while (wl_display_prepare_read(display_) != 0) {
        wl_display_dispatch_pending(display_);
    }

    wl_display_flush(display_);

    // 2. Poll socket without blocking
    struct pollfd pfd;
    pfd.fd     = wl_display_get_fd(display_);
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, 0); // Non-blocking
    if (ret > 0 && (pfd.revents & POLLIN)) {
        wl_display_read_events(display_);
    } else {
        wl_display_cancel_read(display_);
    }

    wl_display_dispatch_pending(display_);
    return true;
}

void WaylandPlatformBackend::registerSurface(LayerSurface* surface) {
    if (surface) surfaces_.insert(surface);
}

void WaylandPlatformBackend::unregisterSurface(LayerSurface* surface) {
    if (surface) surfaces_.erase(surface);
}

void WaylandPlatformBackend::setCursor(SystemCursor cursor) {
    if (current_cursor_ == cursor) return;
    current_cursor_ = cursor;
    updateWaylandCursor();
}

void WaylandPlatformBackend::updateWaylandCursor() {
    if (!pointer_ || !cursor_surface_ || !cursor_theme_ || !pointer_focus_surface_ || last_pointer_serial_ == 0) {
        return;
    }

    const char* names[4] = { "default", "left_ptr", nullptr, nullptr };
    switch (current_cursor_) {
        case SystemCursor::Pointer:
            names[0] = "pointer";
            names[1] = "hand2";
            names[2] = "hand";
            break;
        case SystemCursor::Text:
            names[0] = "text";
            names[1] = "xterm";
            names[2] = "ibeam";
            break;
        case SystemCursor::Crosshair:
            names[0] = "crosshair";
            break;
        case SystemCursor::Move:
            names[0] = "move";
            names[1] = "grab";
            break;
        case SystemCursor::NotAllowed:
            names[0] = "not-allowed";
            names[1] = "forbidden";
            break;
        case SystemCursor::ResizeHorizontal:
            names[0] = "ew-resize";
            names[1] = "h_double_arrow";
            break;
        case SystemCursor::ResizeVertical:
            names[0] = "ns-resize";
            names[1] = "v_double_arrow";
            break;
        case SystemCursor::Wait:
            names[0] = "wait";
            names[1] = "watch";
            break;
        case SystemCursor::Default:
        case SystemCursor::Arrow:
        default:
            names[0] = "default";
            names[1] = "left_ptr";
            names[2] = "arrow";
            break;
    }

    wl_cursor* cur = nullptr;
    for (int i = 0; i < 3 && names[i]; ++i) {
        cur = wl_cursor_theme_get_cursor(cursor_theme_, names[i]);
        if (cur) break;
    }
    if (!cur) {
        cur = wl_cursor_theme_get_cursor(cursor_theme_, "left_ptr");
    }
    if (!cur) {
        cur = wl_cursor_theme_get_cursor(cursor_theme_, "default");
    }
    if (!cur || cur->image_count == 0) return;

    wl_cursor_image* img = cur->images[0];
    wl_buffer* buf = wl_cursor_image_get_buffer(img);
    if (!buf) return;

    wl_pointer_set_cursor(pointer_, last_pointer_serial_, cursor_surface_, img->hotspot_x, img->hotspot_y);
    wl_surface_attach(cursor_surface_, buf, 0, 0);
    wl_surface_damage(cursor_surface_, 0, 0, img->width, img->height);
    wl_surface_commit(cursor_surface_);
}

// ── Event Forwarding to Enki Platform Signals ──────────────────

void WaylandPlatformBackend::handlePointerEnter(uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {
    pointer_focus_surface_ = surface;
    last_pointer_serial_ = serial;
    last_px_ = wl_fixed_to_double(sx);
    last_py_ = wl_fixed_to_double(sy);

    updateWaylandCursor();

    if (owner_) {
        owner_->onMouseMove().emit(last_px_, last_py_);
    }
}

void WaylandPlatformBackend::handlePointerLeave(wl_surface* surface) {
    if (pointer_focus_surface_ == surface) {
        pointer_focus_surface_ = nullptr;
    }
}

void WaylandPlatformBackend::handlePointerMotion(uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    last_px_ = wl_fixed_to_double(sx);
    last_py_ = wl_fixed_to_double(sy);
    if (owner_) {
        owner_->onMouseMove().emit(last_px_, last_py_);
    }
}

void WaylandPlatformBackend::handlePointerButton(uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    if (!owner_) return;

    // Linux button codes: BTN_LEFT (0x110/272)=1, BTN_RIGHT (0x111/273)=3, BTN_MIDDLE (0x112/274)=2
    int btn_code = 1;
    if (button == 273) btn_code = 3;       // Right
    else if (button == 274) btn_code = 2;  // Middle

    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        owner_->onMouseDown().emit(last_px_, last_py_, btn_code);
    } else {
        owner_->onMouseUp().emit(last_px_, last_py_, btn_code);
    }
}

void WaylandPlatformBackend::handlePointerAxis(uint32_t time, uint32_t axis, wl_fixed_t value) {
    if (!owner_) return;
    double v = wl_fixed_to_double(value);
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        owner_->onScroll().emit(0.0f, v < 0 ? 1.0f : -1.0f);
    } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        owner_->onScroll().emit(v < 0 ? -1.0f : 1.0f, 0.0f);
    }
}

void WaylandPlatformBackend::handleKeymap(uint32_t format, int fd, uint32_t size) {
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) return;

    char* map_shm = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if (map_shm == MAP_FAILED) return;

    if (xkb_keymap_) xkb_keymap_unref(xkb_keymap_);
    xkb_keymap_ = xkb_keymap_new_from_string(xkb_context_, map_shm,
                                            XKB_KEYMAP_FORMAT_TEXT_V1,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);

    if (xkb_state_) xkb_state_unref(xkb_state_);
    if (xkb_keymap_) {
        xkb_state_ = xkb_state_new(xkb_keymap_);
    }
}

void WaylandPlatformBackend::handleKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    if (!xkb_state_ || !owner_) return;

    uint32_t keycode = key + 8; // evdev -> XKB offset
    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state_, keycode);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        owner_->onKeyDown().emit(static_cast<int>(sym), active_modifiers_);

        char utf8_buf[64];
        int len = xkb_state_key_get_utf8(xkb_state_, keycode, utf8_buf, sizeof(utf8_buf));
        if (len > 0) {
            owner_->onTextInput().emit(std::string(utf8_buf, len));
        }
    } else {
        owner_->onKeyUp().emit(static_cast<int>(sym), active_modifiers_);
    }
}

void WaylandPlatformBackend::handleModifiers(uint32_t mods_depressed, uint32_t mods_latched,
                                            uint32_t mods_locked, uint32_t group) {
    if (!xkb_state_) return;
    xkb_state_update_mask(xkb_state_, mods_depressed, mods_latched, mods_locked, 0, 0, group);

    active_modifiers_ = 0;
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE)) active_modifiers_ |= 1;
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_CTRL,  XKB_STATE_MODS_EFFECTIVE)) active_modifiers_ |= 2;
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_ALT,   XKB_STATE_MODS_EFFECTIVE)) active_modifiers_ |= 4;
    if (xkb_state_mod_name_is_active(xkb_state_, XKB_MOD_NAME_LOGO,  XKB_STATE_MODS_EFFECTIVE)) active_modifiers_ |= 8;
}

} // namespace enki::wayland
