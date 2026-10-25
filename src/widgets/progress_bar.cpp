#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"

#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkShader.h>
#include <include/core/SkMatrix.h>
#include <layout_engine/Anu.h>

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace enki {

class ProgressBarShaderWrapper : public Shader {
public:
    explicit ProgressBarShaderWrapper(sk_sp<SkShader> shader) : shader_(std::move(shader)) {}
    void* getNativeHandle() const override { return shader_.get(); }
private:
    sk_sp<SkShader> shader_;
};

inline double getSteadyTimeSeconds() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0;
}

// ════════════════════════════════════════════════════════════════
// RenderProgressBar — Custom RenderBox for Linear Progress Bar
// ════════════════════════════════════════════════════════════════

class RenderProgressBar : public RenderBox {
public:
    float height_;
    float border_radius_;
    Color background_color_;
    Color progress_color_;
    std::vector<Color> gradient_colors_;
    Color glow_color_;
    float glow_blur_;
    bool indeterminate_;
    std::string custom_shader_;
    bool show_label_;
    std::string label_format_;
    float min_width_;

    float value;
    float anim_time;
    double start_time;

    sk_sp<SkRuntimeEffect> effect;

    RenderProgressBar(const ProgressBarWidget* opt, float val, float anim_t, double st)
        : height_(opt->height), border_radius_(opt->border_radius),
          background_color_(opt->background_color), progress_color_(opt->progress_color),
          gradient_colors_(opt->gradient_colors), glow_color_(opt->glow_color),
          glow_blur_(opt->glow_blur), indeterminate_(opt->indeterminate),
          custom_shader_(opt->custom_shader), show_label_(opt->show_label),
          label_format_(opt->label_format), min_width_(opt->min_width),
          value(val), anim_time(anim_t), start_time(st) {
        
        updateAnuStyles();

        if (!custom_shader_.empty()) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(custom_shader_.c_str()));
            if (!eff) {
                std::cerr << "ProgressBar SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    void updateAnuStyles() {
        if (anu_node_) {
            ANUNodeStyleSetHeight(anu_node_, height_);
            ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
            ANUNodeStyleSetMinWidth(anu_node_, min_width_);
        }
    }

    void setOptions(const ProgressBarWidget* opt) {
        height_ = opt->height;
        border_radius_ = opt->border_radius;
        background_color_ = opt->background_color;
        progress_color_ = opt->progress_color;
        gradient_colors_ = opt->gradient_colors;
        glow_color_ = opt->glow_color;
        glow_blur_ = opt->glow_blur;
        indeterminate_ = opt->indeterminate;
        custom_shader_ = opt->custom_shader;
        show_label_ = opt->show_label;
        label_format_ = opt->label_format;
        min_width_ = opt->min_width;
        
        updateAnuStyles();
        markNeedsLayout();
    }

    void paint(PaintContext& ctx) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        auto& canvas = ctx.canvas;

        // 1. Background Track
        Rect track_rect{ctx.offset.x, ctx.offset.y, size_.width, size_.height};
        BorderRadius track_radius = BorderRadius::all(border_radius_);

        Paint track_paint;
        track_paint.setAntiAlias(true);
        track_paint.setColor(background_color_);
        canvas.drawRRect(track_rect, track_radius, track_paint);

        // Clip subsequent progress rendering to the rounded track
        canvas.save();
        canvas.clipRRect(track_rect, track_radius, true);

