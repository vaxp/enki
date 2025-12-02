/// @file skia_canvas.cpp
/// @brief Implementation of SkiaCanvasWidget, RenderSkiaCanvas, and factory helpers.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/skia_canvas.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>

#include <algorithm>
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderSkiaCanvas
// ════════════════════════════════════════════════════════════════

class RenderSkiaCanvas : public RenderBox {
public:
    SkiaCanvasStyle style_;

    explicit RenderSkiaCanvas(SkiaCanvasStyle style) : style_(std::move(style)) {
        applyStyleToNode();
        attachRepaintListener();
    }

    ~RenderSkiaCanvas() override {
        detachRepaintListener();
    }

    void attachRepaintListener() {
        if (style_.repaint) {
            style_.repaint->addListener([this]() {
                markNeedsPaint();
            });
        }
    }

    void detachRepaintListener() {
        // AnimationController cleans up its listeners or we let it destruct
    }

    void updateStyle(const SkiaCanvasStyle& new_style) {
        bool layout_changed = (style_.width != new_style.width ||
                               style_.height != new_style.height ||
                               style_.min_width != new_style.min_width ||
                               style_.min_height != new_style.min_height ||
                               style_.max_width != new_style.max_width ||
                               style_.max_height != new_style.max_height);

        if (style_.repaint != new_style.repaint) {
            detachRepaintListener();
            style_ = new_style;
            attachRepaintListener();
        } else {
            style_ = new_style;
        }

        if (layout_changed) {
            applyStyleToNode();
            markNeedsLayout();
        }
        markNeedsPaint();
    }

    void applyStyleToNode() {
        if (!anu_node_) return;

        // Width
        if (style_.width.has_value()) {
            if (style_.width->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, style_.width->value);
            else if (style_.width->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, style_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }

        // Height
        if (style_.height.has_value()) {
            if (style_.height->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, style_.height->value);
            else if (style_.height->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, style_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }

        // Min Width & Height
        if (style_.min_width.has_value()) {
            if (style_.min_width->isPercent()) ANUNodeStyleSetMinWidthPercent(anu_node_, style_.min_width->value);
            else ANUNodeStyleSetMinWidth(anu_node_, style_.min_width->value);
        }
        if (style_.min_height.has_value()) {
            if (style_.min_height->isPercent()) ANUNodeStyleSetMinHeightPercent(anu_node_, style_.min_height->value);
            else ANUNodeStyleSetMinHeight(anu_node_, style_.min_height->value);
        }

        // Max Width & Height
        if (style_.max_width.has_value()) {
            if (style_.max_width->isPercent()) ANUNodeStyleSetMaxWidthPercent(anu_node_, style_.max_width->value);
            else ANUNodeStyleSetMaxWidth(anu_node_, style_.max_width->value);
        }
        if (style_.max_height.has_value()) {
            if (style_.max_height->isPercent()) ANUNodeStyleSetMaxHeightPercent(anu_node_, style_.max_height->value);
            else ANUNodeStyleSetMaxHeight(anu_node_, style_.max_height->value);
        }
    }

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) {
            if (children_.empty()) return;
        }

        const bool needs_clip = (style_.clip_behavior != Clip::None);

        if (needs_clip) {
            context.canvas.save();
            Rect bounds = Rect::fromLTWH(context.offset.x, context.offset.y, size_.width, size_.height);
            if (style_.clip_radius != BorderRadius::zero()) {
                context.canvas.clipRRect(bounds, style_.clip_radius);
            } else {
                context.canvas.clipRect(bounds);
            }
        }

        // 1. Background painter pass (local coordinates: 0,0 is top-left of canvas widget)
        if (style_.painter || style_.skia_painter) {
            context.canvas.save();
            context.canvas.translate(context.offset.x, context.offset.y);

            if (style_.painter) {
                style_.painter(context.canvas, size_);
            }
            if (style_.skia_painter) {
                if (auto* sk_canvas = static_cast<SkCanvas*>(context.canvas.getNativeHandle())) {
                    style_.skia_painter(sk_canvas, size_);
                }
            }

            context.canvas.restore();
        }

        // 2. Child widgets pass (standard tree coordinates)
        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        // 3. Foreground painter pass (painted on top of children)
        if (style_.foreground_painter || style_.skia_foreground_painter) {
            context.canvas.save();
            context.canvas.translate(context.offset.x, context.offset.y);

            if (style_.foreground_painter) {
                style_.foreground_painter(context.canvas, size_);
            }
            if (style_.skia_foreground_painter) {
                if (auto* sk_canvas = static_cast<SkCanvas*>(context.canvas.getNativeHandle())) {
                    style_.skia_foreground_painter(sk_canvas, size_);
                }
            }

            context.canvas.restore();
        }

        if (needs_clip) {
            context.canvas.restore();
        }
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (localPoint.x < 0.0f || localPoint.x > size_.width ||
            localPoint.y < 0.0f || localPoint.y > size_.height) {
            return false;
        }

        if (style_.hit_test) {
            if (!style_.hit_test(localPoint, size_)) {
                return false;
            }
        }

        return RenderBox::hitTest(result, localPoint);
    }
};

// ════════════════════════════════════════════════════════════════
// SkiaCanvasWidget Methods
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> SkiaCanvasWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderSkiaCanvas>(style);
}

void SkiaCanvasWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderSkiaCanvas&>(renderObject);
    r.updateStyle(style);
}

// ════════════════════════════════════════════════════════════════
// SkiaCanvasProps Operator
// ════════════════════════════════════════════════════════════════

SkiaCanvasProps::operator WidgetPtr() const {
    SkiaCanvasStyle s;
    s.painter                 = painter;
    s.skia_painter            = skia_painter;
    s.foreground_painter      = foreground_painter;
    s.skia_foreground_painter = skia_foreground_painter;
    s.hit_test                = hit_test;
    s.repaint                 = repaint;

    s.width                   = width;
    s.height                  = height;
    s.min_width               = min_width;
    s.min_height              = min_height;
    s.max_width               = max_width;
    s.max_height              = max_height;

    s.clip_behavior           = clip_behavior;
    s.clip_radius             = clip_radius;
    s.is_complex              = is_complex;

    return std::make_shared<SkiaCanvasWidget>(std::move(s), child, key);
}

} // namespace enki
