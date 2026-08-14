/// @file icon.cpp
/// @brief Implementation of the Icon widget.

#include "enki/widgets/icon.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/font_manager.hpp"
#include "include/utils/SkParsePath.h"
#include "include/core/SkFont.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkCanvas.h"
#include <iostream>

namespace enki {

RenderIcon::RenderIcon(IconData data, float size, Color color) 
    : data_(std::move(data)), size_(size), color_(color) {
    
    // Completely rely on Anu Layout Engine to enforce size.
    // This removes any need for custom layout computation overrides.
    ANUNodeStyleSetWidth(getAnuNode(), size_);
    ANUNodeStyleSetHeight(getAnuNode(), size_);
    
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
}

void RenderIcon::setIconData(const IconData& data) {
    if (data_ == data) return;
    data_ = data;
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
    markNeedsPaint(); // IconData change only affects painting
}

void RenderIcon::setSize(float size) {
    if (size_ == size) return;
    size_ = size;
    
    // Update Anu node styling
    ANUNodeStyleSetWidth(getAnuNode(), size_);
    ANUNodeStyleSetHeight(getAnuNode(), size_);
    
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
    markNeedsLayout(); 
}

void RenderIcon::setColor(Color color) {
    if (color_ == color) return;
    color_ = color;
    markNeedsPaint();
}

void RenderIcon::rebuildSvgPath() {
    SkPath raw_path;
    if (SkParsePath::FromSVGString(data_.svg_path.c_str(), &raw_path)) {
        // We need to scale the path so it fits exactly inside a (size_ x size_) bounding box.
        SkRect bounds = raw_path.computeTightBounds();
        float max_dim = std::max(bounds.width(), bounds.height());
        if (max_dim > 0.0f) {
            float scale = size_ / max_dim;
            
            SkMatrix matrix;
            matrix.setScale(scale, scale);
            // Translate the path to the origin (if it's not) and center it
            float dx = -bounds.left() * scale + (size_ - bounds.width() * scale) / 2.0f;
            float dy = -bounds.top() * scale + (size_ - bounds.height() * scale) / 2.0f;
            matrix.postTranslate(dx, dy);

            raw_path.transform(matrix, &cached_svg_path_);
        } else {
            cached_svg_path_ = raw_path;
        }
    } else {
        std::cerr << "[RenderIcon] Failed to parse SVG path: " << data_.svg_path << std::endl;
        cached_svg_path_ = SkPath();
    }
}

void RenderIcon::paint(PaintContext& context) {
    SkCanvas* canvas = static_cast<SkCanvas*>(context.canvas.getNativeHandle());
    if (!canvas) return;
    
    SkPaint paint;
    paint.setColor(color_);
    paint.setAntiAlias(true);

    if (data_.isSvg()) {
        // Draw the cached, pre-scaled SVG path
        paint.setStyle(SkPaint::kFill_Style);
        
        SkPath translated_path = cached_svg_path_;
        translated_path.offset(context.offset.x, context.offset.y);
        
        canvas->drawPath(translated_path, paint);
    } else {
        // Draw font icon
        // For custom loaded fonts we should just try MakeFromFile if it's "Material Icons" 
        // to bypass skparagraph limitations, or fallback to default manager.
        sk_sp<SkTypeface> typeface;
        
        // As a fallback for the demo, try matching via default manager.
        sk_sp<SkFontMgr> mgr = SkFontMgr::RefDefault();
        typeface = sk_sp<SkTypeface>(mgr->matchFamilyStyle(data_.font_family.c_str(), SkFontStyle::Normal()));
        
        // Hardcoded bypass for demo if it fails to resolve
        if (!typeface && data_.font_family == "Material Icons") {
            typeface = SkTypeface::MakeFromFile("assets/fonts/MaterialIcons-Regular.ttf");
        }
        
        if (!typeface) {
            // Fallback to default if not found
            typeface = sk_sp<SkTypeface>(mgr->legacyMakeTypeface(nullptr, SkFontStyle::Normal()));
        }

        SkFont font(typeface, size_);
        font.setEdging(SkFont::Edging::kAntiAlias);

        // Convert the codepoint to a UTF-8 string to draw it
        SkString text;
        text.appendUnichar(data_.codepoint);
        
        sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString(text.c_str(), font, SkTextEncoding::kUTF8);
        if (blob) {
            // Font rendering baseline adjustment
            // To perfectly center the icon, we can calculate the font bounds.
            SkRect bounds;
            font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &bounds);
            
            float dx = (size_ - bounds.width()) / 2.0f - bounds.left();
            float dy = (size_ - bounds.height()) / 2.0f - bounds.top();
            
            canvas->drawTextBlob(blob, context.offset.x + dx, context.offset.y + dy, paint);
        }
    }
}

} // namespace enki
