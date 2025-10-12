/// @file motion.cpp
/// @brief Implementation of Section 13 Animation & Motion widgets.
///
/// Features:
///   - AnimatedOpacity
///   - AnimatedContainer
///   - AnimatedScale
///   - AnimatedRotation
///   - AnimatedSlide
///   - AnimatedSwitcher
///   - SlideTransition
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/motion.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"

#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Geometric & Alignment Helpers
// ════════════════════════════════════════════════════════════════

static Point getAlignmentPivot(Alignment align, Size size) {
    switch (align) {
        case Alignment::TopLeft:      return {0.0f, 0.0f};
        case Alignment::TopCenter:    return {size.width * 0.5f, 0.0f};
        case Alignment::TopRight:     return {size.width, 0.0f};
        case Alignment::CenterLeft:   return {0.0f, size.height * 0.5f};
        case Alignment::Center:       return {size.width * 0.5f, size.height * 0.5f};
        case Alignment::CenterRight:  return {size.width, size.height * 0.5f};
        case Alignment::BottomLeft:   return {0.0f, size.height};
        case Alignment::BottomCenter: return {size.width * 0.5f, size.height};
        case Alignment::BottomRight:  return {size.width, size.height};
    }
    return {size.width * 0.5f, size.height * 0.5f};
}

// ════════════════════════════════════════════════════════════════
// 1. AnimatedOpacity
// ════════════════════════════════════════════════════════════════

class RenderOpacityLayer : public RenderBox {
public:
    float opacity_ = 1.0f;

    explicit RenderOpacityLayer(float opacity) : opacity_(std::clamp(opacity, 0.0f, 1.0f)) {}

    void setOpacity(float opacity) {
        float clamped = std::clamp(opacity, 0.0f, 1.0f);
        if (std::abs(opacity_ - clamped) > 0.0001f) {
            opacity_ = clamped;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        if (opacity_ <= 0.001f) return;

        if (opacity_ >= 0.999f) {
            for (auto* child : children_) {
                if (child) {
                    auto child_ctx = ctx.withOffset(child->offset());
                    child->paint(child_ctx);
                }
            }
            return;
        }

        Rect bounds = Rect::fromPointSize(ctx.offset, size_);
        ctx.canvas.saveLayerAlpha(opacity_, &bounds);

        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        ctx.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (opacity_ <= 0.001f) return false;
        return RenderBox::hitTest(result, localPoint);
    }
};

class OpacityRenderWidget : public SingleChildRenderObjectWidget {
public:
    float opacity = 1.0f;

    OpacityRenderWidget(float op, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), opacity(op) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderOpacityLayer>(opacity);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderOpacityLayer&>(ro);
        r.setOpacity(opacity);
    }

    [[nodiscard]] std::string_view typeName() const override { return "OpacityRenderWidget"; }
};

class AnimatedOpacityState : public State {
    std::unique_ptr<Ticker>               ticker_;
    float                                 begin_opacity_ = 1.0f;
    float                                 target_opacity_ = 1.0f;
    float                                 current_opacity_ = 1.0f;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds             duration_{300};
    const Curve*                          curve_ = &Curves::linear;
    std::function<void()>                 on_end_ = nullptr;
    bool                                  is_animating_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedOpacityWidget*>(widget());
        target_opacity_ = w->opacity;
        current_opacity_ = w->opacity;
        duration_ = w->duration;
        curve_ = w->curve;
        on_end_ = w->on_end;
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const AnimatedOpacityWidget*>(&oldWidget);
        auto* new_w = static_cast<const AnimatedOpacityWidget*>(widget());

        if (std::abs(old_w->opacity - new_w->opacity) > 0.0001f) {
            begin_opacity_ = current_opacity_;
            target_opacity_ = new_w->opacity;
            duration_ = new_w->duration;
            curve_ = new_w->curve;
            on_end_ = new_w->on_end;
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
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());
        double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        t = std::clamp(t, 0.0, 1.0);

        double curved_t = curve_ ? curve_->evaluate(t) : t;
        current_opacity_ = static_cast<float>(begin_opacity_ + (target_opacity_ - begin_opacity_) * curved_t);

