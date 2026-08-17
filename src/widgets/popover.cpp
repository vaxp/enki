/// @file popover.cpp
/// @brief Advanced Native Popover implementation built on NativePopup.

#include "enki/widgets/popover.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/effects/SkRuntimeEffect.h>

#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Popover Body & Arrow Pointer
// ════════════════════════════════════════════════════════════════

class RenderPopoverBackground : public RenderBox {
public:
    PopoverOptions options;
    PopoverDirection direction;

    RenderPopoverBackground(PopoverOptions opt, PopoverDirection dir)
        : options(std::move(opt)), direction(dir) {}

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        float arrow = options.show_arrow ? options.arrow_size : 0.0f;
        float w = size_.width;
        float h = size_.height;

        // Calculate body rect excluding arrow tail
        SkRect body_rect;
        switch (direction) {
            case PopoverDirection::Top:
                body_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, w, h - arrow);
                break;
            case PopoverDirection::Bottom:
                body_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y + arrow, w, h - arrow);
                break;
            case PopoverDirection::Left:
                body_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, w - arrow, h);
                break;
            case PopoverDirection::Right:
                body_rect = SkRect::MakeXYWH(ctx.offset.x + arrow, ctx.offset.y, w - arrow, h);
                break;
            default:
                body_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, w, h);
                break;
        }

        SkRRect rrect;
        rrect.setRectXY(body_rect, options.border_radius, options.border_radius);

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

        // 2. Draw Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);

        if (!options.custom_shader.empty()) {
            auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (effect) {
                struct Uniforms {
                    float time;
                    float resolution[2];
                } uniforms = { 0.0f, {w, h} };
                sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
                bg_paint.setShader(effect->makeShader(uniform_data, nullptr, 0));
            } else {
                bg_paint.setColor(options.background_color);
            }
        } else {
            bg_paint.setColor(options.background_color);
        }

        canvas->drawRRect(rrect, bg_paint);

        // 3. Draw Pointer Arrow
        if (options.show_arrow && arrow > 0.0f) {
            SkPath arrow_path;
            float mid_x = ctx.offset.x + w / 2.0f;
            float mid_y = ctx.offset.y + h / 2.0f;

            if (direction == PopoverDirection::Top) {
                arrow_path.moveTo(mid_x - arrow, ctx.offset.y + h - arrow);
                arrow_path.lineTo(mid_x, ctx.offset.y + h);
                arrow_path.lineTo(mid_x + arrow, ctx.offset.y + h - arrow);
            } else if (direction == PopoverDirection::Bottom) {
                arrow_path.moveTo(mid_x - arrow, ctx.offset.y + arrow);
                arrow_path.lineTo(mid_x, ctx.offset.y);
                arrow_path.lineTo(mid_x + arrow, ctx.offset.y + arrow);
            } else if (direction == PopoverDirection::Left) {
                arrow_path.moveTo(ctx.offset.x + w - arrow, mid_y - arrow);
                arrow_path.lineTo(ctx.offset.x + w, mid_y);
                arrow_path.lineTo(ctx.offset.x + w - arrow, mid_y + arrow);
            } else if (direction == PopoverDirection::Right) {
                arrow_path.moveTo(ctx.offset.x + arrow, mid_y - arrow);
                arrow_path.lineTo(ctx.offset.x, mid_y);
                arrow_path.lineTo(ctx.offset.x + arrow, mid_y + arrow);
            }
            arrow_path.close();

            canvas->drawPath(arrow_path, bg_paint);

            if (options.border_width > 0.0f) {
                SkPaint arrow_border_paint;
                arrow_border_paint.setAntiAlias(true);
                arrow_border_paint.setStyle(SkPaint::kStroke_Style);
                arrow_border_paint.setStrokeWidth(options.border_width);
                arrow_border_paint.setColor(options.border_color);
                canvas->drawPath(arrow_path, arrow_border_paint);
            }
        }

        // 4. Draw Border Stroke
        if (options.border_width > 0.0f) {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(options.border_width);
            border_paint.setColor(options.border_color);

            canvas->drawRRect(rrect, border_paint);
        }

        // 5. Paint Child Content
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

class PopoverBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    PopoverOptions options;
    PopoverDirection direction;

    PopoverBackgroundWidget(PopoverOptions opt, PopoverDirection dir, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)), direction(dir) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderPopoverBackground>(options, direction);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderPopoverBackground*>(&renderObject)) {
            rb->options = options;
            rb->direction = direction;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "PopoverBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Popover State & NativePopup Triggering
// ════════════════════════════════════════════════════════════════

class PopoverState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;

public:
    void initState() override {
        State::initState();
        auto* pop_widget = static_cast<const Popover*>(widget());
        if (pop_widget && pop_widget->controller) {
            pop_widget->controller->setToggleCallback([this](bool show) {
                if (show) showPopoverNow();
                else hidePopoverNow();
            });
        }
    }

    void dispose() override {
        hidePopoverNow();
        State::dispose();
    }

    void showPopoverNow() {
        if (active_popup_) return;

        auto* pop_widget = static_cast<const Popover*>(widget());
        if (!pop_widget || !pop_widget->popover_builder) return;

        Element* elem = element();
        if (!elem) return;

        RenderObject* ro = elem->findRenderObject();
        if (!ro) return;

        Rect target_bounds = ro->globalBounds();
        const auto& opt = pop_widget->options;

        float arrow = opt.show_arrow ? opt.arrow_size : 0.0f;
        int32_t pop_w = static_cast<int32_t>(opt.content_size.width + opt.padding.horizontal());
        int32_t pop_h = static_cast<int32_t>(opt.content_size.height + opt.padding.vertical() + arrow);

        // Smart Direction Calculation
        PopoverDirection dir = opt.direction;
        if (dir == PopoverDirection::Auto) {
            if (target_bounds.y - pop_h < 10) {
                dir = PopoverDirection::Bottom;
            } else {
                dir = PopoverDirection::Top;
            }
        }

        // Calculate Position
        float pop_x = target_bounds.x + (target_bounds.width - pop_w) / 2.0f;
        float pop_y = target_bounds.y - pop_h;

        if (dir == PopoverDirection::Bottom) {
            pop_y = target_bounds.y + target_bounds.height;
        } else if (dir == PopoverDirection::Left) {
            pop_x = target_bounds.x - pop_w;
            pop_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
        } else if (dir == PopoverDirection::Right) {
            pop_x = target_bounds.x + target_bounds.width;
            pop_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
        }

        // Screen boundary fitting
        BuildContext ctx(elem);
        Size screen_sz = ctx.mediaSize();

        if (pop_x + pop_w > screen_sz.width - 10.0f) {
            pop_x = screen_sz.width - pop_w - 10.0f;
        }
        if (pop_y + pop_h > screen_sz.height - 10.0f) {
            pop_y = screen_sz.height - pop_h - 10.0f;
        }
        if (pop_x < 5.0f) pop_x = 5.0f;
        if (pop_y < 5.0f) pop_y = 5.0f;

        // Configure PopupOptions
        PopupOptions pop_opts;
        pop_opts.position = {pop_x, pop_y};
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = opt.auto_dismiss;

        // Build Popover Content
        active_popup_ = NativePopup::show(ctx, pop_opts, [pop_widget, dir](BuildContext& sub_ctx, std::shared_ptr<NativePopup>) {
            WidgetPtr inner_content = pop_widget->popover_builder(sub_ctx);

            auto inner_container = container(inner_content);
            inner_container->padding(pop_widget->options.padding);

            return std::make_shared<PopoverBackgroundWidget>(pop_widget->options, dir, inner_container);
        });
    }

    void hidePopoverNow() {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
    }

    void togglePopover() {
        if (active_popup_) hidePopoverNow();
        else showPopoverNow();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* pop_widget = static_cast<const Popover*>(widget());

        auto gesture = gestureDetector(pop_widget->child);
        gesture->hit_test_behavior = HitTestBehavior::Translucent;

        if (pop_widget->options.trigger == PopoverTrigger::Click) {
            gesture->onTapUp([this](const TapUpDetails&) {
                togglePopover();
            });
        } else if (pop_widget->options.trigger == PopoverTrigger::Hover) {
            gesture->onHoverEnter([this](const PointerEvent&) {
                showPopoverNow();
            });
            gesture->onHoverExit([this](const PointerEvent&) {
                hidePopoverNow();
            });
        }

        return gesture;
    }
};

std::unique_ptr<State> Popover::createState() {
    return std::make_unique<PopoverState>();
}

} // namespace enki
