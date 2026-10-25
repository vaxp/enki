#pragma once
/// @file canvas.hpp
/// @brief 2D drawing canvas abstraction with comprehensive text & typography rendering.

#include "enki/core/types.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/rendering/path.hpp"
#include <memory>
#include <string_view>

namespace enki {

class Image;

/// Abstract canvas for drawing shapes, text, and images.
class Canvas {
public:
    Canvas() = default;
    virtual ~Canvas() = default;

    // State management
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void saveLayerAlpha(float alpha, const Rect* bounds = nullptr) = 0;
    virtual void saveLayer(const Rect* bounds, const Paint* paint) = 0;
    virtual void saveLayerWithBackdrop(const Rect* bounds, const Paint* paint, const ImageFilter* backdrop) = 0;
    
    // Transformations
    virtual void translate(float dx, float dy) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void rotate(float degrees) = 0;
    virtual void clipRect(const Rect& rect, bool anti_alias = true) = 0;
    virtual void clipRRect(const Rect& rect, const BorderRadius& radius, bool anti_alias = true) = 0;
    virtual void clipOval(const Rect& rect, bool anti_alias = true) = 0;
    virtual void clipPath(const Path& path, bool anti_alias = true) = 0;
    virtual void concat(const float matrix[9]) = 0;
    virtual void skew(float sx, float sy) = 0;

    // Drawing operations
    virtual void clear(Color color) = 0;
    virtual void drawLine(Point p1, Point p2, const Paint& paint) = 0;
    virtual void drawRect(const Rect& rect, const Paint& paint) = 0;
    virtual void drawRRect(const Rect& rect, const BorderRadius& radius, const Paint& paint) = 0;
    virtual void drawCircle(Point center, float radius, const Paint& paint) = 0;
    virtual void drawPath(const Path& path, const Paint& paint) = 0;

    // Shadows & Elevation
    virtual void drawRRectShadow(const Rect& rect, const BorderRadius& radius, Color shadow_color, float blur, Point offset = {0.0f, 0.0f}) = 0;
    virtual void drawRectShadow(const Rect& rect, Color shadow_color, float blur, Point offset = {0.0f, 0.0f}) = 0;
    virtual void drawCircleShadow(Point center, float radius, Color shadow_color, float blur, Point offset = {0.0f, 0.0f}) = 0;

    // Glassmorphism, Blur & Glow
    virtual void drawGlassPanel(const Rect& rect, const BorderRadius& radius, float blur_sigma, Color tint_color, Color border_highlight = 0x33FFFFFF) = 0;
    virtual void drawBackdropBlur(const Rect& rect, const BorderRadius& radius, float sigma_x, float sigma_y = 0.0f) = 0;
    virtual void drawGlow(const Rect& rect, const BorderRadius& radius, Color glow_color, float radius_extent) = 0;

    // Dashed Primitives
    virtual void drawDashedLine(Point p1, Point p2, const Paint& paint, float dash_len, float gap_len, float phase = 0.0f) = 0;
    virtual void drawDashedRRect(const Rect& rect, const BorderRadius& radius, const Paint& paint, float dash_len, float gap_len, float phase = 0.0f) = 0;
    virtual void drawDashedPath(const Path& path, const Paint& paint, float dash_len, float gap_len, float phase = 0.0f) = 0;
    
    /// Draw an arc within the given rectangle.
    virtual void drawArc(const Rect& rect, float startAngle, float sweepAngle,
                         bool useCenter, const Paint& paint) = 0;

    /// Draw an oval inscribed in the given rectangle.
    virtual void drawOval(const Rect& rect, const Paint& paint) = 0;

    /// Draw a series of points.
    virtual void drawPoints(const Point* points, size_t count, const Paint& paint) = 0;

    /// Draw a text string with baseline offset, optional font family and boldness.
    virtual void drawText(std::string_view text, Point pos, const Paint& paint, float font_size,
                          const char* font_family = nullptr, bool bold = false) = 0;

    /// Measure text width in pixels.
    virtual float measureText(std::string_view text, float font_size,
                              const char* font_family = nullptr, bool bold = false) = 0;

    virtual void drawImageRect(std::shared_ptr<Image> image, const Rect& src, const Rect& dst, const Paint* paint = nullptr) = 0;
    virtual void drawDRRect(const Rect& outer, const BorderRadius& outerRadius, 
                            const Rect& inner, const BorderRadius& innerRadius, const Paint& paint) = 0;
    virtual void drawParagraph(void* paragraph, float x, float y) = 0;

    /// Get native SkCanvas* (for advanced usage)
    virtual void* getNativeHandle() = 0;
};

/// Factory function to wrap a SkCanvas pointer in a Canvas object.
std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas);

}  // namespace enki