        if (t >= 1.0) {
            current_opacity_ = target_opacity_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
            if (on_end_) on_end_();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedOpacityWidget*>(widget());
        return std::make_shared<OpacityRenderWidget>(current_opacity_, w->child);
    }
};

std::unique_ptr<State> AnimatedOpacityWidget::createState() {
    return std::make_unique<AnimatedOpacityState>();
}

// ════════════════════════════════════════════════════════════════
// 2. AnimatedContainer
// ════════════════════════════════════════════════════════════════

static BorderRadius lerpBorderRadius(const BorderRadius& a, const BorderRadius& b, double t) {
    return BorderRadius(
        static_cast<float>(a.top_left + (b.top_left - a.top_left) * t),
        static_cast<float>(a.top_right + (b.top_right - a.top_right) * t),
        static_cast<float>(a.bottom_right + (b.bottom_right - a.bottom_right) * t),
        static_cast<float>(a.bottom_left + (b.bottom_left - a.bottom_left) * t)
    );
}

static Border lerpBorder(const Border& a, const Border& b, double t) {
    Color c = Tween<Color>(a.color, b.color).evaluate(t);
    float w = static_cast<float>(a.width + (b.width - a.width) * t);
    return Border(c, w);
}

static StyleValue lerpStyleValue(const StyleValue& a, const StyleValue& b, double t) {
    if (a.isPoint() && b.isPoint()) {
        return StyleValue::point(static_cast<float>(a.value + (b.value - a.value) * t));
    }
    if (a.isPercent() && b.isPercent()) {
        return StyleValue::percent(static_cast<float>(a.value + (b.value - a.value) * t));
    }
    return t < 0.5 ? a : b;
}

static StyleInsets lerpInsets(const StyleInsets& a, const StyleInsets& b, double t) {
    return StyleInsets{
        lerpStyleValue(a.left, b.left, t),
        lerpStyleValue(a.top, b.top, t),
        lerpStyleValue(a.right, b.right, t),
        lerpStyleValue(a.bottom, b.bottom, t),
    };
}

class AnimatedContainerState : public State {
    std::unique_ptr<Ticker>               ticker_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds             duration_{300};
    const Curve*                          curve_ = &Curves::linear;
    std::function<void()>                 on_end_ = nullptr;
    bool                                  is_animating_ = false;

    // Snapshot of previous properties for interpolation
    Color                       prev_color_{Colors::Transparent};
    Color                       target_color_{Colors::Transparent};

    BorderRadius                prev_radius_{BorderRadius::zero()};
    BorderRadius                target_radius_{BorderRadius::zero()};

    Border                      prev_border_{};
    Border                      target_border_{};
    bool                        has_border_ = false;

    StyleValue                  prev_width_{StyleValue::undefined()};
    StyleValue                  target_width_{StyleValue::undefined()};

    StyleValue                  prev_height_{StyleValue::undefined()};
    StyleValue                  target_height_{StyleValue::undefined()};

    StyleInsets                 prev_padding_{};
    StyleInsets                 target_padding_{};

    StyleInsets                 prev_margin_{};
    StyleInsets                 target_margin_{};

    // Current interpolated state
    Color                       curr_color_{Colors::Transparent};
    BorderRadius                curr_radius_{BorderRadius::zero()};
    Border                      curr_border_{};
    StyleValue                  curr_width_{StyleValue::undefined()};
    StyleValue                  curr_height_{StyleValue::undefined()};
    StyleInsets                 curr_padding_{};
    StyleInsets                 curr_margin_{};

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedContainerWidget*>(widget());
        duration_ = w->duration;
        curve_ = w->curve;
        on_end_ = w->on_end;

        curr_color_ = target_color_ = w->color.value_or(Colors::Transparent);
        curr_radius_ = target_radius_ = w->border_radius.value_or(BorderRadius::zero());
        if (w->border) {
            curr_border_ = target_border_ = *w->border;
            has_border_ = true;
        }
        curr_width_ = target_width_ = w->width.value_or(StyleValue::undefined());
        curr_height_ = target_height_ = w->height.value_or(StyleValue::undefined());
        curr_padding_ = target_padding_ = w->padding.value_or(StyleInsets{});
        curr_margin_ = target_margin_ = w->margin.value_or(StyleInsets{});
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* new_w = static_cast<const AnimatedContainerWidget*>(widget());

