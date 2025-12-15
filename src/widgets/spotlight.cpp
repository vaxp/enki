/// @file spotlight.cpp
/// @brief Implementation of Advanced Spotlight & Interactive Feature Tour overlay widget.

#include "enki/widgets/spotlight.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderSpotlightMask — Inverse-cutout overlay with pulse halo
// ════════════════════════════════════════════════════════════════

class RenderSpotlightMask : public RenderBox {
public:
    float alpha = 1.0f;
    Color base_overlay_color = 0xCC080C14;
    Color pulse_ring_color = 0xFF38BDF8;
    Rect target_rect = {0.0f, 0.0f, 0.0f, 0.0f};
    SpotlightShape shape = SpotlightShape::RoundedRectangle;
    float corner_radius = 10.0f;
    bool allow_target_click = true;
    bool show_pulse_ring = true;
    float pulse_phase = 0.0f; // 0.0f to 1.0f
    std::function<void()> on_scrim_tap;

    RenderSpotlightMask() {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void setPulsePhase(float phase) {
        if (std::abs(pulse_phase - phase) > 0.0005f) {
            pulse_phase = phase;
            markNeedsPaint();
        }
    }

    void setAlpha(float a) {
        if (std::abs(alpha - a) > 0.001f) {
            alpha = a;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        if (alpha <= 0.0f) return;

        float ox = ctx.offset.x;
        float oy = ctx.offset.y;
        float w = size_.width;
        float h = size_.height;

        uint8_t base_a = (base_overlay_color >> 24) & 0xFF;
        uint8_t eff_a  = static_cast<uint8_t>(base_a * std::clamp(alpha, 0.0f, 1.0f));
        Color mask_col = (static_cast<uint32_t>(eff_a) << 24) | (base_overlay_color & 0x00FFFFFF);

        Paint mask_paint;
        mask_paint.setColor(mask_col);

        Rect outer_rect{ox, oy, w, h};
        Rect inner_rect{target_rect.x + ox, target_rect.y + oy, target_rect.width, target_rect.height};

        // Determine inner radius
        BorderRadius inner_radius;
        if (shape == SpotlightShape::Circle) {
            float r = std::max(target_rect.width, target_rect.height) / 2.0f;
            inner_radius = BorderRadius::circular(r);
        } else if (shape == SpotlightShape::Rectangle) {
            inner_radius = BorderRadius::zero();
        } else {
            inner_radius = BorderRadius::circular(corner_radius);
        }

        // Draw the inverse difference rounded rectangle
        ctx.canvas.drawDRRect(outer_rect, BorderRadius::zero(), inner_rect, inner_radius, mask_paint);

        // Render animated beacon / pulse halo around the target
        if (show_pulse_ring && alpha > 0.3f && target_rect.width > 0.0f && target_rect.height > 0.0f) {
            float expand = pulse_phase * 16.0f;
            Rect pulse_rect{
                inner_rect.x - expand,
                inner_rect.y - expand,
                inner_rect.width + expand * 2.0f,
                inner_rect.height + expand * 2.0f
            };

            uint8_t ring_a = static_cast<uint8_t>(200.0f * (1.0f - pulse_phase) * alpha);
            Color ring_col = (static_cast<uint32_t>(ring_a) << 24) | (pulse_ring_color & 0x00FFFFFF);

            Paint ring_paint;
            ring_paint.setColor(ring_col);
            ring_paint.setStyle(PaintStyle::Stroke);
            ring_paint.setStrokeWidth(2.0f);

            BorderRadius pulse_radius;
            if (shape == SpotlightShape::Circle) {
                pulse_radius = BorderRadius::circular((target_rect.width + expand * 2.0f) / 2.0f);
            } else if (shape == SpotlightShape::Rectangle) {
                pulse_radius = BorderRadius::zero();
            } else {
                pulse_radius = BorderRadius::circular(corner_radius + expand);
            }

            ctx.canvas.drawRRect(pulse_rect, pulse_radius, ring_paint);
        }
    }

    bool hitTestSelf(Point p) const override {
        if (alpha <= 0.0f) return false;

        bool in_target = (p.x >= target_rect.x && p.x <= (target_rect.x + target_rect.width) &&
                          p.y >= target_rect.y && p.y <= (target_rect.y + target_rect.height));

        if (in_target) {
            // If click inside hole is allowed, pass through to underlying widget
            return !allow_target_click;
        }

        // Outside hole: intercept tap
        return true;
    }

    void handlePointerDown(const PointerEvent& e) override {
        bool in_target = (e.position.x >= target_rect.x && e.position.x <= (target_rect.x + target_rect.width) &&
                          e.position.y >= target_rect.y && e.position.y <= (target_rect.y + target_rect.height));

        if (!in_target && on_scrim_tap) {
            on_scrim_tap();
        }
    }
};

class SpotlightMaskWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_overlay_color;
    Color pulse_ring_color;
    Rect target_rect;
    SpotlightShape shape;
    float corner_radius;
    bool allow_target_click;
    bool show_pulse_ring;
    float pulse_phase;
    std::function<void()> on_scrim_tap;
    std::shared_ptr<RenderSpotlightMask*> ro_holder;

    SpotlightMaskWidget(float a, Color col, Color ring_col, Rect target, SpotlightShape sh, float cr,
                        bool allow_click, bool pulse, float phase, std::function<void()> tap,
                        std::shared_ptr<RenderSpotlightMask*> holder)
        : SingleChildRenderObjectWidget(Key::none()),
          alpha(a), base_overlay_color(col), pulse_ring_color(ring_col), target_rect(target), shape(sh),
          corner_radius(cr), allow_target_click(allow_click), show_pulse_ring(pulse),
          pulse_phase(phase), on_scrim_tap(std::move(tap)), ro_holder(std::move(holder)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderSpotlightMask>();
        ro->alpha = alpha;
        ro->base_overlay_color = base_overlay_color;
        ro->pulse_ring_color = pulse_ring_color;
        ro->target_rect = target_rect;
        ro->shape = shape;
        ro->corner_radius = corner_radius;
        ro->allow_target_click = allow_target_click;
        ro->show_pulse_ring = show_pulse_ring;
        ro->pulse_phase = pulse_phase;
        ro->on_scrim_tap = on_scrim_tap;
        if (ro_holder) {
            *ro_holder = ro.get();
        }
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        auto& ro = static_cast<RenderSpotlightMask&>(renderObject);
        ro.alpha = alpha;
        ro.base_overlay_color = base_overlay_color;
        ro.pulse_ring_color = pulse_ring_color;
        ro.target_rect = target_rect;
        ro.shape = shape;
        ro.corner_radius = corner_radius;
        ro.allow_target_click = allow_target_click;
        ro.show_pulse_ring = show_pulse_ring;
        ro.pulse_phase = pulse_phase;
        ro.on_scrim_tap = on_scrim_tap;
        if (ro_holder) {
            *ro_holder = &ro;
        }
        ro.markNeedsPaint();
    }

    void didUnmountRenderObject(RenderObject& ro) override {
        if (ro_holder && *ro_holder == &ro) {
            *ro_holder = nullptr;
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "SpotlightMaskWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Spotlight State
// ════════════════════════════════════════════════════════════════

class SpotlightState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;
    std::unique_ptr<Ticker> pulse_ticker_;

    bool is_active_ = false;
    size_t current_step_idx_ = 0;
    float pulse_phase_ = 0.0f;
    Rect dynamic_target_rect_ = {0.0f, 0.0f, 0.0f, 0.0f};

    SlotId key_down_conn_ = 0;
    std::shared_ptr<RenderSpotlightMask*> mask_ro_holder_ = std::make_shared<RenderSpotlightMask*>(nullptr);

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SpotlightWidget*>(widget());
        is_active_ = w->initial_active;

        anim_.setDuration(std::chrono::milliseconds(200));
        anim_.setValue(is_active_ ? 1.0f : 0.0f);

        // Entrance/exit fade updates alpha and rebuilds only during the 200ms animation
        anim_.addListener([this] {
            if (*mask_ro_holder_) {
                (*mask_ro_holder_)->setAlpha(anim_.value());
            }
            setState([] {});
        });

        anim_.addStatusListener([this](AnimationStatus status) {
            if (status == AnimationStatus::Completed) {
                if (ticker_) ticker_->stop();
            } else if (status == AnimationStatus::Dismissed) {
                if (ticker_) ticker_->stop();
                if (pulse_ticker_) pulse_ticker_->stop();
                setState([] {}); // Rebuild to remove overlay from stack
            }
        });

        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) {
                anim_.tick();
            } else if (ticker_) {
                ticker_->stop();
            }
        });

