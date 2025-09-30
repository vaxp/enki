/// @file calendar.cpp
/// @brief Implementation of Advanced Calendar widget for ENKI Framework.

#include "enki/widgets/calendar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <array>

namespace enki {

namespace {

bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int getDaysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    if (m >= 1 && m <= 12) return days[m - 1];
    return 30;
}

/// 0 = Sunday, 1 = Monday, ..., 6 = Saturday
int getDayOfWeek(int y, int m, int d) {
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

const char* getMonthName(int m) {
    static const char* names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (m >= 1 && m <= 12) return names[m - 1];
    return "Month";
}

} // namespace

class CalendarState : public State {
private:
    int view_year_ = 2026;
    int view_month_ = 8;

    CalendarDate selected_date_ = {2026, 8, 19};
    CalendarDate range_start_ = {2026, 8, 10};
    CalendarDate range_end_   = {2026, 8, 18};
    bool is_selecting_range_end_ = false;

    std::vector<CalendarEvent> all_events_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const CalendarWidget*>(widget());
        view_year_ = w->props.initial_date.year;
        view_month_ = w->props.initial_date.month;
        selected_date_ = w->props.initial_date;
        all_events_ = w->props.events;

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        auto* w = static_cast<const CalendarWidget*>(widget());
        all_events_ = w->props.events;
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const CalendarWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->select_date_fn = [this](CalendarDate d) {
                selected_date_ = d;
                view_year_ = d.year;
                view_month_ = d.month;
                setState([] {});
            };
            w->props.controller->select_range_fn = [this](CalendarDate s, CalendarDate e) {
                range_start_ = s;
                range_end_ = e;
                setState([] {});
            };
            w->props.controller->go_to_today_fn = [this] {
                view_year_ = 2026;
                view_month_ = 8;
                selected_date_ = {2026, 8, 19};
                setState([] {});
            };
            w->props.controller->next_month_fn = [this] { nextMonth(); };
            w->props.controller->prev_month_fn = [this] { prevMonth(); };
            w->props.controller->set_month_fn = [this](int y, int m) {
                view_year_ = y;
                view_month_ = m;
                setState([] {});
            };
            w->props.controller->add_event_fn = [this](CalendarEvent ev) {
                all_events_.push_back(std::move(ev));
                setState([] {});
            };
            w->props.controller->get_selected_date_fn = [this] { return selected_date_; };
        }
    }

    void nextMonth() {
        if (view_month_ >= 12) {
            view_month_ = 1;
            view_year_++;
        } else {
            view_month_++;
        }
        auto* w = static_cast<const CalendarWidget*>(widget());
        if (w->props.on_month_changed) w->props.on_month_changed(view_year_, view_month_);
        setState([] {});
    }

    void prevMonth() {
        if (view_month_ <= 1) {
            view_month_ = 12;
            view_year_--;
        } else {
            view_month_--;
        }
        auto* w = static_cast<const CalendarWidget*>(widget());
        if (w->props.on_month_changed) w->props.on_month_changed(view_year_, view_month_);
        setState([] {});
    }

    void handleDayClick(CalendarDate d) {
        auto* w = static_cast<const CalendarWidget*>(widget());

        if (w->props.selection_mode == CalendarSelectionMode::Single) {
            selected_date_ = d;
            if (w->props.on_date_selected) w->props.on_date_selected(d);
        } else if (w->props.selection_mode == CalendarSelectionMode::Range) {
            if (!is_selecting_range_end_ || d < range_start_) {
                range_start_ = d;
                range_end_ = d;
                is_selecting_range_end_ = true;
            } else {
                range_end_ = d;
                is_selecting_range_end_ = false;
                if (w->props.on_range_selected) w->props.on_range_selected(range_start_, range_end_);
            }
        }

        setState([] {});
    }

    // ── Build Single Day Cell ─────────────────────────────────────

    WidgetPtr buildDayCell(CalendarDate date, bool is_current_month, const CalendarProps& opts) {
        bool is_today = (date == CalendarDate{2026, 8, 19});

        bool is_selected = false;
        bool is_in_range = false;

        if (opts.selection_mode == CalendarSelectionMode::Single) {
            is_selected = (date == selected_date_);
        } else if (opts.selection_mode == CalendarSelectionMode::Range) {
            if (date == range_start_ || date == range_end_) {
                is_selected = true;
            } else if (date > range_start_ && date < range_end_) {
                is_in_range = true;
            }
        }

        // Find events for this date
        std::vector<CalendarEvent> date_events;
        for (const auto& ev : all_events_) {
            if (ev.date == date) date_events.push_back(ev);
        }

        // Day Number Text
        Color day_color = opts.day_color;
        FontWeight day_weight = FontWeight::Normal;
        if (is_selected) {
            day_color = 0xFFFFFFFF;
            day_weight = FontWeight::Bold;
        } else if (!is_current_month) {
            day_color = opts.adjacent_day_color;
        } else if (is_today) {
            day_color = opts.today_ring_color;
            day_weight = FontWeight::Bold;
        }
        auto day_txt = text({
            .text = std::to_string(date.day),
            .color = day_color,
            .font_size = 12.5f,
            .font_weight = day_weight,
        });

        std::vector<WidgetPtr> cell_col_items = {day_txt};

        // Event Dot Indicators (up to 3 dots)
        if (!date_events.empty()) {
            std::vector<WidgetPtr> dots;
            size_t dot_count = std::min(date_events.size(), size_t(3));
            for (size_t i = 0; i < dot_count; ++i) {
                auto dot = container({
                    .color = is_selected ? 0xFFFFFFFF : date_events[i].color,
                    .border_radius = BorderRadius::circular(2.0f),
                    .width = StyleValue::point(4.0f),
                    .height = StyleValue::point(4.0f),
                });
                dots.push_back(dot);
            }
            auto dot_row = row({
                .justify_content = Justify::Center,
                .gap = StyleValue::point(2.0f),
                .children = std::move(dots),
            });
            cell_col_items.push_back(dot_row);
        }

        auto cell_col = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(2.0f),
            .children = std::move(cell_col_items),
        });

        std::optional<Color> cell_bg;
        std::optional<BorderRadius> cell_radius;
        std::optional<Border> cell_border;

        if (is_selected) {
            cell_bg = opts.selected_bg_color;
            cell_radius = BorderRadius::circular(8.0f);
        } else if (is_in_range) {
            cell_bg = opts.in_range_bg_color;
            cell_radius = BorderRadius::circular(4.0f);
        } else if (is_today && opts.highlight_today) {
            cell_border = Border(opts.today_ring_color, 1.0f);
            cell_radius = BorderRadius::circular(8.0f);
        }

        auto cell_box = container({
            .color = cell_bg,
            .border_radius = cell_radius,
            .border = cell_border,
            .width = StyleValue::point(42.0f),
            .height = StyleValue::point(40.0f),
            .child = cell_col,
        });

        return gestureDetector({
            .child = cell_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this, date](const TapUpDetails&) {
                handleDayClick(date);
            },
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const CalendarWidget*>(widget());
        const auto& opts = w->props;

        // ── 1. Month Header Bar ───────────────────────────────────────
        std::string month_str = std::string(getMonthName(view_month_)) + " " + std::to_string(view_year_);
        auto month_txt = text({
            .text = month_str,
            .color = opts.title_color,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        // Header Navigation Buttons (◀ Today ▶)
        auto makeNavBtn = [](std::string sym, std::function<void()> cb) -> WidgetPtr {
            auto t = text({
                .text = sym,
                .color = 0xFF94A3B8,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            });

            auto b = container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(4.0f, 10.0f),
                .child = t,
            });

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [cb](const TapUpDetails&) {
                    if (cb) cb();
                },
            });
        };

        auto btn_prev = makeNavBtn("◀", [this] { prevMonth(); });
        auto btn_today = makeNavBtn("Today", [this] {
            view_year_ = 2026;
            view_month_ = 8;
            selected_date_ = {2026, 8, 19};
            setState([] {});
        });
        auto btn_next = makeNavBtn("▶", [this] { nextMonth(); });

        auto nav_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {btn_prev, btn_today, btn_next},
        });

        auto header_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {month_txt, nav_row},
        });

        // ── 2. Weekday Header Row ─────────────────────────────────────
        static const std::array<const char*, 7> weekdays = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
        std::vector<WidgetPtr> weekday_items;

        for (const auto* day_name : weekdays) {
            auto t = text({
                .text = day_name,
                .color = opts.weekday_color,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            });

            auto tc = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {t},
            });

            auto tb = container({
                .width = StyleValue::point(42.0f),
                .height = StyleValue::point(24.0f),
                .child = tc,
            });
            weekday_items.push_back(tb);
        }

        auto weekday_row = row({
            .justify_content = Justify::SpaceBetween,
            .width = StyleValue::percent(100.0f),
            .children = std::move(weekday_items),
        });

        // ── 3. 7x6 Monthly Day Matrix ─────────────────────────────────
        int days_in_cur_month = getDaysInMonth(view_year_, view_month_);

        int prev_m = (view_month_ == 1) ? 12 : (view_month_ - 1);
        int prev_y = (view_month_ == 1) ? (view_year_ - 1) : view_year_;
        int days_in_prev_month = getDaysInMonth(prev_y, prev_m);

        int next_m = (view_month_ == 12) ? 1 : (view_month_ + 1);
        int next_y = (view_month_ == 12) ? (view_year_ + 1) : view_year_;

        // First day of month (0 = Sun, 1 = Mon, ..., 6 = Sat)
        int first_dow = getDayOfWeek(view_year_, view_month_, 1);
        // Convert to Monday start (0 = Mon, ..., 6 = Sun)
        int lead_days = (first_dow == 0) ? 6 : (first_dow - 1);

        std::vector<WidgetPtr> grid_rows;
        int current_day_counter = 1;
        int next_month_counter = 1;

        for (int week = 0; week < 6; ++week) {
            std::vector<WidgetPtr> week_cells;

            for (int col = 0; col < 7; ++col) {
                int cell_index = week * 7 + col;

                if (cell_index < lead_days) {
                    // Previous Month Day
                    int d = days_in_prev_month - (lead_days - 1 - cell_index);
                    week_cells.push_back(buildDayCell({prev_y, prev_m, d}, false, opts));
                } else if (current_day_counter <= days_in_cur_month) {
                    // Current Month Day
                    week_cells.push_back(buildDayCell({view_year_, view_month_, current_day_counter}, true, opts));
                    current_day_counter++;
                } else {
                    // Next Month Day
                    week_cells.push_back(buildDayCell({next_y, next_m, next_month_counter}, false, opts));
                    next_month_counter++;
                }
            }

            auto w_row = row({
                .justify_content = Justify::SpaceBetween,
                .width = StyleValue::percent(100.0f),
                .children = std::move(week_cells),
            });
            grid_rows.push_back(w_row);
        }

        auto grid_col = column({
            .gap = StyleValue::point(4.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(grid_rows),
        });

        // ── 4. Assemble Calendar Container ────────────────────────────
        auto cal_col = column({
            .gap = StyleValue::point(14.0f),
            .width = StyleValue::percent(100.0f),
            .children = {header_row, weekday_row, grid_col},
        });

        auto cal_card = container({
            .color = opts.background_color,
            .border_radius = BorderRadius::circular(opts.border_radius),
            .border = Border(opts.border_color, 1.0f),
            .box_shadow = {BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f)},
            .width = StyleValue::point(opts.width),
            .padding = StyleInsets::all(16.0f),
            .child = cal_col,
        });

        return cal_card;
    }
};

std::unique_ptr<State> CalendarWidget::createState() {
    return std::make_unique<CalendarState>();
}

} // namespace enki