        prev_color_ = curr_color_;
        target_color_ = new_w->color.value_or(Colors::Transparent);

        prev_radius_ = curr_radius_;
        target_radius_ = new_w->border_radius.value_or(BorderRadius::zero());

        prev_border_ = curr_border_;
        if (new_w->border) {
            target_border_ = *new_w->border;
            has_border_ = true;
        } else {
            target_border_ = Border(Colors::Transparent, 0.0f);
        }

        prev_width_ = curr_width_;
        target_width_ = new_w->width.value_or(StyleValue::undefined());

        prev_height_ = curr_height_;
        target_height_ = new_w->height.value_or(StyleValue::undefined());

        prev_padding_ = curr_padding_;
        target_padding_ = new_w->padding.value_or(StyleInsets{});

        prev_margin_ = curr_margin_;
        target_margin_ = new_w->margin.value_or(StyleInsets{});

        bool changed = (prev_color_ != target_color_) ||
                       !(prev_radius_ == target_radius_) ||
                       !(prev_border_ == target_border_) ||
                       !(prev_width_ == target_width_) ||
                       !(prev_height_ == target_height_) ||
                       !(prev_padding_ == target_padding_) ||
                       !(prev_margin_ == target_margin_);

        if (changed) {
            duration_ = new_w->duration;
            curve_ = new_w->curve;
            on_end_ = new_w->on_end;
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
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());
        double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        t = std::clamp(t, 0.0, 1.0);

        double curved_t = curve_ ? curve_->evaluate(t) : t;

        curr_color_ = Tween<Color>(prev_color_, target_color_).evaluate(curved_t);
        curr_radius_ = lerpBorderRadius(prev_radius_, target_radius_, curved_t);
        curr_border_ = lerpBorder(prev_border_, target_border_, curved_t);
        curr_width_ = lerpStyleValue(prev_width_, target_width_, curved_t);
        curr_height_ = lerpStyleValue(prev_height_, target_height_, curved_t);
        curr_padding_ = lerpInsets(prev_padding_, target_padding_, curved_t);
        curr_margin_ = lerpInsets(prev_margin_, target_margin_, curved_t);

        if (t >= 1.0) {
            curr_color_ = target_color_;
            curr_radius_ = target_radius_;
            curr_border_ = target_border_;
            curr_width_ = target_width_;
            curr_height_ = target_height_;
            curr_padding_ = target_padding_;
            curr_margin_ = target_margin_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
            if (on_end_) on_end_();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedContainerWidget*>(widget());

        Container props;
        props.color = curr_color_;
        props.gradient = w->gradient;
        props.border_radius = curr_radius_;
        if (has_border_ || curr_border_.width > 0.0f) {
            props.border = curr_border_;
        }
        props.box_shadow = w->box_shadow;
        props.shape = w->shape;
        props.clip_content = w->clip_content;
        props.align = w->align;

        if (curr_width_.isDefined()) props.width = curr_width_;
        if (curr_height_.isDefined()) props.height = curr_height_;
        props.min_width = w->min_width;
        props.min_height = w->min_height;
        props.max_width = w->max_width;
        props.max_height = w->max_height;
        props.aspect_ratio = w->aspect_ratio;
        props.padding = curr_padding_;
        props.margin = curr_margin_;
        props.child = w->child;

        return container(props);
    }
};

std::unique_ptr<State> AnimatedContainerWidget::createState() {
    return std::make_unique<AnimatedContainerState>();
}

// ════════════════════════════════════════════════════════════════
// 3. AnimatedScale
// ════════════════════════════════════════════════════════════════

class RenderScale : public RenderBox {
public:
    float     scale_ = 1.0f;
    Alignment alignment_ = Alignment::Center;

    RenderScale(float scale, Alignment align) : scale_(scale), alignment_(align) {}

