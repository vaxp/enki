#include "enki/widgets/navigator.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/element.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Transition RenderObject — Slide/Fade/Scale overlay
// ════════════════════════════════════════════════════════════════

class RenderTransitionOverlay : public RenderBox {
public:
    float          progress;   // 0=incoming not visible, 1=fully visible
    RouteTransition type;
    bool           is_entering; // true=push (entering from right), false=pop (exiting to right)

    RenderTransitionOverlay(float p, RouteTransition t, bool entering)
        : progress(p), type(t), is_entering(entering) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext& ctx) override {
        if (children().empty()) return;
        float w = size_.width;
        float h = size_.height;
        if (w <= 0.0f || h <= 0.0f) return;

        auto* child = static_cast<RenderBox*>(children()[0]);
        PaintContext child_ctx = ctx;

        switch (type) {
            case RouteTransition::None: {
                child->paint(child_ctx);
                break;
            }
            case RouteTransition::Slide: {
                // Entering: comes from right (progress 0→1 means x goes from +w to 0)
                // Exiting:  goes to right (progress 1→0 means x goes from 0 to +w)
                float slide_x = w * (1.0f - progress);
                ctx.canvas.save();
                ctx.canvas.translate(slide_x, 0.0f);
                child->paint(child_ctx);
                ctx.canvas.restore();
                break;
            }
            case RouteTransition::Fade: {
                ctx.canvas.saveLayerAlpha(progress);
                child->paint(child_ctx);
                ctx.canvas.restore();
                break;
            }
            case RouteTransition::Scale: {
                float scale = 0.85f + 0.15f * progress;
                float cx = ctx.offset.x + w * 0.5f;
                float cy = ctx.offset.y + h * 0.5f;
                ctx.canvas.saveLayerAlpha(progress);
                ctx.canvas.save();
                ctx.canvas.translate(cx, cy);
                ctx.canvas.scale(scale, scale);
                ctx.canvas.translate(-cx, -cy);
                child->paint(child_ctx);
                ctx.canvas.restore();
                ctx.canvas.restore();
                break;
            }
        }
    }

    bool hitTestSelf(Point p) const override {
        return p.x >= 0 && p.x <= size_.width && p.y >= 0 && p.y <= size_.height;
    }
};

class TransitionOverlayWidget : public SingleChildRenderObjectWidget {
public:
    float          progress;
    RouteTransition type;
    bool           is_entering;

    TransitionOverlayWidget(float p, RouteTransition t, bool entering, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          progress(p), type(t), is_entering(entering) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderTransitionOverlay>(progress, type, is_entering);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderTransitionOverlay&>(ro);
        r.progress    = progress;
        r.type        = type;
        r.is_entering = is_entering;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "TransitionOverlayWidget"; }
};

// ════════════════════════════════════════════════════════════════
// NavigatorState Implementation
// ════════════════════════════════════════════════════════════════

void NavigatorState::initState() {
    State::initState();
    auto* w = static_cast<const Navigator*>(widget());

    for (const auto& rc : w->initial_routes) {
        ActiveRoute ar;
        ar.config       = rc;
        ar.widget_cache = rc.builder ? rc.builder() : nullptr;
        ar.anim_value   = 1.0f;
        ar.entering     = false;
        ar.exiting      = false;
        stack_.push_back(std::move(ar));
    }

    // If no initial routes, push a blank one
    if (stack_.empty()) {
        ActiveRoute ar;
        ar.config.name     = "root";
        ar.widget_cache    = container();
        ar.anim_value      = 1.0f;
        stack_.push_back(std::move(ar));
    }

    ticker_ = createTicker([this] { tickAnimation(); });
}

void NavigatorState::dispose() {
    if (ticker_) ticker_->stop();
    State::dispose();
}

void NavigatorState::push(RouteConfig route) {
    auto* w = static_cast<const Navigator*>(widget());
    int dur = w->options.transition_duration_ms;

    // Mark any entering/exiting as done
    for (auto& r : stack_) {
        r.entering = false;
        r.exiting  = false;
        r.anim_value = 1.0f;
    }

    ActiveRoute ar;
    ar.config       = std::move(route);
    ar.widget_cache = ar.config.builder ? ar.config.builder() : nullptr;
    ar.anim_value   = 0.0f;
    ar.entering     = true;
    ar.exiting      = false;
    stack_.push_back(std::move(ar));

    animating_ = true;
    if (!ticker_->isActive()) ticker_->start();

    setState([] {});
}

