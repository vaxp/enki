/// @file clip.cpp
/// @brief RenderObjects and widget implementation for ClipRect, ClipRRect, ClipOval, ClipPath.

#include "enki/widgets/clip.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderClipRect
// ════════════════════════════════════════════════════════════════

class RenderClipRect : public RenderBox {
public:
    Clip clip_behavior = Clip::AntiAlias;

    explicit RenderClipRect(Clip clip) : clip_behavior(clip) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        context.canvas.save();
        context.canvas.clipRect(bounds);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (localPoint.x < 0.0f || localPoint.x > size_.width ||
            localPoint.y < 0.0f || localPoint.y > size_.height) {
            return false;
        }
        return RenderBox::hitTest(result, localPoint);
    }
};

std::unique_ptr<RenderObject> ClipRectWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderClipRect>(clip_behavior);
}

void ClipRectWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderClipRect&>(renderObject);
    if (r.clip_behavior != clip_behavior) {
        r.clip_behavior = clip_behavior;
        r.markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// RenderClipRRect
// ════════════════════════════════════════════════════════════════

class RenderClipRRect : public RenderBox {
public:
    BorderRadius border_radius = BorderRadius::zero();
    Clip         clip_behavior = Clip::AntiAlias;

    RenderClipRRect(BorderRadius radius, Clip clip)
        : border_radius(radius), clip_behavior(clip) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        context.canvas.save();
        context.canvas.clipRRect(bounds, border_radius);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (localPoint.x < 0.0f || localPoint.x > size_.width ||
            localPoint.y < 0.0f || localPoint.y > size_.height) {
            return false;
        }
        return RenderBox::hitTest(result, localPoint);
    }
};

std::unique_ptr<RenderObject> ClipRRectWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderClipRRect>(border_radius, clip_behavior);
}

void ClipRRectWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderClipRRect&>(renderObject);
    if (r.border_radius != border_radius || r.clip_behavior != clip_behavior) {
        r.border_radius = border_radius;
        r.clip_behavior = clip_behavior;
        r.markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// RenderClipOval
// ════════════════════════════════════════════════════════════════

class RenderClipOval : public RenderBox {
public:
    Clip clip_behavior = Clip::AntiAlias;

    explicit RenderClipOval(Clip clip) : clip_behavior(clip) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        context.canvas.save();
        context.canvas.clipOval(bounds);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        float rx = size_.width * 0.5f;
        float ry = size_.height * 0.5f;
        if (rx <= 0.0f || ry <= 0.0f) return false;

        float dx = localPoint.x - rx;
        float dy = localPoint.y - ry;
        if ((dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) > 1.0f) {
            return false;
        }
        return RenderBox::hitTest(result, localPoint);
    }
};

std::unique_ptr<RenderObject> ClipOvalWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderClipOval>(clip_behavior);
}

void ClipOvalWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderClipOval&>(renderObject);
    if (r.clip_behavior != clip_behavior) {
        r.clip_behavior = clip_behavior;
        r.markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// RenderClipPath
// ════════════════════════════════════════════════════════════════

class RenderClipPath : public RenderBox {
public:
    CustomClipper         clipper = nullptr;
    std::shared_ptr<Path> path = nullptr;
    Clip                  clip_behavior = Clip::AntiAlias;

    RenderClipPath(CustomClipper c, std::shared_ptr<Path> p, Clip clip)
        : clipper(std::move(c)), path(std::move(p)), clip_behavior(clip) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        context.canvas.save();
        context.canvas.translate(context.offset.x, context.offset.y);

        if (clipper) {
            Path p = clipper(size_);
            context.canvas.clipPath(p);
        } else if (path) {
            context.canvas.clipPath(*path);
        }

        context.canvas.translate(-context.offset.x, -context.offset.y);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (localPoint.x < 0.0f || localPoint.x > size_.width ||
            localPoint.y < 0.0f || localPoint.y > size_.height) {
            return false;
        }
        return RenderBox::hitTest(result, localPoint);
    }
};

std::unique_ptr<RenderObject> ClipPathWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderClipPath>(clipper, path, clip_behavior);
}

void ClipPathWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderClipPath&>(renderObject);
    r.clipper = clipper;
    r.path = path;
    r.clip_behavior = clip_behavior;
    r.markNeedsPaint();
}

} // namespace enki
