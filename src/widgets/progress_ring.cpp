#include "enki/widgets/progress_ring.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkMaskFilter.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkRuntimeEffect.h>
#include <layout_engine/Anu.h>

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace enki {

inline double getSteadyTimeSecs() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0;
}

// ════════════════════════════════════════════════════════════════
// RenderProgressRing — Custom RenderBox for Circular Progress Ring
// ════════════════════════════════════════════════════════════════

class RenderProgressRing : public RenderBox {
public:
    float ring_size_;
    float stroke_width_;
    Color background_color_;
    Color progress_color_;
    std::vector<Color> gradient_colors_;
    Color glow_color_;
    float glow_blur_;
    bool round_cap_;
    float start_angle_;
    bool indeterminate_;
    std::string custom_shader_;

    float value;
    float anim_time;
    double start_time;

    sk_sp<SkRuntimeEffect> effect;

    RenderProgressRing(const ProgressRingWidget* opt, float val, float anim_t, double st)
        : ring_size_(opt->size), stroke_width_(opt->stroke_width),
          background_color_(opt->background_color), progress_color_(opt->progress_color),
          gradient_colors_(opt->gradient_colors), glow_color_(opt->glow_color),
          glow_blur_(opt->glow_blur), round_cap_(opt->round_cap),
          start_angle_(opt->start_angle), indeterminate_(opt->indeterminate),
          custom_shader_(opt->custom_shader), value(val), anim_time(anim_t), start_time(st) {
        
        updateAnuStyles();

        if (!custom_shader_.empty()) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(custom_shader_.c_str()));
            if (!eff) {
                std::cerr << "ProgressRing SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    void updateAnuStyles() {
        if (anu_node_) {
            ANUNodeStyleSetWidth(anu_node_, ring_size_);
            ANUNodeStyleSetHeight(anu_node_, ring_size_);
            ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
            ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
        }
    }

    void setOptions(const ProgressRingWidget* opt) {
        ring_size_ = opt->size;
        stroke_width_ = opt->stroke_width;
        background_color_ = opt->background_color;
        progress_color_ = opt->progress_color;
        gradient_colors_ = opt->gradient_colors;
        glow_color_ = opt->glow_color;
        glow_blur_ = opt->glow_blur;
        round_cap_ = opt->round_cap;
        start_angle_ = opt->start_angle;
        indeterminate_ = opt->indeterminate;
        custom_shader_ = opt->custom_shader;
        
        updateAnuStyles();
        markNeedsLayout();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        float diameter = std::min(size_.width, size_.height);
        float stroke = std::clamp(stroke_width_, 1.0f, diameter / 2.0f);
        float radius = (diameter - stroke) / 2.0f;

        float cx = ctx.offset.x + size_.width / 2.0f;
        float cy = ctx.offset.y + size_.height / 2.0f;

        SkRect arc_rect = SkRect::MakeLTRB(cx - radius, cy - radius, cx + radius, cy + radius);

        // 1. Background Ring Track
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setStyle(SkPaint::kStroke_Style);
        bg_paint.setStrokeWidth(stroke);
        bg_paint.setColor(background_color_);
        canvas->drawOval(arc_rect, bg_paint);

        // 2. Active Progress Arc
        SkPaint fill_paint;
        fill_paint.setAntiAlias(true);
        fill_paint.setStyle(SkPaint::kStroke_Style);
        fill_paint.setStrokeWidth(stroke);
        if (round_cap_) {
            fill_paint.setStrokeCap(SkPaint::kRound_Cap);
        }

        // Apply SkSL Shader or Sweep Gradient or Solid Color
        if (effect) {
            double current_time = getSteadyTimeSecs() - start_time;
            struct Uniforms {
                float time;
                float resolution[2];
                float progress;
            } uniforms = {
                (float)current_time,
                {diameter, diameter},
                std::clamp(value, 0.0f, 1.0f)
            };
            sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
            float top_left_x = cx - radius;
            float top_left_y = cy - radius;
            SkMatrix local_matrix = SkMatrix::Translate(-top_left_x, -top_left_y);
            fill_paint.setShader(effect->makeShader(uniform_data, nullptr, 0)->makeWithLocalMatrix(local_matrix));
        } else if (gradient_colors_.size() >= 2) {
            std::vector<SkColor> sk_colors;
            for (auto c : gradient_colors_) sk_colors.push_back(c);

            fill_paint.setShader(SkGradientShader::MakeSweep(
                cx, cy, sk_colors.data(), nullptr, static_cast<int>(sk_colors.size())));
        } else {
            fill_paint.setColor(progress_color_);
        }

        // Draw Arc
        if (indeterminate_) {
            float start_deg = fmod(anim_time * 240.0f, 360.0f) + start_angle_;
            float sweep_deg = 60.0f + 120.0f * (0.5f + 0.5f * sin(anim_time * 3.0f));

            if (glow_blur_ > 0.0f && glow_color_ != 0) {
                SkPaint glow_paint = fill_paint;
                glow_paint.setColor(glow_color_);
                glow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, glow_blur_ * 0.5f));
                canvas->drawArc(arc_rect, start_deg, sweep_deg, false, glow_paint);
            }

