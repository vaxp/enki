/// @file loading_overlay.cpp
/// @brief Implementation of Advanced LoadingOverlay widget for ENKI Framework.

#include "enki/widgets/loading_overlay.hpp"
#include "enki/widgets/spinner.hpp"
#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/progress_ring.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderLoadingScrim — Absorbs pointer events during loading
// ════════════════════════════════════════════════════════════════

class RenderLoadingScrim : public RenderBox {
public:
    float alpha;
    Color base_color;

    RenderLoadingScrim(float a, Color c) : alpha(a), base_color(c) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext& ctx) override {
        if (alpha <= 0.0f) return;
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = size_.height;

        uint8_t base_a = (base_color >> 24) & 0xFF;
        uint8_t eff_a  = static_cast<uint8_t>(base_a * std::clamp(alpha, 0.0f, 1.0f));
        Color col = (static_cast<uint32_t>(eff_a) << 24) | (base_color & 0x00FFFFFF);

        Paint p;
        p.setColor(col);
        ctx.canvas.drawRect(Rect{x, y, w, h}, p);
    }

    bool hitTestSelf(Point) const override {
        return alpha > 0.0f; // Block all pointer events to background
    }

    void handlePointerDown(const PointerEvent&) override {}
};

class LoadingScrimWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_color;

    LoadingScrimWidget(float a, Color c)
        : SingleChildRenderObjectWidget(Key::none()), alpha(a), base_color(c) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderLoadingScrim>(alpha, base_color);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderLoadingScrim&>(ro);
        r.alpha = alpha;
        r.base_color = base_color;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "LoadingScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// LoadingOverlayState
// ════════════════════════════════════════════════════════════════

