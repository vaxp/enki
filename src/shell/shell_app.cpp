/// @file shell_app.cpp
/// @brief Multi-surface Desktop Shell coordinator implementation.

#include "enki/shell/shell_app.hpp"
#include "enki/shell/surface_host.hpp"
#include "enki/animation/ticker.hpp"

// Skia GPU
#include <include/core/SkCanvas.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/gl/GrGLAssembleInterface.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GL/gl.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

namespace enki {

static ShellApp* s_instance = nullptr;

struct ShellApp::Impl {
    AppConfig config;
    std::unique_ptr<Platform> platform;
    sk_sp<GrDirectContext>    gr_context;

    std::vector<std::unique_ptr<SurfaceHost>> surfaces;
    bool quit_requested = false;

    // Pointer routing
    SurfaceHost* active_pointer_host = nullptr;

    SurfaceHost* findSurfaceForHandle(void* handle) {
        if (!handle) return nullptr;
        for (auto& s : surfaces) {
            if (s->getNativeHandle() == handle) return s.get();
        }
        return nullptr;
    }

    bool initPlatform() {
        auto result = Platform::create();
        if (!result.isOk()) return false;
        platform = std::move(result.value());

        platform->onQuit().connect([this] {
            quit_requested = true;
        });

        // Targeted event routing
        platform->onTargetedMouseDown().connect([this](void* handle, float x, float y, int btn) {
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            SurfaceHost* target = findSurfaceForHandle(handle);

            // Auto-dismiss popup surfaces if click is outside popup
            if (!target && surfaces.size() > 1) {
                surfaces.resize(1); // Keep main surface only
            }

            if (!target && !surfaces.empty()) target = surfaces.back().get();
            if (target) {
                active_pointer_host = target;
                target->handlePointerDown(x, y, mb);
            }
        });

        platform->onTargetedMouseUp().connect([this](void* handle, float x, float y, int btn) {
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            SurfaceHost* target = active_pointer_host;
            if (!target) target = findSurfaceForHandle(handle);
            if (!target && !surfaces.empty()) target = surfaces.back().get();
            if (target) {
                target->handlePointerUp(x, y, mb);
            }
            active_pointer_host = nullptr;
        });

        platform->onTargetedMouseMove().connect([this](void* handle, float x, float y) {
            SurfaceHost* target = findSurfaceForHandle(handle);
            if (!target && !surfaces.empty()) target = surfaces.back().get();
            if (target) {
                target->handlePointerMove(x, y);
            }
        });

        platform->onTargetedScroll().connect([this](void* handle, float dx, float dy) {
            SurfaceHost* target = findSurfaceForHandle(handle);
            if (!target && !surfaces.empty()) target = surfaces.back().get();
            if (target) {
                target->handleScroll(dx, dy);
            }
        });

        // Fallback global event connections (if emitted without window target)
        platform->onMouseDown().connect([this](float x, float y, int btn) {
            if (active_pointer_host) return;
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            if (!surfaces.empty()) {
                active_pointer_host = surfaces.back().get();
                active_pointer_host->handlePointerDown(x, y, mb);
            }
        });

        platform->onMouseUp().connect([this](float x, float y, int btn) {
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            if (active_pointer_host) {
                active_pointer_host->handlePointerUp(x, y, mb);
                active_pointer_host = nullptr;
            } else if (!surfaces.empty()) {
                surfaces.back()->handlePointerUp(x, y, mb);
            }
        });

        platform->onMouseMove().connect([this](float x, float y) {
            // Already handled by targeted mouse move if available
        });

        return true;
    }

    bool ensureSkiaContext(SurfaceHost* host) {
        if (gr_context) return true;
        if (!host) return false;

        host->makeCurrent();

        void* libgl = nullptr;
        if (!libgl) libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGL.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGLESv2.so.2", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGLESv2.so", RTLD_LAZY | RTLD_LOCAL);

        sk_sp<const GrGLInterface> gl_interface = nullptr;
        if (libgl) {
            auto proc_loader = [libgl](const char* name) -> GrGLFuncPtr {
                void* sym = dlsym(libgl, name);
                if (!sym) {
                    typedef void* (*GLXGetProcAddressProc)(const char*);
                    static auto glXGetProcAddress = (GLXGetProcAddressProc)dlsym(libgl, "glXGetProcAddressARB");
                    if (glXGetProcAddress) sym = glXGetProcAddress(name);
                }
                if (!sym) {
                    typedef void* (*EGLGetProcAddressProc)(const char*);
                    static auto eglGetProcAddress = (EGLGetProcAddressProc)dlsym(RTLD_DEFAULT, "eglGetProcAddress");
                    if (eglGetProcAddress) sym = eglGetProcAddress(name);
                }
                return reinterpret_cast<GrGLFuncPtr>(sym);
            };

            gl_interface = GrGLMakeAssembledInterface(
                &proc_loader,
                [](void* ctx, const char* name) -> GrGLFuncPtr {
                    auto* loader = static_cast<decltype(proc_loader)*>(ctx);
                    return (*loader)(name);
                }
            );
        }