    void setScale(float s, Alignment a) {
        if (std::abs(scale_ - s) > 0.0001f || alignment_ != a) {
            scale_ = s;
            alignment_ = a;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        if (std::abs(scale_) < 0.0001f) return;

        Point pivot = getAlignmentPivot(alignment_, size_) + ctx.offset;
        ctx.canvas.save();
        ctx.canvas.translate(pivot.x, pivot.y);
        ctx.canvas.scale(scale_, scale_);
        ctx.canvas.translate(-pivot.x, -pivot.y);

        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        ctx.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (std::abs(scale_) < 0.0001f) return false;
        Point pivot = getAlignmentPivot(alignment_, size_);
        Point transformed = {
            pivot.x + (localPoint.x - pivot.x) / scale_,
            pivot.y + (localPoint.y - pivot.y) / scale_
        };
        return RenderBox::hitTest(result, transformed);
    }
};

class ScaleRenderWidget : public SingleChildRenderObjectWidget {
public:
    float     scale = 1.0f;
    Alignment alignment = Alignment::Center;

    ScaleRenderWidget(float sc, Alignment align, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          scale(sc), alignment(align) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderScale>(scale, alignment);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderScale&>(ro);
        r.setScale(scale, alignment);
    }

    [[nodiscard]] std::string_view typeName() const override { return "ScaleRenderWidget"; }
};

class AnimatedScaleState : public State {
    std::unique_ptr<Ticker>               ticker_;
    float                                 begin_scale_ = 1.0f;
    float                                 target_scale_ = 1.0f;
    float                                 current_scale_ = 1.0f;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds             duration_{300};
    const Curve*                          curve_ = &Curves::linear;
    std::function<void()>                 on_end_ = nullptr;
    bool                                  is_animating_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedScaleWidget*>(widget());
        target_scale_ = w->scale;
        current_scale_ = w->scale;
        duration_ = w->duration;
        curve_ = w->curve;
        on_end_ = w->on_end;
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const AnimatedScaleWidget*>(&oldWidget);
        auto* new_w = static_cast<const AnimatedScaleWidget*>(widget());

        if (std::abs(old_w->scale - new_w->scale) > 0.0001f) {
            begin_scale_ = current_scale_;
            target_scale_ = new_w->scale;
            duration_ = new_w->duration;
            curve_ = new_w->curve;
            on_end_ = new_w->on_end;
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
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());
        double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        t = std::clamp(t, 0.0, 1.0);

        double curved_t = curve_ ? curve_->evaluate(t) : t;
        current_scale_ = static_cast<float>(begin_scale_ + (target_scale_ - begin_scale_) * curved_t);

        if (t >= 1.0) {
            current_scale_ = target_scale_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
            if (on_end_) on_end_();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedScaleWidget*>(widget());
        return std::make_shared<ScaleRenderWidget>(current_scale_, w->alignment, w->child);
    }
};

std::unique_ptr<State> AnimatedScaleWidget::createState() {
    return std::make_unique<AnimatedScaleState>();
}

// ════════════════════════════════════════════════════════════════
// 4. AnimatedRotation
// ════════════════════════════════════════════════════════════════

class RenderRotation : public RenderBox {
public:
    float     turns_ = 0.0f;
    Alignment alignment_ = Alignment::Center;

    RenderRotation(float turns, Alignment align) : turns_(turns), alignment_(align) {}

    void setRotation(float turns, Alignment align) {
        if (std::abs(turns_ - turns) > 0.0001f || alignment_ != align) {
            turns_ = turns;
            alignment_ = align;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        Point pivot = getAlignmentPivot(alignment_, size_) + ctx.offset;
        ctx.canvas.save();
        ctx.canvas.translate(pivot.x, pivot.y);
        ctx.canvas.rotate(turns_ * 360.0f);
        ctx.canvas.translate(-pivot.x, -pivot.y);

        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        ctx.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        Point pivot = getAlignmentPivot(alignment_, size_);
        float rad = -turns_ * 360.0f * (3.14159265358979323846f / 180.0f);
        float cos_a = std::cos(rad);
        float sin_a = std::sin(rad);
        float dx = localPoint.x - pivot.x;
        float dy = localPoint.y - pivot.y;
        Point transformed = {
            pivot.x + dx * cos_a - dy * sin_a,
            pivot.y + dx * sin_a + dy * cos_a
        };
        return RenderBox::hitTest(result, transformed);
    }
};

class RotationRenderWidget : public SingleChildRenderObjectWidget {
public:
    float     turns = 0.0f;
    Alignment alignment = Alignment::Center;

