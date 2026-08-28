/// @file feedback_status.cpp
/// @brief Implementation of ENKI Section 18 Feedback & Status Extended widgets.
///
/// Widgets:
///   1. Skeleton
///   2. Ripple
///   3. Pulse
///   4. CountBadge
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/effects/SkGradientShader.h>

#include <cmath>
#include <algorithm>
#include <vector>
#include <sstream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. Skeleton (Shimmer Loading Placeholder)
// ════════════════════════════════════════════════════════════════

class RenderSkeleton : public RenderBox {
public:
    Color                     base_color_;
    Color                     highlight_color_;
    BorderRadius              border_radius_;
    SkeletonShape             shape_;
    std::optional<StyleValue> width_;
    std::optional<StyleValue> height_;
    float                     progress_ = 0.0f;

    RenderSkeleton(Color base, Color highlight, BorderRadius radius, SkeletonShape shape,
                   std::optional<StyleValue> w, std::optional<StyleValue> h)
        : base_color_(base), highlight_color_(highlight), border_radius_(radius), shape_(shape),
          width_(w), height_(h) {
        applyStyle();
    }

    void applyStyle() {
        if (width_) {
            if (width_->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, width_->value);
            else if (width_->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, width_->value);
        }
        if (height_) {
            if (height_->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, height_->value);
            else if (height_->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, height_->value);
        }
    }

    void update(Color base, Color highlight, BorderRadius radius, SkeletonShape shape,
                std::optional<StyleValue> w, std::optional<StyleValue> h) {
        base_color_ = base;
        highlight_color_ = highlight;
        border_radius_ = radius;
        shape_ = shape;
        width_ = w;
        height_ = h;
        applyStyle();
        markNeedsLayout();
        markNeedsPaint();
    }

    void setProgress(float p) {
        progress_ = p;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;

        float w = size_.width;
        float h = size_.height;
        if (w <= 0.0f && width_ && !width_->isPercent() && !width_->isAuto()) w = width_->value;
        if (h <= 0.0f && height_ && !height_->isPercent() && !height_->isAuto()) h = height_->value;
        if (w <= 0.0f || h <= 0.0f) return;

        ctx.canvas.save();
        ctx.canvas.translate(ctx.offset.x, ctx.offset.y);

        // Create linear gradient shimmer sweep
        float shimmer_width = w * 1.5f;
        float start_x = -shimmer_width + (w + shimmer_width * 2.0f) * progress_ - shimmer_width * 0.5f;
        float end_x = start_x + shimmer_width;

        SkPoint pts[2] = { SkPoint::Make(start_x, 0), SkPoint::Make(end_x, h) };
        SkColor colors[3] = {
            static_cast<SkColor>(base_color_),
            static_cast<SkColor>(highlight_color_),
            static_cast<SkColor>(base_color_)
        };
        SkScalar pos[3] = { 0.0f, 0.5f, 1.0f };

        sk_sp<SkShader> shader = SkGradientShader::MakeLinear(
            pts, colors, pos, 3, SkTileMode::kClamp
        );

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(shader);

        if (shape_ == SkeletonShape::Circle) {
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            float radius = std::min(w, h) * 0.5f;
            canvas->drawCircle(cx, cy, radius, paint);
        } else {
            SkRect rect = SkRect::MakeWH(w, h);
            SkVector radii[4] = {
                { border_radius_.top_left, border_radius_.top_left },
                { border_radius_.top_right, border_radius_.top_right },
                { border_radius_.bottom_right, border_radius_.bottom_right },
                { border_radius_.bottom_left, border_radius_.bottom_left }
            };
            SkRRect rrect;
            rrect.setRectRadii(rect, radii);
            canvas->drawRRect(rrect, paint);
        }

        ctx.canvas.restore();

        // Paint child if present
        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }
    }
};

class SkeletonRenderWidget : public SingleChildRenderObjectWidget {
public:
    Color                     base_color;
    Color                     highlight_color;
    BorderRadius              border_radius;
    SkeletonShape             shape;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    float                     progress;

