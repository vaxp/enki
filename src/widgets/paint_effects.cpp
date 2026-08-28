/// @file paint_effects.cpp
/// @brief RenderObjects and widget implementation for BackdropFilter, DecoratedBox, ShaderMask, ColorFiltered.

#include "enki/widgets/paint_effects.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderBackdropFilter
// ════════════════════════════════════════════════════════════════

class RenderBackdropFilter : public RenderBox {
public:
    std::shared_ptr<ImageFilter> filter;
    BlendMode                    blend_mode = BlendMode::SrcOver;

    RenderBackdropFilter(std::shared_ptr<ImageFilter> f, BlendMode mode)
        : filter(std::move(f)), blend_mode(mode) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        context.canvas.save();
        context.canvas.clipRect(bounds);

        Paint layer_paint;
        layer_paint.setBlendMode(blend_mode);

        context.canvas.saveLayerWithBackdrop(&bounds, &layer_paint, filter.get());

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore(); // restore layer
        context.canvas.restore(); // restore clip
    }
};

std::unique_ptr<RenderObject> BackdropFilterWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderBackdropFilter>(filter, blend_mode);
}

void BackdropFilterWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderBackdropFilter&>(renderObject);
    if (r.filter != filter || r.blend_mode != blend_mode) {
        r.filter = filter;
        r.blend_mode = blend_mode;
        r.markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// RenderDecoratedBoxWidget
// ════════════════════════════════════════════════════════════════

class RenderDecoratedBoxStandalone : public RenderBox {
public:
    BoxDecoration      decoration;
    DecorationPosition position = DecorationPosition::Background;

    RenderDecoratedBoxStandalone(BoxDecoration dec, DecorationPosition pos)
        : decoration(std::move(dec)), position(pos) {}

    void paintDecoration(PaintContext& context, const Rect& bounds, const BorderRadius& radius) {
        // Shadows
        if (!decoration.box_shadow.empty()) {
            for (const auto& shadow : decoration.box_shadow) {
                if (shadow.inset) continue;
                Paint shadow_paint;
                shadow_paint.setColor(shadow.color);
                shadow_paint.setAntiAlias(true);
                if (shadow.blur_radius > 0.0f) {
                    shadow_paint.setImageFilter(ImageFilter::blur(shadow.blur_radius * 0.5f, shadow.blur_radius * 0.5f));
                }
                if (decoration.shape == BoxShape::Circle) {
                    float r = std::min(bounds.width, bounds.height) * 0.5f + shadow.spread_radius;
                    Point center = { bounds.x + bounds.width * 0.5f + shadow.offset.x, bounds.y + bounds.height * 0.5f + shadow.offset.y };
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

        // Background
        if (decoration.gradient.has_value()) {
            const auto& grad = *decoration.gradient;
            std::shared_ptr<Shader> shader;
            if (grad.type == GradientType::Linear) {
                Point start = { bounds.x + grad.start_point.x * bounds.width, bounds.y + grad.start_point.y * bounds.height };
                Point end   = { bounds.x + grad.end_point.x * bounds.width, bounds.y + grad.end_point.y * bounds.height };
                shader = Gradient::linear(start, end, grad.colors, grad.stops);
            } else {
                Point center = { bounds.x + grad.center.x * bounds.width, bounds.y + grad.center.y * bounds.height };
                float radius_px = grad.radius * std::min(bounds.width, bounds.height);
                shader = Gradient::radial(center, radius_px, grad.colors, grad.stops);
            }
            Paint p;
            p.setAntiAlias(true);
            p.setShader(shader);
            if (decoration.shape == BoxShape::Circle) {
                float r = std::min(bounds.width, bounds.height) * 0.5f;
                Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
                context.canvas.drawCircle(center, r, p);
            } else {
                context.canvas.drawRRect(bounds, radius, p);
            }
        } else if (decoration.color != Colors::Transparent) {
            Paint p;
            p.setAntiAlias(true);
            p.setColor(decoration.color);
            if (decoration.shape == BoxShape::Circle) {
                float r = std::min(bounds.width, bounds.height) * 0.5f;
                Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
                context.canvas.drawCircle(center, r, p);
            } else {
                context.canvas.drawRRect(bounds, radius, p);
            }
        }

        // Borders
        if (decoration.border.has_value()) {
            const auto& border = *decoration.border;
            Paint border_paint;
            border_paint.setColor(border.color);
            border_paint.setStyle(PaintStyle::Stroke);
            border_paint.setStrokeWidth(border.width);
            border_paint.setAntiAlias(true);

            if (decoration.shape == BoxShape::Circle) {
                float r = std::max(0.0f, std::min(bounds.width, bounds.height) * 0.5f - border.width * 0.5f);
                Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
                context.canvas.drawCircle(center, r, border_paint);
            } else {
                float hw = border.width * 0.5f;
                Rect inset_bounds = {
                    bounds.x + hw, bounds.y + hw,
                    std::max(0.0f, bounds.width - border.width),
                    std::max(0.0f, bounds.height - border.width)
                };
                context.canvas.drawRRect(inset_bounds, radius, border_paint);
            }
        }
    }

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        BorderRadius radius = (decoration.shape == BoxShape::Circle)
            ? BorderRadius::circular(std::min(size_.width, size_.height) * 0.5f)
            : decoration.border_radius;

        if (position == DecorationPosition::Background) {
            paintDecoration(context, bounds, radius);
        }

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        if (position == DecorationPosition::Foreground) {
            paintDecoration(context, bounds, radius);
        }
    }
};

std::unique_ptr<RenderObject> DecoratedBoxWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderDecoratedBoxStandalone>(decoration, position);
}

void DecoratedBoxWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderDecoratedBoxStandalone&>(renderObject);
    if (r.decoration != decoration || r.position != position) {
        r.decoration = decoration;
        r.position = position;
        r.markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// RenderShaderMask
// ════════════════════════════════════════════════════════════════

class RenderShaderMask : public RenderBox {
public:
    ShaderCallback shader_callback;
    BlendMode      blend_mode = BlendMode::Modulate;

    RenderShaderMask(ShaderCallback cb, BlendMode mode)
        : shader_callback(std::move(cb)), blend_mode(mode) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);

        context.canvas.saveLayer(&bounds, nullptr);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        if (shader_callback) {
            auto shader = shader_callback(bounds);
            if (shader) {
                Paint mask_paint;
                mask_paint.setShader(std::move(shader));
                mask_paint.setBlendMode(blend_mode);
                mask_paint.setAntiAlias(true);
                context.canvas.drawRect(bounds, mask_paint);
            }
        }

        context.canvas.restore();
    }
};

std::unique_ptr<RenderObject> ShaderMaskWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderShaderMask>(shader_callback, blend_mode);
}

void ShaderMaskWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderShaderMask&>(renderObject);
    r.shader_callback = shader_callback;
    r.blend_mode = blend_mode;
    r.markNeedsPaint();
}

// ════════════════════════════════════════════════════════════════
// RenderColorFiltered
// ════════════════════════════════════════════════════════════════

class RenderColorFiltered : public RenderBox {
public:
    std::shared_ptr<ColorFilter> color_filter;

    explicit RenderColorFiltered(std::shared_ptr<ColorFilter> filter)
        : color_filter(std::move(filter)) {}

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);

        Paint layer_paint;
        layer_paint.setColorFilter(color_filter);

        context.canvas.saveLayer(&bounds, &layer_paint);

        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        context.canvas.restore();
    }
};

std::unique_ptr<RenderObject> ColorFilteredWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderColorFiltered>(color_filter);
}

void ColorFilteredWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderColorFiltered&>(renderObject);
    if (r.color_filter != color_filter) {
        r.color_filter = color_filter;
        r.markNeedsPaint();
    }
}

} // namespace enki
