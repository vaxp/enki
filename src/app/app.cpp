/// @file app.cpp
/// @brief ENKI App lifecycle implementation with Native Platform (Zero SDL).

#include "enki/app/app.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

// Skia GPU
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/gl/GrGLAssembleInterface.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GL/gl.h>

#include "enki/animation/ticker.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace enki {

// Forward declaration of internal canvas factory
std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas);

// ════════════════════════════════════════════════════════════════
// App::Impl — internal state
// ════════════════════════════════════════════════════════════════

struct App::Impl {
    AppConfig config;
    std::string title;

    // Platform layer (Native X11 / EGL)
    std::unique_ptr<Platform> platform;
    std::unique_ptr<Window>   window;

    // Skia GPU context
    sk_sp<GrDirectContext> gr_context;

    // Widget tree
    WidgetPtr                   root_widget;
    std::unique_ptr<BuildOwner> build_owner;
    std::unique_ptr<Element>    root_element;

    // Skia Surface Cache
    sk_sp<SkSurface> cached_surface;
    int cached_w = 0;
    int cached_h = 0;

    // Loop control
    bool quit_requested = false;

    // Pointer state
    std::unordered_set<RenderObject*> hovered_targets;
    float last_pointer_x = 0.0f;
    float last_pointer_y = 0.0f;

    // Timing
    using Clock = std::chrono::steady_clock;
    Clock::time_point last_frame_time;

    // ── Initialization ──────────────────────────────────────────

    bool initPlatform() {
        auto result = Platform::create();
        if (!result.isOk()) return false;
        platform = std::move(result.value());
        return true;
    }

    bool initWindow() {
        WindowConfig win_cfg;
        win_cfg.title     = config.title;
        win_cfg.width     = config.width;
        win_cfg.height    = config.height;
        win_cfg.resizable = config.resizable;
        win_cfg.vsync     = config.vsync;

        auto result = Window::create(*platform, win_cfg);
        if (!result.isOk()) return false;
        window = std::move(result.value());
        title  = config.title;

        // Connect quit signals
        window->onClose().connect([this]()  { quit_requested = true; });
        platform->onQuit().connect([this]() { quit_requested = true; });

        // Connect input signals
        platform->onMouseDown().connect([this](float x, float y, int btn) {
            dispatchPointerDown(x, y, btn);
        });
        platform->onMouseUp().connect([this](float x, float y, int btn) {
            dispatchPointerUp(x, y, btn);
        });
        platform->onMouseMove().connect([this](float x, float y) {
            dispatchPointerMove(x, y);
        });
        platform->onScroll().connect([this](float dx, float dy) {
            dispatchScroll(dx, dy);
        });

        return true;
    }

    bool initSkia() {
        window->makeCurrent();

        // Strategy 1: Make assembled interface from loaded OpenGL libraries
        void* libgl = nullptr;
        if (!libgl) libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGL.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGLESv2.so.2", RTLD_LAZY | RTLD_LOCAL);
        if (!libgl) libgl = dlopen("libGLESv2.so", RTLD_LAZY | RTLD_LOCAL);

        auto gl_interface = GrGLMakeAssembledInterface(libgl, [](void* ctx, const char* name) -> GrGLFuncPtr {
            // First check eglGetProcAddress
            if (auto proc = eglGetProcAddress(name)) {
                return reinterpret_cast<GrGLFuncPtr>(proc);
            }
            // Second check libGL symbols (for core GL 1.0-1.1 functions like glGetString)
            if (ctx) {
                if (auto proc = dlsym(ctx, name)) {
                    return reinterpret_cast<GrGLFuncPtr>(proc);
                }
            }
            // Third check global symbols
            if (auto proc = dlsym(RTLD_DEFAULT, name)) {
                return reinterpret_cast<GrGLFuncPtr>(proc);
            }
            return nullptr;
        });

        // Strategy 2: Fallback to Skia Native Interface
        if (!gl_interface) {
            gl_interface = GrGLMakeNativeInterface();
        }

        if (!gl_interface) {
            std::cerr << "[ENKI] Failed to create any Skia GL Interface\n";
            return false;
        }

        gr_context = GrDirectContext::MakeGL(gl_interface);
        if (!gr_context) {
            std::cerr << "[ENKI] GrDirectContext::MakeGL failed\n";
            return false;
        }

        return true;
    }

