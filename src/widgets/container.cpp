/// @file container.cpp
/// @brief Comprehensive Container and RenderDecoratedBox implementation.

#include "enki/widgets/container.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/animation/ticker.hpp"

#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkShader.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkString.h>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>

namespace enki {

inline double getCurrentTimeSeconds() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0;
}

class CustomShaderWrapper : public Shader {
public:
    explicit CustomShaderWrapper(sk_sp<SkShader> shader) : shader_(std::move(shader)) {}
    void* getNativeHandle() const override { return shader_.get(); }
private:
    sk_sp<SkShader> shader_;
};

struct RenderDecoratedBox::ShaderData {
    sk_sp<SkRuntimeEffect> bg_effect;
    sk_sp<SkRuntimeEffect> border_effect;
    std::shared_ptr<SvgDocument> bg_svg;
    std::shared_ptr<SvgDocument> border_svg;
    std::unique_ptr<Ticker> ticker;
    double start_time = 0.0;
};

// ════════════════════════════════════════════════════════════════
// RenderDecoratedBox Implementation
// ════════════════════════════════════════════════════════════════

RenderDecoratedBox::RenderDecoratedBox()
    : shader_data_(std::make_unique<ShaderData>()) {
    applyFlexboxStyle(anu_node_, style_);
}

RenderDecoratedBox::RenderDecoratedBox(BoxDecoration decoration, FlexboxStyle style)
    : decoration_(std::move(decoration)), style_(std::move(style)),
      shader_data_(std::make_unique<ShaderData>()) {
    applyFlexboxStyle(anu_node_, style_);
    updateShaders();
}

RenderDecoratedBox::~RenderDecoratedBox() {
    if (shader_data_ && shader_data_->ticker) {
        shader_data_->ticker->stop();
    }
}

void RenderDecoratedBox::updateShaders() {
    if (!shader_data_) {
        shader_data_ = std::make_unique<ShaderData>();
    }

    // 1. Background shader
    if (!decoration_.background_shader.empty()) {
        auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(decoration_.background_shader.c_str()));
        if (!eff) {
            std::cerr << "Container SkSL background_shader Compile Error: " << err.c_str() << "\n";
            shader_data_->bg_effect = nullptr;
        } else {
            shader_data_->bg_effect = eff;
        }
    } else {
        shader_data_->bg_effect = nullptr;
    }

    // 2. Border shader
    if (!decoration_.border_shader.empty()) {
        auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(decoration_.border_shader.c_str()));
        if (!eff) {
            std::cerr << "Container SkSL border_shader Compile Error: " << err.c_str() << "\n";
            shader_data_->border_effect = nullptr;
        } else {
            shader_data_->border_effect = eff;
        }
    } else {
        shader_data_->border_effect = nullptr;
    }

    // 3. Background SVG
    if (!decoration_.background_svg.empty()) {
        shader_data_->bg_svg = SvgDocument::parse(decoration_.background_svg);
    } else {
        shader_data_->bg_svg = nullptr;
    }

    // 4. Border SVG
    if (!decoration_.border_svg.empty()) {
        shader_data_->border_svg = SvgDocument::parse(decoration_.border_svg);
    } else {
        shader_data_->border_svg = nullptr;
    }

    // 5. Animation ticker if any shader uses 'time'
    bool needs_ticker = false;
    if (shader_data_->bg_effect && shader_data_->bg_effect->findUniform("time")) {
        needs_ticker = true;
    }
    if (shader_data_->border_effect && shader_data_->border_effect->findUniform("time")) {
        needs_ticker = true;
    }

    if (needs_ticker) {
        if (!shader_data_->ticker) {
            shader_data_->start_time = getCurrentTimeSeconds();
            shader_data_->ticker = createTicker([this]() {
                markNeedsPaint();
            });
        }
        shader_data_->ticker->start();
    } else {
        if (shader_data_->ticker) {
            shader_data_->ticker->stop();
        }
    }
}