        // 2. Active Progress Fill
        if (indeterminate_) {
            // Indeterminate shimmer sweep animation
            float shimmer_width = size_.width * 0.4f;
            float cycle = fmod(anim_time * 1.2f, 1.0f);
            float start_x = -shimmer_width + (size_.width + shimmer_width * 2.0f) * cycle;

            Rect shimmer_rect{ctx.offset.x + start_x, ctx.offset.y, shimmer_width, size_.height};

            Paint fill_paint;
            fill_paint.setAntiAlias(true);

            if (effect) {
                double current_time = getSteadyTimeSeconds() - start_time;
                struct Uniforms {
                    float time;
                    float resolution[2];
                    float progress;
                } uniforms = {
                    (float)current_time,
                    {size_.width, size_.height},
                    0.5f
                };
                sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
                fill_paint.setShader(std::make_shared<ProgressBarShaderWrapper>(effect->makeShader(uniform_data, nullptr, 0)));
            } else {
                fill_paint.setShader(Gradient::linear(
                    Point{ctx.offset.x + start_x, ctx.offset.y},
                    Point{ctx.offset.x + start_x + shimmer_width, ctx.offset.y},
                    {background_color_, progress_color_, background_color_},
                    {0.0f, 0.5f, 1.0f}
                ));
            }

            canvas.drawRect(shimmer_rect, fill_paint);
        } else {
            // Determinate Progress
            float clamped_val = std::clamp(value, 0.0f, 1.0f);
            float fill_width = clamped_val * size_.width;

            if (fill_width > 0.0f) {
                Rect fill_rect{ctx.offset.x, ctx.offset.y, fill_width, size_.height};
                BorderRadius fill_radius = BorderRadius::all(border_radius_);

                // Glow Effect
                if (glow_blur_ > 0.0f && glow_color_ != 0) {
                    canvas.drawGlow(fill_rect, fill_radius, glow_color_, glow_blur_ * 0.5f);
                }

                Paint fill_paint;
                fill_paint.setAntiAlias(true);

                if (effect) {
                    double current_time = getSteadyTimeSeconds() - start_time;
                    struct Uniforms {
                        float time;
                        float resolution[2];
                        float progress;
                    } uniforms = {
                        (float)current_time,
                        {size_.width, size_.height},
                        clamped_val
                    };
                    sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
                    SkMatrix local_matrix = SkMatrix::Translate(-ctx.offset.x, -ctx.offset.y);
                    fill_paint.setShader(std::make_shared<ProgressBarShaderWrapper>(
                        effect->makeShader(uniform_data, nullptr, 0)->makeWithLocalMatrix(local_matrix)));
                } else if (gradient_colors_.size() >= 2) {
                    fill_paint.setShader(Gradient::linear(
                        Point{ctx.offset.x, ctx.offset.y},
                        Point{ctx.offset.x + fill_width, ctx.offset.y},
                        gradient_colors_
                    ));
                } else {
                    fill_paint.setColor(progress_color_);
                }

                canvas.drawRRect(fill_rect, fill_radius, fill_paint);
            }
        }

        canvas.restore(); // Restore track clip

        // 3. Optional Percentage / Text Label
        if (show_label_ && size_.height >= 12.0f) {
            std::string label_str;
            if (indeterminate_) {
                label_str = "Loading...";
            } else {
                int pct = static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100.0f);
                label_str = std::to_string(pct) + "%";
            }

            float font_size = std::min(size_.height * 0.75f, 12.0f);
            Paint text_paint;
            text_paint.setColor(0xFFFFFFFF);
            float estimated_width = font_size * 0.6f * label_str.length();
            float cx = ctx.offset.x + (size_.width - estimated_width) / 2.0f;
            float cy = ctx.offset.y + (size_.height + font_size * 0.75f) / 2.0f;

            canvas.drawText(label_str, Point{cx, cy}, text_paint, font_size);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget for ProgressBar
// ════════════════════════════════════════════════════════════════

class ProgressBarRenderWidget : public SingleChildRenderObjectWidget {
public:
    const ProgressBarWidget* options;
    float value;
    float anim_time;
    double start_time;

    ProgressBarRenderWidget(const ProgressBarWidget* opt, float val, float anim_t, double st)
        : SingleChildRenderObjectWidget(Key::none(), nullptr),
          options(opt), value(val), anim_time(anim_t), start_time(st) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderProgressBar>(options, value, anim_time, start_time);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderProgressBar*>(&renderObject)) {
            rb->setOptions(options);
            rb->value = value;
            rb->anim_time = anim_time;
            rb->start_time = start_time;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "ProgressBarRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// ProgressBar State
// ════════════════════════════════════════════════════════════════

class ProgressBarState : public State {
    std::unique_ptr<Ticker> ticker_;
    double start_time_ = 0.0;
    float anim_time_ = 0.0f;

public:
    void initState() override {
        State::initState();
        start_time_ = getSteadyTimeSeconds();

        auto* pb = static_cast<const ProgressBarWidget*>(widget());
        if (pb->indeterminate || !pb->custom_shader.empty()) {
            ticker_ = createTicker([this]() {
                anim_time_ = static_cast<float>(getSteadyTimeSeconds() - start_time_);
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
        auto* pb = static_cast<const ProgressBarWidget*>(widget());

        return std::make_shared<ProgressBarRenderWidget>(
            pb, pb->value, anim_time_, start_time_
        );
    }
};

std::unique_ptr<State> ProgressBarWidget::createState() {
    return std::make_unique<ProgressBarState>();
}

} // namespace enki