    SkeletonRenderWidget(Color base, Color highlight, BorderRadius radius, SkeletonShape shp,
                         std::optional<StyleValue> w, std::optional<StyleValue> h, float prog, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          base_color(base), highlight_color(highlight), border_radius(radius), shape(shp),
          width(w), height(h), progress(prog) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderSkeleton>(base_color, highlight_color, border_radius, shape, width, height);
        r->setProgress(progress);
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderSkeleton&>(ro);
        r.update(base_color, highlight_color, border_radius, shape, width, height);
        r.setProgress(progress);
    }

    [[nodiscard]] std::string_view typeName() const override { return "SkeletonRenderWidget"; }
};

class SkeletonState : public State {
    std::unique_ptr<Ticker>               ticker_;
    std::chrono::steady_clock::time_point start_time_;
    float                                 progress_ = 0.0f;

public:
    void initState() override {
        State::initState();
        start_time_ = std::chrono::steady_clock::now();
        auto* w = static_cast<const SkeletonWidget*>(widget());
        if (w->enabled) {
            ticker_ = createTicker([this]() { onTick(); });
            ticker_->start();
        }
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* new_w = static_cast<const SkeletonWidget*>(widget());
        if (new_w->enabled) {
            if (!ticker_) {
                start_time_ = std::chrono::steady_clock::now();
                ticker_ = createTicker([this]() { onTick(); });
                ticker_->start();
            }
        } else {
            if (ticker_) {
                ticker_->stop();
                ticker_.reset();
            }
        }
    }

    void onTick() {
        auto* w = static_cast<const SkeletonWidget*>(widget());
        if (!w || !w->enabled) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(w->duration.count());
        if (dur_ms <= 0.0) dur_ms = 1200.0;

        double cycle = std::fmod(static_cast<double>(elapsed), dur_ms) / dur_ms;
        progress_ = static_cast<float>(cycle);
        setState([] {});
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SkeletonWidget*>(widget());

        if (!w->enabled) {
            return w->child ? w->child : container({});
        }

        return std::make_shared<SkeletonRenderWidget>(
            w->base_color,
            w->highlight_color,
            w->border_radius,
            w->shape,
            w->width,
            w->height,
            progress_,
            w->child
        );
    }
};

std::unique_ptr<State> SkeletonWidget::createState() {
    return std::make_unique<SkeletonState>();
}

// ════════════════════════════════════════════════════════════════
// 2. Ripple (Material Ink-Ripple Effect)
// ════════════════════════════════════════════════════════════════

struct ActiveRipple {
    Point center;
    float current_radius = 0.0f;
    float max_radius = 100.0f;
    float alpha = 1.0f;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds duration{350};
};

class RenderRipple : public RenderBox {
public:
    Color                     ripple_color_;
    BorderRadius              border_radius_;
    bool                      clip_ripple_;
    std::vector<ActiveRipple> ripples_;

    RenderRipple(Color col, BorderRadius radius, bool clip)
        : ripple_color_(col), border_radius_(radius), clip_ripple_(clip) {}

    void update(Color col, BorderRadius radius, bool clip, const std::vector<ActiveRipple>& rips) {
        ripple_color_ = col;
        border_radius_ = radius;
        clip_ripple_ = clip;
        ripples_ = rips;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;

        // Paint child first
        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        if (ripples_.empty()) return;

        ctx.canvas.save();
        ctx.canvas.translate(ctx.offset.x, ctx.offset.y);

        if (clip_ripple_ && size_.width > 0.0f && size_.height > 0.0f) {
            SkRect rect = SkRect::MakeWH(size_.width, size_.height);
            SkVector radii[4] = {
                { border_radius_.top_left, border_radius_.top_left },
                { border_radius_.top_right, border_radius_.top_right },
                { border_radius_.bottom_right, border_radius_.bottom_right },
                { border_radius_.bottom_left, border_radius_.bottom_left }
            };
            SkRRect rrect;
            rrect.setRectRadii(rect, radii);
            canvas->clipRRect(rrect, true);
        }

        uint8_t baseA = (ripple_color_ >> 24) & 0xFF;
        uint8_t baseR = (ripple_color_ >> 16) & 0xFF;
        uint8_t baseG = (ripple_color_ >> 8) & 0xFF;
        uint8_t baseB = ripple_color_ & 0xFF;

        for (const auto& rip : ripples_) {
            if (rip.current_radius <= 0.0f || rip.alpha <= 0.0f) continue;

            uint8_t finalA = static_cast<uint8_t>(baseA * std::clamp(rip.alpha, 0.0f, 1.0f));
            SkColor c = SkColorSetARGB(finalA, baseR, baseG, baseB);

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(c);
            paint.setStyle(SkPaint::kFill_Style);

            canvas->drawCircle(rip.center.x, rip.center.y, rip.current_radius, paint);
        }

        ctx.canvas.restore();
    }
};

