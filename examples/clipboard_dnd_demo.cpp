/// @file clipboard_dnd_demo.cpp
/// @brief ENKI Interactive Clipboard & Drag-and-Drop (DnD) Demo.
/// Demonstrates high-performance native Wayland & X11 clipboard access and XDnD/wl_data_device events.

#include "enki/platform/platform.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/platform/window.hpp"
#include "enki/platform/clipboard.hpp"
#include "enki/platform/dnd.hpp"

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
#include <string>
#include <vector>

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
        std::cerr << "[ENKI Demo] Failed to create Skia GL Interface\n";
        return nullptr;
    }

    return GrDirectContext::MakeGL(gl_interface);
}

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Platform — Clipboard & DnD Showcase     \n";
    std::cout << "  Native Wayland wl_data_device & X11 XDnD      \n";
    std::cout << "================================================\n";

    auto plat_res = enki::Platform::create();
    if (!plat_res.isOk()) {
        std::cerr << "[ENKI] Platform init failed: " << plat_res.error().message << "\n";
        return 1;
    }
    auto platform = std::move(plat_res.value());

    std::cout << "[ENKI] Running on Active Backend: "
              << (platform->isWayland() ? "Wayland (Native wl_data_device)" : "X11 (XDnD & ICCCM)")
              << "\n";

    const int WIN_WIDTH  = 640;
    const int WIN_HEIGHT = 480;

    enki::WindowConfig win_cfg;
    win_cfg.title       = "ENKI Clipboard & DnD Demo";
    win_cfg.width       = WIN_WIDTH;
    win_cfg.height      = WIN_HEIGHT;
    win_cfg.transparent = true;
    win_cfg.vsync       = true;

    auto win_res = enki::Window::create(*platform, win_cfg);
    if (!win_res.isOk()) {
        std::cerr << "[ENKI] Window creation failed: " << win_res.error().message << "\n";
        return 1;
    }
    auto window = std::move(win_res.value());
    window->makeCurrent();

    auto gr_context = initSkiaGPU();
    if (!gr_context) {
        std::cerr << "[ENKI] Skia GPU initialization failed\n";
        return 1;
    }

    // App state
    bool running = true;
    std::string clipboard_status = "Ready. Press [C] to Copy, [V] to Paste, or drag files onto window.";
    std::string last_dropped_text = "(No data dropped yet)";
    bool is_drag_over = false;
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;

    platform->onQuit().connect([&running]() { running = false; });
    window->onClose().connect([&running]() { running = false; });

    // Connect DnD signals
    platform->onDragEnter().connect([&](enki::DragEnterEvent& ev) {
        is_drag_over = true;
        ev.accept(enki::DragAction::Copy);
        clipboard_status = "Drag entered! Supported formats: " + std::to_string(ev.mime_types.size());
    });

    platform->onDragMotion().connect([&](enki::DragMotionEvent& ev) {
        ev.accept(enki::DragAction::Copy);
    });

    platform->onDragLeave().connect([&](const enki::DragLeaveEvent&) {
        is_drag_over = false;
        clipboard_status = "Drag left window.";
    });

    platform->onDrop().connect([&](enki::DropEvent& ev) {
        is_drag_over = false;
        if (ev.data) {
            std::string text = ev.data->readText();
            auto uris = ev.data->readUris();
            if (!uris.empty()) {
                last_dropped_text = "Dropped " + std::to_string(uris.size()) + " URIs:\n" + uris[0];
                clipboard_status = "Successfully received dropped URI(s)!";
            } else if (!text.empty()) {
                last_dropped_text = "Dropped Text:\n" + text;
                clipboard_status = "Successfully received dropped text!";
            } else {
                last_dropped_text = "Dropped raw data with formats: " + std::to_string(ev.data->formats().size());
                clipboard_status = "Received dropped payload.";
            }
            ev.handled = true;
        }
    });

    // Keyboard handlers for Copy / Paste actions
    platform->onKeyDown().connect([&](int key, int /*mods*/) {
        // ESC = Exit
        if (key == 27 || key == 9 || key == 0xff1b || key == 0x09) {
            running = false;
        }
        // 'C' or 'c' = Copy sample text to platform clipboard
        if (key == 'c' || key == 'C' || key == 54 || key == 0x63) {
            enki::ClipboardData clip;
            clip.setText("Enki Shell: Native Wayland & X11 Clipboard Data Payload!");
            clip.setUris({"file:///usr/share/enki/wallpaper.png", "https://github.com/enki/shell"});
            platform->setClipboardData(clip, enki::ClipboardType::Clipboard);
            clipboard_status = "Copied sample text & URIs to system clipboard!";
        }
        // 'V' or 'v' = Paste from platform clipboard
        if (key == 'v' || key == 'V' || key == 55 || key == 0x76) {
            auto clip = platform->getClipboardData(enki::ClipboardType::Clipboard);
            if (clip.hasText()) {
                last_dropped_text = "Pasted from Clipboard:\n" + clip.getText();
                clipboard_status = "Successfully read clipboard text!";
            } else if (clip.hasUris()) {
                auto uris = clip.getUris();
                last_dropped_text = "Pasted " + std::to_string(uris.size()) + " URIs:\n" + uris[0];
                clipboard_status = "Successfully read clipboard URIs!";
            } else {
                clipboard_status = "Clipboard is currently empty or format unsupported.";
            }
        }
    });

    platform->onMouseMove().connect([&](float x, float y) {
        mouse_x = x;
        mouse_y = y;
    });

    // Render target
    GrGLFramebufferInfo fb_info;
    fb_info.fFBOID  = 0;
    fb_info.fFormat = 0x8058; // GL_RGBA8

    GrBackendRenderTarget backend_rt(WIN_WIDTH, WIN_HEIGHT, 0, 8, fb_info);
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

    auto font_mgr = SkFontMgr::RefDefault();
    sk_sp<SkTypeface> typeface(font_mgr->matchFamilyStyle("Inter", SkFontStyle::Bold()));
    if (!typeface) typeface = sk_sp<SkTypeface>(font_mgr->matchFamilyStyle("Ubuntu", SkFontStyle::Bold()));
    if (!typeface) typeface = sk_sp<SkTypeface>(font_mgr->matchFamilyStyle("Sans", SkFontStyle::Bold()));

    SkFont title_font(typeface, 22.0f);
    SkFont label_font(typeface, 13.0f);
    SkFont mono_font(typeface, 11.5f);

    auto start_time = std::chrono::steady_clock::now();

    while (running) {
        platform->pollEvents();
        window->makeCurrent();

        auto now = std::chrono::steady_clock::now();
        float t = std::chrono::duration<float>(now - start_time).count();

        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);

        // Background Glass Panel
        SkRect card_rect = SkRect::MakeXYWH(16.0f, 16.0f, WIN_WIDTH - 32.0f, WIN_HEIGHT - 32.0f);
        SkRRect card_rrect = SkRRect::MakeRectXY(card_rect, 20.0f, 20.0f);

        // Glass background
        SkPaint glass_paint;
        glass_paint.setAntiAlias(true);
        glass_paint.setColor(SkColorSetARGB(230, 15, 23, 42)); // Slate 900
        canvas->drawRRect(card_rrect, glass_paint);

        // Border (highlighted when drag is hovering)
        SkPaint border_paint;
        border_paint.setAntiAlias(true);
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(is_drag_over ? 3.0f : 1.5f);
        border_paint.setColor(is_drag_over ? SkColorSetARGB(255, 34, 197, 94) : SkColorSetARGB(180, 56, 189, 248));
        canvas->drawRRect(card_rrect, border_paint);

        // Title
        SkPaint title_paint;
        title_paint.setAntiAlias(true);
        title_paint.setColor(SK_ColorWHITE);
        canvas->drawString("ENKI Clipboard & Drag-and-Drop", 40.0f, 58.0f, title_font, title_paint);

        // Backend Indicator
        std::string backend_str = platform->isWayland() ? "Backend: Wayland (wl_data_device)" : "Backend: X11 (XDnD & ICCCM)";
        SkPaint backend_paint;
        backend_paint.setAntiAlias(true);
        backend_paint.setColor(SkColorSetARGB(200, 148, 163, 184));
        canvas->drawString(backend_str.c_str(), 40.0f, 82.0f, mono_font, backend_paint);

        // Drop Zone Box
        SkRect drop_box = SkRect::MakeXYWH(40.0f, 105.0f, WIN_WIDTH - 80.0f, 140.0f);
        SkRRect drop_rrect = SkRRect::MakeRectXY(drop_box, 14.0f, 14.0f);

        SkPaint drop_box_bg;
        drop_box_bg.setAntiAlias(true);
        drop_box_bg.setColor(is_drag_over ? SkColorSetARGB(60, 34, 197, 94) : SkColorSetARGB(40, 30, 41, 59));
        canvas->drawRRect(drop_rrect, drop_box_bg);

        SkPaint drop_box_stroke;
        drop_box_stroke.setAntiAlias(true);
        drop_box_stroke.setStyle(SkPaint::kStroke_Style);
        drop_box_stroke.setStrokeWidth(1.5f);
        drop_box_stroke.setColor(is_drag_over ? SkColorSetARGB(255, 34, 197, 94) : SkColorSetARGB(100, 100, 116, 139));
        canvas->drawRRect(drop_rrect, drop_box_stroke);

        // Drop Zone Text
        SkPaint drop_text_paint;
        drop_text_paint.setAntiAlias(true);
        drop_text_paint.setColor(is_drag_over ? SkColorSetARGB(255, 34, 197, 94) : SkColorSetARGB(220, 226, 232, 240));
        const char* drop_prompt = is_drag_over ? "[ Release to Drop Data Here ]" : "[ Drop Zone: Drag Files / Text Here ]";
        canvas->drawString(drop_prompt, 60.0f, 135.0f, label_font, drop_text_paint);

        // Display dropped content
        SkPaint content_paint;
        content_paint.setAntiAlias(true);
        content_paint.setColor(SkColorSetARGB(220, 148, 163, 184));
        canvas->drawString(last_dropped_text.c_str(), 60.0f, 175.0f, mono_font, content_paint);

        // Interactive Key Shortcuts Box
        SkRect act_box = SkRect::MakeXYWH(40.0f, 260.0f, WIN_WIDTH - 80.0f, 110.0f);
        SkRRect act_rrect = SkRRect::MakeRectXY(act_box, 14.0f, 14.0f);
        SkPaint act_bg;
        act_bg.setAntiAlias(true);
        act_bg.setColor(SkColorSetARGB(40, 30, 41, 59));
        canvas->drawRRect(act_rrect, act_bg);

        SkPaint act_header;
        act_header.setAntiAlias(true);
        act_header.setColor(SkColorSetARGB(255, 56, 189, 248)); // Sky 400
        canvas->drawString("KEYBOARD ACTIONS:", 60.0f, 290.0f, label_font, act_header);

        SkPaint act_text;
        act_text.setAntiAlias(true);
        act_text.setColor(SkColorSetARGB(200, 226, 232, 240));
        canvas->drawString("[C]  Copy Sample Payload (Text & URIs) to Clipboard", 60.0f, 315.0f, mono_font, act_text);
        canvas->drawString("[V]  Paste and inspect Data from System Clipboard", 60.0f, 335.0f, mono_font, act_text);
        canvas->drawString("[ESC] Exit application", 60.0f, 355.0f, mono_font, act_text);

        // Status Bar Footer
        SkPaint status_bg;
        status_bg.setAntiAlias(true);
        status_bg.setColor(SkColorSetARGB(50, 15, 23, 42));
        SkRect status_rect = SkRect::MakeXYWH(40.0f, 385.0f, WIN_WIDTH - 80.0f, 40.0f);
        canvas->drawRRect(SkRRect::MakeRectXY(status_rect, 8.0f, 8.0f), status_bg);

        SkPaint status_paint;
        status_paint.setAntiAlias(true);
        status_paint.setColor(SkColorSetARGB(255, 251, 191, 36)); // Amber 400
        canvas->drawString(clipboard_status.c_str(), 55.0f, 410.0f, mono_font, status_paint);

        gr_context->flushAndSubmit();
        window->swapBuffers();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "[ENKI] Demo exiting cleanly.\n";
    return 0;
}
