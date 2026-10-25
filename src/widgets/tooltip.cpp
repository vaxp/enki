/// @file tooltip.cpp
/// @brief Advanced Native Tooltip widget implementation built on NativePopup.

#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <chrono>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Tooltip Body & Arrow Pointer
// ════════════════════════════════════════════════════════════════

class RenderTooltipBody : public RenderBox {
public:
    TooltipOptions options;
    TooltipPosition position;

    RenderTooltipBody(TooltipOptions opt, TooltipPosition pos)
        : options(std::move(opt)), position(pos) {}

    void paint(PaintContext& ctx) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        auto& canvas = ctx.canvas;
        float arrow = options.arrow_size;
        float w = size_.width;
        float h = size_.height;

        // Calculate body bounding box excluding arrow tail
        Rect body_rect;
        switch (position) {
            case TooltipPosition::Top:
                body_rect = Rect{ctx.offset.x, ctx.offset.y, w, std::max(0.0f, h - arrow)};
                break;
            case TooltipPosition::Bottom:
                body_rect = Rect{ctx.offset.x, ctx.offset.y + arrow, w, std::max(0.0f, h - arrow)};
                break;
            case TooltipPosition::Left:
                body_rect = Rect{ctx.offset.x, ctx.offset.y, std::max(0.0f, w - arrow), h};
                break;
            case TooltipPosition::Right:
                body_rect = Rect{ctx.offset.x + arrow, ctx.offset.y, std::max(0.0f, w - arrow), h};
                break;
            default:
                body_rect = Rect{ctx.offset.x, ctx.offset.y, w, h};
                break;
        }

        BorderRadius radius = BorderRadius::circular(options.border_radius);

        // 1. Draw Shadow
        if (options.elevation > 0.0f) {
            canvas.drawRRectShadow(body_rect, radius, options.shadow_color,
                                  options.elevation, Point{0.0f, options.elevation * 0.3f});
        }

        // 2. Build Path for Tooltip Body + Arrow Pointer
        Path tooltip_path;
        tooltip_path.addRRect(body_rect, radius);

        if (arrow > 0.0f) {
            float cx = body_rect.x + body_rect.width * 0.5f;
            float cy = body_rect.y + body_rect.height * 0.5f;
            float bottom = body_rect.y + body_rect.height;
            float right = body_rect.x + body_rect.width;

            if (position == TooltipPosition::Top) {
                // Arrow pointing down at bottom edge
                tooltip_path.moveTo(cx - arrow, bottom);
                tooltip_path.lineTo(cx + arrow, bottom);
                tooltip_path.lineTo(cx, bottom + arrow);
                tooltip_path.close();
            } else if (position == TooltipPosition::Bottom) {
                // Arrow pointing up at top edge
                tooltip_path.moveTo(cx - arrow, body_rect.y);
                tooltip_path.lineTo(cx + arrow, body_rect.y);
                tooltip_path.lineTo(cx, body_rect.y - arrow);
                tooltip_path.close();
            } else if (position == TooltipPosition::Left) {
                // Arrow pointing right at right edge
                tooltip_path.moveTo(right, cy - arrow);
                tooltip_path.lineTo(right, cy + arrow);
                tooltip_path.lineTo(right + arrow, cy);
                tooltip_path.close();
            } else if (position == TooltipPosition::Right) {
                // Arrow pointing left at left edge
                tooltip_path.moveTo(body_rect.x, cy - arrow);
                tooltip_path.lineTo(body_rect.x, cy + arrow);
                tooltip_path.lineTo(body_rect.x - arrow, cy);
                tooltip_path.close();
            }
        }

        // 3. Paint Background
        Paint bg_paint;
        bg_paint.setStyle(PaintStyle::Fill);
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        canvas.drawPath(tooltip_path, bg_paint);

