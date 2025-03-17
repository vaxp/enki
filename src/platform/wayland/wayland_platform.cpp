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
#include <sstream>

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

// ── Wayland Data Offer Wrapper (implements DataOffer) ─────────────

class WaylandDataOfferWrapper : public DataOffer {
public:
    WaylandDataOfferWrapper(wl_display* display, wl_data_offer* offer, std::vector<std::string> mimes)
        : display_(display), offer_(offer), mimes_(std::move(mimes)) {}

    [[nodiscard]] bool hasFormat(std::string_view mime_type) const override {
        for (const auto& m : mimes_) {
            if (m == mime_type) return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<std::string> formats() const override {
        return mimes_;
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
        if (!offer_ || !display_) return {};
        int fds[2];
        if (pipe2(fds, O_CLOEXEC) != 0) return {};

        wl_data_offer_receive(offer_, std::string(mime_type).c_str(), fds[1]);
        close(fds[1]);
        wl_display_flush(display_);

        std::vector<uint8_t> result;
        char buffer[4096];
        ssize_t bytes_read = 0;
        while ((bytes_read = read(fds[0], buffer, sizeof(buffer))) > 0) {
            result.insert(result.end(), buffer, buffer + bytes_read);
        }
        close(fds[0]);
        return result;
    }

private:
    wl_display*    display_ = nullptr;
    wl_data_offer* offer_   = nullptr;
    std::vector<std::string> mimes_;
};

// ── Wayland Data Offer Listeners ──────────────────────────────────

static void data_offer_offer_handler(void* data, wl_data_offer* /*offer*/, const char* mime_type) {
    auto* ctx = static_cast<WaylandPlatformBackend::WaylandOfferContext*>(data);
    if (ctx && mime_type) {
        ctx->mime_types.emplace_back(mime_type);
    }
}

static void data_offer_source_actions_handler(void* data, wl_data_offer* /*offer*/, uint32_t source_actions) {
    auto* ctx = static_cast<WaylandPlatformBackend::WaylandOfferContext*>(data);
    if (ctx) ctx->source_actions = source_actions;
}

static void data_offer_action_handler(void* data, wl_data_offer* /*offer*/, uint32_t dnd_action) {
    auto* ctx = static_cast<WaylandPlatformBackend::WaylandOfferContext*>(data);
    if (ctx) ctx->dnd_action = dnd_action;
}

static const struct wl_data_offer_listener data_offer_listener = {
    .offer = data_offer_offer_handler,
    .source_actions = data_offer_source_actions_handler,
    .action = data_offer_action_handler,
};

// ── Wayland Data Source Listeners ─────────────────────────────────

static void data_source_target_handler(void* /*data*/, wl_data_source* /*source*/, const char* /*mime_type*/) {}

static void data_source_send_handler(void* data, wl_data_source* /*source*/, const char* mime_type, int32_t fd) {
    auto* active_src = static_cast<WaylandPlatformBackend::ActiveDataSource*>(data);
    if (active_src && mime_type) {
        auto raw = active_src->data.getRaw(mime_type);
        if (raw.empty() && active_src->data.hasText()) {
            std::string t = active_src->data.getText();
            raw.assign(t.begin(), t.end());
        }
        if (!raw.empty()) {
            size_t total = 0;
            while (total < raw.size()) {
                ssize_t written = write(fd, raw.data() + total, raw.size() - total);
                if (written <= 0) break;
                total += written;
            }
        }
    }
    close(fd);
}

static void data_source_cancelled_handler(void* /*data*/, wl_data_source* source) {
    wl_data_source_destroy(source);
}

static void data_source_dnd_drop_performed_handler(void* /*data*/, wl_data_source* /*source*/) {}
static void data_source_dnd_finished_handler(void* /*data*/, wl_data_source* /*source*/) {}
static void data_source_action_handler(void* /*data*/, wl_data_source* /*source*/, uint32_t /*dnd_action*/) {}

static const struct wl_data_source_listener data_source_listener = {
    .target = data_source_target_handler,
    .send = data_source_send_handler,
    .cancelled = data_source_cancelled_handler,
    .dnd_drop_performed = data_source_dnd_drop_performed_handler,
    .dnd_finished = data_source_dnd_finished_handler,
    .action = data_source_action_handler,
};

// ── Wayland Data Device Listeners ─────────────────────────────────

static void data_device_data_offer_handler(void* data, wl_data_device* /*data_device*/, wl_data_offer* offer) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataOffer(offer);
}

static void data_device_enter_handler(void* data, wl_data_device* /*data_device*/, uint32_t serial,
                                      wl_surface* surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataDeviceEnter(serial, surface, x, y, offer);
}

static void data_device_leave_handler(void* data, wl_data_device* /*data_device*/) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataDeviceLeave();
}