void NavigatorState::pop() {
    if (stack_.size() <= 1) return;

    // Mark top as exiting
    stack_.back().exiting  = true;
    stack_.back().entering = false;

    animating_ = true;
    if (!ticker_->isActive()) ticker_->start();

    setState([] {});
}

bool NavigatorState::canPop() const {
    return stack_.size() > 1;
}

void NavigatorState::tickAnimation() {
    if (!animating_) return;

    auto* w = static_cast<const Navigator*>(widget());
    float dt = 1.0f / 60.0f; // approximate, good enough
    int dur_ms = w->options.transition_duration_ms;
    float step = dur_ms > 0 ? (dt * 1000.0f / dur_ms) : 1.0f;

    bool still_animating = false;

    for (auto& r : stack_) {
        if (r.entering && r.anim_value < 1.0f) {
            r.anim_value = std::min(r.anim_value + step, 1.0f);
            if (r.anim_value >= 1.0f) r.entering = false;
            still_animating = true;
        }
        if (r.exiting && r.anim_value > 0.0f) {
            r.anim_value = std::max(r.anim_value - step, 0.0f);
            still_animating = true;
        }
    }

    // Remove fully-exited routes
    stack_.erase(
        std::remove_if(stack_.begin(), stack_.end(),
            [](const ActiveRoute& r) { return r.exiting && r.anim_value <= 0.001f; }),
        stack_.end());

    if (!still_animating) {
        animating_ = false;
        ticker_->stop();
    }

    setState([] {});
}

WidgetPtr NavigatorState::build(BuildContext& ctx) {
    auto* w = static_cast<const Navigator*>(widget());
    Color bg = w->options.background_color;

    if (stack_.empty()) {
        auto bx = container();
        bx->color(bg).flex(1.0f);
        return bx;
    }

    std::vector<WidgetPtr> layers;

    for (auto& ar : stack_) {
        WidgetPtr page_widget;
        if (ar.widget_cache) {
            // Wrap in fill container
            auto page_box = container(ar.widget_cache);
            page_box->flex(1.0f)
                     .width(StyleValue::percent(100.0f))
                     .height(StyleValue::percent(100.0f));
            page_widget = page_box;
        } else {
            auto empty = container();
            empty->flex(1.0f)
                  .width(StyleValue::percent(100.0f))
                  .height(StyleValue::percent(100.0f));
            page_widget = empty;
        }

        bool needs_anim = ar.entering || ar.exiting || ar.anim_value < 0.999f;

        if (needs_anim && ar.config.transition != RouteTransition::None) {
            auto overlay = std::make_shared<TransitionOverlayWidget>(
                ar.anim_value, ar.config.transition, ar.entering, page_widget);
            layers.push_back(overlay);
        } else {
            // Positioned to fill parent
            auto pos = Positioned::fill(page_widget);
            layers.push_back(pos);
        }
    }

    auto root_stack = std::make_shared<Stack>(std::move(layers));
    root_stack->style.width  = StyleValue::percent(100.0f);
    root_stack->style.height = StyleValue::percent(100.0f);
    root_stack->style.clip_behavior = Clip::HardEdge;

    auto root_box = container(root_stack);
    root_box->color(bg)
             .flex(1.0f)
             .width(StyleValue::percent(100.0f))
             .height(StyleValue::percent(100.0f));
    return root_box;
}

std::unique_ptr<State> Navigator::createState() {
    return std::make_unique<NavigatorState>();
}

// ════════════════════════════════════════════════════════════════
// Static Navigator helpers
// ════════════════════════════════════════════════════════════════

void Navigator::push(BuildContext& ctx, RouteConfig route) {
    // Walk up the element tree to find the nearest NavigatorState
    Element* el = ctx.element();
    while (el) {
        if (auto* se = dynamic_cast<StatefulElement*>(el)) {
            if (auto* ns = dynamic_cast<NavigatorState*>(se->state())) {
                ns->push(std::move(route));
                return;
            }
        }
        el = el->parent();
    }
}

void Navigator::pop(BuildContext& ctx) {
    Element* el = ctx.element();
    while (el) {
        if (auto* se = dynamic_cast<StatefulElement*>(el)) {
            if (auto* ns = dynamic_cast<NavigatorState*>(se->state())) {
                ns->pop();
                return;
            }
        }
        el = el->parent();
    }
}

bool Navigator::canPop(BuildContext& ctx) {
    Element* el = ctx.element();
    while (el) {
        if (auto* se = dynamic_cast<StatefulElement*>(el)) {
            if (auto* ns = dynamic_cast<NavigatorState*>(se->state())) {
                return ns->canPop();
            }
        }
        el = el->parent();
    }
    return false;
}

} // namespace enki