        // 4. Paint Border Stroke
        if (options.border_width > 0.0f) {
            Paint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(PaintStyle::Stroke);
            border_paint.setStrokeWidth(options.border_width);
            border_paint.setColor(options.border_color);
            canvas.drawPath(tooltip_path, border_paint);
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

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget Wrapper
// ════════════════════════════════════════════════════════════════

class TooltipBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    TooltipOptions options;
    TooltipPosition position;

    TooltipBackgroundWidget(TooltipOptions opt, TooltipPosition pos, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)), position(pos) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderTooltipBody>(options, position);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderTooltipBody*>(&renderObject)) {
            rb->options = options;
            rb->position = position;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "TooltipBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Tooltip State & Lifecycle Management
// ════════════════════════════════════════════════════════════════

class TooltipState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;
    bool is_hovered_ = false;
    std::unique_ptr<Ticker> show_timer_ = nullptr;
    std::unique_ptr<Ticker> hide_timer_ = nullptr;

public:
    void initState() override {
        State::initState();
    }

    void dispose() override {
        hideTooltipNow();
        if (show_timer_) show_timer_->stop();
        if (hide_timer_) hide_timer_->stop();
        State::dispose();
    }

    void scheduleShowTooltip() {
        is_hovered_ = true;
        if (active_popup_) return;

        auto* tooltip_widget = static_cast<const Tooltip*>(widget());
        if (!tooltip_widget) return;

        if (show_timer_) show_timer_->stop();

        auto start = std::chrono::steady_clock::now();
        show_timer_ = createTicker([this, start, tooltip_widget]() {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            if (elapsed >= tooltip_widget->options.show_delay) {
                if (show_timer_) show_timer_->stop();
                if (is_hovered_) {
                    showTooltipNow();
                }
            }
        });
        show_timer_->start();
    }

    void scheduleHideTooltip() {
        is_hovered_ = false;
        if (!active_popup_) {
            if (show_timer_) show_timer_->stop();
            return;
        }

        auto* tooltip_widget = static_cast<const Tooltip*>(widget());
        if (!tooltip_widget) return;

        if (hide_timer_) hide_timer_->stop();

        auto start = std::chrono::steady_clock::now();
        hide_timer_ = createTicker([this, start, tooltip_widget]() {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            if (elapsed >= tooltip_widget->options.hide_delay) {
                if (hide_timer_) hide_timer_->stop();
                if (!is_hovered_) {
                    hideTooltipNow();
                }
            }
        });
        hide_timer_->start();
    }

    void showTooltipNow() {
        if (active_popup_) return;

        auto* tooltip_widget = static_cast<const Tooltip*>(widget());
        if (!tooltip_widget) return;

        Element* elem = element();
        if (!elem) return;

        RenderObject* ro = elem->findRenderObject();
        if (!ro) return;

        Rect target_bounds = ro->globalBounds();

        int32_t pop_w = 180;
        int32_t pop_h = 44;

        if (!tooltip_widget->message.empty()) {
            pop_w = std::max(60, static_cast<int32_t>(tooltip_widget->message.length() * 8 + 32));
            pop_h = 40;
        } else {
            pop_w = 220;
            pop_h = 60;
        }

        float arrow = tooltip_widget->options.arrow_size;

        TooltipPosition position = tooltip_widget->options.position;
        if (position == TooltipPosition::Auto) {
            if (target_bounds.y - pop_h - arrow < 10) {
                position = TooltipPosition::Bottom;
            } else {
                position = TooltipPosition::Top;
            }
        }

        float popup_x = target_bounds.x + (target_bounds.width - pop_w) / 2.0f;
        float popup_y = target_bounds.y - pop_h - arrow;

        if (position == TooltipPosition::Bottom) {
            popup_y = target_bounds.y + target_bounds.height + arrow;
        } else if (position == TooltipPosition::Left) {
            popup_x = target_bounds.x - pop_w - arrow;
            popup_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
        } else if (position == TooltipPosition::Right) {
            popup_x = target_bounds.x + target_bounds.width + arrow;
            popup_y = target_bounds.y + (target_bounds.height - pop_h) / 2.0f;
        }

        if (popup_x < 10) popup_x = 10;
        if (popup_y < 10) popup_y = 10;

        PopupOptions pop_opts;
        pop_opts.position = {popup_x, popup_y};
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = false;

        BuildContext ctx(elem);
        WidgetPtr msg_content;
        if (!tooltip_widget->message.empty()) {
            msg_content = text({
                .text = tooltip_widget->message,
                .color = tooltip_widget->options.text_color,
                .font_size = tooltip_widget->options.font_size,
            });
        } else if (tooltip_widget->rich_message) {
            msg_content = tooltip_widget->rich_message;
        }

        auto inner = container({
            .align = Alignment::Center,
            .padding = StyleInsets::only(tooltip_widget->options.padding.top, tooltip_widget->options.padding.right, tooltip_widget->options.padding.bottom, tooltip_widget->options.padding.left),
            .child = msg_content,
        });

        WidgetPtr body = std::make_shared<TooltipBackgroundWidget>(
            tooltip_widget->options, position, inner
        );

        active_popup_ = NativePopup::show(ctx, pop_opts, [body](BuildContext&, std::shared_ptr<NativePopup>) {
            return body;
        });
    }

    void hideTooltipNow() {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* tooltip_widget = static_cast<const Tooltip*>(widget());

        GestureDetectorProps props;
        props.child = tooltip_widget->child;
        props.hit_test_behavior = HitTestBehavior::Translucent;

        if (tooltip_widget->options.trigger == TooltipTrigger::Hover) {
            props.on_hover_enter = [this](const PointerEvent&) {
                scheduleShowTooltip();
            };
            props.on_hover_exit = [this](const PointerEvent&) {
                scheduleHideTooltip();
            };
        } else if (tooltip_widget->options.trigger == TooltipTrigger::LongPress) {
            props.on_long_press = [this]() {
                if (active_popup_) hideTooltipNow();
                else showTooltipNow();
            };
        } else if (tooltip_widget->options.trigger == TooltipTrigger::Tap) {
            props.on_tap_up = [this](const TapUpDetails&) {
                if (active_popup_) hideTooltipNow();
                else showTooltipNow();
            };
        }

        return gestureDetector(std::move(props));
    }
};

std::unique_ptr<State> Tooltip::createState() {
    return std::make_unique<TooltipState>();
}

} // namespace enki