    RotationRenderWidget(float t, Alignment align, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          turns(t), alignment(align) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderRotation>(turns, alignment);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderRotation&>(ro);
        r.setRotation(turns, alignment);
    }

    [[nodiscard]] std::string_view typeName() const override { return "RotationRenderWidget"; }
};

class AnimatedRotationState : public State {
    std::unique_ptr<Ticker>               ticker_;
    float                                 begin_turns_ = 0.0f;
    float                                 target_turns_ = 0.0f;
    float                                 current_turns_ = 0.0f;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds             duration_{300};
    const Curve*                          curve_ = &Curves::linear;
    std::function<void()>                 on_end_ = nullptr;
    bool                                  is_animating_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedRotationWidget*>(widget());
        target_turns_ = w->turns;
        current_turns_ = w->turns;
        duration_ = w->duration;
        curve_ = w->curve;
        on_end_ = w->on_end;
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const AnimatedRotationWidget*>(&oldWidget);
        auto* new_w = static_cast<const AnimatedRotationWidget*>(widget());

        if (std::abs(old_w->turns - new_w->turns) > 0.0001f) {
            begin_turns_ = current_turns_;
            target_turns_ = new_w->turns;
            duration_ = new_w->duration;
            curve_ = new_w->curve;
            on_end_ = new_w->on_end;
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
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());
        double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        t = std::clamp(t, 0.0, 1.0);

        double curved_t = curve_ ? curve_->evaluate(t) : t;
        current_turns_ = static_cast<float>(begin_turns_ + (target_turns_ - begin_turns_) * curved_t);

        if (t >= 1.0) {
            current_turns_ = target_turns_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
            if (on_end_) on_end_();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedRotationWidget*>(widget());
        return std::make_shared<RotationRenderWidget>(current_turns_, w->alignment, w->child);
    }
};

std::unique_ptr<State> AnimatedRotationWidget::createState() {
    return std::make_unique<AnimatedRotationState>();
}

// ════════════════════════════════════════════════════════════════
// 5. AnimatedSlide
// ════════════════════════════════════════════════════════════════

class RenderSlide : public RenderBox {
public:
    Point offset_ = {0.0f, 0.0f};

    explicit RenderSlide(Point offset) : offset_(offset) {}

    void setSlideOffset(Point offset) {
        if (offset_ != offset) {
            offset_ = offset;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        Point translation = {offset_.x * size_.width, offset_.y * size_.height};
        ctx.canvas.save();
        ctx.canvas.translate(translation.x, translation.y);

        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        ctx.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (!paintBounds().contains(localPoint)) return false;
        Point translation = {offset_.x * size_.width, offset_.y * size_.height};
        Point transformed = {localPoint.x - translation.x, localPoint.y - translation.y};
        return RenderBox::hitTest(result, transformed);
    }
};

class SlideRenderWidget : public SingleChildRenderObjectWidget {
public:
    Point offset = {0.0f, 0.0f};

    SlideRenderWidget(Point off, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), offset(off) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderSlide>(offset);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderSlide&>(ro);
        r.setSlideOffset(offset);
    }

    [[nodiscard]] std::string_view typeName() const override { return "SlideRenderWidget"; }
};

class AnimatedSlideState : public State {
    std::unique_ptr<Ticker>               ticker_;
    Point                                 begin_offset_ = {0.0f, 0.0f};
    Point                                 target_offset_ = {0.0f, 0.0f};
    Point                                 current_offset_ = {0.0f, 0.0f};
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds             duration_{300};
    const Curve*                          curve_ = &Curves::linear;
    std::function<void()>                 on_end_ = nullptr;
    bool                                  is_animating_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedSlideWidget*>(widget());
        target_offset_ = w->offset;
        current_offset_ = w->offset;
        duration_ = w->duration;
        curve_ = w->curve;
        on_end_ = w->on_end;
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const AnimatedSlideWidget*>(&oldWidget);
        auto* new_w = static_cast<const AnimatedSlideWidget*>(widget());

