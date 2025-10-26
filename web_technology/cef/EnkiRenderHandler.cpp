/// @file EnkiRenderHandler.cpp
/// @brief Offscreen Rendering (OSR) implementation.
///
/// @copyright ENKI Framework — MIT License

#include "EnkiRenderHandler.hpp"

namespace enki::web {

EnkiRenderHandler::EnkiRenderHandler(OnPaintCallback paint_cb)
    : paint_cb_(std::move(paint_cb))
{}

void EnkiRenderHandler::setSize(int w, int h) {
    std::lock_guard<std::mutex> lk(size_mutex_);
    width_  = w;
    height_ = h;
}

void EnkiRenderHandler::setDeviceScale(float scale) {
    std::lock_guard<std::mutex> lk(size_mutex_);
    scale_ = scale;
}

// ── CefRenderHandler overrides ─────────────────────────────────

void EnkiRenderHandler::GetViewRect(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRect& rect)
{
    std::lock_guard<std::mutex> lk(size_mutex_);
    rect.x      = 0;
    rect.y      = 0;
    rect.width  = width_;
    rect.height = height_;
}

bool EnkiRenderHandler::GetScreenInfo(CefRefPtr<CefBrowser> /*browser*/,
                                      CefScreenInfo& info)
{
    std::lock_guard<std::mutex> lk(size_mutex_);

    info.device_scale_factor = scale_;
    info.depth               = 32;
    info.depth_per_component = 8;
    info.is_monochrome       = false;
    info.rect                = {0, 0, width_, height_};
    info.available_rect      = {0, 0, width_, height_};
    return true;
}

void EnkiRenderHandler::OnPaint(CefRefPtr<CefBrowser> /*browser*/,
                                PaintElementType       type,
                                const RectList&        dirtyRects,
                                const void*            buffer,
                                int                    width,
                                int                    height)
{
    if (!paint_cb_ || !buffer || width <= 0 || height <= 0) return;

    // Convert CEF dirty rects → web::DirtyRect
    std::vector<DirtyRect> dirty;
    dirty.reserve(dirtyRects.size());
    for (const auto& r : dirtyRects) {
        dirty.push_back({r.x, r.y, r.width, r.height});
    }

    // Deep-copy the buffer so we can deliver it asynchronously.
    // CEF frees `buffer` after OnPaint returns, so borrowing is unsafe
    // unless the callback is guaranteed to complete synchronously.
    // We always deep-copy for safety; a future optimisation could use
    // a shared_ptr<uint8_t[]> pool to reduce allocations.
    bool is_popup = (type == PET_POPUP);
    WebViewFrame frame = WebViewFrame::deepCopy(buffer, width, height,
                                                is_popup, dirty);

    // Invoke the callback (runs on the CEF Renderer thread).
    // The WebView widget is responsible for posting to the Enki main thread.
    paint_cb_(std::move(frame));
}

} // namespace enki::web
