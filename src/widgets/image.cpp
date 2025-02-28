/// @file image.cpp
/// @brief Implementation of ImageWidget, RenderImage, and ImageCache.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/image.hpp"
#include "enki/rendering/canvas.hpp"
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ImageCache Implementation
// ════════════════════════════════════════════════════════════════

namespace {
    std::unordered_map<std::string, std::shared_ptr<Image>> s_image_cache;
    std::mutex s_cache_mutex;
}

std::shared_ptr<Image> ImageCache::getOrLoad(std::string_view path) {
    std::string key(path);
    {
        std::lock_guard<std::mutex> lock(s_cache_mutex);
        auto it = s_image_cache.find(key);
        if (it != s_image_cache.end()) {
            return it->second;
        }
    }

    auto res = Image::loadFromFile(path);
    if (!res.isOk()) {
        // Fallback: try ../path (e.g. running from build/)
        std::string parent_rel = "../" + std::string(path);
        res = Image::loadFromFile(parent_rel);
    }
    if (!res.isOk()) {
        // Fallback: try workspace absolute path
        std::string ws_rel = "/home/x/Work/vaura/" + std::string(path);
        res = Image::loadFromFile(ws_rel);
    }

    if (!res.isOk()) {
        return nullptr;
    }

    auto img = res.value();
    {
        std::lock_guard<std::mutex> lock(s_cache_mutex);
        s_image_cache[key] = img;
    }
    return img;
}

void ImageCache::put(std::string_view key, std::shared_ptr<Image> image) {
    std::lock_guard<std::mutex> lock(s_cache_mutex);
    s_image_cache[std::string(key)] = std::move(image);
}

std::shared_ptr<Image> ImageCache::get(std::string_view key) {
    std::lock_guard<std::mutex> lock(s_cache_mutex);
    auto it = s_image_cache.find(std::string(key));
    if (it != s_image_cache.end()) {
        return it->second;
    }
    return nullptr;
}

void ImageCache::clear() {
    std::lock_guard<std::mutex> lock(s_cache_mutex);
    s_image_cache.clear();
}

size_t ImageCache::count() {
    std::lock_guard<std::mutex> lock(s_cache_mutex);
    return s_image_cache.size();
}

// ════════════════════════════════════════════════════════════════
// Geometry Helpers for BoxFit & Alignment
// ════════════════════════════════════════════════════════════════

