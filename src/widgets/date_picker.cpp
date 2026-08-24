/// @file date_picker.cpp
/// @brief Implementation of Advanced DatePicker & DateRangePicker for ENKI Framework.

#include "enki/widgets/date_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>

namespace enki {

static const char* kMonthNames[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char* kMonthShort[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char* kWeekdaysShort[] = {
    "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"
};

std::string DateVal::formatIso() const {
    std::ostringstream ss;
    ss << year << "-"
       << std::setw(2) << std::setfill('0') << month << "-"
       << std::setw(2) << std::setfill('0') << day;
    return ss.str();
}

std::string DateVal::formatFormatted() const {
    if (month < 1 || month > 12) return formatIso();
    std::ostringstream ss;
    ss << kMonthShort[month - 1] << " " << day << ", " << year;
    return ss.str();
}

static bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int daysInMonth(int y, int m) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12) return 30;
    if (m == 2 && isLeapYear(y)) return 29;
    return kDays[m - 1];
}

// 0 = Mon, 1 = Tue, ..., 6 = Sun
static int dayOfWeek(int y, int m, int d) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    int dow = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
    return (dow + 6) % 7;
}

class DatePickerState : public State {
private:
    bool is_popup_open_ = false;
    DatePickerView current_view_ = DatePickerView::Days;

    int view_year_ = 2026;
    int view_month_ = 8;

