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

class SnackbarState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;

    SnackbarOptions current_opts_;
    bool is_visible_ = false;
    bool is_hovered_ = false;

    double display_duration_sec_ = 4.0;
    double elapsed_sec_ = 0.0;
    std::chrono::steady_clock::time_point last_tick_time_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SnackbarWidget*>(widget());
        current_opts_ = w->initial_options;

        anim_.setDuration(std::chrono::milliseconds(180));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(0.0f);

        last_tick_time_ = std::chrono::steady_clock::now();

        ticker_ = createTicker([this] {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - last_tick_time_).count();
            last_tick_time_ = now;

            // Tick slide animation
            if (anim_.isAnimating()) {
                anim_.tick();
            }

            // Tick auto-dismiss timer
            if (is_visible_ && !anim_.isAnimating() && current_opts_.duration_ms > 0) {
                if (!(is_hovered_ && current_opts_.pause_on_hover)) {
                    elapsed_sec_ += dt;
                    if (elapsed_sec_ >= display_duration_sec_) {
                        hideSnackbar();
                    } else {
                        // Request frame repaint for smooth progress bar
                        setState([] {});
                    }
                }
            }
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
        auto* w = static_cast<const SnackbarWidget*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this](const SnackbarOptions& opts) { showSnackbar(opts); };
            w->controller->hide_fn = [this] { hideSnackbar(); };
            w->controller->is_open_fn = [this] { return is_visible_; };
        }
    }

    void showSnackbar(const SnackbarOptions& opts) {
        current_opts_ = opts;
        display_duration_sec_ = opts.duration_ms > 0 ? (opts.duration_ms / 1000.0) : 0.0;
        elapsed_sec_ = 0.0;
        is_visible_ = true;
        is_hovered_ = false;
        last_tick_time_ = std::chrono::steady_clock::now();

        anim_.forward();
        if (current_opts_.on_shown) current_opts_.on_shown();
        setState([] {});
    }

    void hideSnackbar() {
        if (!is_visible_) return;
        is_visible_ = false;
        anim_.reverse();
        if (current_opts_.on_dismissed) current_opts_.on_dismissed();
        setState([] {});
    }

    // ── Build Snackbar Card ───────────────────────────────────────

    WidgetPtr buildSnackbarCard(float t) {
        const auto& opts = current_opts_;

        // Left Section: Icon badge + Title / Message
        std::vector<WidgetPtr> left_items;

        if (!opts.icon.empty()) {
            auto ic_txt = text({
                .text = opts.icon,
                .font_size = 16.0f,
            });

            auto ic_box = container(ic_txt);
            ic_box->paddingAll(4.0f);
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

            auto act_box = container(act_txt);
            act_box->color(0x2238BDF8)
                   .borderRadius(4.0f)
                   .paddingSymmetric(4.0f, 10.0f);

            auto act_btn = gestureDetector({
                .child = act_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, act](const TapUpDetails&) {
                    if (act.on_click) act.on_click();
                    hideSnackbar();
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

            auto cls_box = container(cls_txt);
            cls_box->paddingAll(4.0f);

            auto cls_btn = gestureDetector({
                .child = cls_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    hideSnackbar();
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

            auto bar_fill = container();
            bar_fill->color(opts.progress_bar_col)
                    .height(2.5f)
                    .width(StyleValue::percent(progress_ratio * 100.0f))
                    .borderRadius(1.0f);

            auto bar_track = container(bar_fill);
            bar_track->color(0x33334155)
                     .height(2.5f)
                     .width(StyleValue::percent(100.0f))
                     .margin(EdgeInsets(6.0f, 0.0f, 0.0f, 0.0f));

            card_elements.push_back(bar_track);
        }

        auto card_col = column({
            .width = StyleValue::percent(100.0f),
            .children = std::move(card_elements),
        });

        auto card_box = container(card_col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(opts.border_radius)
                .paddingAll(14.0f)
                .width(opts.width)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 20.0f));

        // Hover Detector for Pause-on-Hover
        return gestureDetector({
            .child = card_box,
            .on_hover_enter = [this](const PointerEvent&) {
                is_hovered_ = true;
            },
            .on_hover_exit = [this](const PointerEvent&) {
                is_hovered_ = false;
            },
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SnackbarWidget*>(widget());
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

        // When hidden and not animating: render only body
        if (t <= 0.001f && !is_visible_) {
            return Stack {
                .width  = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = { body_widget },
            };
        }

        // ── 2. Floating Snackbar Card & Positioning ───────────────────
        auto card = buildSnackbarCard(t);

        WidgetPtr pos_card;
        float margin = opts.margin;

        // Slide entrance delta:
        float slide_offset = (1.0f - t) * -30.0f;

        switch (opts.placement) {
            case SnackbarPlacement::BottomCenter: {
                auto row_wrap = row({
                    .justify_content = Justify::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {card},
                });

                pos_card = Positioned {
                    .child = row_wrap,
                    .right = StyleValue::point(0.0f),
                    .bottom = StyleValue::point(margin + slide_offset),
                    .left = StyleValue::point(0.0f),
                };
                break;
            }
            case SnackbarPlacement::BottomRight: {
                pos_card = Positioned {
                    .child = card,
                    .right = StyleValue::point(margin),
                    .bottom = StyleValue::point(margin + slide_offset),
                };
                break;
            }
            case SnackbarPlacement::BottomLeft: {
                pos_card = Positioned {
                    .child = card,
                    .bottom = StyleValue::point(margin + slide_offset),
                    .left = StyleValue::point(margin),
                };
                break;
            }
            case SnackbarPlacement::TopCenter: {
                auto row_wrap = row({
                    .justify_content = Justify::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {card},
                });

                pos_card = Positioned {
                    .child = row_wrap,
                    .top = StyleValue::point(margin - slide_offset),
                    .right = StyleValue::point(0.0f),
                    .left = StyleValue::point(0.0f),
                };
                break;
            }
            case SnackbarPlacement::TopRight: {
                pos_card = Positioned {
                    .child = card,
                    .top = StyleValue::point(margin - slide_offset),
                    .right = StyleValue::point(margin),
                };
                break;
            }
            case SnackbarPlacement::TopLeft: {
                pos_card = Positioned {
                    .child = card,
                    .top = StyleValue::point(margin - slide_offset),
                    .left = StyleValue::point(margin),
                };
                break;
            }
        }

        // ── 3. Stack Composition: Body + Floating Snackbar ───────────
        return Stack {
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {
                body_widget,
                pos_card,
            }
        };
    }
};

std::unique_ptr<State> SnackbarWidget::createState() {
    return std::make_unique<SnackbarState>();
}

} // namespace enki
