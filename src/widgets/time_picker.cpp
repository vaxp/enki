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
        auto* w = static_cast<const TimePicker*>(widget());
        time_ = w->props.initial_time;
        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const TimePicker*>(widget());
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
        auto* w = static_cast<const TimePicker*>(widget());
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
        auto* w = static_cast<const TimePicker*>(widget());
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
        auto up_txt = text("▲");
        up_txt->fontSize(11.0f).color(0xFF94A3B8);
        auto up_row = row(std::vector<WidgetPtr>{up_txt});
        up_row->justifyContent(Justify::Center).alignItems(Align::Center).width(StyleValue::percent(100.0f));
        auto up_box = container(up_row);
        up_box->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 10.0f).width(54.0f);
        auto up_gd = std::make_shared<GestureDetector>(up_box);
        up_gd->cursor_type = SystemCursor::Pointer;
        up_gd->on_tap_up = [on_up](const TapUpDetails&) { if (on_up) on_up(); };

        // Value Display Card
        auto val_txt = text(value_str);
        val_txt->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto val_row = row(std::vector<WidgetPtr>{val_txt});
        val_row->justifyContent(Justify::Center).alignItems(Align::Center).width(StyleValue::percent(100.0f));
        auto val_box = container(val_row);
        val_box->color(0xFF0F172A).border(0xFF0284C7, 1.5f).borderRadius(6.0f).paddingSymmetric(8.0f, 6.0f).width(54.0f);

        // Down arrow button
        auto down_txt = text("▼");
        down_txt->fontSize(11.0f).color(0xFF94A3B8);
        auto down_row = row(std::vector<WidgetPtr>{down_txt});
        down_row->justifyContent(Justify::Center).alignItems(Align::Center).width(StyleValue::percent(100.0f));
        auto down_box = container(down_row);
        down_box->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 10.0f).width(54.0f);
        auto down_gd = std::make_shared<GestureDetector>(down_box);
        down_gd->cursor_type = SystemCursor::Pointer;
        down_gd->on_tap_up = [on_down](const TapUpDetails&) { if (on_down) on_down(); };

        // Subtitle Label
        auto lbl_txt = text(label_str);
        lbl_txt->fontSize(10.0f).bold().color(0xFF64748B);
        auto lbl_row = row(std::vector<WidgetPtr>{lbl_txt});
        lbl_row->justifyContent(Justify::Center).alignItems(Align::Center).width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> col_items = {up_gd, val_box, down_gd, lbl_row};
        auto col = column(col_items);
        col->gap(StyleValue::point(6.0f)).alignItems(Align::Center);
        return col;
    }

    // ── Build AM / PM Segmented Switch ────────────────────────────
    WidgetPtr buildAmPmSwitch() {
        auto makePill = [this](std::string label, bool active, bool is_pm_val) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(12.5f).bold().color(active ? 0xFFFFFFFF : 0xFF94A3B8);

            auto r = row(std::vector<WidgetPtr>{t});
            r->justifyContent(Justify::Center).alignItems(Align::Center).width(StyleValue::percent(100.0f));

            auto b = container(r);
            b->color(active ? 0xFF0284C7 : 0xFF0F172A)
             .border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(10.0f, 12.0f)
             .width(52.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, is_pm_val](const TapUpDetails&) {
                toggleAmPm(is_pm_val);
            };
            return gd;
        };

        auto pill_am = makePill("AM", !time_.is_pm, false);
        auto pill_pm = makePill("PM", time_.is_pm, true);

        std::vector<WidgetPtr> items = {pill_am, pill_pm};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f)).justifyContent(Justify::Center);
        return col;
    }

    // ── Build Quick Presets Bar ───────────────────────────────────
    WidgetPtr buildQuickPresets() {
        auto makePreset = [this](std::string label, int h, int m, bool pm) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(11.0f).color(0xFF38BDF8);
            auto b = container(t);
            b->color(0xFF0F172A).borderRadius(4.0f).paddingSymmetric(4.0f, 8.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, h, m, pm](const TapUpDetails&) {
                time_.hour = h;
                time_.minute = m;
                time_.is_pm = pm;
                notifyChanged();
                setState([] {});
            };
            return gd;
        };

        auto p_now = makePreset("Now", 8, 30, true);
        auto p_morn = makePreset("09:00 AM", 9, 0, false);
        auto p_noon = makePreset("12:00 PM", 12, 0, true);
        auto p_eve = makePreset("06:00 PM", 6, 0, true);

        std::vector<WidgetPtr> p_items = {p_now, p_morn, p_noon, p_eve};
        auto p_row = row(p_items);
        p_row->gap(StyleValue::point(6.0f)).justifyContent(Justify::Center);
        return p_row;
    }

    // ── Build Time Picker Card ────────────────────────────────────
    WidgetPtr buildTimePickerCard(const TimePicker* w) {
        const auto& opts = w->props;
        bool is_24h = (opts.format == TimeFormat::TwentyFourHour);

        // Header Title
        auto t_title = text("🕒 Select Time");
        t_title->fontSize(13.5f).bold().color(0xFFFFFFFF);

        std::ostringstream ss_h, ss_m, ss_s;
        ss_h << std::setw(2) << std::setfill('0') << time_.hour;
        ss_m << std::setw(2) << std::setfill('0') << time_.minute;
        ss_s << std::setw(2) << std::setfill('0') << time_.second;

        // Stepper Columns
        auto col_hour = buildStepperColumn(ss_h.str(), "HOUR",
            [this, is_24h] { stepHour(1, is_24h); },
            [this, is_24h] { stepHour(-1, is_24h); });

        auto colon_txt1 = text(":");
        colon_txt1->fontSize(22.0f).bold().color(0xFF94A3B8);
        auto colon_box1 = container(colon_txt1);
        colon_box1->paddingSymmetric(18.0f, 4.0f);

        auto col_min = buildStepperColumn(ss_m.str(), "MINUTE",
            [this] { stepMinute(1); },
            [this] { stepMinute(-1); });

        std::vector<WidgetPtr> stepper_items = {col_hour, colon_box1, col_min};

        if (opts.show_seconds) {
            auto colon_txt2 = text(":");
            colon_txt2->fontSize(22.0f).bold().color(0xFF94A3B8);
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

        auto steppers_row = row(stepper_items);
        steppers_row->justifyContent(Justify::Center).alignItems(Align::Center).gap(StyleValue::point(6.0f));

        std::vector<WidgetPtr> card_items = {t_title, steppers_row};

        if (opts.show_quick_presets) {
            auto div_h = container();
            div_h->color(0xFF334155).height(1.0f).width(StyleValue::percent(100.0f));
            card_items.push_back(div_h);
            card_items.push_back(buildQuickPresets());
        }

        auto card_col = column(card_items);
        card_col->gap(StyleValue::point(14.0f)).alignItems(Align::Center);

        auto card_box = container(card_col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(10.0f)
                .paddingAll(16.0f)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        return card_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const TimePicker*>(widget());
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

        auto input_txt = text(display_str);
        input_txt->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto chev_txt = text(is_popup_open_ ? "⌃" : "⌄");
        chev_txt->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> in_items = {input_txt, chev_txt};
        auto in_row = row(in_items);
        in_row->justifyContent(Justify::SpaceBetween)
              .alignItems(Align::Center)
              .width(StyleValue::percent(100.0f));

        auto input_box = container(in_row);
        input_box->color(0xFF1E293B)
                 .border(is_popup_open_ ? opts.active_color : opts.border_color, 1.0f)
                 .borderRadius(8.0f)
                 .paddingSymmetric(10.0f, 14.0f)
                 .width(280.0f);

        auto input_gd = std::make_shared<GestureDetector>(input_box);
        input_gd->cursor_type = SystemCursor::Pointer;
        input_gd->on_tap_up = [this](const TapUpDetails&) {
            is_popup_open_ = !is_popup_open_;
            setState([] {});
        };

        std::vector<WidgetPtr> col_items = {input_gd};
        if (is_popup_open_) {
            col_items.push_back(buildTimePickerCard(w));
        }

        auto full_col = column(col_items);
        full_col->gap(StyleValue::point(8.0f));
        return full_col;
    }
};

std::unique_ptr<State> TimePicker::createState() {
    return std::make_unique<TimePickerState>();
}

} // namespace enki