    DateVal selected_date_ = {2026, 8, 19};
    DateRangeVal selected_range_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const DatePickerWidget*>(widget());
        selected_date_ = w->props.initial_date;
        selected_range_ = w->props.initial_range;
        view_year_ = selected_date_.year;
        view_month_ = selected_date_.month;

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const DatePickerWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->set_date_fn = [this](const DateVal& d) {
                selected_date_ = d;
                view_year_ = d.year;
                view_month_ = d.month;
                setState([] {});
            };
            w->props.controller->set_range_fn = [this](const DateRangeVal& r) {
                selected_range_ = r;
                if (r.start) {
                    view_year_ = r.start->year;
                    view_month_ = r.start->month;
                }
                setState([] {});
            };
            w->props.controller->open_fn = [this] {
                is_popup_open_ = true;
                setState([] {});
            };
            w->props.controller->close_fn = [this] {
                is_popup_open_ = false;
                setState([] {});
            };
            w->props.controller->clear_fn = [this] {
                selected_range_ = {};
                setState([] {});
            };
            w->props.controller->get_date_fn = [this] { return selected_date_; };
            w->props.controller->get_range_fn = [this] { return selected_range_; };
        }
    }

    void prevMonth() {
        view_month_--;
        if (view_month_ < 1) {
            view_month_ = 12;
            view_year_--;
        }
        setState([] {});
    }

    void nextMonth() {
        view_month_++;
        if (view_month_ > 12) {
            view_month_ = 1;
            view_year_++;
        }
        setState([] {});
    }

    // ── Build Header Bar (Month & Year Switcher) ───────────────────
    WidgetPtr buildHeader(const DatePickerWidget* w) {
        const auto& opts = w->props;

        auto makeNavArrow = [](std::string sym, std::function<void()> cb) -> WidgetPtr {
            auto t = text({
                .text = sym,
                .color = 0xFF94A3B8,
                .font_size = 13.0f,
            });
            auto b = container(t);
            b->paddingSymmetric(4.0f, 8.0f).borderRadius(4.0f);

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [cb](const TapUpDetails&) { if (cb) cb(); },
            });
        };

        auto btn_prev = makeNavArrow("◀", [this] {
            if (current_view_ == DatePickerView::Days) prevMonth();
            else if (current_view_ == DatePickerView::Months) { view_year_--; setState([] {}); }
            else if (current_view_ == DatePickerView::Years) { view_year_ -= 12; setState([] {}); }
        });

        auto btn_next = makeNavArrow("▶", [this] {
            if (current_view_ == DatePickerView::Days) nextMonth();
            else if (current_view_ == DatePickerView::Months) { view_year_++; setState([] {}); }
            else if (current_view_ == DatePickerView::Years) { view_year_ += 12; setState([] {}); }
        });

        // Center Title Button
        std::string title_str = "";
        if (current_view_ == DatePickerView::Days) {
            title_str = std::string(kMonthNames[view_month_ - 1]) + " " + std::to_string(view_year_);
        } else if (current_view_ == DatePickerView::Months) {
            title_str = "Year " + std::to_string(view_year_);
        } else {
            title_str = std::to_string(view_year_ - 5) + " — " + std::to_string(view_year_ + 6);
        }

        auto title_txt = text({
            .text = title_str,
            .color = opts.text_color,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        auto title_box = container(title_txt);
        title_box->paddingSymmetric(4.0f, 10.0f).borderRadius(6.0f);

        auto title_gd = gestureDetector({
            .child = title_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                if (current_view_ == DatePickerView::Days) current_view_ = DatePickerView::Months;
                else if (current_view_ == DatePickerView::Months) current_view_ = DatePickerView::Years;
                else current_view_ = DatePickerView::Days;
                setState([] {});
            },
        });

        return row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {btn_prev, title_gd, btn_next},
        });
    }

    // ── Build Days Grid ───────────────────────────────────────────
    WidgetPtr buildDaysGrid(const DatePickerWidget* w) {
        const auto& opts = w->props;

        // Weekday header row (Mo, Tu, We...)
        std::vector<WidgetPtr> wd_items;
        for (int i = 0; i < 7; ++i) {
            auto wd_txt = text({
                .text = kWeekdaysShort[i],
                .color = opts.muted_text_color,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            });

            auto wd_row = row({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .width = StyleValue::point(36.0f),
                .children = {wd_txt},
            });

            auto wd_box = container(wd_row);
            wd_items.push_back(wd_box);
        }
        auto wd_row = row({
            .justify_content = Justify::SpaceBetween,
            .width = StyleValue::percent(100.0f),
            .children = std::move(wd_items),
        });

        // Compute 42 calendar day cells (6 rows x 7 cols)
        int days_in_cur = daysInMonth(view_year_, view_month_);
        int first_dow = dayOfWeek(view_year_, view_month_, 1);

        int prev_m = (view_month_ == 1) ? 12 : (view_month_ - 1);
        int prev_y = (view_month_ == 1) ? (view_year_ - 1) : view_year_;
        int days_in_prev = daysInMonth(prev_y, prev_m);

        std::vector<WidgetPtr> calendar_rows = {wd_row};

        int day_num = 1;
        int next_m_day = 1;

        for (int r = 0; r < 6; ++r) {
            std::vector<WidgetPtr> row_cells;

            for (int c = 0; c < 7; ++c) {
                int cell_index = r * 7 + c;
                DateVal cur_date;
                bool is_current_month = true;

                if (cell_index < first_dow) {
                    // Previous month day
                    int d = days_in_prev - first_dow + cell_index + 1;
                    cur_date = DateVal{prev_y, prev_m, d};
                    is_current_month = false;
                } else if (day_num <= days_in_cur) {
                    // Current month day
                    cur_date = DateVal{view_year_, view_month_, day_num++};
                } else {
                    // Next month day
                    int next_m = (view_month_ == 12) ? 1 : (view_month_ + 1);
                    int next_y = (view_month_ == 12) ? (view_year_ + 1) : view_year_;
                    cur_date = DateVal{next_y, next_m, next_m_day++};
                    is_current_month = false;
                }

                // Check selection
                bool is_selected = false;
                bool is_in_range = false;

                if (opts.selection_mode == DatePickerSelectionMode::Single) {
                    is_selected = (cur_date == selected_date_);
                } else {
                    if (selected_range_.start && cur_date == *selected_range_.start) {
                        is_selected = true;
                    }
                    if (selected_range_.end && cur_date == *selected_range_.end) {
                        is_selected = true;
                    }
                    if (selected_range_.contains(cur_date)) {
                        is_in_range = true;
                    }
                }

                Color d_color = opts.text_color;
                FontWeight d_weight = FontWeight::Normal;
                if (is_selected) {
                    d_color = 0xFFFFFFFF;
                    d_weight = FontWeight::Bold;
                } else if (!is_current_month) {
                    d_color = 0xFF475569; // Dimmed
                }

                auto d_txt = text({
                    .text = std::to_string(cur_date.day),
                    .color = d_color,
                    .font_size = 12.0f,
                    .font_weight = d_weight,
                });

                auto d_row = row({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::percent(100.0f),
                    .children = {d_txt},
                });

                auto d_box = container(d_row);
                d_box->width(36.0f).height(32.0f).borderRadius(is_selected ? 16.0f : 4.0f);

                if (is_selected) {
                    d_box->color(opts.active_color);
                } else if (is_in_range) {
                    d_box->color(opts.range_fill_color);
                }

                auto d_gd = gestureDetector({
                    .child = d_box,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap_up = [this, cur_date, opts](const TapUpDetails&) {
                        if (opts.selection_mode == DatePickerSelectionMode::Single) {
                            selected_date_ = cur_date;
                            if (opts.on_date_selected) opts.on_date_selected(selected_date_);
                            if (opts.mode == DatePickerMode::InputPopup) {
                                is_popup_open_ = false;
                            }
                        } else {
                            // Range selection logic
                            if (!selected_range_.start || (selected_range_.start && selected_range_.end)) {
                                selected_range_.start = cur_date;
                                selected_range_.end = std::nullopt;
                            } else {
                                if (cur_date < *selected_range_.start) {
                                    selected_range_.end = selected_range_.start;
                                    selected_range_.start = cur_date;
                                } else {
                                    selected_range_.end = cur_date;
                                }
                                if (opts.on_range_selected) opts.on_range_selected(selected_range_);
                            }
                        }
                        setState([] {});
                    },
                });

                row_cells.push_back(d_gd);
            }

            auto r_row = row({
                .justify_content = Justify::SpaceBetween,
                .width = StyleValue::percent(100.0f),
                .children = std::move(row_cells),
            });
            calendar_rows.push_back(r_row);
        }

        return column({
            .gap = StyleValue::point(4.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(calendar_rows),
        });
    }

    // ── Build Months Grid ─────────────────────────────────────────
    WidgetPtr buildMonthsGrid(const DatePickerWidget* w) {
        std::vector<WidgetPtr> month_rows;

        for (int r = 0; r < 4; ++r) {
            std::vector<WidgetPtr> row_items;
            for (int c = 0; c < 3; ++c) {
                int m_idx = r * 3 + c;
                int m_num = m_idx + 1;
                bool is_active = (view_month_ == m_num);

                auto m_txt = text({
                    .text = kMonthShort[m_idx],
                    .color = is_active ? 0xFFFFFFFF : 0xFFCBD5E1,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold,
                });

                auto m_row_center = row({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {m_txt},
                });

                auto m_box = container(m_row_center);
                m_box->color(is_active ? 0xFF0284C7 : 0xFF0F172A)
                     .border(is_active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
                     .borderRadius(6.0f)
                     .paddingSymmetric(10.0f, 0.0f)
                     .width(76.0f);

                auto m_gd = gestureDetector({
                    .child = m_box,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap_up = [this, m_num](const TapUpDetails&) {
                        view_month_ = m_num;
                        current_view_ = DatePickerView::Days;
                        setState([] {});
                    },
                });

                row_items.push_back(m_gd);
            }
            auto m_row = row({
                .justify_content = Justify::SpaceBetween,
                .width = StyleValue::percent(100.0f),
                .children = std::move(row_items),
            });
            month_rows.push_back(m_row);
        }

        return column({
            .gap = StyleValue::point(8.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(month_rows),
        });
    }

    // ── Build Years Grid ──────────────────────────────────────────
    WidgetPtr buildYearsGrid(const DatePickerWidget* w) {
        std::vector<WidgetPtr> year_rows;
        int start_year = view_year_ - 5;

        for (int r = 0; r < 4; ++r) {
            std::vector<WidgetPtr> row_items;
            for (int c = 0; c < 3; ++c) {
                int y_num = start_year + (r * 3 + c);
                bool is_active = (view_year_ == y_num);

                auto y_txt = text({
                    .text = std::to_string(y_num),
                    .color = is_active ? 0xFFFFFFFF : 0xFFCBD5E1,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold,
                });

                auto y_row_center = row({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {y_txt},
                });

                auto y_box = container(y_row_center);
                y_box->color(is_active ? 0xFF0284C7 : 0xFF0F172A)
                     .border(is_active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
                     .borderRadius(6.0f)
                     .paddingSymmetric(10.0f, 0.0f)
                     .width(76.0f);

                auto y_gd = gestureDetector({
                    .child = y_box,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap_up = [this, y_num](const TapUpDetails&) {
                        view_year_ = y_num;
                        current_view_ = DatePickerView::Months;
                        setState([] {});
                    },
                });

                row_items.push_back(y_gd);
            }
            auto y_row = row({
                .justify_content = Justify::SpaceBetween,
                .width = StyleValue::percent(100.0f),
                .children = std::move(row_items),
            });
            year_rows.push_back(y_row);
        }

        return column({
            .gap = StyleValue::point(8.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(year_rows),
        });
    }

    // ── Build Quick Presets ───────────────────────────────────────
    WidgetPtr buildQuickPresets(const DatePickerWidget* w) {
        auto makePreset = [this](std::string label, std::function<void()> cb) -> WidgetPtr {
            auto t = text({
                .text = label,
                .color = 0xFF38BDF8,
                .font_size = 11.0f,
            });
            auto b = container(t);
            b->color(0xFF0F172A).borderRadius(4.0f).paddingSymmetric(4.0f, 8.0f);

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [cb](const TapUpDetails&) { if (cb) cb(); },
            });
        };

        auto p_today = makePreset("Today", [this] {
            selected_date_ = {2026, 8, 19};
            view_year_ = 2026; view_month_ = 8;
            auto* sw = static_cast<const DatePickerWidget*>(widget());
            if (sw->props.on_date_selected) sw->props.on_date_selected(selected_date_);
            setState([] {});
        });

        auto p_week = makePreset("+7 Days", [this] {
            selected_range_.start = DateVal{2026, 8, 19};
            selected_range_.end   = DateVal{2026, 8, 26};
            view_year_ = 2026; view_month_ = 8;
            auto* sw = static_cast<const DatePickerWidget*>(widget());
            if (sw->props.on_range_selected) sw->props.on_range_selected(selected_range_);
            setState([] {});
        });

        auto p_month = makePreset("This Month", [this] {
            selected_range_.start = DateVal{2026, 8, 1};
            selected_range_.end   = DateVal{2026, 8, 31};
            view_year_ = 2026; view_month_ = 8;
            auto* sw = static_cast<const DatePickerWidget*>(widget());
            if (sw->props.on_range_selected) sw->props.on_range_selected(selected_range_);
            setState([] {});
        });

        return row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(6.0f),
            .children = {p_today, p_week, p_month},
        });
    }

    // ── Build Full Calendar Card ──────────────────────────────────
    WidgetPtr buildCalendarCard(const DatePickerWidget* w) {
        const auto& opts = w->props;

        std::vector<WidgetPtr> card_items;
        card_items.push_back(buildHeader(w));

        auto div1 = container();
        div1->color(opts.border_color).height(1.0f).width(StyleValue::percent(100.0f));
        card_items.push_back(div1);

        if (current_view_ == DatePickerView::Days) {
            card_items.push_back(buildDaysGrid(w));
        } else if (current_view_ == DatePickerView::Months) {
            card_items.push_back(buildMonthsGrid(w));
        } else {
            card_items.push_back(buildYearsGrid(w));
        }

        if (opts.show_quick_presets) {
            auto div2 = container();
            div2->color(opts.border_color).height(1.0f).width(StyleValue::percent(100.0f));
            card_items.push_back(div2);
            card_items.push_back(buildQuickPresets(w));
        }

        auto card_col = column({
            .gap = StyleValue::point(10.0f),
            .width = StyleValue::point(280.0f),
            .children = std::move(card_items),
        });

        auto card_box = container(card_col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(10.0f)
                .paddingAll(14.0f)
                .width(308.0f)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        return card_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const DatePickerWidget*>(widget());
        const auto& opts = w->props;

        // ── Inline Mode ───────────────────────────────────────────────
        if (opts.mode == DatePickerMode::Inline) {
            return buildCalendarCard(w);
        }

        // ── Input Popup Mode ──────────────────────────────────────────
        std::string display_str;
        if (opts.selection_mode == DatePickerSelectionMode::Single) {
            display_str = "📅 " + selected_date_.formatFormatted();
        } else {
            if (selected_range_.start && selected_range_.end) {
                display_str = "📅 " + selected_range_.start->formatFormatted() + " ➔ " + selected_range_.end->formatFormatted();
            } else if (selected_range_.start) {
                display_str = "📅 " + selected_range_.start->formatFormatted() + " ➔ ...";
            } else {
                display_str = "📅 Select date range...";
            }
        }

        auto input_txt = text({
            .text = display_str,
            .color = 0xFFFFFFFF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });

        auto chev_txt = text({
            .text = is_popup_open_ ? "⌃" : "⌄",
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto in_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {input_txt, chev_txt},
        });

        auto input_box = container(in_row);
        input_box->color(0xFF1E293B)
                 .border(is_popup_open_ ? opts.active_color : opts.border_color, 1.0f)
                 .borderRadius(8.0f)
                 .paddingSymmetric(10.0f, 14.0f)
                 .width(320.0f);

        auto input_gd = gestureDetector({
            .child = input_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                is_popup_open_ = !is_popup_open_;
                setState([] {});
            },
        });

        std::vector<WidgetPtr> col_items = {input_gd};
        if (is_popup_open_) {
            auto cal_card = buildCalendarCard(w);
            col_items.push_back(cal_card);
        }

        return column({
            .gap = StyleValue::point(8.0f),
            .children = std::move(col_items),
        });
    }
};

std::unique_ptr<State> DatePickerWidget::createState() {
    return std::make_unique<DatePickerState>();
}

} // namespace enki
