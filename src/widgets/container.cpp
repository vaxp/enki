/// @file container.cpp
/// @brief Comprehensive Container and RenderDecoratedBox implementation.

#include "enki/widgets/container.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDecoratedBox Implementation
// ════════════════════════════════════════════════════════════════

RenderDecoratedBox::RenderDecoratedBox() {
    applyFlexboxStyle(anu_node_, style_);
}

RenderDecoratedBox::RenderDecoratedBox(BoxDecoration decoration, FlexboxStyle style)
    : decoration_(std::move(decoration)), style_(std::move(style)) {
    applyFlexboxStyle(anu_node_, style_);
}

void RenderDecoratedBox::setDecoration(const BoxDecoration& decoration) {
    if (decoration_ == decoration) return;
    decoration_ = decoration;
    markNeedsPaint();
}

void RenderDecoratedBox::setStyle(const FlexboxStyle& style) {
    if (style_ == style) return;
    style_ = style;
    applyFlexboxStyle(anu_node_, style_);
    markNeedsLayout();
}

void RenderDecoratedBox::paintShadows(PaintContext& context, const Rect& bounds, const BorderRadius& radius) {
    if (decoration_.box_shadow.empty()) return;

    for (const auto& shadow : decoration_.box_shadow) {
        if (shadow.inset) continue;

        Paint shadow_paint;
        shadow_paint.setColor(shadow.color);
        shadow_paint.setAntiAlias(true);

        if (shadow.blur_radius > 0.0f) {
            shadow_paint.setImageFilter(ImageFilter::blur(shadow.blur_radius * 0.5f, shadow.blur_radius * 0.5f));
        }

        if (decoration_.shape == BoxShape::Circle) {
            float r = std::min(bounds.width, bounds.height) * 0.5f + shadow.spread_radius;
            Point center = {
                bounds.x + bounds.width * 0.5f + shadow.offset.x,
                bounds.y + bounds.height * 0.5f + shadow.offset.y
            };
            context.canvas.drawCircle(center, r, shadow_paint);
        } else {
            Rect s_bounds = {
                bounds.x + shadow.offset.x - shadow.spread_radius,
                bounds.y + shadow.offset.y - shadow.spread_radius,
                bounds.width + shadow.spread_radius * 2.0f,
                bounds.height + shadow.spread_radius * 2.0f
            };
            context.canvas.drawRRect(s_bounds, radius, shadow_paint);
        }
    }
}

void RenderDecoratedBox::paintBackground(PaintContext& context, const Rect& bounds, const BorderRadius& radius) {
    if (decoration_.gradient.has_value()) {
        const auto& grad = *decoration_.gradient;
        std::shared_ptr<Shader> shader;

        if (grad.type == GradientType::Linear) {
            Point start = {
                bounds.x + grad.start_point.x * bounds.width,
                bounds.y + grad.start_point.y * bounds.height
            };
            Point end = {
                bounds.x + grad.end_point.x * bounds.width,
                bounds.y + grad.end_point.y * bounds.height
            };
            shader = Gradient::linear(start, end, grad.colors, grad.stops);
        } else if (grad.type == GradientType::Radial) {
            Point center = {
                bounds.x + grad.center.x * bounds.width,
                bounds.y + grad.center.y * bounds.height
            };
            float r = grad.radius * std::max(bounds.width, bounds.height);
            shader = Gradient::radial(center, r, grad.colors, grad.stops);
        }

        if (shader) {
            Paint grad_paint;
            grad_paint.setShader(shader);
            grad_paint.setAntiAlias(true);

            if (decoration_.shape == BoxShape::Circle) {
                float r = std::min(bounds.width, bounds.height) * 0.5f;
                Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
                context.canvas.drawCircle(center, r, grad_paint);
            } else {
                context.canvas.drawRRect(bounds, radius, grad_paint);
            }
            return;
        }
    }

    if (decoration_.color != Colors::Transparent) {
        Paint bg_paint;
        bg_paint.setColor(decoration_.color);
        bg_paint.setAntiAlias(true);

        if (decoration_.shape == BoxShape::Circle) {
            float r = std::min(bounds.width, bounds.height) * 0.5f;
            Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
            context.canvas.drawCircle(center, r, bg_paint);
        } else {
            context.canvas.drawRRect(bounds, radius, bg_paint);
        }
    }
}

void RenderDecoratedBox::paintBorder(PaintContext& context, const Rect& bounds, const BorderRadius& radius) {
    if (!decoration_.border.has_value() || decoration_.border->width <= 0.0f) return;

    const auto& border = *decoration_.border;
    Paint border_paint;
    border_paint.setStyle(PaintStyle::Stroke);
    border_paint.setStrokeWidth(border.width);
    border_paint.setColor(border.color);
    border_paint.setAntiAlias(true);

    if (decoration_.shape == BoxShape::Circle) {
        float r = std::min(bounds.width, bounds.height) * 0.5f - border.width * 0.5f;
        Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
        if (r > 0.0f) {
            context.canvas.drawCircle(center, r, border_paint);
        }
    } else {
        float hw = border.width * 0.5f;
        Rect inset_bounds = {
            bounds.x + hw,
            bounds.y + hw,
            std::max(0.0f, bounds.width - border.width),
            std::max(0.0f, bounds.height - border.width)
        };
        context.canvas.drawRRect(inset_bounds, radius, border_paint);
    }
}

void RenderDecoratedBox::paint(PaintContext& context) {
    if (size_.width <= 0.0f || size_.height <= 0.0f) return;

    Rect bounds = Rect::fromPointSize(context.offset, size_);
    BorderRadius radius = (decoration_.shape == BoxShape::Circle)
        ? BorderRadius::circular(std::min(size_.width, size_.height) * 0.5f)
        : decoration_.border_radius;

    // 1. Shadows
    paintShadows(context, bounds, radius);

    // 2. Background (Color or Gradient)
    paintBackground(context, bounds, radius);

    // 3. Child with optional Clipping
    if (decoration_.clip_content) {
        context.canvas.save();
        context.canvas.clipRRect(bounds, radius);
    }

    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    if (decoration_.clip_content) {
        context.canvas.restore();
    }

    // 4. Border (Foreground stroke)
    paintBorder(context, bounds, radius);
}

bool RenderDecoratedBox::hitTestSelf(Point localPoint) const {
    if (decoration_.shape == BoxShape::Circle) {
        float r = std::min(size_.width, size_.height) * 0.5f;
        Point center = { size_.width * 0.5f, size_.height * 0.5f };
        float dx = localPoint.x - center.x;
        float dy = localPoint.y - center.y;
        return (dx * dx + dy * dy <= r * r);
    }
    return RenderBox::hitTestSelf(localPoint);
}

// ════════════════════════════════════════════════════════════════
// Container Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Container::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderDecoratedBox>(decoration, style);
}

void Container::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rdb = dynamic_cast<RenderDecoratedBox*>(&renderObject)) {
        rdb->setDecoration(decoration);
        rdb->setStyle(style);
    }
}

} // namespace enki