static void data_device_motion_handler(void* data, wl_data_device* /*data_device*/, uint32_t time,
                                       wl_fixed_t x, wl_fixed_t y) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataDeviceMotion(time, x, y);
}

static void data_device_drop_handler(void* data, wl_data_device* /*data_device*/) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataDeviceDrop();
}

static void data_device_selection_handler(void* data, wl_data_device* /*data_device*/, wl_data_offer* offer) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleDataDeviceSelection(offer);
}

static const struct wl_data_device_listener data_device_listener = {
    .data_offer = data_device_data_offer_handler,
    .enter = data_device_enter_handler,
    .leave = data_device_leave_handler,
    .motion = data_device_motion_handler,
    .drop = data_device_drop_handler,
    .selection = data_device_selection_handler,
};

// ── Wayland Toplevel Implementation ───────────────────────────────

class WaylandPlatformBackend::WaylandToplevel : public ToplevelWindow {
public:
    WaylandToplevel(zwlr_foreign_toplevel_handle_v1* handle, wl_seat* seat)
        : handle_(handle), seat_(seat) {}

    ~WaylandToplevel() override {
        if (handle_) {
            zwlr_foreign_toplevel_handle_v1_destroy(handle_);
            handle_ = nullptr;
        }
    }

    [[nodiscard]] uint64_t id() const override { return reinterpret_cast<uint64_t>(handle_); }
    [[nodiscard]] std::string title() const override { return title_; }
    [[nodiscard]] std::string appId() const override { return app_id_; }
    [[nodiscard]] WindowState state() const override { return state_; }

    void setTitle(std::string t) { title_ = std::move(t); }
    void setAppId(std::string a) { app_id_ = std::move(a); }
    void setState(WindowState s) { state_ = s; }

    void activate() override {
        if (handle_ && seat_) {
            zwlr_foreign_toplevel_handle_v1_activate(handle_, seat_);
        }
    }

    void setMinimized(bool min) override {
        if (!handle_) return;
        if (min) {
            zwlr_foreign_toplevel_handle_v1_set_minimized(handle_);
        } else {
            zwlr_foreign_toplevel_handle_v1_unset_minimized(handle_);
        }
    }

    void setMaximized(bool max) override {
        if (!handle_) return;
        if (max) {
            zwlr_foreign_toplevel_handle_v1_set_maximized(handle_);
        } else {
            zwlr_foreign_toplevel_handle_v1_unset_maximized(handle_);
        }
    }

    void setFullscreen(bool full) override {
        if (!handle_) return;
        if (full) {
            zwlr_foreign_toplevel_handle_v1_set_fullscreen(handle_, nullptr);
        } else {
            zwlr_foreign_toplevel_handle_v1_unset_fullscreen(handle_);
        }
    }

    void close() override {
        if (handle_) {
            zwlr_foreign_toplevel_handle_v1_close(handle_);
        }
    }

    [[nodiscard]] zwlr_foreign_toplevel_handle_v1* getHandle() const { return handle_; }
    void detachHandle() { handle_ = nullptr; }

private:
    zwlr_foreign_toplevel_handle_v1* handle_ = nullptr;
    wl_seat* seat_ = nullptr;
    std::string title_;
    std::string app_id_;
    WindowState state_ = WindowState::Normal;
};

// ── Foreign Toplevel Listeners ────────────────────────────────────

static void toplevel_handle_title(void* data, zwlr_foreign_toplevel_handle_v1* handle, const char* title) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelTitle(handle, title);
}