class LoadingOverlayState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;
    bool is_loading_ = false;
    LoadingOverlayProps current_opts_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const LoadingOverlayWidget*>(widget());
        is_loading_ = w->initial_loading;
        current_opts_ = w->initial_options;

        anim_.setDuration(std::chrono::milliseconds(180));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(is_loading_ ? 1.0f : 0.0f);

        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) anim_.tick();
        });
        ticker_->start();

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const LoadingOverlayWidget*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this](const LoadingOverlayProps& opts) { showOverlay(opts); };
            w->controller->hide_fn = [this] { hideOverlay(); };
            w->controller->set_progress_fn = [this](float p, const std::string& msg) {
                current_opts_.progress = p;
                current_opts_.is_determinate = true;
                if (!msg.empty()) current_opts_.message = msg;
                setState([] {});
            };
            w->controller->set_message_fn = [this](const std::string& msg) {
                current_opts_.message = msg;
                setState([] {});
            };
            w->controller->is_loading_fn = [this] { return is_loading_; };
        }
    }

    void showOverlay(const LoadingOverlayProps& opts) {
        current_opts_ = opts;
        is_loading_ = true;
        anim_.forward();
        if (current_opts_.on_shown) current_opts_.on_shown();
        setState([] {});
    }

    void hideOverlay() {
        if (!is_loading_) return;
        is_loading_ = false;
        anim_.reverse();
        if (current_opts_.on_hidden) current_opts_.on_hidden();
        setState([] {});
    }

    // ── Build Indicator Widget ────────────────────────────────────

    WidgetPtr buildIndicator(const LoadingOverlayProps& opts) {
        switch (opts.indicator_style) {
            case LoadingIndicatorStyle::Spinner: {
                return Spinner {
                    .style = SpinnerStyle::DualArc,
                    .size = 44.0f,
                    .color = opts.accent_color
                };
            }
            case LoadingIndicatorStyle::ProgressRing: {
                int pct = static_cast<int>(std::clamp(opts.progress, 0.0f, 1.0f) * 100.0f);
                auto pct_txt = text({
                    .text = std::to_string(pct) + "%",
                    .color = opts.title_color,
                    .font_size = 11.0f,
                    .font_weight = FontWeight::Bold,
                });

                return ProgressRing {
                    .value = opts.progress,
                    .size = 56.0f,
                    .stroke_width = 5.0f,
                    .progress_color = opts.accent_color,
                    .indeterminate = !opts.is_determinate,
                    .child = pct_txt
                };
            }
            case LoadingIndicatorStyle::ProgressBar: {
                return ProgressBar {
                    .value = opts.progress,
                    .height = 6.0f,
                    .progress_color = opts.accent_color,
                    .indeterminate = !opts.is_determinate,
                    .min_width = 240.0f
                };
            }
            case LoadingIndicatorStyle::DotsPulse: {
                return Spinner {
                    .style = SpinnerStyle::OrbitDots,
                    .size = 38.0f,
                    .color = opts.accent_color
                };
            }
            case LoadingIndicatorStyle::Custom: {
                if (opts.custom_indicator) return opts.custom_indicator;
                return Spinner {
                    .style = SpinnerStyle::DualArc,
                    .size = 44.0f,
                    .color = opts.accent_color
                };
            }
        }
        return container();
    }

    // ── Build Centered Card ───────────────────────────────────────

    WidgetPtr buildLoadingCard(const LoadingOverlayProps& opts) {
        std::vector<WidgetPtr> card_elements;

        // 1. Indicator
        auto ind_widget = buildIndicator(opts);
        card_elements.push_back(ind_widget);

        // 2. Title + Message
        std::vector<WidgetPtr> text_items;
        if (!opts.title.empty()) {
            auto t_txt = text({
                .text = opts.title,
                .color = opts.title_color,
                .font_size = 15.0f,
                .font_weight = FontWeight::Bold,
            });
            text_items.push_back(t_txt);
        }

        if (!opts.message.empty()) {
            auto m_txt = text({
                .text = opts.message,
                .color = opts.message_color,
                .font_size = 12.5f,
            });
            text_items.push_back(m_txt);
        }

        auto text_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(4.0f),
            .children = std::move(text_items),
        });
        card_elements.push_back(text_col);

        // 3. Optional Cancel Button
        if (opts.allow_cancel) {
            auto c_lbl = text({
                .text = opts.cancel_label,
                .color = 0xFFEF4444,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            });

            auto c_box = container(c_lbl);
            c_box->color(0x22EF4444)
                 .border(0xFFEF4444, 1.0f)
                 .borderRadius(6.0f)
                 .paddingSymmetric(6.0f, 16.0f)
                 .margin(EdgeInsets(6.0f, 0.0f, 0.0f, 0.0f));

            auto c_btn = gestureDetector({
                .child = c_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, opts](const TapUpDetails&) {
                    if (opts.on_cancel) opts.on_cancel();
                    hideOverlay();
                },
            });
            card_elements.push_back(c_btn);
        }

        auto card_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(14.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(card_elements),
        });

        auto card_box = container(card_col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(opts.border_radius)
                .paddingAll(24.0f)
                .width(opts.width)
                .shadow(BoxShadow(0x99000000, {0.0f, 12.0f}, 28.0f));

        return card_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const LoadingOverlayWidget*>(widget());
        const auto& opts = current_opts_;
        float t = anim_.value();

        // ── 1. Invariant Page Body (100% dimensions) ──────────────────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container(w->body);
            bx->width(StyleValue::percent(100.0f))
              .height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f))
                 .height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(empty);
        }

        if (t <= 0.001f && !is_loading_) {
            return Stack {
                .width  = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = { body_widget },
            };
        }

        // ── 2. Loading Scrim Backdrop ─────────────────────────────────
        auto scrim = std::make_shared<LoadingScrimWidget>(t, opts.overlay_color);

        // ── 3. Centered Loading Modal Card ────────────────────────────
        auto card = buildLoadingCard(opts);

        auto center_col = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = { card },
        });

        auto pos_center = Positioned::fill(center_col);

        // ── 4. Stack Composition: Body + Scrim + Centered Card ────────
        return Stack {
            .width  = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {
                body_widget,
                scrim,
                pos_center,
            }
        };
    }
};

std::unique_ptr<State> LoadingOverlayWidget::createState() {
    return std::make_unique<LoadingOverlayState>();
}

} // namespace enki