class RippleRenderWidget : public SingleChildRenderObjectWidget {
public:
    Color                     color;
    BorderRadius              border_radius;
    bool                      clip_ripple;
    std::vector<ActiveRipple> ripples;

    RippleRenderWidget(Color col, BorderRadius radius, bool clip, std::vector<ActiveRipple> rips, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          color(col), border_radius(radius), clip_ripple(clip), ripples(std::move(rips)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderRipple>(color, border_radius, clip_ripple);
        r->update(color, border_radius, clip_ripple, ripples);
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderRipple&>(ro);
        r.update(color, border_radius, clip_ripple, ripples);
    }

    [[nodiscard]] std::string_view typeName() const override { return "RippleRenderWidget"; }
};

class RippleState : public State {
    std::unique_ptr<Ticker>   ticker_;
    std::vector<ActiveRipple> ripples_;

public:
    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    void addRipple(Point pos, Size box_size) {
        auto* w = static_cast<const RippleWidget*>(widget());
        float max_d = std::sqrt(box_size.width * box_size.width + box_size.height * box_size.height);
        if (max_d <= 0.0f) max_d = 200.0f;

        ActiveRipple r;
        r.center = pos;
        r.current_radius = 0.0f;
        r.max_radius = max_d * 1.2f;
        r.alpha = 1.0f;
        r.start_time = std::chrono::steady_clock::now();
        r.duration = w ? w->duration : std::chrono::milliseconds(350);

        ripples_.push_back(r);

        if (!ticker_) {
            ticker_ = createTicker([this]() { onTick(); });
        }
        ticker_->start();
        setState([] {});
    }

    void onTick() {
        auto now = std::chrono::steady_clock::now();

        for (auto it = ripples_.begin(); it != ripples_.end();) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->start_time).count();
            double dur_ms = static_cast<double>(it->duration.count());
            double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
            t = std::clamp(t, 0.0, 1.0);

            double curved_t = Curves::easeOut.evaluate(t);
            it->current_radius = static_cast<float>(it->max_radius * curved_t);
            it->alpha = static_cast<float>(1.0 - t);

            if (t >= 1.0) {
                it = ripples_.erase(it);
            } else {
                ++it;
            }
        }

        if (ripples_.empty() && ticker_) {
            ticker_->stop();
        }

        setState([] {});
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const RippleWidget*>(widget());

        auto render_widget = std::make_shared<RippleRenderWidget>(
            w->color,
            w->border_radius,
            w->clip_ripple,
            ripples_,
            w->child
        );

        return gestureDetector({
            .child = render_widget,
            .on_tap_down = [this, w](const TapDownDetails& e) {
                addRipple(e.local_position, {200.0f, 100.0f});
                if (w->on_tap) w->on_tap();
            },
        });
    }
};

std::unique_ptr<State> RippleWidget::createState() {
    return std::make_unique<RippleState>();
}

// ════════════════════════════════════════════════════════════════
// 3. Pulse (Concentric Beacon / Radar Status Animation)
// ════════════════════════════════════════════════════════════════

class RenderPulse : public RenderBox {
public:
    Color        color_;
    size_t       ring_count_;
    float        max_radius_;
    float        dot_radius_;
    bool         center_dot_;
    const Curve* curve_;
    float        progress_ = 0.0f;

    RenderPulse(Color col, size_t rings, float max_r, float dot_r, bool center_dot, const Curve* c)
        : color_(col), ring_count_(rings), max_radius_(max_r), dot_radius_(dot_r),
          center_dot_(center_dot), curve_(c) {}

