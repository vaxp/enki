/// @file platform_native.cpp
/// @brief Platform abstraction layer — auto-detects and owns Wayland or X11 backend.
/// Downstream code (Window, LayerSurface, App) is fully backend-agnostic.

#include "enki/platform/platform.hpp"
#include "enki/platform/window.hpp"

#if defined(_WIN32)
#include "enki/platform/windows/win32_platform.hpp"
#else
#include "enki/platform/x11/x11_platform.hpp"
#if defined(ENKI_HAS_WAYLAND)
#include "enki/platform/wayland/wayland_platform.hpp"
#endif
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace enki {

static Platform* g_platform_instance = nullptr;

// ════════════════════════════════════════════════════════════════
// Platform::Impl  — owns active backend (Win32 on Windows, Wayland/X11 on Linux)
// ════════════════════════════════════════════════════════════════
struct Platform::Impl {
    Platform* owner = nullptr;

#if defined(_WIN32)
    std::unique_ptr<win32::Win32PlatformBackend> win32;
#else
    std::unique_ptr<wayland::WaylandPlatformBackend> wayland;
    std::unique_ptr<x11::X11PlatformBackend>         x11;
#endif

    std::chrono::steady_clock::time_point start_time;
    std::unordered_set<Window*> windows;
    std::string clipboard_buffer;

    // ── Backend Selection ────────────────────────────────────────
    bool init() {
        start_time = std::chrono::steady_clock::now();

#if defined(_WIN32)
        win32 = std::make_unique<win32::Win32PlatformBackend>(owner);
        if (!win32->init()) {
            win32.reset();
            return false;
        }
        return true;
#else
        // Prefer Wayland if environment indicates it
        const char* wl_disp = std::getenv("WAYLAND_DISPLAY");
        if (wl_disp || std::getenv("WAYLAND_SOCKET")) {
#if defined(ENKI_HAS_WAYLAND)
            wayland = std::make_unique<wayland::WaylandPlatformBackend>(owner);
            if (wayland->init()) {
                return true;
            }
            std::cerr << "[ENKI Platform] Wayland failed — falling back to X11\n";
            wayland.reset();
#endif
        }

        // X11 fallback
        x11 = std::make_unique<x11::X11PlatformBackend>(owner);
        if (!x11->init()) {
            x11.reset();
            return false;
        }
        return true;
#endif
    }

    void shutdown() {
#if defined(_WIN32)
        if (win32)   { win32->shutdown();   win32.reset(); }
#else
        if (wayland) { wayland->shutdown(); wayland.reset(); }
        if (x11)     { x11->shutdown();     x11.reset(); }
#endif
    }

    bool isWayland() const {
#if defined(_WIN32)
        return false;
#else
        return wayland != nullptr;
#endif
    }
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
            ErrorCode::PlatformError, "Failed to initialize platform");
    }
    g_platform_instance = p.get();
    return Result<std::unique_ptr<Platform>>::ok(std::move(p));
}

Platform* Platform::instance() { return g_platform_instance; }

// ── Event Loop ──────────────────────────────────────────────────
bool Platform::pollEvents() {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->pollEvents();
    return false;
#else
    if (impl_->wayland) return impl_->wayland->pollEvents();
    if (impl_->x11)     return impl_->x11->pollEvents();
    return false;
#endif
}

// ── Window Registration ──────────────────────────────────────────
void Platform::registerWindow(Window* w) {
    if (!w) return;
    impl_->windows.insert(w);
#if defined(_WIN32)
    if (impl_->win32) impl_->win32->registerWindow(w);
#else
    if (impl_->x11) impl_->x11->registerWindow(w);
#endif
}

void Platform::unregisterWindow(Window* w) {
    if (!w) return;
    impl_->windows.erase(w);
#if defined(_WIN32)
    if (impl_->win32) impl_->win32->unregisterWindow(w);
#else
    if (impl_->x11) impl_->x11->unregisterWindow(w);
#endif
}

// ── Clipboard Subsystem ──────────────────────────────────────
void Platform::setClipboardText(std::string_view text, ClipboardType type) {
    ClipboardData data;
    data.setText(text);
    setClipboardData(data, type);
}

std::string Platform::getClipboardText(ClipboardType type) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardText(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardData(type).getText();
    if (impl_->x11)     return impl_->x11->getClipboardText(type);
#endif
    return impl_->clipboard_buffer;
}

void Platform::setClipboardData(const ClipboardData& data, ClipboardType type) {
    impl_->clipboard_buffer = data.getText();
#if defined(_WIN32)
    if (impl_->win32) impl_->win32->setClipboardData(data, type);
#else
    if (impl_->wayland) impl_->wayland->setClipboardData(data, type);
    if (impl_->x11)     impl_->x11->setClipboardData(data, type);
#endif
}

ClipboardData Platform::getClipboardData(ClipboardType type) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardData(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardData(type);
    if (impl_->x11)     return impl_->x11->getClipboardData(type);
#endif
    ClipboardData cd;
    cd.setText(impl_->clipboard_buffer);
    return cd;
}