namespace {

float alignRatioX(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::CenterLeft:
        case Alignment::BottomLeft:
            return 0.0f;
        case Alignment::TopCenter:
        case Alignment::Center:
        case Alignment::BottomCenter:
            return 0.5f;
        case Alignment::TopRight:
        case Alignment::CenterRight:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

float alignRatioY(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::TopCenter:
        case Alignment::TopRight:
            return 0.0f;
        case Alignment::CenterLeft:
        case Alignment::Center:
        case Alignment::CenterRight:
            return 0.5f;
        case Alignment::BottomLeft:
        case Alignment::BottomCenter:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

} // namespace

void RenderImage::calculateBoxFitGeometry(BoxFit fit, Alignment align,
                                          Size src_size, Size dst_size,
                                          Rect& out_src, Rect& out_dst) {
    const float sw = src_size.width;
    const float sh = src_size.height;
    const float dw = dst_size.width;
    const float dh = dst_size.height;

    if (sw <= 0.0f || sh <= 0.0f || dw <= 0.0f || dh <= 0.0f) {
        out_src = Rect(0, 0, 0, 0);
        out_dst = Rect(0, 0, 0, 0);
        return;
    }

    const float ax = alignRatioX(align);
    const float ay = alignRatioY(align);

    if (fit == BoxFit::ScaleDown) {
        if (sw > dw || sh > dh) {
            fit = BoxFit::Contain;
        } else {
            fit = BoxFit::None;
        }
    }

    switch (fit) {
        case BoxFit::Fill: {
            out_src = Rect(0.0f, 0.0f, sw, sh);
            out_dst = Rect(0.0f, 0.0f, dw, dh);
            break;
        }

        case BoxFit::Contain: {
            const float scale = std::min(dw / sw, dh / sh);
            const float rw = sw * scale;
            const float rh = sh * scale;
            const float dx = (dw - rw) * ax;
            const float dy = (dh - rh) * ay;

            out_src = Rect(0.0f, 0.0f, sw, sh);
            out_dst = Rect(dx, dy, rw, rh);
            break;
        }

        case BoxFit::Cover: {
            const float scale = std::max(dw / sw, dh / sh);
            const float vsw = dw / scale;
            const float vsh = dh / scale;
            const float sx = (sw - vsw) * ax;
            const float sy = (sh - vsh) * ay;

            out_src = Rect(sx, sy, vsw, vsh);
            out_dst = Rect(0.0f, 0.0f, dw, dh);
            break;
        }

        case BoxFit::FitWidth: {
            const float scale = dw / sw;
            const float vsh = dh / scale;
            const float sy = std::max(0.0f, (sh - vsh) * ay);
            const float actual_src_h = std::min(sh, vsh);
            const float actual_dst_h = std::min(dh, actual_src_h * scale);
            const float dy = (dh - actual_dst_h) * ay;

            out_src = Rect(0.0f, sy, sw, actual_src_h);
            out_dst = Rect(0.0f, dy, dw, actual_dst_h);
            break;
        }

        case BoxFit::FitHeight: {
            const float scale = dh / sh;
            const float vsw = dw / scale;
            const float sx = std::max(0.0f, (sw - vsw) * ax);
            const float actual_src_w = std::min(sw, vsw);
            const float actual_dst_w = std::min(dw, actual_src_w * scale);
            const float dx = (dw - actual_dst_w) * ax;

            out_src = Rect(sx, 0.0f, actual_src_w, sh);
            out_dst = Rect(dx, 0.0f, actual_dst_w, dh);
            break;
        }

        case BoxFit::None: {
            const float vsw = std::min(sw, dw);
            const float vsh = std::min(sh, dh);
            const float sx = (sw - vsw) * ax;
            const float sy = (sh - vsh) * ay;
            const float dx = (dw - vsw) * ax;
            const float dy = (dh - vsh) * ay;

            out_src = Rect(sx, sy, vsw, vsh);
            out_dst = Rect(dx, dy, vsw, vsh);
            break;
        }

        case BoxFit::ScaleDown:
            break;
    }
}

// ════════════════════════════════════════════════════════════════
// RenderImage Implementation
// ════════════════════════════════════════════════════════════════

RenderImage::RenderImage() {
    anu_node_ = ANUNodeNew();
    ANUNodeSetContext(anu_node_, this);
    ANUNodeSetMeasureFunc(anu_node_, &RenderImage::measureCallback);
}

RenderImage::RenderImage(ImageStyle style) : RenderImage() {
    setStyle(std::move(style));
}

RenderImage::~RenderImage() {
    if (anu_node_) {
        ANUNodeSetContext(anu_node_, nullptr);
        ANUNodeFree(anu_node_);
        anu_node_ = nullptr;
    }
}

void RenderImage::setStyle(const ImageStyle& style) {
    if (style_ == style) return;
    style_ = style;
    applyStyleToNode();
    markNeedsLayout();
}

void RenderImage::applyStyleToNode() {
    if (!anu_node_) return;

    if (style_.width.has_value()) {
        if (style_.width->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, style_.width->value);
        else if (style_.width->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
        else ANUNodeStyleSetWidth(anu_node_, style_.width->value);
    } else {
        ANUNodeStyleSetWidthAuto(anu_node_);
    }

    if (style_.height.has_value()) {
        if (style_.height->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, style_.height->value);
        else if (style_.height->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
        else ANUNodeStyleSetHeight(anu_node_, style_.height->value);
    } else {
        ANUNodeStyleSetHeightAuto(anu_node_);
    }

    if (style_.min_width.has_value()) {
        if (style_.min_width->isPercent()) ANUNodeStyleSetMinWidthPercent(anu_node_, style_.min_width->value);
        else ANUNodeStyleSetMinWidth(anu_node_, style_.min_width->value);
    }
    if (style_.min_height.has_value()) {
        if (style_.min_height->isPercent()) ANUNodeStyleSetMinHeightPercent(anu_node_, style_.min_height->value);
        else ANUNodeStyleSetMinHeight(anu_node_, style_.min_height->value);
    }
    if (style_.max_width.has_value()) {
        if (style_.max_width->isPercent()) ANUNodeStyleSetMaxWidthPercent(anu_node_, style_.max_width->value);
        else ANUNodeStyleSetMaxWidth(anu_node_, style_.max_width->value);
    }
    if (style_.max_height.has_value()) {
        if (style_.max_height->isPercent()) ANUNodeStyleSetMaxHeightPercent(anu_node_, style_.max_height->value);
        else ANUNodeStyleSetMaxHeight(anu_node_, style_.max_height->value);
    }
}

ANUSize RenderImage::measureCallback(ANUNodeConstRef node, float width, ANUMeasureMode widthMode,
                                     float height, ANUMeasureMode heightMode) {
    auto* self = static_cast<RenderImage*>(ANUNodeGetContext(node));
    if (!self || !self->style_.image) {
        return ANUSize{0.0f, 0.0f};
    }

    const float img_w = static_cast<float>(self->style_.image->getWidth());
    const float img_h = static_cast<float>(self->style_.image->getHeight());

    if (img_w <= 0.0f || img_h <= 0.0f) {
        return ANUSize{0.0f, 0.0f};
    }

    const float aspect = img_w / img_h;
    float result_w = img_w;
    float result_h = img_h;

    if (widthMode == ANUMeasureModeExactly) {
        result_w = width;
        if (heightMode == ANUMeasureModeExactly) {
            result_h = height;
        } else {
            result_h = width / aspect;
        }
    } else if (heightMode == ANUMeasureModeExactly) {
        result_h = height;
        result_w = height * aspect;
    } else {
        if (widthMode == ANUMeasureModeAtMost && result_w > width) {
            result_w = width;
            result_h = width / aspect;
        }
        if (heightMode == ANUMeasureModeAtMost && result_h > height) {
            result_h = height;
            result_w = height * aspect;
        }
    }

    return ANUSize{result_w, result_h};
}

void RenderImage::paint(PaintContext& context) {
    if (!style_.image) return;
    if (size_.width <= 0.0f || size_.height <= 0.0f) return;

    Rect bounds = Rect::fromPointSize(context.offset, size_);
    BorderRadius radius = (style_.shape == BoxShape::Circle)
        ? BorderRadius::circular(std::min(size_.width, size_.height) * 0.5f)
        : style_.border_radius;

    const bool need_clip = style_.clip_content &&
                          (style_.shape == BoxShape::Circle || radius != BorderRadius::zero());

    if (need_clip) {
        context.canvas.save();
        context.canvas.clipRRect(bounds, radius);
    }

    Rect src_rect, dst_local;
    calculateBoxFitGeometry(style_.fit, style_.alignment,
                            style_.image->getSize(), size_,
                            src_rect, dst_local);

    Rect dst_world(
        bounds.x + dst_local.x,
        bounds.y + dst_local.y,
        dst_local.width,
        dst_local.height
    );

    Paint paint;
    paint.setAntiAlias(true);

    if (style_.opacity < 1.0f) {
        uint8_t alpha = static_cast<uint8_t>(std::clamp(style_.opacity * context.opacity, 0.0f, 1.0f) * 255.0f);
        paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF);
    }

    if (style_.tint_color.has_value()) {
        paint.setColor(*style_.tint_color);
        paint.setBlendMode(style_.blend_mode);
    }

    context.canvas.drawImageRect(style_.image, src_rect, dst_world, &paint);

    if (need_clip) {
        context.canvas.restore();
    }
}

// ════════════════════════════════════════════════════════════════
// ImageWidget Element Binding
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> ImageWidget::createRenderObject(BuildContext& /*ctx*/) {
    return std::make_unique<RenderImage>(style);
}

void ImageWidget::updateRenderObject(BuildContext& /*ctx*/, RenderObject& renderObject) {
    if (auto* r = dynamic_cast<RenderImage*>(&renderObject)) {
        r->setStyle(style);
    }
}

} // namespace enki
