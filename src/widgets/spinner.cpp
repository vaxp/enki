#include "enki/widgets/spinner.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkMatrix.h>
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
// RenderSpinner — Custom RenderBox for Loading Spinner
// ════════════════════════════════════════════════════════════════

class RenderSpinner : public RenderBox {
public:
    SpinnerStyle style_;
    float spinner_size_;
    Color color_;
    std::vector<Color> gradient_colors_;
    int spoke_count_;
    float spoke_width_;
    float spoke_length_;
    int dot_count_;
    float dot_size_;
    float rotation_speed_;
    Color glow_color_;
    float glow_blur_;
    std::string custom_shader_;

    float anim_time;
    double start_time;

    sk_sp<SkRuntimeEffect> effect;

    RenderSpinner(const SpinnerWidget* opt, float anim_t, double st)
        : style_(opt->style), spinner_size_(opt->size), color_(opt->color),
          gradient_colors_(opt->gradient_colors), spoke_count_(opt->spoke_count),
          spoke_width_(opt->spoke_width), spoke_length_(opt->spoke_length),
          dot_count_(opt->dot_count), dot_size_(opt->dot_size),
          rotation_speed_(opt->rotation_speed), glow_color_(opt->glow_color),
          glow_blur_(opt->glow_blur), custom_shader_(opt->custom_shader),
          anim_time(anim_t), start_time(st) {
        
        updateAnuStyles();

        if (!custom_shader_.empty() || style_ == SpinnerStyle::CustomShader) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(custom_shader_.c_str()));
            if (!eff) {
                std::cerr << "Spinner SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    void updateAnuStyles() {
        if (anu_node_) {
            ANUNodeStyleSetWidth(anu_node_, spinner_size_);
            ANUNodeStyleSetHeight(anu_node_, spinner_size_);
            ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
            ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
        }
    }

    void setOptions(const SpinnerWidget* opt) {
        style_ = opt->style;
        spinner_size_ = opt->size;
        color_ = opt->color;
        gradient_colors_ = opt->gradient_colors;
        spoke_count_ = opt->spoke_count;
        spoke_width_ = opt->spoke_width;
        spoke_length_ = opt->spoke_length;
        dot_count_ = opt->dot_count;
        dot_size_ = opt->dot_size;
        rotation_speed_ = opt->rotation_speed;
        glow_color_ = opt->glow_color;
        glow_blur_ = opt->glow_blur;
        custom_shader_ = opt->custom_shader;

        updateAnuStyles();
        markNeedsLayout();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        float diameter = std::min(size_.width, size_.height);
        float radius = diameter / 2.0f;
        float cx = ctx.offset.x + size_.width / 2.0f;
        float cy = ctx.offset.y + size_.height / 2.0f;
        float top_left_x = cx - radius;
        float top_left_y = cy - radius;

        constexpr float kPI = 3.14159265358979323846f;

        // Custom SkSL Shader Mode
        if (effect && (style_ == SpinnerStyle::CustomShader || !custom_shader_.empty())) {
            double current_time = getSteadyTimeSecs() - start_time;
            struct Uniforms {
                float resolution[2];
                float time;
            } uniforms = {
                {diameter, diameter},
                (float)(current_time * rotation_speed_)
            };

            SkPaint shader_paint;
            shader_paint.setAntiAlias(true);

            sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
            
            // Translate local matrix so fragCoord is local (0,0) to (diameter, diameter)
            SkMatrix local_matrix = SkMatrix::Translate(top_left_x, top_left_y);
            auto shader_base = effect->makeShader(uniform_data, nullptr, 0);
            if (shader_base) {
                auto shader = shader_base->makeWithLocalMatrix(local_matrix);
                shader_paint.setShader(shader);

                SkRect rect = SkRect::MakeXYWH(top_left_x, top_left_y, diameter, diameter);
                canvas->drawRect(rect, shader_paint);
            }
        }
        else if (style_ == SpinnerStyle::Spokes) {
            // Style 1: Spokes (iOS / macOS Radiating Ticks)
            int count = std::max(4, spoke_count_);
            float step_angle = 360.0f / count;
            float rot_angle = anim_time * rotation_speed_ * 180.0f;
            int active_idx = static_cast<int>(fmod(anim_time * rotation_speed_ * count * 0.8f, count));

            float inner_r = radius * 0.45f;
            float outer_r = radius * 0.85f;

            SkPaint spoke_paint;
            spoke_paint.setAntiAlias(true);
            spoke_paint.setStrokeCap(SkPaint::kRound_Cap);
            spoke_paint.setStrokeWidth(spoke_width_);

            if (glow_blur_ > 0.0f && glow_color_ != 0) {
                spoke_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, glow_blur_ * 0.5f));
            }

