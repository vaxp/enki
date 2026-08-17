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
    SpinnerOptions options;
    float anim_time;
    double start_time;

    sk_sp<SkRuntimeEffect> effect;

    RenderSpinner(const SpinnerOptions& opt, float anim_t, double st)
        : options(opt), anim_time(anim_t), start_time(st) {
        
        updateAnuStyles();

        if (!options.custom_shader.empty() || options.style == SpinnerStyle::CustomShader) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (!eff) {
                std::cerr << "Spinner SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    void updateAnuStyles() {
        if (anu_node_) {
            ANUNodeStyleSetWidth(anu_node_, options.size);
            ANUNodeStyleSetHeight(anu_node_, options.size);
            ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
            ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
        }
    }

    void setOptions(const SpinnerOptions& opt) {
        options = opt;
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
        if (effect && (options.style == SpinnerStyle::CustomShader || !options.custom_shader.empty())) {
            double current_time = getSteadyTimeSecs() - start_time;
            struct Uniforms {
                float resolution[2];
                float time;
            } uniforms = {
                {diameter, diameter},
                (float)(current_time * options.rotation_speed)
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
        else if (options.style == SpinnerStyle::Spokes) {
            // Style 1: Spokes (iOS / macOS Radiating Ticks)
            int count = std::max(4, options.spoke_count);
            float step_angle = 360.0f / count;
            float rot_angle = anim_time * options.rotation_speed * 180.0f;
            int active_idx = static_cast<int>(fmod(anim_time * options.rotation_speed * count * 0.8f, count));

            float inner_r = radius * 0.45f;
            float outer_r = radius * 0.85f;

            SkPaint spoke_paint;
            spoke_paint.setAntiAlias(true);
            spoke_paint.setStrokeCap(SkPaint::kRound_Cap);
            spoke_paint.setStrokeWidth(options.spoke_width);

            if (options.glow_blur > 0.0f && options.glow_color != 0) {
                spoke_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.glow_blur * 0.5f));
            }

            for (int i = 0; i < count; ++i) {
                float angle_deg = i * step_angle + rot_angle;
                float angle_rad = angle_deg * (kPI / 180.0f);

                int relative_idx = (i - active_idx + count) % count;
                float alpha_ratio = static_cast<float>(relative_idx) / static_cast<float>(count);
                alpha_ratio = 0.2f + 0.8f * alpha_ratio; // min 20% opacity

                uint32_t base_c = options.color;
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
        else if (options.style == SpinnerStyle::OrbitDots) {
            // Style 2: OrbitDots (Material / Fluent Orbiting Dots)
            int count = std::max(2, options.dot_count);
            float base_rot = anim_time * options.rotation_speed * 200.0f;
            float orbit_r = radius - options.dot_size;

            SkPaint dot_paint;
            dot_paint.setAntiAlias(true);
            dot_paint.setStyle(SkPaint::kFill_Style);

            if (options.glow_blur > 0.0f && options.glow_color != 0) {
                dot_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.glow_blur * 0.5f));
            }

            for (int i = 0; i < count; ++i) {
                float fraction = static_cast<float>(i) / static_cast<float>(count);
                float angle_deg = base_rot + fraction * 360.0f * (1.0f + 0.2f * sin(anim_time * 2.0f));
                float angle_rad = angle_deg * (kPI / 180.0f);

                float dx = cx + orbit_r * cos(angle_rad);
                float dy = cy + orbit_r * sin(angle_rad);
                float dot_r = (options.dot_size / 2.0f) * (0.4f + 0.6f * (1.0f - fraction));

                if (!options.gradient_colors.empty()) {
                    size_t c_idx = i % options.gradient_colors.size();
                    dot_paint.setColor(options.gradient_colors[c_idx]);
                } else {
                    dot_paint.setColor(options.color);
                }

                canvas->drawCircle(dx, dy, dot_r, dot_paint);
            }
        }
        else if (options.style == SpinnerStyle::DualArc) {
            // Style 3: DualArc (Futuristic Dual Counter-Rotating Arcs)
            float rot1 = fmod(anim_time * options.rotation_speed * 180.0f, 360.0f);
            float rot2 = fmod(-anim_time * options.rotation_speed * 260.0f, 360.0f);

            SkPaint arc_paint;
            arc_paint.setAntiAlias(true);
            arc_paint.setStyle(SkPaint::kStroke_Style);
            arc_paint.setStrokeCap(SkPaint::kRound_Cap);

            if (options.glow_blur > 0.0f && options.glow_color != 0) {
                arc_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.glow_blur * 0.5f));
            }

            // Outer Arc
            float r1 = radius - 3.0f;
            SkRect rect1 = SkRect::MakeLTRB(cx - r1, cy - r1, cx + r1, cy + r1);
            arc_paint.setStrokeWidth(3.0f);
            arc_paint.setColor(options.color);
            canvas->drawArc(rect1, rot1, 270.0f, false, arc_paint);

            // Inner Arc
            float r2 = radius - 9.0f;
            SkRect rect2 = SkRect::MakeLTRB(cx - r2, cy - r2, cx + r2, cy + r2);
            arc_paint.setStrokeWidth(2.5f);

            Color inner_c = options.gradient_colors.empty() ? 0xFFEC4899 : options.gradient_colors[0];
            arc_paint.setColor(inner_c);
            canvas->drawArc(rect2, rot2, 180.0f, false, arc_paint);
        }

        // Paint Center Child Widget if provided
        if (!children().empty()) {
            RenderBox* child_ro = static_cast<RenderBox*>(children()[0]);
            Point child_offset = {
                (size_.width - child_ro->size().width) / 2.0f,
                (size_.height - child_ro->size().height) / 2.0f
            };
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
    SpinnerOptions options;
    float anim_time;
    double start_time;

    SpinnerRenderWidget(SpinnerOptions opt, float anim_t, double st, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)), anim_time(anim_t), start_time(st) {}

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
        auto* sp = static_cast<const Spinner*>(widget());

        return std::make_shared<SpinnerRenderWidget>(
            sp->options, anim_time_, start_time_, sp->child
        );
    }
};

std::unique_ptr<State> Spinner::createState() {
    return std::make_unique<SpinnerState>();
}

} // namespace enki
