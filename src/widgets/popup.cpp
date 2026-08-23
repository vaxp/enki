/// @file popup.cpp
/// @brief Universal Native Popup implementation built on NativePopup.

#include "enki/widgets/popup.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/effects/SkRuntimeEffect.h>

#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Popup Surface Frame
// ════════════════════════════════════════════════════════════════

class RenderPopupBackground : public RenderBox {
public:
    PopupWidgetOptions options;

    explicit RenderPopupBackground(PopupWidgetOptions opt)
        : options(std::move(opt)) {}

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        SkRect rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(rect, options.border_radius, options.border_radius);

        // 1. Draw Drop Shadow
        if (options.elevation > 0.0f) {
            SkPaint shadow_paint;
            shadow_paint.setAntiAlias(true);
            shadow_paint.setColor(options.shadow_color);
            shadow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.elevation * 0.5f));

            canvas->save();
            canvas->translate(0, options.elevation * 0.3f);
            canvas->drawRRect(rrect, shadow_paint);
            canvas->restore();
        }

        // 2. Draw Main Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);

        if (!options.custom_shader.empty()) {
            auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (effect) {
                struct Uniforms {
                    float time;
                    float resolution[2];
                } uniforms = { 0.0f, {size_.width, size_.height} };
                sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
                bg_paint.setShader(effect->makeShader(uniform_data, nullptr, 0));
            } else {
                bg_paint.setColor(options.background_color);
            }
        } else {
            bg_paint.setColor(options.background_color);
        }

        canvas->drawRRect(rrect, bg_paint);

        // 3. Draw Outer Border Stroke
        if (options.border_width > 0.0f) {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(options.border_width);
            border_paint.setColor(options.border_color);

            canvas->drawRRect(rrect, border_paint);
        }

        // 4. Paint Child Content
        if (!children().empty()) {
            RenderBox* child = static_cast<RenderBox*>(children()[0]);
            PaintContext child_ctx = ctx.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }
};

class PopupBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    PopupWidgetOptions options;

    PopupBackgroundWidget(PopupWidgetOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderPopupBackground>(options);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderPopupBackground*>(&renderObject)) {
            rb->options = options;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "PopupBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Popup State & Placement Calculations
// ════════════════════════════════════════════════════════════════

static Point calculatePopupPosition(
    PopupPlacement placement,
    const Rect& target_bounds,
    int32_t pop_w,
    int32_t pop_h,
    const Point& offset,
    const Size& screen_sz,
    const Point& cursor_pos = {0, 0}) {

    float pop_x = 0.0f;
    float pop_y = 0.0f;

    switch (placement) {
        case PopupPlacement::TopLeft:
            pop_x = target_bounds.x;
            pop_y = target_bounds.y - pop_h;
            break;
        case PopupPlacement::TopCenter:
            pop_x = target_bounds.x + (target_bounds.width - pop_w) / 2.0f;
            pop_y = target_bounds.y - pop_h;
            break;
        case PopupPlacement::TopRight:
            pop_x = target_bounds.x + target_bounds.width - pop_w;
            pop_y = target_bounds.y - pop_h;
            break;

        case PopupPlacement::BottomLeft:
            pop_x = target_bounds.x;
            pop_y = target_bounds.y + target_bounds.height;
            break;
        case PopupPlacement::BottomCenter:
            pop_x = target_bounds.x + (target_bounds.width - pop_w) / 2.0f;
            pop_y = target_bounds.y + target_bounds.height;
            break;
        case PopupPlacement::BottomRight:
            pop_x = target_bounds.x + target_bounds.width - pop_w;
            pop_y = target_bounds.y + target_bounds.height;
            break;

        case PopupPlacement::LeftTop:
            pop_x = target_bounds.x - pop_w;
            pop_y = target_bounds.y;
            break;
        case PopupPlacement::LeftCenter:
            pop_x = target_bounds.x - pop_w;
            pop_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
            break;
        case PopupPlacement::LeftBottom:
            pop_x = target_bounds.x - pop_w;
            pop_y = target_bounds.y + target_bounds.height - pop_h;
            break;

        case PopupPlacement::RightTop:
            pop_x = target_bounds.x + target_bounds.width;
            pop_y = target_bounds.y;
            break;
        case PopupPlacement::RightCenter:
            pop_x = target_bounds.x + target_bounds.width;
            pop_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
            break;
        case PopupPlacement::RightBottom:
            pop_x = target_bounds.x + target_bounds.width;
            pop_y = target_bounds.y + target_bounds.height - pop_h;
            break;

        case PopupPlacement::FollowCursor:
            pop_x = cursor_pos.x + 10.0f;
            pop_y = cursor_pos.y + 10.0f;
            break;

        case PopupPlacement::CenterScreen:
            pop_x = (screen_sz.width - pop_w) / 2.0f;
            pop_y = (screen_sz.height - pop_h) / 2.0f;
            break;

        case PopupPlacement::Manual:
            // Handled separately
            break;
    }

    pop_x += offset.x;
    pop_y += offset.y;

    // Boundary constraints fitting
    if (pop_x + pop_w > screen_sz.width - 10.0f) {
        pop_x = screen_sz.width - pop_w - 10.0f;
    }
    if (pop_y + pop_h > screen_sz.height - 10.0f) {
        pop_y = screen_sz.height - pop_h - 10.0f;
    }
    if (pop_x < 5.0f) pop_x = 5.0f;
    if (pop_y < 5.0f) pop_y = 5.0f;

    return {pop_x, pop_y};
}

std::shared_ptr<NativePopup> PopupWidget::show(
    BuildContext& context,
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
    PopupWidgetOptions options) {

    Element* elem = context.element();
    if (!elem) return nullptr;

    Size screen_sz = context.mediaSize();

    int32_t pop_w = static_cast<int32_t>(options.content_size.width + options.padding.horizontal());
    int32_t pop_h = static_cast<int32_t>(options.content_size.height + options.padding.vertical());

    Point pos;
    if (options.placement == PopupPlacement::Manual) {
        pos = options.manual_position;
    } else {
        RenderObject* ro = elem->findRenderObject();
        Rect target_bounds = ro ? ro->globalBounds() : Rect{0, 0, 0, 0};
        pos = calculatePopupPosition(options.placement, target_bounds, pop_w, pop_h, options.offset, screen_sz);
    }

    PopupOptions pop_opts;
    pop_opts.position = pos;
    pop_opts.width = pop_w;
    pop_opts.height = pop_h;
    pop_opts.auto_dismiss = options.auto_dismiss;

    return NativePopup::show(context, pop_opts, [options, popup_builder](BuildContext& sub_ctx, std::shared_ptr<NativePopup> popup) {
        WidgetPtr inner_content = popup_builder ? popup_builder(sub_ctx, popup) : nullptr;

        auto inner_container = container(inner_content);
        inner_container->padding(options.padding);

        return std::make_shared<PopupBackgroundWidget>(options, inner_container);
    });
}

class PopupState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;
    Point last_cursor_pos_ = {0.0f, 0.0f};

public:
    void initState() override {
        State::initState();
        auto* pop_widget = static_cast<const PopupWidget*>(widget());
        if (pop_widget && pop_widget->controller) {
            pop_widget->controller->setToggleCallback([this](bool show) {
                if (show) showPopupNow();
                else hidePopupNow();
            });
        }
    }

    void dispose() override {
        hidePopupNow();
        State::dispose();
    }

    void showPopupNow() {
        if (active_popup_) return;

        auto* pop_widget = static_cast<const PopupWidget*>(widget());
        if (!pop_widget || !pop_widget->popup_builder) return;

        Element* elem = element();
        if (!elem) return;

        BuildContext ctx(elem);
        Size screen_sz = ctx.mediaSize();
        const auto& opt = pop_widget->options;

        int32_t pop_w = static_cast<int32_t>(opt.content_size.width + opt.padding.horizontal());
        int32_t pop_h = static_cast<int32_t>(opt.content_size.height + opt.padding.vertical());

        Point pos;
        if (opt.placement == PopupPlacement::Manual) {
            pos = opt.manual_position;
        } else {
            RenderObject* ro = elem->findRenderObject();
            Rect target_bounds = ro ? ro->globalBounds() : Rect{0, 0, 0, 0};
            pos = calculatePopupPosition(opt.placement, target_bounds, pop_w, pop_h, opt.offset, screen_sz, last_cursor_pos_);
        }

        PopupOptions pop_opts;
        pop_opts.position = pos;
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = opt.auto_dismiss;
        pop_opts.on_close = [this, pop_widget]() {
            active_popup_ = nullptr;
            if (pop_widget && pop_widget->controller) {
                pop_widget->controller->hide();
            }
        };

        active_popup_ = NativePopup::show(ctx, pop_opts, [pop_widget](BuildContext& sub_ctx, std::shared_ptr<NativePopup> popup) {
            WidgetPtr inner_content = pop_widget->popup_builder ? pop_widget->popup_builder(sub_ctx, popup) : nullptr;

            auto inner_container = container(inner_content);
            inner_container->padding(pop_widget->options.padding);

            return std::make_shared<PopupBackgroundWidget>(pop_widget->options, inner_container);
        });
    }

    void hidePopupNow() {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
    }

    void togglePopup() {
        if (active_popup_) hidePopupNow();
        else showPopupNow();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* pop_widget = static_cast<const PopupWidget*>(widget());

        GestureDetectorProps props;
        props.child = pop_widget->child;
        props.hit_test_behavior = HitTestBehavior::Translucent;

        if (pop_widget->options.trigger == PopupTrigger::Click) {
            props.on_tap_up = [this](const TapUpDetails&) {
                togglePopup();
            };
        } else if (pop_widget->options.trigger == PopupTrigger::Hover) {
            props.on_hover_enter = [this](const PointerEvent& e) {
                last_cursor_pos_ = {e.position.x, e.position.y};
                showPopupNow();
            };
            props.on_hover_exit = [this](const PointerEvent& e) {
                hidePopupNow();
            };
        } else if (pop_widget->options.trigger == PopupTrigger::SecondaryClick) {
            props.on_secondary_tap_up = [this](const TapUpDetails& details) {
                last_cursor_pos_ = {details.global_position.x, details.global_position.y};
                togglePopup();
            };
        }

        return gestureDetector(std::move(props));
    }
};

std::unique_ptr<State> PopupWidget::createState() {
    return std::make_unique<PopupState>();
}

} // namespace enki
