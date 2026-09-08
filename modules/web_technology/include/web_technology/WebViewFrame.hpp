#pragma once
/// @file WebViewFrame.hpp
/// @brief Represents a rendered pixel frame produced by the WebView backend.
///
/// The CEF RenderHandler delivers pixel data via OnPaint(). This struct
/// wraps that data so it can travel from the CEF thread to the Enki
/// render thread without exposing any CEF types.
///
/// @copyright ENKI Framework — MIT License

#include <cstdint>
#include <vector>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// Dirty Rectangle — sub-region that changed in this frame
// ════════════════════════════════════════════════════════════════

struct DirtyRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

// ════════════════════════════════════════════════════════════════
// WebViewFrame — one rendered frame from CEF OnPaint
// ════════════════════════════════════════════════════════════════

/// @brief Carries raw pixel data produced by CEF's offscreen renderer.
///
/// The pixel format is always **BGRA 32-bit** (4 bytes per pixel),
/// which is what CEF delivers natively and what Skia's SkBitmap
/// accepts directly as kBGRA_8888_SkColorType.
///
/// Layout in memory:
///   pixel[y * width + x] starts at bytes: (y * width + x) * 4
///   byte 0 = Blue, byte 1 = Green, byte 2 = Red, byte 3 = Alpha
///
/// Ownership:
///   When @p owns_data is true the struct owns the buffer and will
///   free it on destruction.  When false the pointer is borrowed
///   (only valid during the OnPaint callback lifetime).
struct WebViewFrame {
    const void*            pixels      = nullptr;
    int                    width       = 0;
    int                    height      = 0;
    bool                   is_popup    = false;   // true → overlay popup frame
    std::vector<DirtyRect> dirty_rects;

    // When we need to copy the frame for async delivery:
    std::vector<uint8_t>   owned_data;
    bool                   owns_data   = false;

    // ── Factory: deep-copy for async delivery ──────────────────

    /// Create a frame that owns a copy of the pixel data.
    /// Use this when you need to keep the frame beyond the OnPaint callback.
    static WebViewFrame deepCopy(
        const void*             pixels,
        int                     width,
        int                     height,
        bool                    is_popup,
        const std::vector<DirtyRect>& dirty)
    {
        WebViewFrame f;
        f.width      = width;
        f.height     = height;
        f.is_popup   = is_popup;
        f.dirty_rects = dirty;
        f.owns_data  = true;

        const std::size_t bytes = static_cast<std::size_t>(width * height * 4);
        f.owned_data.resize(bytes);
        if (pixels && bytes > 0) {
            std::copy(
                static_cast<const uint8_t*>(pixels),
                static_cast<const uint8_t*>(pixels) + bytes,
                f.owned_data.data());
        }
        f.pixels = f.owned_data.data();
        return f;
    }

    /// Create a borrowed (zero-copy) view into existing memory.
    /// Only valid for the duration of the OnPaint callback.
    static WebViewFrame borrow(
        const void*             pixels,
        int                     width,
        int                     height,
        bool                    is_popup,
        const std::vector<DirtyRect>& dirty)
    {
        WebViewFrame f;
        f.pixels     = pixels;
        f.width      = width;
        f.height     = height;
        f.is_popup   = is_popup;
        f.dirty_rects = dirty;
        f.owns_data  = false;
        return f;
    }

    // ── Geometry helpers ───────────────────────────────────────

    [[nodiscard]] bool empty() const {
        return pixels == nullptr || width <= 0 || height <= 0;
    }

    [[nodiscard]] std::size_t byteSize() const {
        return static_cast<std::size_t>(width * height * 4);
    }
};

} // namespace enki::web