        if (old_w->offset != new_w->offset) {
            begin_offset_ = current_offset_;
            target_offset_ = new_w->offset;
            duration_ = new_w->duration;
            curve_ = new_w->curve;
            on_end_ = new_w->on_end;
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
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());
        double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        t = std::clamp(t, 0.0, 1.0);

        double curved_t = curve_ ? curve_->evaluate(t) : t;
        current_offset_ = Point{
            static_cast<float>(begin_offset_.x + (target_offset_.x - begin_offset_.x) * curved_t),
            static_cast<float>(begin_offset_.y + (target_offset_.y - begin_offset_.y) * curved_t)
        };

        if (t >= 1.0) {
            current_offset_ = target_offset_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
            if (on_end_) on_end_();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedSlideWidget*>(widget());
        return std::make_shared<SlideRenderWidget>(current_offset_, w->child);
    }
};

std::unique_ptr<State> AnimatedSlideWidget::createState() {
    return std::make_unique<AnimatedSlideState>();
}

// ════════════════════════════════════════════════════════════════
// 6. AnimatedSwitcher
// ════════════════════════════════════════════════════════════════

struct SwitcherEntry {
    WidgetPtr                             child;
    float                                 progress = 0.0f;
    float                                 start_progress = 0.0f;
    float                                 end_progress = 1.0f;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds             duration;
    const Curve*                          curve = &Curves::linear;
    bool                                  is_incoming = true;
};

class AnimatedSwitcherState : public State {
    std::unique_ptr<Ticker>     ticker_;
    std::vector<SwitcherEntry>  entries_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AnimatedSwitcherWidget*>(widget());
        if (w->child) {
            SwitcherEntry current;
            current.child = w->child;
            current.progress = 1.0f;
            current.start_progress = 1.0f;
            current.end_progress = 1.0f;
            current.duration = w->duration;
            current.curve = w->switch_in_curve;
            current.is_incoming = true;
            entries_.push_back(std::move(current));
        }
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto* old_w = static_cast<const AnimatedSwitcherWidget*>(&oldWidget);
        auto* new_w = static_cast<const AnimatedSwitcherWidget*>(widget());

        bool child_changed = false;
        if (!old_w->child && new_w->child) {
            child_changed = true;
        } else if (old_w->child && !new_w->child) {
            child_changed = true;
        } else if (old_w->child && new_w->child) {
            child_changed = !new_w->child->canUpdate(*old_w->child);
        }

        if (child_changed && new_w->child) {
            auto now = std::chrono::steady_clock::now();

            // Mark existing entries as outgoing
            for (auto& entry : entries_) {
                if (entry.is_incoming) {
                    entry.is_incoming = false;
                    entry.start_progress = entry.progress;
                    entry.end_progress = 0.0f;
                    entry.start_time = now;
                    entry.duration = new_w->reverse_duration.value_or(new_w->duration);
                    entry.curve = new_w->switch_out_curve;
                }
            }

            // Create new incoming entry
            SwitcherEntry incoming;
            incoming.child = new_w->child;
            incoming.progress = 0.0f;
            incoming.start_progress = 0.0f;
            incoming.end_progress = 1.0f;
            incoming.start_time = now;
            incoming.duration = new_w->duration;
            incoming.curve = new_w->switch_in_curve;
            incoming.is_incoming = true;
            entries_.push_back(std::move(incoming));

            if (!ticker_) {
                ticker_ = createTicker([this]() { onTick(); });
            }
            ticker_->start();
        } else if (!child_changed && new_w->child) {
            // Update child reference without re-triggering transition
            for (auto& entry : entries_) {
                if (entry.is_incoming) {
                    entry.child = new_w->child;
                }
            }
        }
    }

    void onTick() {
        auto now = std::chrono::steady_clock::now();
        bool any_running = false;

        for (auto& entry : entries_) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - entry.start_time).count();
            double dur_ms = static_cast<double>(entry.duration.count());
            double t = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
            t = std::clamp(t, 0.0, 1.0);