    bool initWidgetTree() {
        build_owner  = std::make_unique<BuildOwner>();
        root_element = root_widget->createElement();
        root_element->setOwner(build_owner.get());
        root_element->mount(nullptr, 0);
        build_owner->buildScope(root_element.get());
        return true;
    }

    // ── Event Dispatching ───────────────────────────────────────

    void dispatchPointerMove(float x, float y) {
        last_pointer_x = x;
        last_pointer_y = y;

        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        HitTestResult result;
        root_ro->hitTest(result, {x, y});

        std::unordered_set<RenderObject*> current_hovered;
        SystemCursor desired_cursor = SystemCursor::Arrow;

        for (const auto& entry : result.path()) {
            if (entry.target) {
                current_hovered.insert(entry.target);
                PointerEvent e{
                    .type          = PointerEvent::Move,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = MouseButton::None
                };
                entry.target->handlePointerMove(e);

                if (desired_cursor == SystemCursor::Arrow && entry.target->cursor() != SystemCursor::Default) {
                    desired_cursor = entry.target->cursor();
                }
            }
        }

        // Notify exit for targets no longer hovered
        for (RenderObject* ro : hovered_targets) {
            if (current_hovered.find(ro) == current_hovered.end()) {
                ro->handlePointerExit({});
            }
        }

        // Notify enter for newly hovered targets
        for (RenderObject* ro : current_hovered) {
            if (hovered_targets.find(ro) == hovered_targets.end()) {
                ro->handlePointerEnter({});
            }
        }

        hovered_targets = std::move(current_hovered);

        if (platform) {
            platform->setCursor(desired_cursor);
        }
    }

    void dispatchScroll(float dx, float dy) {
        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        HitTestResult result;
        root_ro->hitTest(result, {last_pointer_x, last_pointer_y});

        for (const auto& entry : result.path()) {
            if (entry.target && entry.target->handlesScroll()) {
                entry.target->handlePointerScroll(dx, dy);
                break;
            }
        }
    }

    void dispatchPointerDown(float x, float y, int btn) {
        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        HitTestResult result;
        root_ro->hitTest(result, {x, y});

        for (const auto& entry : result.path()) {
            if (entry.target) {
                PointerEvent e{
                    .type          = PointerEvent::Down,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = static_cast<MouseButton>(btn)
                };
                entry.target->handlePointerDown(e);
            }
        }
    }

    void dispatchPointerUp(float x, float y, int btn) {
        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        HitTestResult result;
        root_ro->hitTest(result, {x, y});

        for (const auto& entry : result.path()) {
            if (entry.target) {
                PointerEvent e{
                    .type          = PointerEvent::Up,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = static_cast<MouseButton>(btn)
                };
                entry.target->handlePointerUp(e);
            }
        }
    }

    // ── Per-frame rendering ─────────────────────────────────────

