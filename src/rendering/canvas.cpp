/// @file canvas.cpp
/// @brief Canvas implementation using Skia with full Linux FontConfig typography support.

#include "enki/rendering/canvas.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/core/SkTextBlob.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkBlendMode.h>
#include <include/core/SkShader.h>
#include <include/core/SkColorFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkImage.h>
#include <include/core/SkSamplingOptions.h>
#include <modules/skparagraph/include/Paragraph.h>
#include "enki/rendering/image.hpp"
#include <algorithm>

namespace enki {

namespace {

// Global Font Manager singleton for Linux
sk_sp<SkFontMgr> getGlobalFontMgr() {
    static sk_sp<SkFontMgr> mgr = []() {
        auto m = SkFontMgr_New_FontConfig(nullptr);
        if (!m) m = SkFontMgr::RefDefault();
        return m;
    }();
    return mgr;
}

sk_sp<SkTypeface> getTypeface(const char* family, bool bold) {
    static std::unordered_map<std::string, sk_sp<SkTypeface>> s_cache;
    std::string key = (family ? family : "default") + std::string(bold ? ":bold" : ":normal");
    
    auto it = s_cache.find(key);
    if (it != s_cache.end()) {
        return it->second;
    }

    auto mgr = getGlobalFontMgr();
    SkFontStyle style = bold ? SkFontStyle::Bold() : SkFontStyle::Normal();
    sk_sp<SkTypeface> result;

    if (mgr) {
        if (family && family[0] != '\0') {
            if (auto tf = mgr->matchFamilyStyle(family, style)) {
                result = sk_sp<SkTypeface>(tf);
            }
        }
        if (!result) {
            if (auto tf = mgr->matchFamilyStyle("Inter", style)) result = sk_sp<SkTypeface>(tf);
            else if (auto tf = mgr->matchFamilyStyle("Roboto", style)) result = sk_sp<SkTypeface>(tf);
            else if (auto tf = mgr->matchFamilyStyle("Ubuntu", style)) result = sk_sp<SkTypeface>(tf);
            else if (auto tf = mgr->matchFamilyStyle("DejaVu Sans", style)) result = sk_sp<SkTypeface>(tf);
            else if (auto tf = mgr->matchFamilyStyle("sans-serif", style)) result = sk_sp<SkTypeface>(tf);
            else if (auto tf = mgr->matchFamilyStyle(nullptr, style)) result = sk_sp<SkTypeface>(tf);
        }
    }

    if (!result) {
        result = SkTypeface::MakeDefault();
    }

    s_cache[key] = result;
    return result;
}

// Helper to convert enki::Paint to SkPaint
SkPaint toSkPaint(const Paint& paint) {
    SkPaint sk_paint;
    sk_paint.setColor(paint.getColor());
    sk_paint.setAntiAlias(paint.isAntiAlias());
    sk_paint.setStrokeWidth(paint.getStrokeWidth());

    switch (paint.getStyle()) {
        case PaintStyle::Fill:          sk_paint.setStyle(SkPaint::kFill_Style); break;
        case PaintStyle::Stroke:        sk_paint.setStyle(SkPaint::kStroke_Style); break;
        case PaintStyle::StrokeAndFill: sk_paint.setStyle(SkPaint::kStrokeAndFill_Style); break;
    }

    switch (paint.getStrokeCap()) {
        case StrokeCap::Butt:   sk_paint.setStrokeCap(SkPaint::kButt_Cap); break;
        case StrokeCap::Round:  sk_paint.setStrokeCap(SkPaint::kRound_Cap); break;
        case StrokeCap::Square: sk_paint.setStrokeCap(SkPaint::kSquare_Cap); break;
    }

    switch (paint.getStrokeJoin()) {
        case StrokeJoin::Miter: sk_paint.setStrokeJoin(SkPaint::kMiter_Join); break;
        case StrokeJoin::Round: sk_paint.setStrokeJoin(SkPaint::kRound_Join); break;
        case StrokeJoin::Bevel: sk_paint.setStrokeJoin(SkPaint::kBevel_Join); break;
    }

    sk_paint.setBlendMode(static_cast<SkBlendMode>(paint.getBlendMode()));

    if (auto shader = paint.getShader()) {
        sk_paint.setShader(sk_ref_sp(static_cast<SkShader*>(shader->getNativeHandle())));
    }
    if (auto filter = paint.getImageFilter()) {
        sk_paint.setImageFilter(sk_ref_sp(static_cast<SkImageFilter*>(filter->getNativeHandle())));
    }
    if (auto filter = paint.getColorFilter()) {
        sk_paint.setColorFilter(sk_ref_sp(static_cast<SkColorFilter*>(filter->getNativeHandle())));
    }

    if (paint.hasShadow()) {
        sk_paint.setImageFilter(SkImageFilters::DropShadow(
            paint.getShadowDx(), paint.getShadowDy(),
            paint.getShadowBlur(), paint.getShadowBlur(),
            paint.getShadowColor(),
            nullptr, nullptr
        ));
    }

    return sk_paint;
}

// Concrete Skia canvas wrapper
class SkiaCanvas : public Canvas {
public:
    explicit SkiaCanvas(SkCanvas* canvas) : canvas_(canvas) {}

