#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkRuntimeEffect.h>
#include <layout_engine/Anu.h>

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace enki {

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
    ProgressBarOptions options;
    float value;
    float anim_time;
    double start_time;

    sk_sp<SkRuntimeEffect> effect;

    RenderProgressBar(const ProgressBarOptions& opt, float val, float anim_t, double st)
        : options(opt), value(val), anim_time(anim_t), start_time(st) {
        
        updateAnuStyles();

        if (!options.custom_shader.empty()) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (!eff) {
                std::cerr << "ProgressBar SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    void updateAnuStyles() {
        if (anu_node_) {
            ANUNodeStyleSetHeight(anu_node_, options.height);
            ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
            ANUNodeStyleSetMinWidth(anu_node_, options.min_width);
        }
    }

    void setOptions(const ProgressBarOptions& opt) {
        options = opt;
        updateAnuStyles();
        markNeedsLayout();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        // 1. Background Track
        SkRect track_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect track_rrect;
        track_rrect.setRectXY(track_rect, options.border_radius, options.border_radius);

        SkPaint track_paint;
        track_paint.setAntiAlias(true);
        track_paint.setColor(options.background_color);
        canvas->drawRRect(track_rrect, track_paint);

        // Clip subsequent progress rendering to the rounded track
        canvas->save();
        canvas->clipRRect(track_rrect, true);

        // 2. Active Progress Fill
        if (options.indeterminate) {
            // Indeterminate shimmer sweep animation
            float shimmer_width = size_.width * 0.4f;
            float cycle = fmod(anim_time * 1.2f, 1.0f);
            float start_x = -shimmer_width + (size_.width + shimmer_width * 2.0f) * cycle;

            SkRect shimmer_rect = SkRect::MakeXYWH(ctx.offset.x + start_x, ctx.offset.y, shimmer_width, size_.height);

            SkPaint fill_paint;
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
                fill_paint.setShader(effect->makeShader(uniform_data, nullptr, 0));
            } else {
                // Gradient shimmer sweep
                SkPoint pts[2] = {
                    {ctx.offset.x + start_x, ctx.offset.y},
                    {ctx.offset.x + start_x + shimmer_width, ctx.offset.y}
                };
                SkColor colors[3] = {
                    options.background_color,
                    options.progress_color,
                    options.background_color
                };
                fill_paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 3, SkTileMode::kClamp));
            }

            canvas->drawRect(shimmer_rect, fill_paint);
        } else {
            // Determinate Progress
            float clamped_val = std::clamp(value, 0.0f, 1.0f);
            float fill_width = clamped_val * size_.width;

            if (fill_width > 0.0f) {
                SkRect fill_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, fill_width, size_.height);
                SkRRect fill_rrect;
                fill_rrect.setRectXY(fill_rect, options.border_radius, options.border_radius);

                // Glow Effect
                if (options.glow_blur > 0.0f && options.glow_color != 0) {
                    SkPaint glow_paint;
                    glow_paint.setColor(options.glow_color);
                    glow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.glow_blur * 0.5f));
                    canvas->drawRRect(fill_rrect, glow_paint);
                }

                SkPaint fill_paint;
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
                fill_paint.setShader(effect->makeShader(uniform_data, nullptr, 0)->makeWithLocalMatrix(local_matrix));
                } else if (options.gradient_colors.size() >= 2) {
                    std::vector<SkColor> sk_colors;
                    for (auto c : options.gradient_colors) sk_colors.push_back(c);

                    SkPoint pts[2] = {
                        {ctx.offset.x, ctx.offset.y},
                        {ctx.offset.x + fill_width, ctx.offset.y}
                    };
                    fill_paint.setShader(SkGradientShader::MakeLinear(
                        pts, sk_colors.data(), nullptr, static_cast<int>(sk_colors.size()), SkTileMode::kClamp));
                } else {
                    fill_paint.setColor(options.progress_color);
                }

                canvas->drawRRect(fill_rrect, fill_paint);
            }
        }

        canvas->restore(); // Restore track clip

        // 3. Optional Percentage / Text Label
        if (options.show_label && size_.height >= 12.0f) {
            std::string label_str;
            if (options.indeterminate) {
                label_str = "Loading...";
            } else {
                int pct = static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100.0f);
                label_str = std::to_string(pct) + "%";
            }

            SkPaint text_paint;
            text_paint.setAntiAlias(true);
            text_paint.setColor(0xFFFFFFFF);

            SkFont font;
            font.setSize(std::min(size_.height * 0.75f, 12.0f));

            SkRect bounds;
            font.measureText(label_str.c_str(), label_str.length(), SkTextEncoding::kUTF8, &bounds);

            float cx = ctx.offset.x + (size_.width - bounds.width()) / 2.0f;
            float cy = ctx.offset.y + (size_.height + bounds.height()) / 2.0f - bounds.fBottom;

            canvas->drawSimpleText(label_str.c_str(), label_str.length(), SkTextEncoding::kUTF8, cx, cy, font, text_paint);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget for ProgressBar
// ════════════════════════════════════════════════════════════════

class ProgressBarRenderWidget : public SingleChildRenderObjectWidget {
public:
    ProgressBarOptions options;
    float value;
    float anim_time;
    double start_time;

    ProgressBarRenderWidget(ProgressBarOptions opt, float val, float anim_t, double st)
        : SingleChildRenderObjectWidget(Key::none(), nullptr),
          options(std::move(opt)), value(val), anim_time(anim_t), start_time(st) {}

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

        auto* pb = static_cast<const ProgressBar*>(widget());
        if (pb->options.indeterminate || !pb->options.custom_shader.empty()) {
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
        auto* pb = static_cast<const ProgressBar*>(widget());

        return std::make_shared<ProgressBarRenderWidget>(
            pb->options, pb->value, anim_time_, start_time_
        );
    }
};

std::unique_ptr<State> ProgressBar::createState() {
    return std::make_unique<ProgressBarState>();
}

} // namespace enki