    void renderFrame() {
        auto s = window->getDrawableSize();
        if (s.width <= 0 || s.height <= 0) return;

        int w = static_cast<int>(s.width);
        int h = static_cast<int>(s.height);

        if (!cached_surface || cached_w != w || cached_h != h) {
            GrGLFramebufferInfo fbInfo;
            fbInfo.fFBOID  = 0;
            fbInfo.fFormat = 0x8058; // GL_RGBA8

            GrBackendRenderTarget backendRT(w, h, 0, 8, fbInfo);

            cached_surface = SkSurface::MakeFromBackendRenderTarget(
                gr_context.get(), backendRT,
                kBottomLeft_GrSurfaceOrigin,
                kRGBA_8888_SkColorType,
                nullptr, nullptr);

            cached_w = w;
            cached_h = h;
        }

        if (!cached_surface) return;

        SkCanvas* sk_canvas = cached_surface->getCanvas();

        // Clear background
        const Color cc = config.clear_color;
        sk_canvas->clear(SkColorSetARGB(
            (cc >> 24) & 0xFF,
            (cc >> 16) & 0xFF,
            (cc >>  8) & 0xFF,
            (cc >>  0) & 0xFF));

        // Advance all animations (Tickers / AnimationControllers / Springs)
        SchedulerBinding::instance().tick();

        // Rebuild any dirty elements before painting
        build_owner->buildScope(root_element.get());

        // Layout + Paint
        auto* root_ro = root_element->findRenderObject();
        if (root_ro) {
            root_ro->layout(s.width, s.height);

            auto canvas = createCanvasWrapper(sk_canvas);
            PaintContext pctx{*canvas, Point{0, 0},
                              Rect{0, 0, s.width, s.height}, 1.0f};
            root_ro->paint(pctx);
        }

        gr_context->flush();
        window->swapBuffers();
    }

    // ── Frame timing ────────────────────────────────────────────

    void capFrameRate() {
        // When VSync is enabled, eglSwapBuffers handles synchronization with zero jitter.
        if (config.vsync) {
            last_frame_time = Clock::now();
            return;
        }

        if (config.target_fps <= 0) return;

        using namespace std::chrono;
        const auto frame_duration = duration_cast<Clock::duration>(
            duration<double>(1.0 / config.target_fps));

        auto now     = Clock::now();
        auto elapsed = now - last_frame_time;
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
        last_frame_time = Clock::now();
    }
};

// ════════════════════════════════════════════════════════════════
// App — public API
// ════════════════════════════════════════════════════════════════

App::App() : impl_(std::make_unique<Impl>()) {}

App::~App() {
    if (impl_->root_element) {
        impl_->root_element->unmount();
    }
}

Result<std::unique_ptr<App>> App::create(WidgetPtr root_widget, AppConfig config) {
    auto app = std::unique_ptr<App>(new App());
    auto& impl = *app->impl_;

    impl.config      = config;
    impl.root_widget = std::move(root_widget);

    if (!impl.initPlatform())   return Result<std::unique_ptr<App>>::err(ErrorCode::PlatformError, "Failed to initialize Native Linux Platform");
    if (!impl.initWindow())     return Result<std::unique_ptr<App>>::err(ErrorCode::WindowError,   "Failed to create Native Window");
    if (!impl.initSkia())       return Result<std::unique_ptr<App>>::err(ErrorCode::RenderingError, "Failed to initialize Skia GPU context");
    if (!impl.initWidgetTree()) return Result<std::unique_ptr<App>>::err(ErrorCode::NotInitialized, "Failed to build widget tree");

    impl.last_frame_time = Impl::Clock::now();

    return Result<std::unique_ptr<App>>::ok(std::move(app));
}

int App::run() {
    auto& impl = *impl_;

    while (!impl.quit_requested) {
        // 1. Poll platform events
        if (!impl.platform->pollEvents()) {
            impl.quit_requested = true;
            break;
        }

        // 2. Render the frame
        impl.renderFrame();

        // 3. Cap to target FPS
        impl.capFrameRate();
    }

    return 0;
}

void App::quit() { impl_->quit_requested = true; }

const std::string& App::title() const { return impl_->title; }

void App::setTitle(std::string_view title) {
    impl_->title = title;
    impl_->window->setTitle(title);
}

Size App::windowSize() const { return impl_->window->getSize(); }
float App::dpiScale()   const { return impl_->window->getDpiScale(); }
Platform& App::platform()    { return *impl_->platform; }
Window&   App::window()      { return *impl_->window; }

// ════════════════════════════════════════════════════════════════
// runApp — convenience entry point
// ════════════════════════════════════════════════════════════════

int runApp(WidgetPtr root_widget, AppConfig config) {
    auto result = App::create(std::move(root_widget), std::move(config));
    if (!result.isOk()) {
        std::cerr << "[ENKI App] Launch Error: " << result.error().message << "\n";
        return 1;
    }
    return result.value()->run();
}

}  // namespace enki