    void update(Color col, size_t rings, float max_r, float dot_r, bool center_dot, const Curve* c, float prog) {
        color_ = col;
        ring_count_ = rings;
        max_radius_ = max_r;
        dot_radius_ = dot_r;
        center_dot_ = center_dot;
        curve_ = c;
        progress_ = prog;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;

        ctx.canvas.save();
        ctx.canvas.translate(ctx.offset.x, ctx.offset.y);

        float cx = size_.width * 0.5f;
        float cy = size_.height * 0.5f;

        uint8_t baseA = (color_ >> 24) & 0xFF;
        uint8_t baseR = (color_ >> 16) & 0xFF;
        uint8_t baseG = (color_ >> 8) & 0xFF;
        uint8_t baseB = color_ & 0xFF;

        // Render radiating rings
        if (ring_count_ > 0 && max_radius_ > dot_radius_) {
            for (size_t i = 0; i < ring_count_; ++i) {
                float phase = std::fmod(progress_ + static_cast<float>(i) / static_cast<float>(ring_count_), 1.0f);
                float curved_phase = curve_ ? curve_->evaluateF(phase) : phase;

                float r = dot_radius_ + (max_radius_ - dot_radius_) * curved_phase;
                float ring_alpha = (1.0f - phase) * 0.7f;
                uint8_t curA = static_cast<uint8_t>(baseA * ring_alpha);

                SkPaint ring_paint;
                ring_paint.setAntiAlias(true);
                ring_paint.setColor(SkColorSetARGB(curA, baseR, baseG, baseB));
                ring_paint.setStyle(SkPaint::kFill_Style);

                canvas->drawCircle(cx, cy, r, ring_paint);
            }
        }

        // Render solid center dot
        if (center_dot_ && dot_radius_ > 0.0f) {
            SkPaint dot_paint;
            dot_paint.setAntiAlias(true);
            dot_paint.setColor(static_cast<SkColor>(color_));
            dot_paint.setStyle(SkPaint::kFill_Style);
            canvas->drawCircle(cx, cy, dot_radius_, dot_paint);
        }

        ctx.canvas.restore();

        // Paint child centered
        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }
    }
};

class PulseRenderWidget : public SingleChildRenderObjectWidget {
public:
    Color        color;
    size_t       ring_count;
    float        max_radius;
    float        dot_radius;
    bool         center_dot;
    const Curve* curve;
    float        progress;

    PulseRenderWidget(Color col, size_t rings, float max_r, float dot_r, bool center_dot, const Curve* c, float prog, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          color(col), ring_count(rings), max_radius(max_r), dot_radius(dot_r),
          center_dot(center_dot), curve(c), progress(prog) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderPulse>(color, ring_count, max_radius, dot_radius, center_dot, curve);
        r->update(color, ring_count, max_radius, dot_radius, center_dot, curve, progress);
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderPulse&>(ro);
        r.update(color, ring_count, max_radius, dot_radius, center_dot, curve, progress);
    }

    [[nodiscard]] std::string_view typeName() const override { return "PulseRenderWidget"; }
};

class PulseState : public State {
    std::unique_ptr<Ticker>               ticker_;
    std::chrono::steady_clock::time_point start_time_;
    float                                 progress_ = 0.0f;

public:
    void initState() override {
        State::initState();
        start_time_ = std::chrono::steady_clock::now();
        ticker_ = createTicker([this]() { onTick(); });
        ticker_->start();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    void onTick() {
        auto* w = static_cast<const PulseWidget*>(widget());
        if (!w) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(w->duration.count());
        if (dur_ms <= 0.0) dur_ms = 1500.0;

        double cycle = std::fmod(static_cast<double>(elapsed), dur_ms) / dur_ms;
        progress_ = static_cast<float>(cycle);
        setState([] {});
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const PulseWidget*>(widget());
        float dim = w->max_radius * 2.0f + 4.0f;

        auto render_widget = std::make_shared<PulseRenderWidget>(
            w->color,
            w->ring_count,
            w->max_radius,
            w->dot_radius,
            w->center_dot,
            w->curve,
            progress_,
            w->child
        );

        return container({
            .align = Alignment::Center,
            .width = StyleValue::point(dim),
            .height = StyleValue::point(dim),
            .child = render_widget,
        });
    }
};

std::unique_ptr<State> PulseWidget::createState() {
    return std::make_unique<PulseState>();
}

// ════════════════════════════════════════════════════════════════
// 4. CountBadge (Animated Numeric Badge with Spring / Overflow)
// ════════════════════════════════════════════════════════════════

class CountBadgeState : public State {
    std::unique_ptr<Ticker>               ticker_;
    int                                   prev_count_ = 0;
    int                                   current_count_ = 0;
    float                                 scale_ = 1.0f;
    std::chrono::steady_clock::time_point start_time_;
    bool                                  is_animating_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const CountBadgeWidget*>(widget());
        prev_count_ = w->count;
        current_count_ = w->count;
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const CountBadgeWidget*>(&oldWidget);
        auto* new_w = static_cast<const CountBadgeWidget*>(widget());