            for (int i = 0; i < count; ++i) {
                float angle_deg = i * step_angle + rot_angle;
                float angle_rad = angle_deg * (kPI / 180.0f);

                int relative_idx = (i - active_idx + count) % count;
                float alpha_ratio = static_cast<float>(relative_idx) / static_cast<float>(count);
                alpha_ratio = 0.2f + 0.8f * alpha_ratio; // min 20% opacity

                uint32_t base_c = color_;
                uint8_t orig_a = (base_c >> 24) & 0xFF;
                uint8_t final_a = static_cast<uint8_t>(orig_a * alpha_ratio);
                uint32_t final_c = (final_a << 24) | (base_c & 0x00FFFFFF);

                spoke_paint.setColor(final_c);

                float x1 = cx + inner_r * cos(angle_rad);
                float y1 = cy + inner_r * sin(angle_rad);
                float x2 = cx + outer_r * cos(angle_rad);
                float y2 = cy + outer_r * sin(angle_rad);

                canvas->drawLine(x1, y1, x2, y2, spoke_paint);
            }
        }
        else if (style_ == SpinnerStyle::OrbitDots) {
            // Style 2: OrbitDots (Material / Fluent Orbiting Dots)
            int count = std::max(2, dot_count_);
            float base_rot = anim_time * rotation_speed_ * 200.0f;
            float orbit_r = radius - dot_size_;

            SkPaint dot_paint;
            dot_paint.setAntiAlias(true);
            dot_paint.setStyle(SkPaint::kFill_Style);

            if (glow_blur_ > 0.0f && glow_color_ != 0) {
                dot_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, glow_blur_ * 0.5f));
            }

            for (int i = 0; i < count; ++i) {
                float fraction = static_cast<float>(i) / static_cast<float>(count);
                float angle_deg = base_rot + fraction * 360.0f * (1.0f + 0.2f * sin(anim_time * 2.0f));
                float angle_rad = angle_deg * (kPI / 180.0f);

                float dx = cx + orbit_r * cos(angle_rad);
                float dy = cy + orbit_r * sin(angle_rad);
                float dot_r = (dot_size_ / 2.0f) * (0.4f + 0.6f * (1.0f - fraction));

                if (!gradient_colors_.empty()) {
                    size_t c_idx = i % gradient_colors_.size();
                    dot_paint.setColor(gradient_colors_[c_idx]);
                } else {
                    dot_paint.setColor(color_);
                }

                canvas->drawCircle(dx, dy, dot_r, dot_paint);
            }
        }
        else if (style_ == SpinnerStyle::DualArc) {
            // Style 3: DualArc (Futuristic Dual Counter-Rotating Arcs)
            float rot1 = fmod(anim_time * rotation_speed_ * 180.0f, 360.0f);
            float rot2 = fmod(-anim_time * rotation_speed_ * 260.0f, 360.0f);

            SkPaint arc_paint;
            arc_paint.setAntiAlias(true);
            arc_paint.setStyle(SkPaint::kStroke_Style);
            arc_paint.setStrokeCap(SkPaint::kRound_Cap);

            if (glow_blur_ > 0.0f && glow_color_ != 0) {
                arc_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, glow_blur_ * 0.5f));
            }

            // Outer Arc
            float r1 = radius - 3.0f;
            SkRect rect1 = SkRect::MakeLTRB(cx - r1, cy - r1, cx + r1, cy + r1);
            arc_paint.setStrokeWidth(3.0f);
            arc_paint.setColor(color_);
            canvas->drawArc(rect1, rot1, 270.0f, false, arc_paint);

            // Inner Arc
            float r2 = radius - 9.0f;
            SkRect rect2 = SkRect::MakeLTRB(cx - r2, cy - r2, cx + r2, cy + r2);
            arc_paint.setStrokeWidth(2.5f);

            Color inner_c = gradient_colors_.empty() ? 0xFFEC4899 : gradient_colors_[0];
            arc_paint.setColor(inner_c);
            canvas->drawArc(rect2, rot2, 180.0f, false, arc_paint);
        }

        // Paint Center Child Widget if provided
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
// SingleChildRenderObjectWidget for Spinner
// ════════════════════════════════════════════════════════════════

class SpinnerRenderWidget : public SingleChildRenderObjectWidget {
public:
    const SpinnerWidget* options;
    float anim_time;
    double start_time;

    SpinnerRenderWidget(const SpinnerWidget* opt, float anim_t, double st, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(opt), anim_time(anim_t), start_time(st) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderSpinner>(options, anim_time, start_time);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderSpinner*>(&renderObject)) {
            rb->setOptions(options);
            rb->anim_time = anim_time;
            rb->start_time = start_time;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "SpinnerRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Spinner State
// ════════════════════════════════════════════════════════════════

class SpinnerState : public State {
    std::unique_ptr<Ticker> ticker_;
    double start_time_ = 0.0;
    float anim_time_ = 0.0f;

public:
    void initState() override {
        State::initState();
        start_time_ = getSteadyTimeSecs();

        ticker_ = createTicker([this]() {
            anim_time_ = static_cast<float>(getSteadyTimeSecs() - start_time_);
            setState([]{});
        });
        ticker_->start();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* sp = static_cast<const SpinnerWidget*>(widget());

        return std::make_shared<SpinnerRenderWidget>(
            sp, anim_time_, start_time_, sp->child
        );
    }
};

std::unique_ptr<State> SpinnerWidget::createState() {
    return std::make_unique<SpinnerState>();
}

} // namespace enki