        // Beacon pulse ticker — strictly paint-level updates without calling setState()
        pulse_ticker_ = createTicker([this] {
            if (is_active_ && *mask_ro_holder_) {
                pulse_phase_ += 0.016f; // ~60 FPS step
                if (pulse_phase_ >= 1.0f) {
                    pulse_phase_ = 0.0f;
                }
                (*mask_ro_holder_)->setPulsePhase(pulse_phase_);
            }
        });

        if (is_active_ && hasPulseRing()) {
            pulse_ticker_->start();
        }

        wireController();

        if (Platform::instance()) {
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int) {
                if (!is_active_) return;
                const int KEY_ESCAPE = 0xff1b;
                if (key == KEY_ESCAPE) {
                    skipTour();
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void dispose() override {
        if (Platform::instance() && key_down_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        if (ticker_) ticker_->stop();
        if (pulse_ticker_) pulse_ticker_->stop();
        anim_.dispose();
        *mask_ro_holder_ = nullptr;
        State::dispose();
    }

    bool hasPulseRing() const {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (current_step_idx_ < w->steps.size()) {
            return w->steps[current_step_idx_].show_pulse_ring;
        }
        return false;
    }

    void wireController() {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (w->controller) {
            w->controller->start_fn = [this] { startTour(); };
            w->controller->next_fn = [this] { nextStep(); };
            w->controller->previous_fn = [this] { previousStep(); };
            w->controller->skip_fn = [this] { skipTour(); };
            w->controller->finish_fn = [this] { finishTour(); };
            w->controller->go_to_step_fn = [this](size_t idx) { goToStep(idx); };
            w->controller->get_step_index_fn = [this] { return current_step_idx_; };
            w->controller->get_total_steps_fn = [w] { return w->steps.size(); };
            w->controller->is_active_fn = [this] { return is_active_; };
            w->controller->update_target_rect_fn = [this](Rect r) {
                dynamic_target_rect_ = r;
                if (*mask_ro_holder_) {
                    (*mask_ro_holder_)->target_rect = r;
                    (*mask_ro_holder_)->markNeedsPaint();
                }
                setState([] {});
            };
            w->controller->set_steps_fn = [this](std::vector<SpotlightStep>) {
                current_step_idx_ = 0;
                setState([] {});
            };
        }
    }

    void startTour() {
        if (is_active_) return;
        is_active_ = true;
        current_step_idx_ = 0;
        pulse_phase_ = 0.0f;
        anim_.forward();
        if (ticker_) ticker_->start();
        if (pulse_ticker_ && hasPulseRing()) {
            pulse_ticker_->start();
        }
        notifyStepChange();
        setState([] {}); // Rebuild to mount overlay
    }

    void nextStep() {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (w->steps.empty()) return;

        if (current_step_idx_ + 1 < w->steps.size()) {
            current_step_idx_++;
            pulse_phase_ = 0.0f;
            if (pulse_ticker_) {
                if (hasPulseRing()) {
                    pulse_ticker_->start();
                } else {
                    pulse_ticker_->stop();
                }
            }
            notifyStepChange();
            setState([] {}); // Rebuild card for next step
        } else {
            finishTour();
        }
    }

    void previousStep() {
        if (current_step_idx_ > 0) {
            current_step_idx_--;
            pulse_phase_ = 0.0f;
            if (pulse_ticker_) {
                if (hasPulseRing()) {
                    pulse_ticker_->start();
                } else {
                    pulse_ticker_->stop();
                }
            }
            notifyStepChange();
            setState([] {});
        }
    }

    void goToStep(size_t idx) {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (idx < w->steps.size()) {
            current_step_idx_ = idx;
            pulse_phase_ = 0.0f;
            if (pulse_ticker_) {
                if (hasPulseRing()) {
                    pulse_ticker_->start();
                } else {
                    pulse_ticker_->stop();
                }
            }
            notifyStepChange();
            setState([] {});
        }
    }

    void skipTour() {
        if (!is_active_) return;
        is_active_ = false;
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (w->options.on_skip) w->options.on_skip();
        anim_.reverse();
        if (ticker_) ticker_->start();
        if (pulse_ticker_) pulse_ticker_->stop();
        setState([] {});
    }

    void finishTour() {
        if (!is_active_) return;
        is_active_ = false;
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (w->options.on_finish) w->options.on_finish();
        anim_.reverse();
        if (ticker_) ticker_->start();
        if (pulse_ticker_) pulse_ticker_->stop();
        setState([] {});
    }

    void notifyStepChange() {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        if (w->options.on_step_change && current_step_idx_ < w->steps.size()) {
            w->options.on_step_change(current_step_idx_, w->steps[current_step_idx_]);
        }
    }

    // ── Build Tour Popover Card ───────────────────────────────────
    WidgetPtr buildPopoverCard(const SpotlightWidget* w, const SpotlightStep& step, const Rect& padded_target) {
        const auto& opts = w->options;
        size_t total_steps = w->steps.size();
        bool is_last_step = (current_step_idx_ + 1 >= total_steps);

        // 1. Header Row (Step Counter + Skip Button)
        std::vector<WidgetPtr> header_items;

        if (opts.show_step_indicator && total_steps > 1) {
            std::string step_text = "STEP " + std::to_string(current_step_idx_ + 1) + " OF " + std::to_string(total_steps);
            header_items.push_back(container({
                .color = 0x2E38BDF8,
                .border_radius = BorderRadius::circular(5.0f),
                .padding = StyleInsets::symmetric(3.0f, 8.0f),
                .child = text(step_text, {
                    .color = opts.step_badge_color,
                    .font_size = 10.5f,
                    .font_weight = FontWeight::Bold
                })
            }));
        }

        if (opts.show_skip_button && !is_last_step) {
            header_items.push_back(gestureDetector({
                .child = text(step.skip_button_label, {
                    .color = 0xFF64748B,
                    .font_size = 11.5f,
                    .font_weight = FontWeight::Medium
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    skipTour();
                }
            }));
        }

        auto header_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = header_items
        });

        // 2. Title + Description Body
        std::vector<WidgetPtr> body_children;
        if (!header_items.empty()) {
            body_children.push_back(header_row);
        }

        if (!step.title.empty()) {
            body_children.push_back(text(step.title, {
                .color = opts.title_color,
                .font_size = 15.5f,
                .font_weight = FontWeight::Bold
            }));
        }

        if (!step.description.empty()) {
            body_children.push_back(text(step.description, {
                .color = opts.description_color,
                .font_size = 12.5f
            }));
        }

        // 3. Footer Action Buttons
        std::vector<WidgetPtr> footer_left;
        // Step indicator dots
        if (total_steps > 1) {
            std::vector<WidgetPtr> dots;
            for (size_t i = 0; i < total_steps; ++i) {
                bool is_cur = (i == current_step_idx_);
                dots.push_back(container({
                    .color = is_cur ? 0xFF38BDF8 : 0xFF334155,
                    .border_radius = BorderRadius::circular(3.0f),
                    .width = StyleValue::point(is_cur ? 14.0f : 6.0f),
                    .height = StyleValue::point(6.0f)
                }));
            }
            footer_left.push_back(row({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = dots
            }));
        }

        std::vector<WidgetPtr> action_buttons;

        // Back button
        if (current_step_idx_ > 0) {
            action_buttons.push_back(gestureDetector({
                .child = container({
                    .color = 0xFF1E293B,
                    .border_radius = BorderRadius::circular(6.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::symmetric(6.0f, 14.0f),
                    .child = text(step.back_button_label, {
                        .color = 0xFFCBD5E1,
                        .font_size = 12.0f,
                        .font_weight = FontWeight::Medium
                    })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    previousStep();
                }
            }));
        }

        // Next / Finish primary button
        std::string next_label = !step.next_button_label.empty() ? step.next_button_label : (is_last_step ? "Got it!" : "Next");
        action_buttons.push_back(gestureDetector({
            .child = container({
                .color = 0xFF0284C7, // Sky 600
                .border_radius = BorderRadius::circular(6.0f),
                .padding = StyleInsets::symmetric(6.0f, 16.0f),
                .child = text(next_label, {
                    .color = 0xFFFFFFFF,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold
                })
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                nextStep();
            }
        }));

        auto footer_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                row({
                    .align_items = Align::Center,
                    .children = footer_left
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = action_buttons
                })
            }
        });

        body_children.push_back(footer_row);

        return container({
            .color = opts.card_bg_color,
            .border_radius = BorderRadius::circular(opts.card_border_radius),
            .border = Border(opts.card_border_color, 1.0f),
            .box_shadow = {
                BoxShadow(0x99000000, {0.0f, 12.0f}, 32.0f),
                BoxShadow(0x3338BDF8, {0.0f, 0.0f}, 20.0f)
            },
            .width = StyleValue::point(opts.card_width),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = body_children
            })
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SpotlightWidget*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // ── 1. Page Body ──────────────────────────────────────────────
        WidgetPtr body_widget = w->body ? Positioned::fill(w->body) : Positioned::fill(container({}));

        // When closed and animation finished: render only body
        if ((!is_active_ && t <= 0.001f) || w->steps.empty() || current_step_idx_ >= w->steps.size()) {
            return stack({
                .children = {body_widget},
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            });
        }

        const auto& step = w->steps[current_step_idx_];

        // Resolve target bounds (with padding)
        Rect raw_bounds = (dynamic_target_rect_.width > 0.0f) ? dynamic_target_rect_ : step.target_bounds;
        Rect padded_target = {
            raw_bounds.x - step.padding.left,
            raw_bounds.y - step.padding.top,
            raw_bounds.width + step.padding.left + step.padding.right,
            raw_bounds.height + step.padding.top + step.padding.bottom
        };

        // ── 2. Inverse Cutout Mask Overlay ────────────────────────────
        std::function<void()> scrim_tap = nullptr;
        if (opts.dismiss_on_scrim_tap) {
            scrim_tap = [this] { nextStep(); };
        }

        auto mask = std::make_shared<SpotlightMaskWidget>(
            std::max(t, 0.001f),
            opts.overlay_color,
            opts.pulse_ring_color,
            padded_target,
            step.shape,
            step.corner_radius,
            step.allow_target_click,
            step.show_pulse_ring,
            pulse_phase_,
            scrim_tap,
            mask_ro_holder_
        );

        // ── 3. Position Popover Card ──────────────────────────────────
        auto card = buildPopoverCard(w, step, padded_target);

        // Calculate card placement coordinates
        float card_w = opts.card_width;
        float card_h = 170.0f; // Approx card height
        float dist = opts.popover_distance;

        SpotlightPlacement placement = step.placement;
        if (placement == SpotlightPlacement::Auto) {
            if (padded_target.y + padded_target.height + card_h + dist < 650.0f) {
                placement = SpotlightPlacement::Bottom;
            } else if (padded_target.y - card_h - dist > 10.0f) {
                placement = SpotlightPlacement::Top;
            } else {
                placement = SpotlightPlacement::Bottom;
            }
        }

        float card_x = padded_target.x + (padded_target.width - card_w) / 2.0f;
        float card_y = padded_target.y + padded_target.height + dist;

        if (placement == SpotlightPlacement::Top) {
            card_y = padded_target.y - card_h - dist;
        } else if (placement == SpotlightPlacement::Left) {
            card_x = padded_target.x - card_w - dist;
            card_y = padded_target.y + (padded_target.height - card_h) / 2.0f;
        } else if (placement == SpotlightPlacement::Right) {
            card_x = padded_target.x + padded_target.width + dist;
            card_y = padded_target.y + (padded_target.height - card_h) / 2.0f;
        }

        // Clamp to screen boundaries
        if (card_x < 16.0f) card_x = 16.0f;
        if (card_y < 16.0f) card_y = 16.0f;

        auto pos_card = Positioned {
            .child = card,
            .top = StyleValue::point(card_y),
            .left = StyleValue::point(card_x)
        };

        // ── 4. Stack Composition: Body + Cutout Mask + Popover Card ───
        return stack({
            .children = {
                body_widget,
                mask,
                pos_card
            },
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f)
        });
    }
};

std::unique_ptr<State> SpotlightWidget::createState() {
    return std::make_unique<SpotlightState>();
}

} // namespace enki
