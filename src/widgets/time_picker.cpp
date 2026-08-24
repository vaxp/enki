/// @file time_picker.cpp
/// @brief Implementation of Advanced TimePicker for ENKI Framework.

#include "enki/widgets/time_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>

namespace enki {

std::string TimeVal::format12h(bool show_seconds) const {
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << hour << ":"
       << std::setw(2) << std::setfill('0') << minute;
    if (show_seconds) {
        ss << ":" << std::setw(2) << std::setfill('0') << second;
    }
    ss << " " << (is_pm ? "PM" : "AM");
    return ss.str();
}

std::string TimeVal::format24h(bool show_seconds) const {
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << hour << ":"
       << std::setw(2) << std::setfill('0') << minute;
    if (show_seconds) {
        ss << ":" << std::setw(2) << std::setfill('0') << second;
    }
    return ss.str();
}

class TimePickerState : public State {
private:
    bool is_popup_open_ = false;
    TimeVal time_ = {8, 30, 0, true};

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const TimePickerWidget*>(widget());
        time_ = w->props.initial_time;
        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const TimePickerWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->set_time_fn = [this](const TimeVal& t) {
                time_ = t;
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
            w->props.controller->get_time_fn = [this] { return time_; };
        }
    }

    void notifyChanged() {
        auto* w = static_cast<const TimePickerWidget*>(widget());
        if (w->props.on_time_selected) {
            w->props.on_time_selected(time_);
        }
    }

    // ── Stepper Math Helpers ──────────────────────────────────────
    void stepHour(int delta, bool is_24h) {
        if (is_24h) {
            time_.hour = (time_.hour + delta + 24) % 24;
        } else {
            time_.hour += delta;
            if (time_.hour > 12) time_.hour = 1;
            else if (time_.hour < 1) time_.hour = 12;
        }
        notifyChanged();
        setState([] {});
    }

    void stepMinute(int delta) {
        auto* w = static_cast<const TimePickerWidget*>(widget());
        int step = (w->props.minute_step > 0) ? w->props.minute_step : 1;
        time_.minute = (time_.minute + delta * step + 60) % 60;
        notifyChanged();
        setState([] {});
    }

    void stepSecond(int delta) {
        time_.second = (time_.second + delta + 60) % 60;
        notifyChanged();
        setState([] {});
    }

    void toggleAmPm(bool pm) {
        if (time_.is_pm != pm) {
            time_.is_pm = pm;
            notifyChanged();
            setState([] {});
        }
    }