    void save() override { canvas_->save(); }
    void restore() override { canvas_->restore(); }
    
    void saveLayerAlpha(float alpha, const Rect* bounds = nullptr) override {
        SkRect sk_rect;
        if (bounds) {
            sk_rect = SkRect::MakeXYWH(bounds->x, bounds->y, bounds->width, bounds->height);
        }
        
        SkPaint paint;
        paint.setAlphaf(std::clamp(alpha, 0.0f, 1.0f));
        canvas_->saveLayer(bounds ? &sk_rect : nullptr, &paint);
    }
    
    void saveLayer(const Rect* bounds, const Paint* paint) override {
        SkRect sk_rect;
        if (bounds) {
            sk_rect = SkRect::MakeXYWH(bounds->x, bounds->y, bounds->width, bounds->height);
        }
        
        SkPaint sk_paint;
        if (paint) {
            sk_paint = toSkPaint(*paint);
        }
        canvas_->saveLayer(bounds ? &sk_rect : nullptr, paint ? &sk_paint : nullptr);
    }
    
    void translate(float dx, float dy) override { canvas_->translate(dx, dy); }
    void scale(float sx, float sy) override { canvas_->scale(sx, sy); }
    void rotate(float degrees) override { canvas_->rotate(degrees); }
    
    void concat(const float matrix[9]) override {
        SkMatrix m;
        m.set9(matrix);
        canvas_->concat(m);
    }
    
    void skew(float sx, float sy) override {
        canvas_->skew(sx, sy);
    }

    void clipRect(const Rect& rect) override {
        canvas_->clipRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height), true);
    }

