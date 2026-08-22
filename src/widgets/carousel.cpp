#include "enki/widgets/carousel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>

namespace enki {

class CarouselState : public State {
private:
    int current_index_ = 0;
    bool is_hovered_ = false;
    bool auto_play_enabled_ = true;

    double elapsed_sec_ = 0.0;
    double interval_sec_ = 3.5;
    std::chrono::steady_clock::time_point last_tick_time_;
    std::unique_ptr<Ticker> ticker_;

    float pan_start_x_ = 0.0f;
    float pan_accum_x_ = 0.0f;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const CarouselWidget*>(widget());
        current_index_ = std::clamp(w->initial_index, 0, std::max(0, (int)w->slides.size() - 1));
        auto_play_enabled_ = w->auto_play;
        interval_sec_ = w->auto_play_interval_ms > 0 ? (w->auto_play_interval_ms / 1000.0) : 3.5;

        last_tick_time_ = std::chrono::steady_clock::now();

        ticker_ = createTicker([this] {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - last_tick_time_).count();
            last_tick_time_ = now;

            auto* sw = static_cast<const CarouselWidget*>(widget());
            if (auto_play_enabled_ && sw->slides.size() > 1) {
                if (!(is_hovered_ && sw->pause_on_hover)) {
                    elapsed_sec_ += dt;
                    if (elapsed_sec_ >= interval_sec_) {
                        nextPage();
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
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const CarouselWidget*>(widget());
        if (w->controller) {
            w->controller->next_page_fn = [this] { nextPage(); };
            w->controller->prev_page_fn = [this] { previousPage(); };
            w->controller->jump_to_page_fn = [this](int idx) { jumpToPage(idx); };
            w->controller->set_auto_play_fn = [this](bool play) {
                auto_play_enabled_ = play;
                elapsed_sec_ = 0.0;
                setState([] {});
            };
            w->controller->get_current_page_fn = [this] { return current_index_; };
            w->controller->get_page_count_fn = [this] {
                auto* sw = static_cast<const CarouselWidget*>(widget());
                return static_cast<int>(sw->slides.size());
            };
        }
    }

    void nextPage() {
        auto* w = static_cast<const CarouselWidget*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total <= 1) return;

        if (current_index_ >= total - 1) {
            if (w->infinite_loop) current_index_ = 0;
            else return;
        } else {
            current_index_++;
        }

        elapsed_sec_ = 0.0;
        if (w->on_page_changed) w->on_page_changed(current_index_);
        setState([] {});
    }

    void previousPage() {
        auto* w = static_cast<const CarouselWidget*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total <= 1) return;

        if (current_index_ <= 0) {
            if (w->infinite_loop) current_index_ = total - 1;
            else return;
        } else {
            current_index_--;
        }

        elapsed_sec_ = 0.0;
        if (w->on_page_changed) w->on_page_changed(current_index_);
        setState([] {});
    }

    void jumpToPage(int idx) {
        auto* w = static_cast<const CarouselWidget*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total == 0) return;

        current_index_ = std::clamp(idx, 0, total - 1);
        elapsed_sec_ = 0.0;
        if (w->on_page_changed) w->on_page_changed(current_index_);
        setState([] {});
    }

    // ── Build Floating Navigation Arrow ───────────────────────────