    // ── Build Single Stepper Column ───────────────────────────────
    WidgetPtr buildStepperColumn(std::string value_str, std::string label_str,
                                 std::function<void()> on_up, std::function<void()> on_down) {
        // Up arrow button
        auto up_txt = text({
            .text = "▲",
            .color = 0xFF94A3B8,
            .font_size = 11.0f,
        });
        auto up_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {up_txt},
        });
        auto up_box = container(up_row);
        up_box->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 10.0f).width(54.0f);
        auto up_gd = gestureDetector({
            .child = up_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [on_up](const TapUpDetails&) { if (on_up) on_up(); },
        });

        // Value Display Card
        auto val_txt = text({
            .text = value_str,
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });
        auto val_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {val_txt},
        });
        auto val_box = container(val_row);
        val_box->color(0xFF0F172A).border(0xFF0284C7, 1.5f).borderRadius(6.0f).paddingSymmetric(8.0f, 6.0f).width(54.0f);

        // Down arrow button
        auto down_txt = text({
            .text = "▼",
            .color = 0xFF94A3B8,
            .font_size = 11.0f,
        });
        auto down_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {down_txt},
        });
        auto down_box = container(down_row);
        down_box->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 10.0f).width(54.0f);
        auto down_gd = gestureDetector({
            .child = down_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [on_down](const TapUpDetails&) { if (on_down) on_down(); },
        });

        // Subtitle Label
        auto lbl_txt = text({
            .text = label_str,
            .color = 0xFF64748B,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        });
        auto lbl_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {lbl_txt},
        });

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {up_gd, val_box, down_gd, lbl_row},
        });
    }

    // ── Build AM / PM Segmented Switch ────────────────────────────
    WidgetPtr buildAmPmSwitch() {
        auto makePill = [this](std::string label, bool active, bool is_pm_val) -> WidgetPtr {
            auto t = text({
                .text = label,
                .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 12.5f,
                .font_weight = FontWeight::Bold,
            });

            auto r = row({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .children = {t},
            });

            auto b = container(r);
            b->color(active ? 0xFF0284C7 : 0xFF0F172A)
             .border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(10.0f, 12.0f)
             .width(52.0f);

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, is_pm_val](const TapUpDetails&) {
                    toggleAmPm(is_pm_val);
                },
            });
        };

        auto pill_am = makePill("AM", !time_.is_pm, false);
        auto pill_pm = makePill("PM", time_.is_pm, true);

        return column({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(8.0f),
            .children = {pill_am, pill_pm},
        });
    }

    // ── Build Quick Presets Bar ───────────────────────────────────
    WidgetPtr buildQuickPresets() {
        auto makePreset = [this](std::string label, int h, int m, bool pm) -> WidgetPtr {
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
                .on_tap_up = [this, h, m, pm](const TapUpDetails&) {
                    time_.hour = h;
                    time_.minute = m;
                    time_.is_pm = pm;
                    notifyChanged();
                    setState([] {});
                },
            });
        };

        auto p_now = makePreset("Now", 8, 30, true);
        auto p_morn = makePreset("09:00 AM", 9, 0, false);
        auto p_noon = makePreset("12:00 PM", 12, 0, true);
        auto p_eve = makePreset("06:00 PM", 6, 0, true);

        return row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(6.0f),
            .children = {p_now, p_morn, p_noon, p_eve},
        });
    }

    // ── Build Time Picker Card ────────────────────────────────────
    WidgetPtr buildTimePickerCard(const TimePickerWidget* w) {
        const auto& opts = w->props;
        bool is_24h = (opts.format == TimeFormat::TwentyFourHour);

        // Header Title
        auto t_title = text({
            .text = "🕒 Select Time",
            .color = 0xFFFFFFFF,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        std::ostringstream ss_h, ss_m, ss_s;
        ss_h << std::setw(2) << std::setfill('0') << time_.hour;
        ss_m << std::setw(2) << std::setfill('0') << time_.minute;
        ss_s << std::setw(2) << std::setfill('0') << time_.second;

        // Stepper Columns
        auto col_hour = buildStepperColumn(ss_h.str(), "HOUR",
            [this, is_24h] { stepHour(1, is_24h); },
            [this, is_24h] { stepHour(-1, is_24h); });

        auto colon_txt1 = text({
            .text = ":",
            .color = 0xFF94A3B8,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });
        auto colon_box1 = container(colon_txt1);
        colon_box1->paddingSymmetric(18.0f, 4.0f);

        auto col_min = buildStepperColumn(ss_m.str(), "MINUTE",
            [this] { stepMinute(1); },
            [this] { stepMinute(-1); });

        std::vector<WidgetPtr> stepper_items = {col_hour, colon_box1, col_min};

        if (opts.show_seconds) {
            auto colon_txt2 = text({
                .text = ":",
                .color = 0xFF94A3B8,
                .font_size = 22.0f,
                .font_weight = FontWeight::Bold,
            });
            auto colon_box2 = container(colon_txt2);
            colon_box2->paddingSymmetric(18.0f, 4.0f);

            auto col_sec = buildStepperColumn(ss_s.str(), "SECOND",
                [this] { stepSecond(1); },
                [this] { stepSecond(-1); });

            stepper_items.push_back(colon_box2);
            stepper_items.push_back(col_sec);
        }

        if (!is_24h) {
            auto div_v = container();
            div_v->color(0xFF334155).width(1.0f).height(74.0f).paddingSymmetric(0.0f, 6.0f);
            stepper_items.push_back(div_v);
            stepper_items.push_back(buildAmPmSwitch());
        }

        auto steppers_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = std::move(stepper_items),
        });

        std::vector<WidgetPtr> card_items = {t_title, steppers_row};

        if (opts.show_quick_presets) {
            auto div_h = container();
            div_h->color(0xFF334155).height(1.0f).width(StyleValue::percent(100.0f));
            card_items.push_back(div_h);
            card_items.push_back(buildQuickPresets());
        }

        auto card_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(14.0f),
            .children = std::move(card_items),
        });

        auto card_box = container(card_col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(10.0f)
                .paddingAll(16.0f)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        return card_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const TimePickerWidget*>(widget());
        const auto& opts = w->props;

        if (opts.mode == TimePickerMode::Inline) {
            return buildTimePickerCard(w);
        }

        // Input Popup Mode
        std::string display_str;
        if (opts.format == TimeFormat::TwelveHour) {
            display_str = "🕒 " + time_.format12h(opts.show_seconds);
        } else {
            display_str = "🕒 " + time_.format24h(opts.show_seconds);
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
                 .width(280.0f);

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
            col_items.push_back(buildTimePickerCard(w));
        }

        return column({
            .gap = StyleValue::point(8.0f),
            .children = std::move(col_items),
        });
    }
};

std::unique_ptr<State> TimePickerWidget::createState() {
    return std::make_unique<TimePickerState>();
}

} // namespace enki