            canvas->drawArc(arc_rect, start_deg, sweep_deg, false, fill_paint);
        } else {
            float clamped_val = std::clamp(value, 0.0f, 1.0f);
            float sweep_deg = clamped_val * 360.0f;

            if (sweep_deg > 0.0f) {
                if (glow_blur_ > 0.0f && glow_color_ != 0) {
                    SkPaint glow_paint = fill_paint;
                    glow_paint.setColor(glow_color_);
                    glow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, glow_blur_ * 0.5f));
                    canvas->drawArc(arc_rect, start_angle_, sweep_deg, false, glow_paint);
                }

                canvas->drawArc(arc_rect, start_angle_, sweep_deg, false, fill_paint);
            }
        }

        // 3. Paint Center Child Widget if provided
        if (!children().empty()) {
            RenderBox* child_ro = static_cast<RenderBox*>(children()[0]);
            Point child_offset(
                (size_.width - child_ro->size().width) / 2.0f,
                (size_.height - child_ro->size().height) / 2.0f
            );
            PaintContext child_ctx = ctx.withOffset(child_offset);
            child_ro->paint(child_ctx);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget for ProgressRing
// ════════════════════════════════════════════════════════════════

class ProgressRingRenderWidget : public SingleChildRenderObjectWidget {
public:
    const ProgressRingWidget* options;
    float value;
    float anim_time;
    double start_time;

    ProgressRingRenderWidget(const ProgressRingWidget* opt, float val, float anim_t, double st, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(opt), value(val), anim_time(anim_t), start_time(st) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderProgressRing>(options, value, anim_time, start_time);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderProgressRing*>(&renderObject)) {
            rb->setOptions(options);
            rb->value = value;
            rb->anim_time = anim_time;
            rb->start_time = start_time;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "ProgressRingRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// ProgressRing State
// ════════════════════════════════════════════════════════════════

class ProgressRingState : public State {
    std::unique_ptr<Ticker> ticker_;
    double start_time_ = 0.0;
    float anim_time_ = 0.0f;

public:
    void initState() override {
        State::initState();
        start_time_ = getSteadyTimeSecs();

        auto* pr = static_cast<const ProgressRingWidget*>(widget());
        if (pr->indeterminate || !pr->custom_shader.empty()) {
            ticker_ = createTicker([this]() {
                anim_time_ = static_cast<float>(getSteadyTimeSecs() - start_time_);
                setState([]{});
            });
            ticker_->start();
        }
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* pr = static_cast<const ProgressRingWidget*>(widget());

        return std::make_shared<ProgressRingRenderWidget>(
            pr, pr->value, anim_time_, start_time_, pr->child
        );
    }
};

std::unique_ptr<State> ProgressRingWidget::createState() {
    return std::make_unique<ProgressRingState>();
}

} // namespace enki