    WidgetPtr buildArrowBtn(const std::string& arrow_symbol, std::function<void()> cb, const CarouselWidget* opts) {
        auto arr_txt = text(arrow_symbol, { .color = opts->arrow_fg_color, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto arr_box = container({
            .color = opts->arrow_bg_color,
            .border_radius = BorderRadius::circular(18.0f),
            .border = Border(opts->border_color, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = arr_txt
        });

        auto arr_gd = std::make_shared<GestureDetector>(arr_box);
        arr_gd->cursor_type = SystemCursor::Pointer;
        arr_gd->on_tap_up = [cb](const TapUpDetails&) {
            if (cb) cb();
        };

        return arr_gd;
    }

    // ── Build Bottom Pagination Dots ──────────────────────────────

    WidgetPtr buildPaginationDots(int total, const CarouselWidget* opts) {
        std::vector<WidgetPtr> dots;

        for (int i = 0; i < total; ++i) {
            bool is_active = (i == current_index_);

            auto dot_box = container({
                .color = is_active ? opts->indicator_active : opts->indicator_inactive,
                .border_radius = BorderRadius::circular(3.0f),
                .width = StyleValue::point(is_active ? 22.0f : 6.0f),
                .height = StyleValue::point(6.0f)
            });

            auto dot_gd = std::make_shared<GestureDetector>(dot_box);
            dot_gd->cursor_type = SystemCursor::Pointer;
            dot_gd->on_tap_up = [this, i](const TapUpDetails&) {
                jumpToPage(i);
            };

            dots.push_back(dot_gd);
        }

        return row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = dots
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* opts = static_cast<const CarouselWidget*>(widget());
        int total = static_cast<int>(opts->slides.size());

        if (total == 0) {
            return container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::point(opts->height)
            });
        }

        // ── 1. Active Slide Content ───────────────────────────────────
        WidgetPtr active_slide_widget = opts->slides[std::clamp(current_index_, 0, total - 1)];

        auto slide_box = container({
            .color = opts->background_color,
            .border_radius = BorderRadius::circular(opts->border_radius),
            .border = Border(opts->border_color, 1.0f),
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(opts->height),
            .child = active_slide_widget
        });

        // Slide wrapper with hover and swipe gesture tracking
        auto gesture_wrapper = std::make_shared<GestureDetector>(slide_box);

        gesture_wrapper->on_hover_enter = [this](const PointerEvent&) {
            is_hovered_ = true;
            setState([] {});
        };

        gesture_wrapper->on_hover_exit = [this](const PointerEvent&) {
            is_hovered_ = false;
            setState([] {});
        };

        gesture_wrapper->on_pan_start = [this](const DragStartDetails& d) {
            pan_start_x_ = d.global_position.x;
            pan_accum_x_ = 0.0f;
        };

        gesture_wrapper->on_pan_update = [this](const DragUpdateDetails& d) {
            pan_accum_x_ += d.delta.x;
        };

        gesture_wrapper->on_pan_end = [this](const DragEndDetails&) {
            if (pan_accum_x_ < -50.0f) {
                nextPage(); // Swiped left -> advance
            } else if (pan_accum_x_ > 50.0f) {
                previousPage(); // Swiped right -> go back
            }
        };

        auto pos_slide = Positioned::fill(gesture_wrapper);
        std::vector<WidgetPtr> stack_items = {pos_slide};

        // ── 2. Floating Navigation Arrows (◀ / ▶) ─────────────────────
        if (opts->show_arrows && total > 1) {
            // Left Arrow
            auto left_btn = buildArrowBtn("◀", [this] { previousPage(); }, opts);
            auto l_col = column({
                .justify_content = Justify::Center,
                .height = StyleValue::percent(100.0f),
                .children = {left_btn}
            });

            auto pos_left = std::make_shared<Positioned>(l_col);
            pos_left->style.left = StyleValue::point(14.0f);
            pos_left->style.top = StyleValue::point(0.0f);
            pos_left->style.bottom = StyleValue::point(0.0f);
            stack_items.push_back(pos_left);

            // Right Arrow
            auto right_btn = buildArrowBtn("▶", [this] { nextPage(); }, opts);
            auto r_col = column({
                .justify_content = Justify::Center,
                .height = StyleValue::percent(100.0f),
                .children = {right_btn}
            });

            auto pos_right = std::make_shared<Positioned>(r_col);
            pos_right->style.right = StyleValue::point(14.0f);
            pos_right->style.top = StyleValue::point(0.0f);
            pos_right->style.bottom = StyleValue::point(0.0f);
            stack_items.push_back(pos_right);
        }

        // ── 3. Bottom Pagination Dots ─────────────────────────────────
        if (opts->show_indicators && total > 1) {
            auto dots_widget = buildPaginationDots(total, opts);

            auto dot_row = row({
                .justify_content = Justify::Center,
                .width = StyleValue::percent(100.0f),
                .children = {dots_widget}
            });

            auto pos_dots = std::make_shared<Positioned>(dot_row);
            pos_dots->style.bottom = StyleValue::point(14.0f);
            pos_dots->style.left = StyleValue::point(0.0f);
            pos_dots->style.right = StyleValue::point(0.0f);
            stack_items.push_back(pos_dots);
        }

        auto root = stack(stack_items);
        root->style.width = StyleValue::percent(100.0f);
        root->style.height = StyleValue::point(opts->height);

        return root;
    }
};

std::unique_ptr<State> CarouselWidget::createState() {
    return std::make_unique<CarouselState>();
}

} // namespace enki
