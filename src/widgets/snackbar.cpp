/// @file snackbar.cpp
/// @brief Implementation of Advanced In-Window Overlay Snackbar / Toast widget.

#include "enki/widgets/snackbar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <chrono>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Internal Floating Snackbar Card Widget (Isolated State)
// ════════════════════════════════════════════════════════════════

class SnackbarCardWidget : public StatefulWidget {
public:
    SnackbarOptions options;
    std::function<void()> on_close_request;

    SnackbarCardWidget(SnackbarOptions opts, std::function<void()> on_close)
        : options(std::move(opts)), on_close_request(std::move(on_close)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "SnackbarCard"; }
};

class SnackbarCardState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;
    bool is_hovered_ = false;
    bool is_closing_ = false;

    double display_duration_sec_ = 4.0;
    double elapsed_sec_ = 0.0;
    std::chrono::steady_clock::time_point last_tick_time_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SnackbarCardWidget*>(widget());
        display_duration_sec_ = w->options.duration_ms > 0 ? (w->options.duration_ms / 1000.0) : 0.0;
        elapsed_sec_ = 0.0;
        is_hovered_ = false;
        is_closing_ = false;

        anim_.setDuration(std::chrono::milliseconds(180));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(0.0f);

        last_tick_time_ = std::chrono::steady_clock::now();

        ticker_ = createTicker([this] {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - last_tick_time_).count();
            last_tick_time_ = now;

            auto* card_w = static_cast<const SnackbarCardWidget*>(widget());
            bool needs_rebuild = false;

            // Tick slide animation
            if (anim_.isAnimating()) {
                anim_.tick();
                needs_rebuild = true;
                if (!anim_.isAnimating() && is_closing_) {
                    if (card_w->on_close_request) card_w->on_close_request();
                    return;
                }
            }

            // Tick auto-dismiss timer when not closing
            if (!is_closing_ && card_w->options.duration_ms > 0) {
                if (!(is_hovered_ && card_w->options.pause_on_hover)) {
                    elapsed_sec_ += dt;
                    if (elapsed_sec_ >= display_duration_sec_) {
                        startDismiss();
                    } else if (card_w->options.show_progress_bar) {
                        needs_rebuild = true;
                    }
                }
            }

            if (needs_rebuild) {
                setState([] {});
            }
        });

        ticker_->start();
        anim_.forward();
        if (w->options.on_shown) w->options.on_shown();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    void startDismiss() {
        if (is_closing_) return;
        is_closing_ = true;
        auto* w = static_cast<const SnackbarCardWidget*>(widget());
        if (w->options.on_dismissed) w->options.on_dismissed();
        anim_.reverse();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SnackbarCardWidget*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // Left Section: Icon badge + Title / Message
        std::vector<WidgetPtr> left_items;

        if (!opts.icon.empty()) {
            auto ic_txt = text({
                .text = opts.icon,
                .font_size = 16.0f,
            });

            auto ic_box = container({
                .padding = StyleInsets::all(4.0f),
                .child = ic_txt,
            });
            left_items.push_back(ic_box);
        }

        std::vector<WidgetPtr> text_col_items;
        if (!opts.title.empty()) {
            auto t_txt = text({
                .text = opts.title,
                .color = opts.title_color,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            });
            text_col_items.push_back(t_txt);
        }

        if (!opts.message.empty()) {
            auto m_txt = text({
                .text = opts.message,
                .color = opts.message_color,
                .font_size = 12.0f,
            });
            text_col_items.push_back(m_txt);
        }

        auto text_col = column({
            .flex = 1.0f,
            .gap = StyleValue::point(2.0f),
            .children = std::move(text_col_items),
        });
        left_items.push_back(text_col);

        auto left_row = row({
            .align_items = Align::Center,
            .flex = 1.0f,
            .gap = StyleValue::point(10.0f),
            .children = std::move(left_items),
        });

        // Right Section: Action Button + Close ✕
        std::vector<WidgetPtr> right_items;

        if (opts.action.has_value()) {
            const auto& act = opts.action.value();
            Color act_color = act.is_danger ? 0xFFEF4444 : opts.accent_color;
            auto act_txt = text({
                .text = act.label,
                .color = act_color,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            });

            auto act_box = container({
                .color = 0x2238BDF8,
                .border_radius = BorderRadius::circular(4.0f),
                .padding = StyleInsets::symmetric(4.0f, 10.0f),
                .child = act_txt,
            });

            auto act_btn = gestureDetector({
                .child = act_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, act](const TapUpDetails&) {
                    if (act.on_click) act.on_click();
                    startDismiss();
                },
            });
            right_items.push_back(act_btn);
        }

        if (opts.show_close_button) {
            auto cls_txt = text({
                .text = "✕",
                .color = 0xFF94A3B8,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            });

            auto cls_box = container({
                .padding = StyleInsets::all(4.0f),
                .child = cls_txt,
            });

            auto cls_btn = gestureDetector({
                .child = cls_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    startDismiss();
                },
            });
            right_items.push_back(cls_btn);
        }

        auto right_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = std::move(right_items),
        });

        auto content_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {left_row, right_row},
        });

        std::vector<WidgetPtr> card_elements = {content_row};

        // Bottom Progress Bar
        if (opts.show_progress_bar && opts.duration_ms > 0) {
            float progress_ratio = 1.0f;
            if (display_duration_sec_ > 0.0) {
                progress_ratio = static_cast<float>(std::clamp(1.0 - (elapsed_sec_ / display_duration_sec_), 0.0, 1.0));
            }

            auto bar_fill = container({
                .color = opts.progress_bar_col,
                .border_radius = BorderRadius::circular(1.0f),
                .width = StyleValue::percent(progress_ratio * 100.0f),
                .height = StyleValue::point(2.5f),
            });

            auto bar_track = container({
                .color = 0x33334155,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::point(2.5f),
                .margin = StyleInsets::only(6.0f, 0.0f, 0.0f, 0.0f),
                .child = bar_fill,
            });

            card_elements.push_back(bar_track);
        }

        auto card_col = column({
            .width = StyleValue::percent(100.0f),
            .children = std::move(card_elements),
        });

        auto card_box = container({
            .color = opts.background_color,
            .border_radius = BorderRadius::circular(opts.border_radius),
            .border = Border(opts.border_color, 1.0f),
            .box_shadow = {BoxShadow(0x99000000, {0.0f, 8.0f}, 20.0f)},
            .width = StyleValue::point(opts.width),
            .padding = StyleInsets::all(14.0f),
            .child = card_col,
        });

        // Hover Detector for Pause-on-Hover
        auto card = gestureDetector({
            .child = card_box,
            .on_hover_enter = [this](const PointerEvent&) {
                is_hovered_ = true;
            },
            .on_hover_exit = [this](const PointerEvent&) {
                is_hovered_ = false;
            },
        });

        float margin = opts.margin;
        float slide_offset = (1.0f - t) * -30.0f;

        switch (opts.placement) {
            case SnackbarPlacement::BottomCenter: {
                auto row_wrap = row({
                    .justify_content = Justify::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {card},
                });

                return Positioned {
                    .child = row_wrap,
                    .right = StyleValue::point(0.0f),
                    .bottom = StyleValue::point(margin + slide_offset),
                    .left = StyleValue::point(0.0f),
                };
            }
            case SnackbarPlacement::BottomRight: {
                return Positioned {
                    .child = card,
                    .right = StyleValue::point(margin),
                    .bottom = StyleValue::point(margin + slide_offset),
                };
            }
            case SnackbarPlacement::BottomLeft: {
                return Positioned {
                    .child = card,
                    .bottom = StyleValue::point(margin + slide_offset),
                    .left = StyleValue::point(margin),
                };
            }
            case SnackbarPlacement::TopCenter: {
                auto row_wrap = row({
                    .justify_content = Justify::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {card},
                });

                return Positioned {
                    .child = row_wrap,
                    .top = StyleValue::point(margin - slide_offset),
                    .right = StyleValue::point(0.0f),
                    .left = StyleValue::point(0.0f),
                };
            }
            case SnackbarPlacement::TopRight: {
                return Positioned {
                    .child = card,
                    .top = StyleValue::point(margin - slide_offset),
                    .right = StyleValue::point(margin),
                };
            }
            case SnackbarPlacement::TopLeft: {
                return Positioned {
                    .child = card,
                    .top = StyleValue::point(margin - slide_offset),
                    .left = StyleValue::point(margin),
                };
            }
        }
        return card;
    }
};