void RenderDecoratedBox::setDecoration(const BoxDecoration& decoration) {
    if (decoration_ == decoration) return;
    bool shaders_changed = (decoration_.background_shader != decoration.background_shader) ||
                           (decoration_.border_shader != decoration.border_shader) ||
                           (decoration_.background_svg != decoration.background_svg) ||
                           (decoration_.border_svg != decoration.border_svg);
    decoration_ = decoration;
    if (shaders_changed) {
        updateShaders();
    }
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
    // 0. Background SVG (Vector injection)
    if (shader_data_ && shader_data_->bg_svg) {
        std::unique_ptr<Paint> shader_override = nullptr;
        if (shader_data_->bg_effect) {
            double current_time = getCurrentTimeSeconds() - shader_data_->start_time;
            SkRuntimeShaderBuilder builder(shader_data_->bg_effect);
            if (shader_data_->bg_effect->findUniform("time")) builder.uniform("time") = (float)current_time;
            if (shader_data_->bg_effect->findUniform("resolution")) {
                float res[2] = {bounds.width, bounds.height};
                builder.uniform("resolution").set(res, 2);
            }
            SkMatrix local_matrix = SkMatrix::Translate(bounds.x, bounds.y);
            sk_sp<SkShader> shader = builder.makeShader(&local_matrix);
            if (shader) {
                shader_override = std::make_unique<Paint>();
                shader_override->setShader(std::make_shared<CustomShaderWrapper>(std::move(shader)));
                shader_override->setAntiAlias(true);
            }
        }

        if (decoration_.svg_slice.has_value()) {
            shader_data_->bg_svg->renderNineSlice(context.canvas, bounds, *decoration_.svg_slice, shader_override.get(), false);
        } else {
            shader_data_->bg_svg->render(context.canvas, bounds, decoration_.svg_fit, shader_override.get(), false);
        }
        return;
    }

    if (shader_data_ && shader_data_->bg_effect) {
        double current_time = getCurrentTimeSeconds() - shader_data_->start_time;
        SkRuntimeShaderBuilder builder(shader_data_->bg_effect);

        if (shader_data_->bg_effect->findUniform("time")) {
            builder.uniform("time") = (float)current_time;
        }
        if (shader_data_->bg_effect->findUniform("resolution")) {
            float res[2] = {bounds.width, bounds.height};
            builder.uniform("resolution").set(res, 2);
        }

        SkMatrix local_matrix = SkMatrix::Translate(bounds.x, bounds.y);
        sk_sp<SkShader> shader = builder.makeShader(&local_matrix);
        if (shader) {
            Paint bg_paint;
            bg_paint.setShader(std::make_shared<CustomShaderWrapper>(std::move(shader)));
            bg_paint.setAntiAlias(true);

            if (decoration_.shape == BoxShape::Circle) {
                float r = std::min(bounds.width, bounds.height) * 0.5f;
                Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
                context.canvas.drawCircle(center, r, bg_paint);
            } else {
                context.canvas.drawRRect(bounds, radius, bg_paint);
            }
            return;
        }
    }

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
    // 0. Border SVG (Vector injection)
    if (shader_data_ && shader_data_->border_svg) {
        std::unique_ptr<Paint> shader_override = nullptr;
        if (shader_data_->border_effect) {
            double current_time = getCurrentTimeSeconds() - shader_data_->start_time;
            SkRuntimeShaderBuilder builder(shader_data_->border_effect);
            if (shader_data_->border_effect->findUniform("time")) builder.uniform("time") = (float)current_time;
            if (shader_data_->border_effect->findUniform("resolution")) {
                float res[2] = {bounds.width, bounds.height};
                builder.uniform("resolution").set(res, 2);
            }
            SkMatrix local_matrix = SkMatrix::Translate(bounds.x, bounds.y);
            sk_sp<SkShader> shader = builder.makeShader(&local_matrix);
            if (shader) {
                shader_override = std::make_unique<Paint>();
                shader_override->setShader(std::make_shared<CustomShaderWrapper>(std::move(shader)));
                shader_override->setAntiAlias(true);
                if (decoration_.border.has_value() && decoration_.border->width > 0.0f) {
                    shader_override->setStrokeWidth(decoration_.border->width);
                }
            }
        } else if (decoration_.border.has_value() && decoration_.border->color != Colors::Transparent) {
            shader_override = std::make_unique<Paint>();
            shader_override->setColor(decoration_.border->color);
            shader_override->setStrokeWidth(decoration_.border->width);
            shader_override->setAntiAlias(true);
        }

        if (decoration_.svg_slice.has_value()) {
            shader_data_->border_svg->renderNineSlice(context.canvas, bounds, *decoration_.svg_slice, shader_override.get(), true);
        } else {
            shader_data_->border_svg->render(context.canvas, bounds, decoration_.svg_fit, shader_override.get(), true);
        }
        return;
    }

    float border_width = decoration_.border.has_value() ? decoration_.border->width : 0.0f;
    if (border_width <= 0.0f && !decoration_.border_shader.empty()) {
        border_width = 1.0f;
    }
    if (border_width <= 0.0f) return;

    Paint border_paint;
    border_paint.setStyle(PaintStyle::Stroke);
    border_paint.setStrokeWidth(border_width);
    border_paint.setAntiAlias(true);

    if (shader_data_ && shader_data_->border_effect) {
        double current_time = getCurrentTimeSeconds() - shader_data_->start_time;
        SkRuntimeShaderBuilder builder(shader_data_->border_effect);

        if (shader_data_->border_effect->findUniform("time")) {
            builder.uniform("time") = (float)current_time;
        }
        if (shader_data_->border_effect->findUniform("resolution")) {
            float res[2] = {bounds.width, bounds.height};
            builder.uniform("resolution").set(res, 2);
        }

        SkMatrix local_matrix = SkMatrix::Translate(bounds.x, bounds.y);
        sk_sp<SkShader> shader = builder.makeShader(&local_matrix);
        if (shader) {
            border_paint.setShader(std::make_shared<CustomShaderWrapper>(std::move(shader)));
        } else if (decoration_.border.has_value()) {
            border_paint.setColor(decoration_.border->color);
        }
    } else if (decoration_.border.has_value()) {
        border_paint.setColor(decoration_.border->color);
    }

    if (decoration_.shape == BoxShape::Circle) {
        float r = std::min(bounds.width, bounds.height) * 0.5f - border_width * 0.5f;
        Point center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
        if (r > 0.0f) {
            context.canvas.drawCircle(center, r, border_paint);
        }
    } else {
        float hw = border_width * 0.5f;
        Rect inset_bounds = {
            bounds.x + hw,
            bounds.y + hw,
            std::max(0.0f, bounds.width - border_width),
            std::max(0.0f, bounds.height - border_width)
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

std::unique_ptr<RenderObject> ContainerWidget::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderDecoratedBox>(decoration, style);
}

void ContainerWidget::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rdb = dynamic_cast<RenderDecoratedBox*>(&renderObject)) {
        rdb->setDecoration(decoration);
        rdb->setStyle(style);
    }
}

} // namespace enki