static void toplevel_handle_app_id(void* data, zwlr_foreign_toplevel_handle_v1* handle, const char* app_id) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelAppId(handle, app_id);
}

static void toplevel_handle_output_enter(void* /*data*/, zwlr_foreign_toplevel_handle_v1* /*handle*/, wl_output* /*output*/) {}
static void toplevel_handle_output_leave(void* /*data*/, zwlr_foreign_toplevel_handle_v1* /*handle*/, wl_output* /*output*/) {}

static void toplevel_handle_state(void* data, zwlr_foreign_toplevel_handle_v1* handle, wl_array* state) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelState(handle, state);
}

static void toplevel_handle_done(void* data, zwlr_foreign_toplevel_handle_v1* handle) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelDone(handle);
}

static void toplevel_handle_closed(void* data, zwlr_foreign_toplevel_handle_v1* handle) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelClosed(handle);
}

static void toplevel_handle_parent(void* /*data*/, zwlr_foreign_toplevel_handle_v1* /*handle*/, zwlr_foreign_toplevel_handle_v1* /*parent*/) {}

static const struct zwlr_foreign_toplevel_handle_v1_listener toplevel_handle_listener = {
    .title = toplevel_handle_title,
    .app_id = toplevel_handle_app_id,
    .output_enter = toplevel_handle_output_enter,
    .output_leave = toplevel_handle_output_leave,
    .state = toplevel_handle_state,
    .done = toplevel_handle_done,
    .closed = toplevel_handle_closed,
    .parent = toplevel_handle_parent,
};

static void toplevel_manager_toplevel(void* data, zwlr_foreign_toplevel_manager_v1* /*manager*/, zwlr_foreign_toplevel_handle_v1* handle) {
    auto* self = static_cast<WaylandPlatformBackend*>(data);
    self->handleToplevelHandle(handle);
}

static void toplevel_manager_finished(void* /*data*/, zwlr_foreign_toplevel_manager_v1* /*manager*/) {}

