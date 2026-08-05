/// @file overlay_logo_demo.cpp
/// @brief ENKI Mock Overlay Window Demo with Animated Skia GPU Logo.
/// Demonstrates Wayland wlr-layer-shell Overlay Layer & X11 Transparent Overlay fallback.

#include "enki/platform/platform.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/platform/window.hpp"

// Skia Includes
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkMaskFilter.h>
#include <include/effects/SkGradientShader.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/gl/GrGLAssembleInterface.h>

#include <dlfcn.h>
#include <EGL/egl.h>
#include <GL/gl.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// Helper to initialize Skia GPU Context from active EGL context
static sk_sp<GrDirectContext> initSkiaGPU() {
    static void* libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!libgl) libgl = dlopen("libGL.so", RTLD_LAZY | RTLD_LOCAL);
    if (!libgl) libgl = dlopen("libGLESv2.so.2", RTLD_LAZY | RTLD_LOCAL);

    sk_sp<const GrGLInterface> gl_interface = GrGLMakeAssembledInterface(libgl, [](void* ctx, const char* name) -> GrGLFuncPtr {
        if (auto proc = eglGetProcAddress(name)) {
            return reinterpret_cast<GrGLFuncPtr>(proc);
        }
        if (ctx) {
            if (auto proc = dlsym(ctx, name)) {
                return reinterpret_cast<GrGLFuncPtr>(proc);
            }
        }
        if (auto proc = dlsym(RTLD_DEFAULT, name)) {
            return reinterpret_cast<GrGLFuncPtr>(proc);
        }
        return nullptr;
    });

    if (!gl_interface) {
        gl_interface = GrGLMakeNativeInterface();
    }

    if (!gl_interface) {
        std::cerr << "[ENKI Overlay] Failed to create Skia GL Interface\n";
        return nullptr;
    }

    return GrDirectContext::MakeGL(gl_interface);
}

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Linux Shell Framework — Overlay Demo    \n";
    std::cout << "  Wayland wlr-layer-shell Overlay & Skia GPU    \n";
    std::cout << "================================================\n";

    // 1. Initialize Platform Backend
    auto plat_res = enki::Platform::create();
    if (!plat_res.isOk()) {
        std::cerr << "[ENKI] Platform init failed: " << plat_res.error().message << "\n";
        return 1;
    }
    auto platform = std::move(plat_res.value());

    const int OVERLAY_WIDTH  = 540;
    const int OVERLAY_HEIGHT = 400;

    std::unique_ptr<enki::LayerSurface> layer_surface;
    std::unique_ptr<enki::Window>       fallback_window;
    bool is_wayland_layer = false;

    // 2. Create Overlay Surface (Wayland Layer Shell or X11 Fallback)
    if (platform->isWayland()) {
        enki::LayerSurfaceConfig cfg;
        cfg.namespace_id   = "enki-logo-overlay";
        cfg.layer          = enki::ShellLayer::Overlay;   // Top-most layer above all windows
        cfg.anchor         = enki::ShellAnchor::None;     // Floating centered overlay
        cfg.width          = OVERLAY_WIDTH;
        cfg.height         = OVERLAY_HEIGHT;
        cfg.exclusive_zone = 0;                          // Overlay does not shift windows
        cfg.keyboard_mode  = enki::KeyboardMode::OnDemand;
        cfg.transparent    = true;
        cfg.vsync          = true;

        auto surf_res = enki::LayerSurface::create(*platform, cfg);
        if (surf_res.isOk()) {
            layer_surface = std::move(surf_res.value());
            is_wayland_layer = true;
            std::cout << "[ENKI] Created Wayland Layer Surface (Overlay Layer)\n";
        }
    }

    if (!layer_surface) {
        // Fallback to transparent borderless window (for X11 or nested tests)
        enki::WindowConfig win_cfg;
        win_cfg.title         = "ENKI Overlay Logo Demo";
        win_cfg.width         = OVERLAY_WIDTH;
        win_cfg.height        = OVERLAY_HEIGHT;
        win_cfg.transparent   = true;
        win_cfg.borderless    = true;
        win_cfg.always_on_top = true;
        win_cfg.vsync         = true;

        auto win_res = enki::Window::create(*platform, win_cfg);
        if (!win_res.isOk()) {
            std::cerr << "[ENKI] Window creation failed: " << win_res.error().message << "\n";
            return 1;
        }
        fallback_window = std::move(win_res.value());
        std::cout << "[ENKI] Created X11 Overlay Window (Transparent / Always-on-top)\n";
    }

    // 3. Setup Skia GPU Context
    if (layer_surface) {
        layer_surface->makeCurrent();
    } else {
        fallback_window->makeCurrent();
    }

    auto gr_context = initSkiaGPU();
    if (!gr_context) {
        std::cerr << "[ENKI] Skia GPU initialization failed\n";
        return 1;
    }

    // 4. Connect input / exit handlers
    bool running = true;
    platform->onQuit().connect([&running]() { running = false; });
    platform->onKeyDown().connect([&running](int key, int mods) {
        // ESC key (XKB / evdev escape = 9 or 0xff1b or 27)
        if (key == 27 || key == 9 || key == 0xff1b || key == 0x09) {
            running = false;
        }
    });

    if (fallback_window) {
        fallback_window->onClose().connect([&running]() { running = false; });
    }

    // Create Skia render surface
    int width  = OVERLAY_WIDTH;
    int height = OVERLAY_HEIGHT;

    GrGLFramebufferInfo fb_info;
    fb_info.fFBOID  = 0;
    fb_info.fFormat = 0x8058; // GL_RGBA8

    GrBackendRenderTarget backend_rt(width, height, 0, 8, fb_info);
    SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);

    auto surface = SkSurface::MakeFromBackendRenderTarget(
        gr_context.get(),
        backend_rt,
        kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr,
        &props
    );

    if (!surface) {
        std::cerr << "[ENKI] SkSurface creation failed\n";
        return 1;
    }

    // Typography
    auto font_mgr = SkFontMgr::RefDefault();
    sk_sp<SkTypeface> typeface(font_mgr->matchFamilyStyle("Inter", SkFontStyle::Bold()));
    if (!typeface) typeface = sk_sp<SkTypeface>(font_mgr->matchFamilyStyle("Ubuntu", SkFontStyle::Bold()));
    if (!typeface) typeface = sk_sp<SkTypeface>(font_mgr->matchFamilyStyle("Sans", SkFontStyle::Bold()));

    SkFont title_font(typeface, 32.0f);
    SkFont subtitle_font(typeface, 11.5f);
    SkFont badge_font(typeface, 10.0f);

    auto start_time = std::chrono::steady_clock::now();

    std::cout << "[ENKI] Overlay running. Press ESC to dismiss.\n";

    // 5. Main Render Loop
    while (running) {
        platform->pollEvents();

        auto now = std::chrono::steady_clock::now();
        float t = std::chrono::duration<float>(now - start_time).count();

        if (layer_surface) {
            layer_surface->makeCurrent();
        } else {
            fallback_window->makeCurrent();
        }

        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);

        float cx = width * 0.5f;
        float cy = height * 0.5f;

        // ── A. Glassmorphism Card Container ───────────────────────────
        SkRect card_rect = SkRect::MakeXYWH(16.0f, 16.0f, width - 32.0f, height - 32.0f);
        SkRRect card_rrect = SkRRect::MakeRectXY(card_rect, 24.0f, 24.0f);

        // 1. Ambient Backdrop Glow
        {
            SkPaint glow_paint;
            glow_paint.setAntiAlias(true);
            glow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 30.0f));

            SkColor glow_colors[] = {
                SkColorSetARGB(100, 0, 240, 255),  // Neon Cyan
                SkColorSetARGB(60, 139, 92, 246),  // Violet
                SkColorSetARGB(0, 0, 0, 0)
            };
            SkPoint glow_pts[] = { {cx, cy - 30.0f}, {cx, cy + 120.0f} };
            glow_paint.setShader(SkGradientShader::MakeLinear(
                glow_pts, glow_colors, nullptr, 3, SkTileMode::kClamp));

            canvas->drawRRect(card_rrect, glow_paint);
        }

        // 2. Translucent Glass Fill
        {
            SkPaint glass_paint;
            glass_paint.setAntiAlias(true);
            glass_paint.setColor(SkColorSetARGB(225, 10, 15, 29)); // Deep acrylic dark
            canvas->drawRRect(card_rrect, glass_paint);
        }

        // 3. Dynamic Animated Glowing Border
        {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(1.75f);

            float angle = t * 1.2f;
            float cos_a = std::cos(angle) * (width * 0.5f);
            float sin_a = std::sin(angle) * (height * 0.5f);

            SkPoint border_pts[] = {
                {cx - cos_a, cy - sin_a},
                {cx + cos_a, cy + sin_a}
            };
            SkColor border_colors[] = {
                SkColorSetARGB(255, 0, 240, 255),   // Neon Cyan
                SkColorSetARGB(220, 168, 85, 247),  // Purple
                SkColorSetARGB(255, 245, 158, 11),  // Amber Gold
                SkColorSetARGB(255, 0, 240, 255)
            };
            float border_pos[] = {0.0f, 0.35f, 0.7f, 1.0f};

            border_paint.setShader(SkGradientShader::MakeLinear(
                border_pts, border_colors, border_pos, 4, SkTileMode::kClamp));

            canvas->drawRRect(card_rrect, border_paint);
        }

        // ── B. The ENKI Insignia / Emblem ─────────────────────────────
        float emblem_y = cy - 45.0f;
        float pulse = 1.0f + 0.04f * std::sin(t * 3.0f);

        canvas->save();
        canvas->translate(cx, emblem_y);
        canvas->scale(pulse, pulse);

        // 1. Rotating Outer Techno-Ring
        {
            SkPaint ring_paint;
            ring_paint.setAntiAlias(true);
            ring_paint.setStyle(SkPaint::kStroke_Style);
            ring_paint.setStrokeWidth(1.5f);
            ring_paint.setColor(SkColorSetARGB(120, 0, 240, 255));

            canvas->save();
            canvas->rotate(t * 25.0f);
            for (int i = 0; i < 4; ++i) {
                canvas->rotate(90.0f);
                SkPath arc;
                arc.addArc(SkRect::MakeLTRB(-48, -48, 48, 48), 15.0f, 60.0f);
                canvas->drawPath(arc, ring_paint);
            }
            canvas->restore();
        }

        // 2. Inner Winged Emblem / Diamond Core
        {
            SkPath emblem_path;
            // Central Diamond
            emblem_path.moveTo(0, -32);
            emblem_path.lineTo(26, 0);
            emblem_path.lineTo(0, 32);
            emblem_path.lineTo(-26, 0);
            emblem_path.close();

            // Wing Brackets
            emblem_path.moveTo(-36, -18);
            emblem_path.lineTo(-44, 0);
            emblem_path.lineTo(-36, 18);

            emblem_path.moveTo(36, -18);
            emblem_path.lineTo(44, 0);
            emblem_path.lineTo(36, 18);

            SkPaint emblem_fill;
            emblem_fill.setAntiAlias(true);
            emblem_fill.setStyle(SkPaint::kFill_Style);

            SkPoint grad_pts[] = { {0, -35}, {0, 35} };
            SkColor grad_colors[] = {
                SkColorSetARGB(255, 0, 240, 255),  // Cyan
                SkColorSetARGB(255, 139, 92, 246)  // Violet
            };
            emblem_fill.setShader(SkGradientShader::MakeLinear(
                grad_pts, grad_colors, nullptr, 2, SkTileMode::kClamp));

            canvas->drawPath(emblem_path, emblem_fill);

            // Glowing Core Center Dot
            SkPaint core_paint;
            core_paint.setAntiAlias(true);
            core_paint.setColor(SK_ColorWHITE);
            core_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4.0f));
            canvas->drawCircle(0, 0, 6.0f + 2.0f * std::sin(t * 5.0f), core_paint);
        }

        canvas->restore();

        // ── C. Typography: ENKI Logo & Subtitle ────────────────────────
        {
            // Title "E N K I"
            SkPaint text_paint;
            text_paint.setAntiAlias(true);

            SkPoint title_pts[] = { {cx - 80, emblem_y + 60}, {cx + 80, emblem_y + 90} };
            SkColor title_colors[] = {
                SkColorSetARGB(255, 255, 255, 255),
                SkColorSetARGB(255, 0, 240, 255)
            };
            text_paint.setShader(SkGradientShader::MakeLinear(
                title_pts, title_colors, nullptr, 2, SkTileMode::kClamp));

            const char* title_text = "E N K I";
            SkRect bounds;
            title_font.measureText(title_text, std::strlen(title_text), SkTextEncoding::kUTF8, &bounds);
            canvas->drawString(title_text, cx - bounds.width() * 0.5f, emblem_y + 80.0f, title_font, text_paint);

            // Subtitle
            SkPaint sub_paint;
            sub_paint.setAntiAlias(true);
            sub_paint.setColor(SkColorSetARGB(180, 148, 163, 184)); // Slate 400

            const char* sub_text = "MODERN LINUX SHELL FRAMEWORK";
            subtitle_font.measureText(sub_text, std::strlen(sub_text), SkTextEncoding::kUTF8, &bounds);
            canvas->drawString(sub_text, cx - bounds.width() * 0.5f, emblem_y + 102.0f, subtitle_font, sub_paint);
        }

        // ── D. Overlay HUD Badges & Dismiss Hint ───────────────────────
        {
            float badge_y = cy + 115.0f;

            // Mode Badge
            const char* badge_text = is_wayland_layer
                ? "WAYLAND LAYER: OVERLAY"
                : "X11 MOCK OVERLAY (TRANSPARENT)";

            SkRect b_bounds;
            badge_font.measureText(badge_text, std::strlen(badge_text), SkTextEncoding::kUTF8, &b_bounds);

            float bw = b_bounds.width() + 20.0f;
            float bh = 22.0f;
            SkRect b_rect = SkRect::MakeXYWH(cx - bw * 0.5f, badge_y, bw, bh);
            SkRRect b_rrect = SkRRect::MakeRectXY(b_rect, 11.0f, 11.0f);

            SkPaint b_bg;
            b_bg.setAntiAlias(true);
            b_bg.setColor(SkColorSetARGB(70, 0, 240, 255));
            canvas->drawRRect(b_rrect, b_bg);

            SkPaint b_stroke;
            b_stroke.setAntiAlias(true);
            b_stroke.setStyle(SkPaint::kStroke_Style);
            b_stroke.setStrokeWidth(1.0f);
            b_stroke.setColor(SkColorSetARGB(140, 0, 240, 255));
            canvas->drawRRect(b_rrect, b_stroke);

            SkPaint b_text_paint;
            b_text_paint.setAntiAlias(true);
            b_text_paint.setColor(SkColorSetARGB(240, 0, 240, 255));
            canvas->drawString(badge_text, cx - b_bounds.width() * 0.5f, badge_y + 15.0f, badge_font, b_text_paint);

            // Dismiss Hint
            SkPaint hint_paint;
            hint_paint.setAntiAlias(true);
            hint_paint.setColor(SkColorSetARGB(130, 100, 116, 139)); // Slate 500

            const char* hint_text = "Press ESC to close";
            badge_font.measureText(hint_text, std::strlen(hint_text), SkTextEncoding::kUTF8, &b_bounds);
            canvas->drawString(hint_text, cx - b_bounds.width() * 0.5f, badge_y + 38.0f, badge_font, hint_paint);
        }

        // Flush Skia commands to GPU
        gr_context->flushAndSubmit();

        // Swap EGL front/back buffers
        if (layer_surface) {
            layer_surface->swapBuffers();
        } else {
            fallback_window->swapBuffers();
        }

        // Maintain ~60-120 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "[ENKI] Overlay closed successfully.\n";
    return 0;
}
