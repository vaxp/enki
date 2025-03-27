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

// ── Clipboard Subsystem ──────────────────────────────────────
void Platform::setClipboardText(std::string_view text, ClipboardType type) {
    ClipboardData data;
    data.setText(text);
    setClipboardData(data, type);
}

std::string Platform::getClipboardText(ClipboardType type) const {
    if (impl_->wayland) return impl_->wayland->getClipboardData(type).getText();
    if (impl_->x11)     return impl_->x11->getClipboardText(type);
    return impl_->clipboard_buffer;
}

void Platform::setClipboardData(const ClipboardData& data, ClipboardType type) {
    impl_->clipboard_buffer = data.getText();
    if (impl_->wayland) impl_->wayland->setClipboardData(data, type);
    if (impl_->x11)     impl_->x11->setClipboardData(data, type);
}

ClipboardData Platform::getClipboardData(ClipboardType type) const {
    if (impl_->wayland) return impl_->wayland->getClipboardData(type);
    if (impl_->x11)     return impl_->x11->getClipboardData(type);
    ClipboardData cd;
    cd.setText(impl_->clipboard_buffer);
    return cd;
}

std::vector<uint8_t> Platform::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
    if (impl_->wayland) return impl_->wayland->getClipboardDataForMime(mime_type, type);
    if (impl_->x11)     return impl_->x11->getClipboardDataForMime(mime_type, type);
    return {};
}

std::vector<std::string> Platform::getClipboardFormats(ClipboardType type) const {
    if (impl_->wayland) return impl_->wayland->getClipboardFormats(type);
    if (impl_->x11)     return impl_->x11->getClipboardFormats(type);
    return { std::string(mime::TextPlainUtf8) };
}

bool Platform::hasClipboardFormat(std::string_view mime_type, ClipboardType type) const {
    if (impl_->wayland) return impl_->wayland->hasClipboardFormat(mime_type, type);
    if (impl_->x11)     return impl_->x11->hasClipboardFormat(mime_type, type);
    return mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain;
}

// ── Drag & Drop Subsystem ────────────────────────────────────
bool Platform::startDrag(const DragData& data, DragAction actions) {
    if (impl_->wayland) return impl_->wayland->startDrag(data, actions);
    if (impl_->x11)     return impl_->x11->startDrag(data, actions);
    return false;
}

// ── Foreign Toplevel Subsystem ───────────────────────────────
std::vector<std::shared_ptr<ToplevelWindow>> Platform::getToplevels() const {
    if (impl_->wayland) return impl_->wayland->getToplevels();
    if (impl_->x11)     return impl_->x11->getToplevels();
    return {};
}

std::shared_ptr<ToplevelWindow> Platform::getActiveToplevel() const {
    if (impl_->wayland) return impl_->wayland->getActiveToplevel();
    if (impl_->x11)     return impl_->x11->getActiveToplevel();
    return nullptr;
}

// ── Output / Monitor Subsystem ──────────────────────────────
std::vector<std::shared_ptr<Output>> Platform::getOutputs() const {
    if (impl_->wayland) return impl_->wayland->getOutputs();
    if (impl_->x11)     return impl_->x11->getOutputs();
    return {};
}

std::shared_ptr<Output> Platform::getOutputByName(std::string_view name) const {
    if (impl_->wayland) return impl_->wayland->getOutputByName(name);
    if (impl_->x11)     return impl_->x11->getOutputByName(name);
    return nullptr;
}

std::shared_ptr<Output> Platform::getPrimaryOutput() const {
    if (impl_->wayland) return impl_->wayland->getPrimaryOutput();
    if (impl_->x11)     return impl_->x11->getPrimaryOutput();
    return nullptr;
}

// ── Cursor ───────────────────────────────────────────────────────
void Platform::setCursor(SystemCursor cursor) {
    if (impl_->wayland) impl_->wayland->setCursor(cursor);
    if (impl_->x11)     impl_->x11->setCursor(cursor);
}

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
