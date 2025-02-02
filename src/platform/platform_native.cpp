/// @file platform_native.cpp
/// @brief Platform abstraction layer — auto-detects and owns Wayland or X11 backend.
/// Downstream code (Window, LayerSurface, App) is fully backend-agnostic.

#include "enki/platform/platform.hpp"
#include "enki/platform/window.hpp"
#include "enki/platform/x11/x11_platform.hpp"
#include "enki/platform/wayland/wayland_platform.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace enki {

static Platform* g_platform_instance = nullptr;

// ════════════════════════════════════════════════════════════════
// Platform::Impl  — owns exactly one backend (Wayland XOR X11)
// ════════════════════════════════════════════════════════════════
struct Platform::Impl {
    Platform* owner = nullptr;

    std::unique_ptr<wayland::WaylandPlatformBackend> wayland;
    std::unique_ptr<x11::X11PlatformBackend>         x11;

    std::chrono::steady_clock::time_point start_time;
    std::unordered_set<Window*> windows;
    std::string clipboard_buffer;

    // ── Backend Selection ────────────────────────────────────────
    bool init() {
        start_time = std::chrono::steady_clock::now();

        // Prefer Wayland if environment indicates it
        const char* wl_disp = std::getenv("WAYLAND_DISPLAY");
        if (wl_disp || std::getenv("WAYLAND_SOCKET")) {
            wayland = std::make_unique<wayland::WaylandPlatformBackend>(owner);
            if (wayland->init()) {
                return true;
            }
            std::cerr << "[ENKI Platform] Wayland failed — falling back to X11\n";
            wayland.reset();
        }

        // X11 fallback
        x11 = std::make_unique<x11::X11PlatformBackend>(owner);
        if (!x11->init()) {
            x11.reset();
            return false;
        }
        return true;
    }

    void shutdown() {
        if (wayland) { wayland->shutdown(); wayland.reset(); }
        if (x11)     { x11->shutdown();     x11.reset(); }
    }

    bool isWayland() const { return wayland != nullptr; }
};

// ════════════════════════════════════════════════════════════════
// Platform — Public API
// ════════════════════════════════════════════════════════════════

Platform::Platform() : impl_(std::make_unique<Impl>()) {}

Platform::~Platform() {
    if (impl_) impl_->shutdown();
    if (g_platform_instance == this) g_platform_instance = nullptr;
}

Result<std::unique_ptr<Platform>> Platform::create() {
    if (g_platform_instance) {
        return Result<std::unique_ptr<Platform>>::err(
            ErrorCode::PlatformError, "Platform already exists");
    }
    auto p = std::unique_ptr<Platform>(new Platform());
    p->impl_->owner = p.get();
    if (!p->impl_->init()) {
        return Result<std::unique_ptr<Platform>>::err(
            ErrorCode::PlatformError, "Failed to initialize platform (tried Wayland, then X11)");
    }
    g_platform_instance = p.get();
    return Result<std::unique_ptr<Platform>>::ok(std::move(p));
}

Platform* Platform::instance() { return g_platform_instance; }

// ── Event Loop ──────────────────────────────────────────────────
bool Platform::pollEvents() {
    if (impl_->wayland) return impl_->wayland->pollEvents();
    if (impl_->x11)     return impl_->x11->pollEvents();
    return false;
}

// ── Window Registration ──────────────────────────────────────────
void Platform::registerWindow(Window* w) {
    if (!w) return;
    impl_->windows.insert(w);
    if (impl_->x11) impl_->x11->registerWindow(w);
}

void Platform::unregisterWindow(Window* w) {
    if (!w) return;
    impl_->windows.erase(w);
    if (impl_->x11) impl_->x11->unregisterWindow(w);
}

// ── Clipboard ────────────────────────────────────────────────────
void Platform::setClipboardText(std::string_view text) {
    impl_->clipboard_buffer = std::string(text);
    if (impl_->x11) impl_->x11->setClipboardText(impl_->clipboard_buffer);
    // Wayland clipboard via wl_data_device — future
}
std::string Platform::getClipboardText() const { return impl_->clipboard_buffer; }

// ── Timing ───────────────────────────────────────────────────────
double Platform::getTime() const {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - impl_->start_time).count();
}

// ── Backend accessors ─────────────────────────────────────────────
void* Platform::getNativeDisplay() const {
    if (impl_->wayland) return (void*)impl_->wayland->getDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getDisplay();
    return nullptr;
}
void* Platform::getEGLDisplay() const {
    if (impl_->wayland) return (void*)impl_->wayland->getEGLDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getEGLDisplay();
    return nullptr;
}
void* Platform::getEGLConfig() const {
    if (impl_->wayland) return (void*)impl_->wayland->getEGLConfig();
    if (impl_->x11)     return (void*)impl_->x11->getEGLConfig();
    return nullptr;
}

bool  Platform::isWayland()        const { return impl_->isWayland(); }
void* Platform::getWaylandBackend() const { return (void*)impl_->wayland.get(); }
void* Platform::getX11Backend()    const { return (void*)impl_->x11.get(); }

}  // namespace enki