std::unique_ptr<State> SnackbarCardWidget::createState() {
    return std::make_unique<SnackbarCardState>();
}

// ════════════════════════════════════════════════════════════════
// Snackbar Root Overlay State
// ════════════════════════════════════════════════════════════════

class SnackbarState : public State {
private:
    SnackbarOptions current_opts_;
    bool is_visible_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SnackbarWidget*>(widget());
        current_opts_ = w->initial_options;
        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const SnackbarWidget*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this](const SnackbarOptions& opts) { showSnackbar(opts); };
            w->controller->hide_fn = [this] { hideSnackbar(); };
            w->controller->is_open_fn = [this] { return is_visible_; };
        }
    }

    void showSnackbar(const SnackbarOptions& opts) {
        current_opts_ = opts;
        is_visible_ = true;
        setState([] {});
    }

    void hideSnackbar() {
        if (!is_visible_) return;
        is_visible_ = false;
        setState([] {});
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SnackbarWidget*>(widget());

        // ── 1. Invariant Page Body (Fill Stack directly) ─────────────
        WidgetPtr body_widget = w->body ? Positioned::fill(w->body) : Positioned::fill(container());

        if (!is_visible_) {
            return Stack {
                .width  = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = { body_widget },
            };
        }

        // ── 2. Floating Snackbar Card (Isolated Widget) ───────────────
        auto card_widget = std::make_shared<SnackbarCardWidget>(
            current_opts_,
            [this] { hideSnackbar(); }
        );

        return Stack {
            .width  = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {
                body_widget,
                card_widget
            }
        };
    }
};

std::unique_ptr<State> SnackbarWidget::createState() {
    return std::make_unique<SnackbarState>();
}

} // namespace enki
