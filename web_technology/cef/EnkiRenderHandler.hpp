#pragma once
/// @file EnkiRenderHandler.hpp
/// @brief CefRenderHandler — the heart of Offscreen Rendering (OSR).
///
/// CEF calls OnPaint() on the Renderer thread every time a frame changes.
/// EnkiRenderHandler deep-copies the pixel buffer into a WebViewFrame
/// and delivers it to the Enki main thread via the OnPaintCallback.
///
/// Pixel format: BGRA 32-bit (kBGRA_8888 in Skia terms).
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_render_handler.h>
#include <web_technology/IWebViewBackend.hpp>
#include <mutex>
#include <atomic>

namespace enki::web {

class EnkiRenderHandler : public CefRenderHandler {
public:
    explicit EnkiRenderHandler(OnPaintCallback paint_cb);

    // ── Size control ───────────────────────────────────────────

    /// Update the logical size the browser should render at.
    void setSize(int w, int h);

    /// Update device pixel ratio.
    void setDeviceScale(float scale);

    // ── CefRenderHandler ───────────────────────────────────────

    /// CEF asks for the view rect (logical size at 1x scale).
    void GetViewRect(CefRefPtr<CefBrowser> browser,
                     CefRect& rect) override;

    /// CEF asks for the screen info (DPI / device scale).
    bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                       CefScreenInfo& info) override;

    /// CEF delivers a freshly rendered frame (or partial update).
    ///
    /// @param type     PET_VIEW (main) or PET_POPUP (overlay).
    /// @param dirtyRects  Rectangles that changed.
    /// @param buffer   Raw BGRA pixels; valid only during this call.
    /// @param width    Full frame width in pixels.
    /// @param height   Full frame height in pixels.
    void OnPaint(CefRefPtr<CefBrowser>     browser,
                 PaintElementType          type,
                 const RectList&           dirtyRects,
                 const void*               buffer,
                 int                       width,
                 int                       height) override;

private:
    OnPaintCallback  paint_cb_;

    std::mutex      size_mutex_;
    int             width_  = 800;
    int             height_ = 600;
    float           scale_  = 1.0f;

    IMPLEMENT_REFCOUNTING(EnkiRenderHandler);
};

} // namespace enki::web
