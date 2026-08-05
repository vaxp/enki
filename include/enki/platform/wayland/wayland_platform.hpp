#pragma once
/// @file wayland_platform.hpp
/// @brief Native Wayland Client Platform backend with wlr-layer-shell, xdg-shell, and EGL.

#include "enki/platform/platform.hpp"
#include "enki/platform/layer_surface.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// Wayland protocol headers are generated as C — must be wrapped in extern "C"
// to prevent the parameter named 'namespace' from conflicting with C++ keyword.
extern "C" {
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
}

#include <vector>
#include <unordered_set>
#include <memory>
#include <string>

namespace enki::wayland {

struct WaylandOutput {
    wl_output* output = nullptr;
    uint32_t   id     = 0;
    int32_t    x      = 0;
    int32_t    y      = 0;
    int32_t    width  = 0;
    int32_t    height = 0;
    int32_t    scale  = 1;
    std::string name;
};

class WaylandPlatformBackend {
public:
    WaylandPlatformBackend(Platform* owner);
    ~WaylandPlatformBackend();

    bool init();
    void shutdown();
    bool pollEvents();

    [[nodiscard]] wl_display* getDisplay() const { return display_; }
    [[nodiscard]] wl_compositor* getCompositor() const { return compositor_; }
    [[nodiscard]] zwlr_layer_shell_v1* getLayerShell() const { return layer_shell_; }
    [[nodiscard]] xdg_wm_base* getXdgWmBase() const { return xdg_wm_base_; }
    [[nodiscard]] wl_seat* getSeat() const { return seat_; }
    [[nodiscard]] EGLDisplay getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig getEGLConfig() const { return egl_config_; }
    [[nodiscard]] const std::vector<WaylandOutput>& getOutputs() const { return outputs_; }

    void registerSurface(LayerSurface* surface);
    void unregisterSurface(LayerSurface* surface);

    // Internal registry & seat listeners
    void handleGlobal(uint32_t name, const char* interface, uint32_t version);
    void handleGlobalRemove(uint32_t name);

    // Pointer events
    void handlePointerEnter(wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
    void handlePointerLeave(wl_surface* surface);
    void handlePointerMotion(uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
    void handlePointerButton(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void handlePointerAxis(uint32_t time, uint32_t axis, wl_fixed_t value);

    // Keyboard events
    void handleKeymap(uint32_t format, int fd, uint32_t size);
    void handleKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void handleModifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

private:
    Platform* owner_ = nullptr;

    wl_display*          display_        = nullptr;
    wl_registry*         registry_       = nullptr;
    wl_compositor*       compositor_     = nullptr;
    wl_subcompositor*    subcompositor_  = nullptr;
    wl_shm*              shm_            = nullptr;
    wl_seat*             seat_           = nullptr;
    wl_pointer*          pointer_        = nullptr;
    wl_keyboard*         keyboard_       = nullptr;
    wl_touch*            touch_          = nullptr;
    zwlr_layer_shell_v1* layer_shell_    = nullptr;
    xdg_wm_base*         xdg_wm_base_    = nullptr;

    std::vector<WaylandOutput> outputs_;
    std::unordered_set<LayerSurface*> surfaces_;

    // XKB Keymap
    xkb_context* xkb_context_ = nullptr;
    xkb_keymap*  xkb_keymap_  = nullptr;
    xkb_state*   xkb_state_   = nullptr;
    int          active_modifiers_ = 0;

    // EGL
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLConfig  egl_config_  = nullptr;
    int        egl_major_   = 0;
    int        egl_minor_   = 0;

    // Active surface under pointer
    wl_surface* pointer_focus_surface_ = nullptr;
    float last_px_ = 0.0f;
    float last_py_ = 0.0f;
};

} // namespace enki::wayland