std::vector<uint8_t> Platform::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardDataForMime(mime_type, type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardDataForMime(mime_type, type);
    if (impl_->x11)     return impl_->x11->getClipboardDataForMime(mime_type, type);
#endif
    return {};
}

std::vector<std::string> Platform::getClipboardFormats(ClipboardType type) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getClipboardFormats(type);
#else
    if (impl_->wayland) return impl_->wayland->getClipboardFormats(type);
    if (impl_->x11)     return impl_->x11->getClipboardFormats(type);
#endif
    return { std::string(mime::TextPlainUtf8) };
}

bool Platform::hasClipboardFormat(std::string_view mime_type, ClipboardType type) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->hasClipboardFormat(mime_type, type);
#else
    if (impl_->wayland) return impl_->wayland->hasClipboardFormat(mime_type, type);
    if (impl_->x11)     return impl_->x11->hasClipboardFormat(mime_type, type);
#endif
    return mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain;
}

// ── Drag & Drop Subsystem ────────────────────────────────────
bool Platform::startDrag(const DragData& data, DragAction actions) {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->startDrag(data, actions);
#else
    if (impl_->wayland) return impl_->wayland->startDrag(data, actions);
    if (impl_->x11)     return impl_->x11->startDrag(data, actions);
#endif
    return false;
}

// ── Foreign Toplevel Subsystem ───────────────────────────────
std::vector<std::shared_ptr<ToplevelWindow>> Platform::getToplevels() const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getToplevels();
#else
    if (impl_->wayland) return impl_->wayland->getToplevels();
    if (impl_->x11)     return impl_->x11->getToplevels();
#endif
    return {};
}

std::shared_ptr<ToplevelWindow> Platform::getActiveToplevel() const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getActiveToplevel();
#else
    if (impl_->wayland) return impl_->wayland->getActiveToplevel();
    if (impl_->x11)     return impl_->x11->getActiveToplevel();
#endif
    return nullptr;
}

// ── Output / Monitor Subsystem ──────────────────────────────
std::vector<std::shared_ptr<Output>> Platform::getOutputs() const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getOutputs();
#else
    if (impl_->wayland) return impl_->wayland->getOutputs();
    if (impl_->x11)     return impl_->x11->getOutputs();
#endif
    return {};
}

std::shared_ptr<Output> Platform::getOutputByName(std::string_view name) const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getOutputByName(name);
#else
    if (impl_->wayland) return impl_->wayland->getOutputByName(name);
    if (impl_->x11)     return impl_->x11->getOutputByName(name);
#endif
    return nullptr;
}

std::shared_ptr<Output> Platform::getPrimaryOutput() const {
#if defined(_WIN32)
    if (impl_->win32) return impl_->win32->getPrimaryOutput();
#else
    if (impl_->wayland) return impl_->wayland->getPrimaryOutput();
    if (impl_->x11)     return impl_->x11->getPrimaryOutput();
#endif
    return nullptr;
}

// ── Cursor ───────────────────────────────────────────────────────
void Platform::setCursor(SystemCursor cursor) {
#if defined(_WIN32)
    if (impl_->win32) impl_->win32->setCursor(cursor);
#else
    if (impl_->wayland) impl_->wayland->setCursor(cursor);
    if (impl_->x11)     impl_->x11->setCursor(cursor);
#endif
}

// ── Timing ───────────────────────────────────────────────────────
double Platform::getTime() const {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - impl_->start_time).count();
}

// ── Backend accessors ─────────────────────────────────────────────
void* Platform::getNativeDisplay() const {
#if defined(_WIN32)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getDisplay();
    return nullptr;
#endif
}
void* Platform::getEGLDisplay() const {
#if defined(_WIN32)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLDisplay();
    if (impl_->x11)     return (void*)impl_->x11->getEGLDisplay();
    return nullptr;
#endif
}
void* Platform::getEGLConfig() const {
#if defined(_WIN32)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLConfig();
    if (impl_->x11)     return (void*)impl_->x11->getEGLConfig();
    return nullptr;
#endif
}
void* Platform::getEGLContext() const {
#if defined(_WIN32)
    return nullptr;
#else
    if (impl_->wayland) return (void*)impl_->wayland->getEGLContext();
    if (impl_->x11)     return (void*)impl_->x11->getEGLContext();
    return nullptr;
#endif
}

bool  Platform::isWayland()        const { return impl_->isWayland(); }
void* Platform::getWaylandBackend() const {
#if defined(_WIN32)
    return nullptr;
#else
    return (void*)impl_->wayland.get();
#endif
}
void* Platform::getX11Backend()    const {
#if defined(_WIN32)
    return nullptr;
#else
    return (void*)impl_->x11.get();
#endif
}

void* Platform::getWin32Backend() const {
#if defined(_WIN32)
    return (void*)impl_->win32.get();
#else
    return nullptr;
#endif
}

}  // namespace enki