    void clipRRect(const Rect& rect, const BorderRadius& radius) override {
        SkRect sk_rect = SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height);
        SkVector radii[4] = {
            {radius.top_left, radius.top_left},
            {radius.top_right, radius.top_right},
            {radius.bottom_right, radius.bottom_right},
            {radius.bottom_left, radius.bottom_left}
        };
        SkRRect rrect;
        rrect.setRectRadii(sk_rect, radii);
        canvas_->clipRRect(rrect, true);
    }

    void clear(Color color) override {
        canvas_->clear(color);
    }

    void drawLine(Point p1, Point p2, const Paint& paint) override {
        canvas_->drawLine(p1.x, p1.y, p2.x, p2.y, toSkPaint(paint));
    }

    void drawRect(const Rect& rect, const Paint& paint) override {
        canvas_->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height), toSkPaint(paint));
    }

    void drawRRect(const Rect& rect, const BorderRadius& radius, const Paint& paint) override {
        SkRect sk_rect = SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height);
        SkVector radii[4] = {
            {radius.top_left, radius.top_left},
            {radius.top_right, radius.top_right},
            {radius.bottom_right, radius.bottom_right},
            {radius.bottom_left, radius.bottom_left}
        };
        SkRRect rrect;
        rrect.setRectRadii(sk_rect, radii);
        canvas_->drawRRect(rrect, toSkPaint(paint));
    }

    void drawCircle(Point center, float radius, const Paint& paint) override {
        canvas_->drawCircle(center.x, center.y, radius, toSkPaint(paint));
    }

    void drawPath(const Path& path, const Paint& paint) override {
        SkPath* sk_path = static_cast<SkPath*>(path.getNativeHandle());
        if (sk_path) {
            canvas_->drawPath(*sk_path, toSkPaint(paint));
        }
    }
    
    void drawText(std::string_view text, Point pos, const Paint& paint, float font_size,
                  const char* font_family, bool bold) override {
        if (text.empty()) return;
        auto tf = getTypeface(font_family, bold);
        SkFont font(tf, font_size);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
        auto blob = SkTextBlob::MakeFromText(text.data(), text.size(), font, SkTextEncoding::kUTF8);
        if (blob) {
            canvas_->drawTextBlob(blob, pos.x, pos.y, toSkPaint(paint));
        }
    }

    float measureText(std::string_view text, float font_size,
                      const char* font_family, bool bold) override {
        if (text.empty()) return 0.0f;
        auto tf = getTypeface(font_family, bold);
        SkFont font(tf, font_size);
        return font.measureText(text.data(), text.size(), SkTextEncoding::kUTF8);
    }

    void drawArc(const Rect& rect, float startAngle, float sweepAngle,
                 bool useCenter, const Paint& paint) override {
        SkRect sk_rect = SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height);
        canvas_->drawArc(sk_rect, startAngle, sweepAngle, useCenter, toSkPaint(paint));
    }

    void drawOval(const Rect& rect, const Paint& paint) override {
        SkRect sk_rect = SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height);
        canvas_->drawOval(sk_rect, toSkPaint(paint));
    }

    void drawPoints(const Point* points, size_t count, const Paint& paint) override {
        std::vector<SkPoint> sk_points(count);
        for (size_t i = 0; i < count; ++i) {
            sk_points[i] = SkPoint::Make(points[i].x, points[i].y);
        }
        canvas_->drawPoints(SkCanvas::kPoints_PointMode,
                            sk_points.size(), sk_points.data(), toSkPaint(paint));
    }

    void drawImageRect(std::shared_ptr<Image> image, const Rect& src, const Rect& dst, const Paint* paint) override {
        if (!image) return;
        SkImage* sk_image = static_cast<SkImage*>(image->getNativeHandle());
        if (!sk_image) return;
        
        SkRect sk_src = SkRect::MakeXYWH(src.x, src.y, src.width, src.height);
        SkRect sk_dst = SkRect::MakeXYWH(dst.x, dst.y, dst.width, dst.height);
        
        SkPaint sk_paint;
        if (paint) sk_paint = toSkPaint(*paint);
        
        canvas_->drawImageRect(sk_image, sk_src, sk_dst, SkSamplingOptions(SkFilterMode::kLinear), 
                               paint ? &sk_paint : nullptr, SkCanvas::kFast_SrcRectConstraint);
    }

    void drawDRRect(const Rect& outer, const BorderRadius& outerRadius, 
                    const Rect& inner, const BorderRadius& innerRadius, const Paint& paint) override {
        auto make_rrect = [](const Rect& r, const BorderRadius& br) {
            SkRect sk_r = SkRect::MakeXYWH(r.x, r.y, r.width, r.height);
            SkVector radii[4] = {
                {br.top_left, br.top_left}, {br.top_right, br.top_right},
                {br.bottom_right, br.bottom_right}, {br.bottom_left, br.bottom_left}
            };
            SkRRect rrect;
            rrect.setRectRadii(sk_r, radii);
            return rrect;
        };
        canvas_->drawDRRect(make_rrect(outer, outerRadius), make_rrect(inner, innerRadius), toSkPaint(paint));
    }

    void drawParagraph(void* paragraph, float x, float y) override {
        if (paragraph) {
            auto* para = static_cast<skia::textlayout::Paragraph*>(paragraph);
            para->paint(canvas_, x, y);
        }
    }

    void* getNativeHandle() override { return canvas_; }

private:
    SkCanvas* canvas_;
};

} // namespace

// Factory function (will be used by the framework surface wrapper)
std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas) {
    return std::make_unique<SkiaCanvas>(static_cast<SkCanvas*>(sk_canvas));
}

}  // namespace enki