            double curved_t = entry.curve ? entry.curve->evaluate(t) : t;
            entry.progress = static_cast<float>(entry.start_progress + (entry.end_progress - entry.start_progress) * curved_t);

            if (t < 1.0) {
                any_running = true;
            } else {
                entry.progress = entry.end_progress;
            }
        }

        // Remove fully animated out entries
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                [](const SwitcherEntry& e) {
                    return !e.is_incoming && e.progress <= 0.001f;
                }),
            entries_.end());

        if (!any_running) {
            if (ticker_) ticker_->stop();
        }

        setState([] {});
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AnimatedSwitcherWidget*>(widget());

        std::vector<WidgetPtr> rendered_children;
        WidgetPtr current_rendered = nullptr;
        std::vector<WidgetPtr> previous_rendered;

        for (const auto& entry : entries_) {
            WidgetPtr transitioned = nullptr;
            if (w->transition_builder) {
                transitioned = w->transition_builder(entry.child, entry.progress);
            } else {
                // Default transition: smooth fade
                transitioned = std::make_shared<OpacityRenderWidget>(entry.progress, entry.child);
            }

            if (entry.is_incoming) {
                current_rendered = transitioned;
            } else {
                previous_rendered.push_back(transitioned);
            }
            rendered_children.push_back(transitioned);
        }

        if (w->layout_builder) {
            return w->layout_builder(current_rendered, previous_rendered);
        }

        // Default layout: Stack of all children centered
        return stack(Alignment::Center, rendered_children);
    }
};

std::unique_ptr<State> AnimatedSwitcherWidget::createState() {
    return std::make_unique<AnimatedSwitcherState>();
}

// ════════════════════════════════════════════════════════════════
// 7. SlideTransition
// ════════════════════════════════════════════════════════════════

class RenderSlideTransition : public RenderBox {
public:
    std::shared_ptr<AnimationController> position_;
    Point                                begin_;
    Point                                end_;
    const Curve*                         curve_;
    bool                                 transform_hit_tests_;

    RenderSlideTransition(std::shared_ptr<AnimationController> pos, Point begin, Point end, const Curve* c, bool hit_test)
        : position_(std::move(pos)), begin_(begin), end_(end), curve_(c), transform_hit_tests_(hit_test) {
        attachListener();
    }

    void attachListener() {
        if (position_) {
            position_->addListener([this]() {
                markNeedsPaint();
            });
        }
    }

    void update(std::shared_ptr<AnimationController> pos, Point begin, Point end, const Curve* c, bool hit_test) {
        if (position_ != pos) {
            position_ = std::move(pos);
            attachListener();
        }
        begin_ = begin;
        end_ = end;
        curve_ = c;
        transform_hit_tests_ = hit_test;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        float t = position_ ? position_->value() : 1.0f;
        if (curve_) t = curve_->evaluateF(t);
        Point current_offset = {
            begin_.x + (end_.x - begin_.x) * t,
            begin_.y + (end_.y - begin_.y) * t
        };
        Point translation = {current_offset.x * size_.width, current_offset.y * size_.height};

        ctx.canvas.save();
        ctx.canvas.translate(translation.x, translation.y);

        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }

        ctx.canvas.restore();
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (!transform_hit_tests_) return RenderBox::hitTest(result, localPoint);
        if (!paintBounds().contains(localPoint)) return false;
        float t = position_ ? position_->value() : 1.0f;
        if (curve_) t = curve_->evaluateF(t);
        Point current_offset = {
            begin_.x + (end_.x - begin_.x) * t,
            begin_.y + (end_.y - begin_.y) * t
        };
        Point translation = {current_offset.x * size_.width, current_offset.y * size_.height};
        Point transformed = {localPoint.x - translation.x, localPoint.y - translation.y};
        return RenderBox::hitTest(result, transformed);
    }
};

std::unique_ptr<RenderObject> SlideTransitionWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderSlideTransition>(position, begin, end, curve, transform_hit_tests);
}

void SlideTransitionWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderSlideTransition&>(renderObject);
    r.update(position, begin, end, curve, transform_hit_tests);
}

} // namespace enki
