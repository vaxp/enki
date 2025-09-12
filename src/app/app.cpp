/// @file app.cpp
/// @brief ENKI App lifecycle implementation with Native Platform (Zero SDL).

#include "enki/app/app.hpp"
#include "enki/shell/surface_host.hpp"
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
#include <deque>
#include <algorithm>

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

    // Multi-surface / Popups owned by App
    std::vector<std::unique_ptr<SurfaceHost>> surfaces;
    SurfaceHost* active_popup_host = nullptr;

    SurfaceHost* findSurfaceForHandle(void* handle) {
        if (!handle) return nullptr;
        for (auto& s : surfaces) {
            if (s && s->getNativeHandle() == handle) {
                return s.get();
            }
        }
        return nullptr;
    }

    // Pointer state
    std::unordered_set<RenderObject*> hovered_targets;
    std::vector<RenderObject*>        active_pointer_targets;
    bool                              is_pointer_down = false;
    MouseButton                       active_button   = MouseButton::None;
    float                             last_pointer_x = 0.0f;
    float                             last_pointer_y = 0.0f;

    // Timing & Real-time performance metrics
    using Clock = std::chrono::steady_clock;
    Clock::time_point last_frame_time;
    Clock::time_point last_fps_sample_time;
    uint32_t          frames_in_sample = 0;
    FrameStats        stats;

    // Sliding window for p95 frame time (last 120 frames ~ 2s at 60fps)
    static constexpr size_t kFrameWindow = 120;
    std::deque<double>      frame_times_window;

    // Cached canvas wrapper — recreated only when SkCanvas pointer changes
    std::unique_ptr<Canvas> cached_canvas;
    SkCanvas*               cached_sk_canvas_ptr = nullptr;

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
        win_cfg.mode      = config.window_mode;

        auto result = Window::create(*platform, win_cfg);
        if (!result.isOk()) return false;
        window = std::move(result.value());
        title  = config.title;

        // Connect quit signals
        window->onClose().connect([this]()  { quit_requested = true; });
        platform->onQuit().connect([this]() { quit_requested = true; });

        // Targeted event routing for multi-surface / popups
        platform->onTargetedMouseDown().connect([this](void* handle, float x, float y, int btn) {
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            SurfaceHost* popup_target = findSurfaceForHandle(handle);

            // Auto-dismiss check: if click is outside active popups, dismiss auto-dismiss popups in LIFO order (topmost first)
            if (!popup_target && !surfaces.empty()) {
                for (int i = static_cast<int>(surfaces.size()) - 1; i >= 0; --i) {
                    if (surfaces[i]->isAutoDismiss()) {
                        surfaces.erase(surfaces.begin() + i);
                    }
                }
            }

            if (popup_target) {
                active_popup_host = popup_target;
                popup_target->handlePointerDown(x, y, mb);
            } else if (!window || handle == window->getNativeHandle()) {
                dispatchPointerDown(x, y, btn);
            }
        });

        platform->onTargetedMouseUp().connect([this](void* handle, float x, float y, int btn) {
            MouseButton mb = (btn == 1) ? MouseButton::Left : (btn == 3 ? MouseButton::Right : MouseButton::Middle);
            SurfaceHost* popup_target = active_popup_host ? active_popup_host : findSurfaceForHandle(handle);

            if (popup_target) {
                popup_target->handlePointerUp(x, y, mb);
                active_popup_host = nullptr;
            } else if (!window || handle == window->getNativeHandle()) {
                dispatchPointerUp(x, y, btn);
            }
        });

        platform->onTargetedMouseMove().connect([this](void* handle, float x, float y) {
            SurfaceHost* popup_target = findSurfaceForHandle(handle);
            if (popup_target) {
                popup_target->handlePointerMove(x, y);
            } else if (!window || handle == window->getNativeHandle()) {
                dispatchPointerMove(x, y);
            }
        });

        platform->onTargetedScroll().connect([this](void* handle, float dx, float dy) {
            SurfaceHost* popup_target = findSurfaceForHandle(handle);
            if (popup_target) {
                popup_target->handleScroll(dx, dy);
            } else if (!window || handle == window->getNativeHandle()) {
                dispatchScroll(dx, dy);
            }
        });

        // Connect fallback global input signals
        platform->onMouseDown().connect([this](float x, float y, int btn) {
            if (!active_popup_host && !surfaces.empty()) {
                for (int i = static_cast<int>(surfaces.size()) - 1; i >= 0; --i) {
                    if (surfaces[i]->isAutoDismiss()) {
                        surfaces.erase(surfaces.begin() + i);
                    }
                }
            }
            if (!active_popup_host) {
                dispatchPointerDown(x, y, btn);
            }
        });

        platform->onMouseUp().connect([this](float x, float y, int btn) {
            if (!active_popup_host) {
                dispatchPointerUp(x, y, btn);
            }
        });

        platform->onMouseMove().connect([this](float x, float y) {
            if (!active_popup_host && surfaces.empty()) {
                dispatchPointerMove(x, y);
            }
        });

        platform->onScroll().connect([this](float dx, float dy) {
            if (!active_popup_host) {
                dispatchScroll(dx, dy);
            }
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

    void tickPointer() {
        if (is_pointer_down && !active_pointer_targets.empty()) {
            double now = std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
            for (RenderObject* ro : active_pointer_targets) {
                if (RenderObject::isAlive(ro)) {
                    ro->tick(now);
                }
            }
        }
    }

    void dispatchPointerMove(float x, float y) {
        last_pointer_x = x;
        last_pointer_y = y;

        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        // 1. If pointer is captured (active drag), dispatch move to all active targets
        if (is_pointer_down && !active_pointer_targets.empty()) {
            for (RenderObject* ro : active_pointer_targets) {
                if (RenderObject::isAlive(ro)) {
                    Rect bounds = ro->globalBounds();
                    Point localPos = {x - bounds.x, y - bounds.y};
                    PointerEvent e{
                        .type          = PointerEvent::Move,
                        .position      = {x, y},
                        .localPosition = localPos,
                        .button        = active_button
                    };
                    ro->handlePointerMove(e);
                }
            }
        }

        // 2. Perform hit test for hover and cursor updates
        HitTestResult result;
        root_ro->hitTest(result, {x, y});

        std::unordered_set<RenderObject*> current_hovered;
        SystemCursor desired_cursor = SystemCursor::Arrow;

        for (const auto& entry : result.path()) {
            if (entry.target && RenderObject::isAlive(entry.target)) {
                current_hovered.insert(entry.target);

                if (!is_pointer_down) {
                    PointerEvent e{
                        .type          = PointerEvent::Move,
                        .position      = {x, y},
                        .localPosition = entry.localPosition,
                        .button        = MouseButton::None
                    };
                    entry.target->handlePointerMove(e);
                }

                if (desired_cursor == SystemCursor::Arrow && entry.target->cursor() != SystemCursor::Default) {
                    desired_cursor = entry.target->cursor();
                }
            }
        }

        // Notify exit for targets no longer hovered
        for (RenderObject* ro : hovered_targets) {
            if (RenderObject::isAlive(ro) && current_hovered.find(ro) == current_hovered.end()) {
                ro->handlePointerExit({});
            }
        }

        // Notify enter for newly hovered targets
        for (RenderObject* ro : current_hovered) {
            if (RenderObject::isAlive(ro) && hovered_targets.find(ro) == hovered_targets.end()) {
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
            if (entry.target && RenderObject::isAlive(entry.target) && entry.target->handlesScroll()) {
                entry.target->handlePointerScroll(dx, dy);
                break;
            }
        }
    }

    void dispatchPointerDown(float x, float y, int btn) {
        is_pointer_down = true;
        active_button   = static_cast<MouseButton>(btn);
        active_pointer_targets.clear();

        if (!root_element) return;
        auto* root_ro = root_element->findRenderObject();
        if (!root_ro) return;

        HitTestResult result;
        root_ro->hitTest(result, {x, y});

        for (const auto& entry : result.path()) {
            if (entry.target && RenderObject::isAlive(entry.target)) {
                active_pointer_targets.push_back(entry.target);
                PointerEvent e{
                    .type          = PointerEvent::Down,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = active_button
                };
                entry.target->handlePointerDown(e);
            }
        }
    }

    void dispatchPointerUp(float x, float y, int btn) {
        is_pointer_down = false;
        MouseButton released_button = static_cast<MouseButton>(btn);

        if (!active_pointer_targets.empty()) {
            for (RenderObject* ro : active_pointer_targets) {
                if (RenderObject::isAlive(ro)) {
                    Rect bounds = ro->globalBounds();
                    Point localPos = {x - bounds.x, y - bounds.y};
                    PointerEvent e{
                        .type          = PointerEvent::Up,
                        .position      = {x, y},
                        .localPosition = localPos,
                        .button        = released_button
                    };
                    ro->handlePointerUp(e);
                }
            }
            active_pointer_targets.clear();
        } else {
            if (!root_element) return;
            auto* root_ro = root_element->findRenderObject();
            if (!root_ro) return;

            HitTestResult result;
            root_ro->hitTest(result, {x, y});

            for (const auto& entry : result.path()) {
                if (entry.target && RenderObject::isAlive(entry.target)) {
                    PointerEvent e{
                        .type          = PointerEvent::Up,
                        .position      = {x, y},
                        .localPosition = entry.localPosition,
                        .button        = released_button
                    };
                    entry.target->handlePointerUp(e);
                }
            }
        }
    }

    // ── Per-frame rendering ─────────────────────────────────────

    void renderFrame() {
        auto frame_start = Clock::now();

        auto s = window->getDrawableSize();
        if (s.width <= 0 || s.height <= 0) return;

        int w = static_cast<int>(s.width);
        int h = static_cast<int>(s.height);

        // ── Recreate surface on resize ──────────────────────────
        bool surface_resized = false;
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
            surface_resized = true;

            // Invalidate canvas cache when surface changes
            cached_sk_canvas_ptr = nullptr;
            cached_canvas.reset();

            // Force full relayout + repaint on resize
            if (root_element) {
                if (auto* root_ro = root_element->findRenderObject()) {
                    root_ro->markNeedsLayout();
                    root_ro->markNeedsPaint();
                }
            }
        }

        if (!cached_surface) return;

        SkCanvas* sk_canvas = cached_surface->getCanvas();

        // ── Rebuild canvas wrapper only when pointer changes ────
        if (sk_canvas != cached_sk_canvas_ptr) {
            cached_canvas         = createCanvasWrapper(sk_canvas);
            cached_sk_canvas_ptr  = sk_canvas;
        }

        // ── Phase 0: Capture pre-build tree state ───────────────
        stats.dirty_elements = static_cast<uint32_t>(build_owner->dirtyElementCount());
        stats.active_tickers = static_cast<uint32_t>(SchedulerBinding::instance().tickerCount());

        // ── Phase 1: Advance animations ─────────────────────────
        SchedulerBinding::instance().tick();

        // ── Phase 2: Rebuild dirty elements ─────────────────────
        auto build_start = Clock::now();
        build_owner->buildScope(root_element.get());
        auto build_end = Clock::now();
        stats.build_time_ms = std::chrono::duration<double, std::milli>(build_end - build_start).count();

        auto* root_ro = root_element->findRenderObject();

        // ── Phase 3: Layout ──────────────────────────────────────
        auto layout_start = Clock::now();
        if (root_ro && root_ro->needsLayout()) {
            root_ro->layout(s.width, s.height);
        }
        auto layout_end = Clock::now();
        stats.layout_time_ms = std::chrono::duration<double, std::milli>(layout_end - layout_start).count();

        // ── Idle-skip decision ───────────────────────────────────
        // Only repaint if something in the scene actually changed:
        //   • dirty elements were rebuilt (setState fired)
        //   • active tickers are running (animations in flight)
        //   • layout was recalculated (size/position changed)
        //   • any render object flagged itself as needing repaint
        //   • the performance overlay is shown (it reads live stats)
        bool scene_dirty = (stats.dirty_elements > 0)
                        || (stats.active_tickers > 0)
                        || (stats.layout_time_ms > 0.0)
                        || (root_ro && root_ro->subtreeNeedsPaint())
                        || config.show_performance_overlay
                        || surface_resized;

        double paint_ms = 0.0;
        double gpu_flush_ms = 0.0;
        double swap_ms = 0.0;

        if (scene_dirty && root_ro) {
            // ── Phase 4: Clear background + Paint ───────────────
            const Color cc = config.clear_color;
            sk_canvas->clear(SkColorSetARGB(
                (cc >> 24) & 0xFF, (cc >> 16) & 0xFF,
                (cc >>  8) & 0xFF, (cc >>  0) & 0xFF));

            PaintContext pctx{*cached_canvas, Point{0, 0},
                              Rect{0, 0, s.width, s.height}, 1.0f};
            auto paint_start = Clock::now();
            root_ro->paint(pctx);
            auto paint_end = Clock::now();
            paint_ms = std::chrono::duration<double, std::milli>(paint_end - paint_start).count();

            // Clear dirty flag — scene is now fully rendered
            root_ro->clearPaintFlag();

            if (config.show_performance_overlay) {
                drawPerformanceOverlay(*cached_canvas, s);
            }

            // ── Phase 5: GPU flush ───────────────────────────────
            auto gpu_start = Clock::now();
            gr_context->flush();
            auto gpu_end = Clock::now();
            gpu_flush_ms = std::chrono::duration<double, std::milli>(gpu_end - gpu_start).count();

            // ── Phase 6: Wayland/EGL buffer swap ────────────────
            window->swapBuffers();
            auto swap_end = Clock::now();
            swap_ms = std::chrono::duration<double, std::milli>(swap_end - gpu_end).count();
        }
        // idle path: no paint, no flush, no swap — near-zero CPU

        auto cpu_end = Clock::now();
        stats.paint_time_ms = paint_ms;
        stats.cpu_time_ms   = std::chrono::duration<double, std::milli>(cpu_end - frame_start).count();
        stats.gpu_render_ms = gpu_flush_ms;
        stats.swap_time_ms  = swap_ms;
        stats.gpu_time_ms   = gpu_flush_ms + swap_ms;

        auto frame_end = cpu_end; // frame ends when CPU work is done (swap included above)
        stats.frame_time_ms = std::chrono::duration<double, std::milli>(cpu_end - frame_start).count();
        stats.total_frames++;
        frames_in_sample++;

        // ── Tree stats ───────────────────────────────────────────
        if (root_element && stats.total_frames % 30 == 0) {
            stats.element_count = static_cast<uint32_t>(root_element->subtreeSize());
        }

        // ── Frame budget & jank ──────────────────────────────────
        stats.frame_budget_ms = (config.target_fps > 0)
            ? (1000.0 / config.target_fps) : 16.667;
        stats.budget_used_percent = (stats.frame_time_ms / stats.frame_budget_ms) * 100.0;
        if (stats.frame_time_ms > stats.frame_budget_ms) {
            stats.jank_frames++;
        }
        if (stats.frame_time_ms > stats.max_frame_time_ms) {
            stats.max_frame_time_ms = stats.frame_time_ms;
        }

        // ── p95 sliding window ───────────────────────────────────
        frame_times_window.push_back(stats.frame_time_ms);
        if (frame_times_window.size() > kFrameWindow) {
            frame_times_window.pop_front();
        }
        if (!frame_times_window.empty()) {
            std::vector<double> sorted(frame_times_window.begin(), frame_times_window.end());
            std::sort(sorted.begin(), sorted.end());
            size_t p95_idx = static_cast<size_t>(sorted.size() * 0.95);
            if (p95_idx >= sorted.size()) p95_idx = sorted.size() - 1;
            stats.p95_frame_time_ms = sorted[p95_idx];
        }

        // ── FPS sample ───────────────────────────────────────────
        auto sample_elapsed = std::chrono::duration<double>(cpu_end - last_fps_sample_time).count();
        if (sample_elapsed >= 0.20) {
            stats.fps = frames_in_sample / sample_elapsed;
            frames_in_sample = 0;
            last_fps_sample_time = cpu_end;
        }
    }

    void drawPerformanceOverlay(Canvas& canvas, Size s) {
        // ── Layout constants ────────────────────────────────────
        constexpr float HUD_W    = 310.0f;
        constexpr float HUD_H    = 168.0f;
        constexpr float HUD_PAD  = 14.0f;
        constexpr float FONT_SM  = 9.0f;
        constexpr float FONT_MD  = 10.5f;
        constexpr float FONT_LG  = 13.0f;

        float hud_x = s.width  - HUD_W - HUD_PAD;
        float hud_y = HUD_PAD;

        // ── Background ─────────────────────────────────────────
        Paint bg;
        bg.setColor(0xF00A0E1A);
        bg.setStyle(PaintStyle::Fill);
        canvas.drawRRect(Rect{hud_x, hud_y, HUD_W, HUD_H}, BorderRadius::circular(10.0f), bg);

        Paint border;
        border.setColor(0x6000E5FF);
        border.setStyle(PaintStyle::Stroke);
        border.setStrokeWidth(1.0f);
        canvas.drawRRect(Rect{hud_x, hud_y, HUD_W, HUD_H}, BorderRadius::circular(10.0f), border);

        // Helper lambdas
        float cy = hud_y + 16.0f;
        auto txt = [&](const char* str, float x, float color, float sz, bool bold = false) {
            Paint p; p.setColor(color);
            canvas.drawText(str, Point{x, cy}, p, sz, nullptr, bold);
        };
        // nextLine: default step = 13.5f (one text line height)
        auto nextLine = [&](float gap = 13.5f) { cy += gap; };

        // ── Section: FPS & Frame Time ───────────────────────────
        // Status dot
        uint32_t dot_col = (stats.fps >= 55.0) ? 0xFF10B981
                         : (stats.fps >= 30.0) ? 0xFFF59E0B
                                               : 0xFFEF4444;
        Paint dot; dot.setColor(dot_col); dot.setStyle(PaintStyle::Fill);
        canvas.drawCircle(Point{hud_x + 13.0f, cy - 3.0f}, 4.5f, dot);

        char buf[128];
        std::snprintf(buf, sizeof(buf), "%.1f FPS", stats.fps);
        txt(buf, hud_x + 24.0f, 0xFFFFFFFF, FONT_LG, true);

        std::snprintf(buf, sizeof(buf), "frame: %.2fms / budget: %.1fms  (%.0f%%)",
                      stats.frame_time_ms, stats.frame_budget_ms, stats.budget_used_percent);
        txt(buf, hud_x + 100.0f, 0xFF94A3B8, FONT_SM);
        nextLine(15.0f);

        // Jank / p95 / worst
        std::snprintf(buf, sizeof(buf), "p95: %.2fms   worst: %.2fms   jank: %llu",
                      stats.p95_frame_time_ms, stats.max_frame_time_ms,
                      static_cast<unsigned long long>(stats.jank_frames));
        uint32_t jank_col = (stats.jank_frames == 0) ? 0xFF64748B : 0xFFFB923C;
        txt(buf, hud_x + 10.0f, jank_col, FONT_SM);
        nextLine(14.0f);

        // ── Divider ─────────────────────────────────────────────
        Paint div; div.setColor(0x3000E5FF); div.setStyle(PaintStyle::Fill);
        canvas.drawRect(Rect{hud_x + 8.0f, cy - 2.0f, HUD_W - 16.0f, 1.0f}, div);
        nextLine(7.0f);

        // ── Section: CPU Phase Breakdown ────────────────────────
        txt("CPU", hud_x + 10.0f, 0xFF7DD3FC, FONT_MD, true);
        nextLine(13.0f);

        // Build
        std::snprintf(buf, sizeof(buf), "Build   (setState/rebuild):  %.3f ms  [%u dirty]",
                      stats.build_time_ms, stats.dirty_elements);
        uint32_t build_col = (stats.build_time_ms > 4.0) ? 0xFFFB923C : 0xFFCBD5E1;
        txt(buf, hud_x + 14.0f, build_col, FONT_SM);
        nextLine();

        // Layout
        std::snprintf(buf, sizeof(buf), "Layout  (Anu Flexbox):       %.3f ms",
                      stats.layout_time_ms);
        uint32_t layout_col = (stats.layout_time_ms > 4.0) ? 0xFFFB923C : 0xFFCBD5E1;
        txt(buf, hud_x + 14.0f, layout_col, FONT_SM);
        nextLine();

        // Paint
        std::snprintf(buf, sizeof(buf), "Paint   (Skia draw calls):   %.3f ms",
                      stats.paint_time_ms);
        uint32_t paint_col = (stats.paint_time_ms > 6.0) ? 0xFFFB923C : 0xFFCBD5E1;
        txt(buf, hud_x + 14.0f, paint_col, FONT_SM);
        nextLine();

        // CPU total
        std::snprintf(buf, sizeof(buf), "Total CPU:                   %.3f ms",
                      stats.cpu_time_ms);
        txt(buf, hud_x + 14.0f, 0xFF7DD3FC, FONT_SM);
        nextLine(14.0f);

        // ── Divider ─────────────────────────────────────────────
        canvas.drawRect(Rect{hud_x + 8.0f, cy - 2.0f, HUD_W - 16.0f, 1.0f}, div);
        nextLine(7.0f);

        // ── Section: GPU Phase Breakdown ────────────────────────
        txt("GPU", hud_x + 10.0f, 0xFFA78BFA, FONT_MD, true);
        nextLine(13.0f);

        std::snprintf(buf, sizeof(buf), "Raster  (Skia flush):        %.3f ms",
                      stats.gpu_render_ms);
        uint32_t gpu_col = (stats.gpu_render_ms > 6.0) ? 0xFFFB923C : 0xFFCBD5E1;
        txt(buf, hud_x + 14.0f, gpu_col, FONT_SM);
        nextLine();

        std::snprintf(buf, sizeof(buf), "Swap    (Wayland/EGL):       %.3f ms",
                      stats.swap_time_ms);
        uint32_t swap_col = (stats.swap_time_ms > 4.0) ? 0xFFFB923C : 0xFFCBD5E1;
        txt(buf, hud_x + 14.0f, swap_col, FONT_SM);
        nextLine(14.0f);

        // ── Divider ─────────────────────────────────────────────
        canvas.drawRect(Rect{hud_x + 8.0f, cy - 2.0f, HUD_W - 16.0f, 1.0f}, div);
        nextLine(7.0f);

        // ── Section: Tree & Animation ───────────────────────────
        std::snprintf(buf, sizeof(buf),
                      "Elements: %u   Tickers: %u   Frames: %llu",
                      stats.element_count, stats.active_tickers,
                      static_cast<unsigned long long>(stats.total_frames));
        txt(buf, hud_x + 10.0f, 0xFF94A3B8, FONT_SM);
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

static App* s_app_instance = nullptr;

App::App() : impl_(std::make_unique<Impl>()) {
    s_app_instance = this;
}

App::~App() {
    if (s_app_instance == this) {
        s_app_instance = nullptr;
    }
    if (impl_->root_element) {
        impl_->root_element->unmount();
    }
}

App* App::instance() {
    return s_app_instance;
}

SurfaceHost* App::addPopup(SurfaceHost* parent, WindowConfig config, WidgetPtr root_widget) {
    config.mode = WindowMode::Popup;
    config.parent_window = parent ? parent->getWindow() : impl_->window.get();
    config.parent_layer = parent ? parent->getLayerSurface() : nullptr;

    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[ENKI App] Failed to create Popup Window: " << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<SurfaceHost>(std::move(win_res.value()), std::move(root_widget));
    SurfaceHost* ptr = host.get();
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

SurfaceHost* App::addWindow(WindowConfig config, WidgetPtr root_widget) {
    auto win_res = Window::create(*impl_->platform, config);
    if (!win_res.isOk()) {
        std::cerr << "[ENKI App] Failed to create Window: " << win_res.error().message << "\n";
        return nullptr;
    }

    auto host = std::make_unique<SurfaceHost>(std::move(win_res.value()), std::move(root_widget));
    SurfaceHost* ptr = host.get();
    impl_->surfaces.push_back(std::move(host));
    return ptr;
}

void App::removeSurface(SurfaceHost* host) {
    if (!host) return;
    auto it = std::find_if(impl_->surfaces.begin(), impl_->surfaces.end(),
                           [host](const std::unique_ptr<SurfaceHost>& h) { return h.get() == host; });
    if (it != impl_->surfaces.end()) {
        impl_->surfaces.erase(it);
    }
}

SurfaceHost* App::findSurfaceByOwner(const BuildOwner* owner) const {
    for (auto& s : impl_->surfaces) {
        if (s->getRootElement() && s->getRootElement()->owner() == owner) {
            return s.get();
        }
    }
    return nullptr;
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

    impl.last_frame_time      = Impl::Clock::now();
    impl.last_fps_sample_time = impl.last_frame_time;

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

        // 2. Tick active pointers / gesture timers
        impl.tickPointer();

        // 3. Render main frame
        impl.renderFrame();

        // 4. Update and render all active popup and secondary surfaces
        for (size_t i = 0; i < impl.surfaces.size(); ++i) {
            auto& host = impl.surfaces[i];
            if (!host) continue;

            host->rebuild();
            host->layout();
            host->paint(impl.gr_context.get(), 0x00000000);
            host->swapBuffers();
        }

        // 5. Cap to target FPS
        impl.capFrameRate();
    }

    return 0;
}

void App::quit() { impl_->quit_requested = true; }

FrameStats App::frameStats() const { return impl_->stats; }
double App::currentFps() const { return impl_->stats.fps; }
double App::currentFrameTimeMs() const { return impl_->stats.frame_time_ms; }

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
