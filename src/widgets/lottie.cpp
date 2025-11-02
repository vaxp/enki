/// @file lottie.cpp
/// @brief Implementation of Lottie widget, RenderLottie, and LottieState.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/lottie.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace enki {

namespace {

float alignRatioX(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::CenterLeft:
        case Alignment::BottomLeft:
            return 0.0f;
        case Alignment::TopCenter:
        case Alignment::Center:
        case Alignment::BottomCenter:
            return 0.5f;
        case Alignment::TopRight:
        case Alignment::CenterRight:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

float alignRatioY(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::TopCenter:
        case Alignment::TopRight:
            return 0.0f;
        case Alignment::CenterLeft:
        case Alignment::Center:
        case Alignment::CenterRight:
            return 0.5f;
        case Alignment::BottomLeft:
        case Alignment::BottomCenter:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

void calculateLottieBoxFit(BoxFit fit, Alignment align,
                           Size src_size, Size dst_size,
                           Rect& out_src, Rect& out_dst) {
    const float sw = src_size.width;
    const float sh = src_size.height;
    const float dw = dst_size.width;
    const float dh = dst_size.height;

    if (sw <= 0.0f || sh <= 0.0f || dw <= 0.0f || dh <= 0.0f) {
        out_src = Rect(0, 0, 0, 0);
        out_dst = Rect(0, 0, 0, 0);
        return;
    }

    const float ax = alignRatioX(align);
    const float ay = alignRatioY(align);

    if (fit == BoxFit::ScaleDown) {
        if (sw > dw || sh > dh) {
            fit = BoxFit::Contain;
        } else {
            fit = BoxFit::None;
        }
    }

    switch (fit) {
        case BoxFit::Fill: {
            out_src = Rect(0.0f, 0.0f, sw, sh);
            out_dst = Rect(0.0f, 0.0f, dw, dh);
            break;
        }
        case BoxFit::Contain: {
            const float scale = std::min(dw / sw, dh / sh);
            const float rw = sw * scale;
            const float rh = sh * scale;
            const float dx = (dw - rw) * ax;
            const float dy = (dh - rh) * ay;

            out_src = Rect(0.0f, 0.0f, sw, sh);
            out_dst = Rect(dx, dy, rw, rh);
            break;
        }
        case BoxFit::Cover: {
            const float scale = std::max(dw / sw, dh / sh);
            const float vsw = dw / scale;
            const float vsh = dh / scale;
            const float sx = (sw - vsw) * ax;
            const float sy = (sh - vsh) * ay;

            out_src = Rect(sx, sy, vsw, vsh);
            out_dst = Rect(0.0f, 0.0f, dw, dh);
            break;
        }
        case BoxFit::FitWidth: {
            const float scale = dw / sw;
            const float vsh = dh / scale;
            const float sy = std::max(0.0f, (sh - vsh) * ay);
            const float actual_src_h = std::min(sh, vsh);
            const float actual_dst_h = std::min(dh, actual_src_h * scale);
            const float dy = (dh - actual_dst_h) * ay;

            out_src = Rect(0.0f, sy, sw, actual_src_h);
            out_dst = Rect(0.0f, dy, dw, actual_dst_h);
            break;
        }
        case BoxFit::FitHeight: {
            const float scale = dh / sh;
            const float vsw = dw / scale;
            const float sx = std::max(0.0f, (sw - vsw) * ax);
            const float actual_src_w = std::min(sw, vsw);
            const float actual_dst_w = std::min(dw, actual_src_w * scale);
            const float dx = (dw - actual_dst_w) * ax;

            out_src = Rect(sx, 0.0f, actual_src_w, sh);
            out_dst = Rect(dx, 0.0f, actual_dst_w, dh);
            break;
        }
        case BoxFit::None: {
            const float vsw = std::min(sw, dw);
            const float vsh = std::min(sh, dh);
            const float sx = (sw - vsw) * ax;
            const float sy = (sh - vsh) * ay;
            const float dx = (dw - vsw) * ax;
            const float dy = (dh - vsh) * ay;

            out_src = Rect(sx, sy, vsw, vsh);
            out_dst = Rect(dx, dy, vsw, vsh);
            break;
        }
        case BoxFit::ScaleDown:
            break;
    }
}

} // namespace

// ════════════════════════════════════════════════════════════════
// RenderLottie
// ════════════════════════════════════════════════════════════════

class RenderLottie : public RenderBox {
public:
    std::shared_ptr<LottieComposition> composition_;
    LottieStyle                        style_;

    RenderLottie(std::shared_ptr<LottieComposition> comp, LottieStyle style)
        : composition_(std::move(comp)), style_(std::move(style)) {
        anu_node_ = ANUNodeNew();
        ANUNodeSetContext(anu_node_, this);
        ANUNodeSetMeasureFunc(anu_node_, &RenderLottie::measureCallback);
        applyStyleToNode();
    }

    ~RenderLottie() override {
        if (anu_node_) {
            ANUNodeSetContext(anu_node_, nullptr);
            ANUNodeFree(anu_node_);
            anu_node_ = nullptr;
        }
    }

    void setComposition(std::shared_ptr<LottieComposition> comp) {
        composition_ = std::move(comp);
        markNeedsLayout();
        markNeedsPaint();
    }

    void setStyle(const LottieStyle& s) {
        style_ = s;
        applyStyleToNode();
        markNeedsLayout();
        markNeedsPaint();
    }

    void applyStyleToNode() {
        if (!anu_node_) return;

        if (style_.width.has_value()) {
            if (style_.width->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, style_.width->value);
            else if (style_.width->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, style_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }

        if (style_.height.has_value()) {
            if (style_.height->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, style_.height->value);
            else if (style_.height->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, style_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }

        if (style_.min_width.has_value()) {
            if (style_.min_width->isPercent()) ANUNodeStyleSetMinWidthPercent(anu_node_, style_.min_width->value);
            else ANUNodeStyleSetMinWidth(anu_node_, style_.min_width->value);
        }
        if (style_.min_height.has_value()) {
            if (style_.min_height->isPercent()) ANUNodeStyleSetMinHeightPercent(anu_node_, style_.min_height->value);
            else ANUNodeStyleSetMinHeight(anu_node_, style_.min_height->value);
        }
        if (style_.max_width.has_value()) {
            if (style_.max_width->isPercent()) ANUNodeStyleSetMaxWidthPercent(anu_node_, style_.max_width->value);
            else ANUNodeStyleSetMaxWidth(anu_node_, style_.max_width->value);
        }
        if (style_.max_height.has_value()) {
            if (style_.max_height->isPercent()) ANUNodeStyleSetMaxHeightPercent(anu_node_, style_.max_height->value);
            else ANUNodeStyleSetMaxHeight(anu_node_, style_.max_height->value);
        }
    }

    static ANUSize measureCallback(ANUNodeConstRef node, float width, ANUMeasureMode widthMode,
                                   float height, ANUMeasureMode heightMode) {
        auto* self = static_cast<RenderLottie*>(ANUNodeGetContext(node));
        if (!self || !self->composition_) {
            return ANUSize{0.0f, 0.0f};
        }

        const float comp_w = self->composition_->getWidth();
        const float comp_h = self->composition_->getHeight();

        if (comp_w <= 0.0f || comp_h <= 0.0f) {
            return ANUSize{0.0f, 0.0f};
        }

        const float aspect = comp_w / comp_h;
        float result_w = comp_w;
        float result_h = comp_h;

        if (widthMode == ANUMeasureModeExactly) {
            result_w = width;
            if (heightMode == ANUMeasureModeExactly) {
                result_h = height;
            } else {
                result_h = width / aspect;
            }
        } else if (heightMode == ANUMeasureModeExactly) {
            result_h = height;
            result_w = height * aspect;
        } else {
            if (widthMode == ANUMeasureModeAtMost && result_w > width) {
                result_w = width;
                result_h = width / aspect;
            }
            if (heightMode == ANUMeasureModeAtMost && result_h > height) {
                result_h = height;
                result_w = height * aspect;
            }
        }

        return ANUSize{result_w, result_h};
    }

    void paint(PaintContext& context) override {
        if (!composition_) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        Rect bounds = Rect::fromPointSize(context.offset, size_);
        BorderRadius radius = (style_.shape == BoxShape::Circle)
            ? BorderRadius::circular(std::min(size_.width, size_.height) * 0.5f)
            : style_.border_radius;

        const bool need_clip = style_.clip_content &&
                              (style_.shape == BoxShape::Circle || radius != BorderRadius::zero());

        if (need_clip) {
            context.canvas.save();
            context.canvas.clipRRect(bounds, radius);
        }

        Rect src_rect, dst_local;
        calculateLottieBoxFit(style_.fit, style_.alignment,
                              composition_->getSize(), size_,
                              src_rect, dst_local);

        Rect dst_world(
            bounds.x + dst_local.x,
            bounds.y + dst_local.y,
            dst_local.width,
            dst_local.height
        );

        if (style_.opacity < 1.0f || context.opacity < 1.0f) {
            float combined_alpha = std::clamp(style_.opacity * context.opacity, 0.0f, 1.0f);
            context.canvas.saveLayerAlpha(combined_alpha, &dst_world);
            composition_->render(context.canvas, dst_world);
            context.canvas.restore();
        } else {
            composition_->render(context.canvas, dst_world);
        }

        if (need_clip) {
            context.canvas.restore();
        }
    }
};

// ════════════════════════════════════════════════════════════════
// _RawLottieRenderWidget
// ════════════════════════════════════════════════════════════════

class _RawLottieRenderWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<LottieComposition> composition;
    LottieStyle                        style;

    _RawLottieRenderWidget(std::shared_ptr<LottieComposition> comp, LottieStyle style, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key), nullptr), composition(std::move(comp)), style(std::move(style)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderLottie>(composition, style);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* r = dynamic_cast<RenderLottie*>(&renderObject)) {
            r->setComposition(composition);
            r->setStyle(style);
        }
    }

    std::string_view typeName() const override { return "_RawLottieRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// LottieState
// ════════════════════════════════════════════════════════════════

class LottieState : public State {
public:
    std::shared_ptr<LottieComposition> composition_;
    std::shared_ptr<LottieController>  controller_;
    bool                               owns_controller_ = false;
    size_t                             listener_id_ = 0;

    void initState() override {
        State::initState();
        loadComposition();
        setupController();
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        loadComposition();
        setupController();
    }

    void loadComposition() {
        const auto* w = dynamic_cast<const LottieWidget*>(widget());
        if (!w) return;

        if (w->style.composition) {
            composition_ = w->style.composition;
        } else if (!w->style.asset_path.empty()) {
            composition_ = LottieCache::getOrLoad(w->style.asset_path);
        } else if (!w->style.json_data.empty()) {
            auto res = LottieComposition::loadFromJson(w->style.json_data);
            if (res.isOk()) {
                composition_ = res.value();
            }
        }
    }

    void detachController() {
        if (controller_ && listener_id_ != 0) {
            controller_->removeListener(listener_id_);
            listener_id_ = 0;
        }
        if (owns_controller_ && controller_) {
            controller_->dispose();
        }
        controller_ = nullptr;
        owns_controller_ = false;
    }

    void setupController() {
        const auto* w = dynamic_cast<const LottieWidget*>(widget());
        if (!w) return;

        if (w->style.controller) {
            if (controller_ != w->style.controller) {
                detachController();
                controller_ = w->style.controller;
                owns_controller_ = false;
                controller_->setComposition(composition_);
                listener_id_ = controller_->addListener([this]() {
                    if (mounted()) {
                        setState([] {});
                    }
                });
            }
        } else {
            if (!controller_ || !owns_controller_) {
                detachController();
                controller_ = std::make_shared<LottieController>(composition_);
                owns_controller_ = true;
                listener_id_ = controller_->addListener([this]() {
                    if (mounted()) {
                        setState([] {});
                    }
                });
            } else {
                controller_->setComposition(composition_);
            }
        }

        if (controller_) {
            controller_->setRepeat(w->style.repeat);
            controller_->setSpeed(w->style.speed);

            if (w->style.on_end) controller_->onEnd(w->style.on_end);
            if (w->style.on_loop) controller_->onLoop(w->style.on_loop);

            if (!w->style.marker.empty()) {
                controller_->playMarker(w->style.marker, w->style.repeat);
            } else if (w->style.auto_play && !controller_->isPlaying()) {
                controller_->play();
            }
        }
    }

    void dispose() override {
        detachController();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        const auto* w = dynamic_cast<const LottieWidget*>(widget());
        if (!w) return nullptr;

        WidgetPtr raw = std::make_shared<_RawLottieRenderWidget>(composition_, w->style, w->key);

        if (w->style.animate_on_hover || w->style.animate_on_tap) {
            GestureDetectorProps gd_props;
            gd_props.child = raw;

            if (w->style.animate_on_hover) {
                gd_props.on_hover_enter = [this](const PointerEvent&) {
                    if (mounted() && controller_) controller_->forward();
                };
                gd_props.on_hover_exit = [this](const PointerEvent&) {
                    if (mounted() && controller_) controller_->reset();
                };
            }

            if (w->style.animate_on_tap) {
                gd_props.on_tap = [this]() {
                    if (mounted() && controller_) {
                        controller_->reset();
                        controller_->forward();
                    }
                };
            }

            return gestureDetector(gd_props);
        }

        return raw;
    }
};

std::unique_ptr<State> LottieWidget::createState() {
    return std::make_unique<LottieState>();
}

LottieProps::operator WidgetPtr() const {
    LottieStyle s;
    s.composition = composition;
    s.asset_path = asset;
    s.json_data = json_data;
    s.controller = controller;

    s.width = width;
    s.height = height;
    s.min_width = min_width;
    s.min_height = min_height;
    s.max_width = max_width;
    s.max_height = max_height;

    s.fit = fit;
    s.alignment = alignment;
    s.border_radius = border_radius;
    s.shape = shape;

    s.opacity = opacity;
    s.clip_content = clip_content;
    s.auto_play = auto_play;
    s.repeat = repeat;
    s.speed = speed;
    s.marker = marker;

    s.animate_on_hover = animate_on_hover;
    s.animate_on_tap = animate_on_tap;

    s.on_end = std::move(on_end);
    s.on_loop = std::move(on_loop);

    return std::make_shared<LottieWidget>(std::move(s), key);
}

} // namespace enki