static const struct zwlr_foreign_toplevel_manager_v1_listener toplevel_manager_listener = {
    .toplevel = toplevel_manager_toplevel,
    .finished = toplevel_manager_finished,
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

    if (data_device_manager_ && seat_ && !data_device_) {
        data_device_ = wl_data_device_manager_get_data_device(data_device_manager_, seat_);
        wl_data_device_add_listener(data_device_, &data_device_listener, this);
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
    if (outgoing_selection_source_) {
        wl_data_source_destroy(outgoing_selection_source_->source);
        outgoing_selection_source_.reset();
    }
    if (outgoing_dnd_source_) {
        wl_data_source_destroy(outgoing_dnd_source_->source);
        outgoing_dnd_source_.reset();
    }
    if (data_device_) {
        wl_data_device_destroy(data_device_);
        data_device_ = nullptr;
    }
    if (data_device_manager_) {
        wl_data_device_manager_destroy(data_device_manager_);
        data_device_manager_ = nullptr;
    }

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
    if (toplevel_manager_) {
        zwlr_foreign_toplevel_manager_v1_stop(toplevel_manager_);
        zwlr_foreign_toplevel_manager_v1_destroy(toplevel_manager_);
        toplevel_manager_ = nullptr;
    }
    toplevels_.clear();
    toplevel_map_.clear();
    active_toplevel_.reset();

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
        if (data_device_manager_ && !data_device_) {
            data_device_ = wl_data_device_manager_get_data_device(data_device_manager_, seat_);
            wl_data_device_add_listener(data_device_, &data_device_listener, this);
        }
    } else if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0) {
        data_device_manager_ = static_cast<wl_data_device_manager*>(
            wl_registry_bind(registry_, name, &wl_data_device_manager_interface, std::min<uint32_t>(version, 3)));
        if (seat_ && !data_device_) {
            data_device_ = wl_data_device_manager_get_data_device(data_device_manager_, seat_);
            wl_data_device_add_listener(data_device_, &data_device_listener, this);
        }
    } else if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        layer_shell_ = static_cast<zwlr_layer_shell_v1*>(
            wl_registry_bind(registry_, name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(version, 4)));
    } else if (std::strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        toplevel_manager_ = static_cast<zwlr_foreign_toplevel_manager_v1*>(
            wl_registry_bind(registry_, name, &zwlr_foreign_toplevel_manager_v1_interface, std::min<uint32_t>(version, 3)));
        zwlr_foreign_toplevel_manager_v1_add_listener(toplevel_manager_, &toplevel_manager_listener, this);
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

// ── Wayland Data Device, Clipboard & DnD Subsystem ───────────────

void WaylandPlatformBackend::handleDataOffer(wl_data_offer* offer) {
    if (!offer) return;
    auto ctx = std::make_shared<WaylandOfferContext>();
    ctx->offer = offer;
    active_offers_[offer] = ctx;
    wl_data_offer_add_listener(offer, &data_offer_listener, ctx.get());
}

void WaylandPlatformBackend::handleDataDeviceEnter(uint32_t serial, wl_surface* /*surface*/, wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer) {
    last_dnd_serial_ = serial;
    if (offer && active_offers_.find(offer) != active_offers_.end()) {
        current_dnd_offer_ = active_offers_[offer];
        wl_data_offer_set_actions(offer,
                                  WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                                  WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
        for (const auto& mime : current_dnd_offer_->mime_types) {
            wl_data_offer_accept(offer, serial, mime.c_str());
            break;
        }
    }

    if (owner_) {
        DragEnterEvent ev;
        ev.position = Point{static_cast<float>(wl_fixed_to_double(x)), static_cast<float>(wl_fixed_to_double(y))};
        if (current_dnd_offer_) {
            ev.mime_types = current_dnd_offer_->mime_types;
        }
        ev.suggested_action = DragAction::Copy;
        owner_->onDragEnter().emit(ev);
    }
}

void WaylandPlatformBackend::handleDataDeviceLeave() {
    current_dnd_offer_.reset();
    if (owner_) {
        DragLeaveEvent ev;
        owner_->onDragLeave().emit(ev);
    }
}

void WaylandPlatformBackend::handleDataDeviceMotion(uint32_t /*time*/, wl_fixed_t x, wl_fixed_t y) {
    if (owner_) {
        DragMotionEvent ev;
        ev.position = Point{static_cast<float>(wl_fixed_to_double(x)), static_cast<float>(wl_fixed_to_double(y))};
        ev.suggested_action = DragAction::Copy;
        owner_->onDragMotion().emit(ev);
    }
}

void WaylandPlatformBackend::handleDataDeviceDrop() {
    if (owner_ && current_dnd_offer_) {
        DropEvent ev;
        ev.position = Point{last_px_, last_py_};
        ev.action = DragAction::Copy;
        ev.data = std::make_shared<WaylandDataOfferWrapper>(display_, current_dnd_offer_->offer, current_dnd_offer_->mime_types);
        owner_->onDrop().emit(ev);

        if (current_dnd_offer_->offer) {
            wl_data_offer_finish(current_dnd_offer_->offer);
        }
    }
    current_dnd_offer_.reset();
}

void WaylandPlatformBackend::handleDataDeviceSelection(wl_data_offer* offer) {
    if (offer && active_offers_.find(offer) != active_offers_.end()) {
        current_selection_offer_ = active_offers_[offer];
    } else {
        current_selection_offer_.reset();
    }
    if (owner_) {
        owner_->onClipboardChanged().emit(ClipboardType::Clipboard);
    }
}

void WaylandPlatformBackend::setClipboardData(const ClipboardData& data, ClipboardType type) {
    if (type == ClipboardType::Primary) {
        local_primary_ = data;
        return;
    }

    local_clipboard_ = data;

    if (!data_device_manager_ || !data_device_) return;

    if (outgoing_selection_source_) {
        wl_data_source_destroy(outgoing_selection_source_->source);
        outgoing_selection_source_.reset();
    }

    auto src_obj = wl_data_device_manager_create_data_source(data_device_manager_);
    if (!src_obj) return;

    outgoing_selection_source_ = std::make_unique<ActiveDataSource>();
    outgoing_selection_source_->source = src_obj;
    outgoing_selection_source_->data = data;

    wl_data_source_add_listener(src_obj, &data_source_listener, outgoing_selection_source_.get());

    for (const auto& mime : data.formats()) {
        wl_data_source_offer(src_obj, mime.c_str());
    }

    uint32_t serial = last_pointer_serial_ ? last_pointer_serial_ : last_keyboard_serial_;
    wl_data_device_set_selection(data_device_, src_obj, serial);
    wl_display_flush(display_);
}

ClipboardData WaylandPlatformBackend::getClipboardData(ClipboardType type) const {
    if (type == ClipboardType::Primary) {
        return local_primary_;
    }

    if (current_selection_offer_ && display_) {
        ClipboardData result;
        for (const auto& mime : current_selection_offer_->mime_types) {
            auto raw = const_cast<WaylandPlatformBackend*>(this)->getClipboardDataForMime(mime, type);
            if (!raw.empty()) {
                result.setRaw(mime, raw);
            }
        }
        if (!result.empty()) return result;
    }
    return local_clipboard_;
}

std::vector<uint8_t> WaylandPlatformBackend::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
    if (type == ClipboardType::Primary) {
        return local_primary_.getRaw(mime_type);
    }

    if (current_selection_offer_ && current_selection_offer_->offer && display_) {
        bool format_supported = false;
        for (const auto& m : current_selection_offer_->mime_types) {
            if (m == mime_type) {
                format_supported = true;
                break;
            }
        }
        if (format_supported) {
            int fds[2];
            if (pipe2(fds, O_CLOEXEC) == 0) {
                wl_data_offer_receive(current_selection_offer_->offer, std::string(mime_type).c_str(), fds[1]);
                close(fds[1]);
                wl_display_flush(display_);

                std::vector<uint8_t> result;
                char buffer[4096];
                ssize_t bytes_read = 0;
                while ((bytes_read = read(fds[0], buffer, sizeof(buffer))) > 0) {
                    result.insert(result.end(), buffer, buffer + bytes_read);
                }
                close(fds[0]);
                return result;
            }
        }
    }
    return local_clipboard_.getRaw(mime_type);
}

std::vector<std::string> WaylandPlatformBackend::getClipboardFormats(ClipboardType type) const {
    if (type == ClipboardType::Primary) {
        return local_primary_.formats();
    }
    if (current_selection_offer_ && !current_selection_offer_->mime_types.empty()) {
        return current_selection_offer_->mime_types;
    }
    return local_clipboard_.formats();
}

bool WaylandPlatformBackend::hasClipboardFormat(std::string_view mime_type, ClipboardType type) const {
    if (type == ClipboardType::Primary) {
        return local_primary_.hasFormat(mime_type);
    }
    if (current_selection_offer_) {
        for (const auto& m : current_selection_offer_->mime_types) {
            if (m == mime_type) return true;
        }
    }
    return local_clipboard_.hasFormat(mime_type);
}

bool WaylandPlatformBackend::startDrag(const DragData& data, DragAction actions) {
    if (!data_device_manager_ || !data_device_ || !pointer_focus_surface_) {
        return false;
    }

    if (outgoing_dnd_source_) {
        wl_data_source_destroy(outgoing_dnd_source_->source);
        outgoing_dnd_source_.reset();
    }

    auto src_obj = wl_data_device_manager_create_data_source(data_device_manager_);
    if (!src_obj) return false;

    outgoing_dnd_source_ = std::make_unique<ActiveDataSource>();
    outgoing_dnd_source_->source = src_obj;
    outgoing_dnd_source_->data = data.payload;

    wl_data_source_add_listener(src_obj, &data_source_listener, outgoing_dnd_source_.get());

    for (const auto& mime : data.payload.formats()) {
        wl_data_source_offer(src_obj, mime.c_str());
    }

    uint32_t wl_actions = 0;
    if (hasDragAction(actions, DragAction::Copy)) wl_actions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    if (hasDragAction(actions, DragAction::Move)) wl_actions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    wl_data_source_set_actions(src_obj, wl_actions);

    uint32_t serial = last_pointer_serial_ ? last_pointer_serial_ : last_keyboard_serial_;
    wl_data_device_start_drag(data_device_, src_obj, pointer_focus_surface_, nullptr /* icon */, serial);
    wl_display_flush(display_);
    return true;
}

// ── Foreign Toplevel Subsystem Implementation ─────────────────────

std::vector<std::shared_ptr<ToplevelWindow>> WaylandPlatformBackend::getToplevels() const {
    std::vector<std::shared_ptr<ToplevelWindow>> result;
    result.reserve(toplevels_.size());
    for (const auto& tl : toplevels_) {
        result.push_back(tl);
    }
    return result;
}

std::shared_ptr<ToplevelWindow> WaylandPlatformBackend::getActiveToplevel() const {
    return active_toplevel_;
}

void WaylandPlatformBackend::handleToplevelHandle(zwlr_foreign_toplevel_handle_v1* handle) {
    if (!handle) return;
    auto tl = std::make_shared<WaylandToplevel>(handle, seat_);
    toplevel_map_[handle] = tl;
    toplevels_.push_back(tl);
    zwlr_foreign_toplevel_handle_v1_add_listener(handle, &toplevel_handle_listener, this);
}

void WaylandPlatformBackend::handleToplevelTitle(zwlr_foreign_toplevel_handle_v1* handle, const char* title) {
    auto it = toplevel_map_.find(handle);
    if (it != toplevel_map_.end()) {
        it->second->setTitle(title ? title : "");
        if (owner_) {
            owner_->onToplevelTitleChanged().emit(it->second, it->second->title());
        }
    }
}

void WaylandPlatformBackend::handleToplevelAppId(zwlr_foreign_toplevel_handle_v1* handle, const char* app_id) {
    auto it = toplevel_map_.find(handle);
    if (it != toplevel_map_.end()) {
        it->second->setAppId(app_id ? app_id : "");
        if (owner_) {
            owner_->onToplevelAppIdChanged().emit(it->second, it->second->appId());
        }
    }
}

void WaylandPlatformBackend::handleToplevelState(zwlr_foreign_toplevel_handle_v1* handle, wl_array* state) {
    auto it = toplevel_map_.find(handle);
    if (it != toplevel_map_.end()) {
        WindowState s = WindowState::Normal;
        if (state && state->data && state->size >= sizeof(uint32_t)) {
            const auto* entries = static_cast<const uint32_t*>(state->data);
            size_t count = state->size / sizeof(uint32_t);
            for (size_t i = 0; i < count; ++i) {
                uint32_t entry = entries[i];
                if (entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED) {
                    s |= WindowState::Maximized;
                } else if (entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
                    s |= WindowState::Minimized;
                } else if (entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
                    s |= WindowState::Activated;
                } else if (entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN) {
                    s |= WindowState::Fullscreen;
                }
            }
        }
        it->second->setState(s);
        if (hasWindowState(s, WindowState::Activated)) {
            active_toplevel_ = it->second;
            if (owner_) {
                owner_->onActiveToplevelChanged().emit(it->second);
            }
        } else if (active_toplevel_ == it->second) {
            active_toplevel_.reset();
        }
        if (owner_) {
            owner_->onToplevelStateChanged().emit(it->second, s);
        }
    }
}

void WaylandPlatformBackend::handleToplevelDone(zwlr_foreign_toplevel_handle_v1* handle) {
    auto it = toplevel_map_.find(handle);
    if (it != toplevel_map_.end()) {
        if (owner_) {
            owner_->onToplevelCreated().emit(it->second);
        }
    }
}

void WaylandPlatformBackend::handleToplevelClosed(zwlr_foreign_toplevel_handle_v1* handle) {
    auto it = toplevel_map_.find(handle);
    if (it != toplevel_map_.end()) {
        auto tl = it->second;
        tl->detachHandle();
        if (active_toplevel_ == tl) {
            active_toplevel_.reset();
            if (owner_) {
                owner_->onActiveToplevelChanged().emit(nullptr);
            }
        }
        for (auto v_it = toplevels_.begin(); v_it != toplevels_.end(); ++v_it) {
            if (*v_it == tl) {
                toplevels_.erase(v_it);
                break;
            }
        }
        toplevel_map_.erase(it);
        if (owner_) {
            owner_->onToplevelClosed().emit(tl);
        }
    }
}

} // namespace enki::wayland