        if (!gl_interface) {
            gl_interface = GrGLMakeNativeInterface();
        }

        if (!gl_interface) {
            std::cerr << "[ENKI ShellApp] Failed to create GrGLInterface\n";
            return false;
        }

        gr_context = GrDirectContext::MakeGL(gl_interface);
        if (!gr_context) {
            std::cerr << "[ENKI ShellApp] Failed to create GrDirectContext\n";
            return false;
        }

        return true;
    }
};

ShellApp::ShellApp()
    : impl_(std::make_unique<Impl>()) {
    s_instance = this;
}

ShellApp::~ShellApp() {
    if (s_instance == this) s_instance = nullptr;
}

ShellApp* ShellApp::instance() {
    return s_instance;
}

Result<std::unique_ptr<ShellApp>> ShellApp::create(AppConfig config) {
    auto app = std::unique_ptr<ShellApp>(new ShellApp());
    app->impl_->config = config;

    if (!app->impl_->initPlatform()) {
        return Result<std::unique_ptr<ShellApp>>::err(
            ErrorCode::PlatformError,
            "Failed to initialize platform for ShellApp"
        );
    }

    return Result<std::unique_ptr<ShellApp>>::ok(std::move(app));
}

SurfaceHost* ShellApp::addLayerSurface(LayerSurfaceConfig config, WidgetPtr root_widget) {
    auto surface_res = LayerSurface::create(*impl_->platform, config);
    if (!surface_res.isOk()) {
        std::cerr << "[ENKI ShellApp] Failed to create LayerSurface: "
                  << surface_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<SurfaceHost>(std::move(surface_res.value()), std::move(root_widget));
    SurfaceHost* ptr = host.get();

    impl_->ensureSkiaContext(ptr);
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

SurfaceHost* ShellApp::addWindow(WindowConfig config, WidgetPtr root_widget) {
    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[ENKI ShellApp] Failed to create Window: "
                  << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<SurfaceHost>(std::move(win_res.value()), std::move(root_widget));
    SurfaceHost* ptr = host.get();

    impl_->ensureSkiaContext(ptr);
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

SurfaceHost* ShellApp::addPopup(SurfaceHost* parent, WindowConfig config, WidgetPtr root_widget) {
    config.mode = WindowMode::Popup;
    config.parent_window = parent ? parent->getWindow() : nullptr;
    config.parent_layer = parent ? parent->getLayerSurface() : nullptr;
    // Layer surfaces handles popup internally via wl_surface for now or we fallback to absolute.
    
    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[ENKI ShellApp] Failed to create Popup Window: "
                  << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<SurfaceHost>(std::move(win_res.value()), std::move(root_widget));
    SurfaceHost* ptr = host.get();

    impl_->ensureSkiaContext(ptr);
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

void ShellApp::removeSurface(SurfaceHost* host) {
    if (!host) return;
    auto it = std::find_if(impl_->surfaces.begin(), impl_->surfaces.end(),
                           [host](const std::unique_ptr<SurfaceHost>& h) { return h.get() == host; });
    if (it != impl_->surfaces.end()) {
        impl_->surfaces.erase(it);
    }
}

int ShellApp::run() {
    using Clock = std::chrono::steady_clock;
    const auto frame_duration = std::chrono::microseconds(1000000 / (impl_->config.target_fps > 0 ? impl_->config.target_fps : 60));

    while (!impl_->quit_requested && !impl_->surfaces.empty()) {
        auto frame_start = Clock::now();

        // 1. Process platform events
        if (!impl_->platform->pollEvents()) {
            break;
        }

        // 2. Advance all animations and Ticker timers
        SchedulerBinding::instance().tick();

        // 3. Update and render each active surface
        for (size_t i = 0; i < impl_->surfaces.size(); ++i) {
            auto& host = impl_->surfaces[i];
            if (!host) continue;

            host->rebuild();
            host->layout();
            host->paint(impl_->gr_context.get(), 0x00000000);
            host->swapBuffers();
        }

        // 3. Frame rate pacing
        auto elapsed = Clock::now() - frame_start;
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
    }

    return 0;
}

void ShellApp::quit() {
    impl_->quit_requested = true;
}

Platform& ShellApp::platform() {
    return *impl_->platform;
}

GrDirectContext* ShellApp::grContext() {
    return impl_->gr_context.get();
}

size_t ShellApp::surfaceCount() const {
    return impl_->surfaces.size();
}

SurfaceHost* ShellApp::findSurfaceByOwner(const BuildOwner* owner) const {
    for (auto& s : impl_->surfaces) {
        if (s->getRootElement() && s->getRootElement()->owner() == owner) {
            return s.get();
        }
    }
    return nullptr;
}

} // namespace enki
