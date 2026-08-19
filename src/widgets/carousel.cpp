/// @file carousel.cpp
/// @brief Implementation of Advanced Carousel widget for ENKI Framework.

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
        auto* w = static_cast<const Carousel*>(widget());
        current_index_ = std::clamp(w->options.initial_index, 0, std::max(0, (int)w->slides.size() - 1));
        auto_play_enabled_ = w->options.auto_play;
        interval_sec_ = w->options.auto_play_interval_ms > 0 ? (w->options.auto_play_interval_ms / 1000.0) : 3.5;

        last_tick_time_ = std::chrono::steady_clock::now();

        ticker_ = createTicker([this] {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - last_tick_time_).count();
            last_tick_time_ = now;

            auto* sw = static_cast<const Carousel*>(widget());
            if (auto_play_enabled_ && sw->slides.size() > 1) {
                if (!(is_hovered_ && sw->options.pause_on_hover)) {
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
        auto* w = static_cast<const Carousel*>(widget());
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
                auto* sw = static_cast<const Carousel*>(widget());
                return static_cast<int>(sw->slides.size());
            };
        }
    }

    void nextPage() {
        auto* w = static_cast<const Carousel*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total <= 1) return;

        if (current_index_ >= total - 1) {
            if (w->options.infinite_loop) current_index_ = 0;
            else return;
        } else {
            current_index_++;
        }

        elapsed_sec_ = 0.0;
        if (w->options.on_page_changed) w->options.on_page_changed(current_index_);
        setState([] {});
    }

    void previousPage() {
        auto* w = static_cast<const Carousel*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total <= 1) return;

        if (current_index_ <= 0) {
            if (w->options.infinite_loop) current_index_ = total - 1;
            else return;
        } else {
            current_index_--;
        }

        elapsed_sec_ = 0.0;
        if (w->options.on_page_changed) w->options.on_page_changed(current_index_);
        setState([] {});
    }

    void jumpToPage(int idx) {
        auto* w = static_cast<const Carousel*>(widget());
        int total = static_cast<int>(w->slides.size());
        if (total == 0) return;

        current_index_ = std::clamp(idx, 0, total - 1);
        elapsed_sec_ = 0.0;
        if (w->options.on_page_changed) w->options.on_page_changed(current_index_);
        setState([] {});
    }

    // ── Build Floating Navigation Arrow ───────────────────────────

    WidgetPtr buildArrowBtn(const std::string& arrow_symbol, std::function<void()> cb, const CarouselOptions& opts) {
        auto arr_txt = text(arrow_symbol);
        arr_txt->fontSize(14.0f).bold().color(opts.arrow_fg_color);

        auto arr_box = container(arr_txt);
        arr_box->color(opts.arrow_bg_color)
               .border(opts.border_color, 1.0f)
               .borderRadius(18.0f)
               .paddingSymmetric(8.0f, 12.0f);

        auto arr_gd = std::make_shared<GestureDetector>(arr_box);
        arr_gd->cursor_type = SystemCursor::Pointer;
        arr_gd->on_tap_up = [cb](const TapUpDetails&) {
            if (cb) cb();
        };

        return arr_gd;
    }

    // ── Build Bottom Pagination Dots ──────────────────────────────

    WidgetPtr buildPaginationDots(int total, const CarouselOptions& opts) {
        std::vector<WidgetPtr> dots;

        for (int i = 0; i < total; ++i) {
            bool is_active = (i == current_index_);

            auto dot_box = container();
            dot_box->color(is_active ? opts.indicator_active : opts.indicator_inactive)
                   .borderRadius(3.0f)
                   .height(6.0f)
                   .width(is_active ? 22.0f : 6.0f);

            auto dot_gd = std::make_shared<GestureDetector>(dot_box);
            dot_gd->cursor_type = SystemCursor::Pointer;
            dot_gd->on_tap_up = [this, i](const TapUpDetails&) {
                jumpToPage(i);
            };

            dots.push_back(dot_gd);
        }

        auto dots_row = row(dots);
        dots_row->gap(StyleValue::point(6.0f))
                .justifyContent(Justify::Center)
                .alignItems(Align::Center);

        return dots_row;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const Carousel*>(widget());
        const auto& opts = w->options;
        int total = static_cast<int>(w->slides.size());

        if (total == 0) {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f)).height(opts.height);
            return empty;
        }

        // ── 1. Active Slide Content ───────────────────────────────────
        WidgetPtr active_slide_widget = w->slides[std::clamp(current_index_, 0, total - 1)];

        auto slide_box = container(active_slide_widget);
        slide_box->color(opts.background_color)
                 .border(opts.border_color, 1.0f)
                 .borderRadius(opts.border_radius)
                 .width(StyleValue::percent(100.0f))
                 .height(opts.height);

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
        if (opts.show_arrows && total > 1) {
            // Left Arrow
            auto left_btn = buildArrowBtn("◀", [this] { previousPage(); }, opts);
            std::vector<WidgetPtr> l_items = {left_btn};
            auto l_col = column(l_items);
            l_col->justifyContent(Justify::Center).height(StyleValue::percent(100.0f));

            auto pos_left = std::make_shared<Positioned>(l_col);
            pos_left->style.left = StyleValue::point(14.0f);
            pos_left->style.top = StyleValue::point(0.0f);
            pos_left->style.bottom = StyleValue::point(0.0f);
            stack_items.push_back(pos_left);

            // Right Arrow
            auto right_btn = buildArrowBtn("▶", [this] { nextPage(); }, opts);
            std::vector<WidgetPtr> r_items = {right_btn};
            auto r_col = column(r_items);
            r_col->justifyContent(Justify::Center).height(StyleValue::percent(100.0f));

            auto pos_right = std::make_shared<Positioned>(r_col);
            pos_right->style.right = StyleValue::point(14.0f);
            pos_right->style.top = StyleValue::point(0.0f);
            pos_right->style.bottom = StyleValue::point(0.0f);
            stack_items.push_back(pos_right);
        }

        // ── 3. Bottom Pagination Dots ─────────────────────────────────
        if (opts.show_indicators && total > 1) {
            auto dots_widget = buildPaginationDots(total, opts);

            std::vector<WidgetPtr> dot_items = {dots_widget};
            auto dot_row = row(dot_items);
            dot_row->justifyContent(Justify::Center).width(StyleValue::percent(100.0f));

            auto pos_dots = std::make_shared<Positioned>(dot_row);
            pos_dots->style.bottom = StyleValue::point(14.0f);
            pos_dots->style.left = StyleValue::point(0.0f);
            pos_dots->style.right = StyleValue::point(0.0f);
            stack_items.push_back(pos_dots);
        }

        auto root = stack(stack_items);
        root->style.width = StyleValue::percent(100.0f);
        root->style.height = StyleValue::point(opts.height);

        return root;
    }
};

std::unique_ptr<State> Carousel::createState() {
    return std::make_unique<CarouselState>();
}

} // namespace enki