        if (old_w->count != new_w->count) {
            prev_count_ = current_count_;
            current_count_ = new_w->count;
            start_time_ = std::chrono::steady_clock::now();
            is_animating_ = true;

            if (!ticker_) {
                ticker_ = createTicker([this]() { onTick(); });
            }
            ticker_->start();
        }
    }

    void onTick() {
        if (!is_animating_) return;
        auto* w = static_cast<const CountBadgeWidget*>(widget());
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(w->animation_duration.count());
        if (dur_ms <= 0.0) dur_ms = 300.0;

        double t = std::clamp(static_cast<double>(elapsed) / dur_ms, 0.0, 1.0);

        // Spring pop: scale 1.0 -> 1.35 -> 1.0
        if (t < 0.5) {
            scale_ = static_cast<float>(1.0 + (t / 0.5) * 0.35);
        } else {
            scale_ = static_cast<float>(1.35 - ((t - 0.5) / 0.5) * 0.35);
        }

        if (t >= 1.0) {
            scale_ = 1.0f;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
        }

        setState([] {});
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const CountBadgeWidget*>(widget());

        if (w->count == 0 && !w->show_zero) {
            return w->child ? w->child : container({});
        }

        std::string display_str;
        if (w->max_count.has_value() && w->count > *w->max_count) {
            display_str = std::to_string(*w->max_count) + "+";
        } else {
            display_str = std::to_string(w->count);
        }

        auto badge_box = container({
            .color = w->bg_color,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFFFFFFFF, 1.5f),
            .align = Alignment::Center,
            .height = StyleValue::point(20.0f),
            .min_width = StyleValue::point(20.0f),
            .padding = StyleInsets::symmetric(StyleValue::point(1.0f), StyleValue::point(6.0f)),
            .child = text(display_str, {
                .color = w->text_color,
                .font_size = w->font_size,
                .font_weight = FontWeight::Bold,
            }),
        });

        if (!w->child) {
            return badge_box;
        }

        PositionedProps p_props = {
            .child = badge_box,
        };

        float ox = w->offset.x;
        float oy = w->offset.y;

        if (w->alignment == Alignment::TopRight) {
            p_props.top = StyleValue::point(-8.0f + oy);
            p_props.right = StyleValue::point(-8.0f - ox);
        } else if (w->alignment == Alignment::TopLeft) {
            p_props.top = StyleValue::point(-8.0f + oy);
            p_props.left = StyleValue::point(-8.0f + ox);
        } else if (w->alignment == Alignment::BottomRight) {
            p_props.bottom = StyleValue::point(-8.0f - oy);
            p_props.right = StyleValue::point(-8.0f - ox);
        } else if (w->alignment == Alignment::BottomLeft) {
            p_props.bottom = StyleValue::point(-8.0f - oy);
            p_props.left = StyleValue::point(-8.0f + ox);
        } else {
            p_props.top = StyleValue::point(-8.0f + oy);
            p_props.right = StyleValue::point(-8.0f - ox);
        }

        return stack({
            .children = { w->child, positioned(p_props) },
            .clip_behavior = Clip::None,
        });
    }
};

std::unique_ptr<State> CountBadgeWidget::createState() {
    return std::make_unique<CountBadgeState>();
}

} // namespace enki
