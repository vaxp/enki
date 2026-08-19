/// @file main.cpp
/// @brief ENKI Advanced TimePicker Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/time_picker.hpp"
#include "enki/widgets/text_field.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class TimePickerDemoState : public State {
private:
    std::string hud_msg_ = "Click the input field on the left to open the 12-Hour TimePicker dropdown, or use the 24-Hour Inline Picker with Seconds on the right.";

    TimeVal flight_time_ = {8, 45, 0, true};
    TimeVal backup_time_ = {23, 15, 30, true};

    std::shared_ptr<TextFieldController> flight_no_ctrl_;

public:
    void initState() override {
        State::initState();
        flight_no_ctrl_ = std::make_shared<TextFieldController>("EK-302 (Emirates Airlines)");
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced TimePicker Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise time selector (Category 3. Input / Forms), 12h AM/PM & 24h Military formats, Seconds precision, Steppers, and Quick Presets");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Left Column: Flight Departure Time (Input Popup) ──────────
        auto form_title = text("✈️ Flight Departure Time");
        form_title->fontSize(15.0f).bold().color(0xFF38BDF8);

        auto lbl_fno = text("Flight Number & Carrier:");
        lbl_fno->fontSize(11.5f).bold().color(0xFF94A3B8);

        TextFieldOptions fno_opts;
        fno_opts.hint_text = "Enter flight...";
        auto fno_field = std::make_shared<TextField>(flight_no_ctrl_, fno_opts);
        auto fno_box = container(fno_field);
        fno_box->color(0xFF1E293B).border(0xFF334155, 1.0f).borderRadius(8.0f).paddingSymmetric(8.0f, 12.0f).width(340.0f);

        auto lbl_time = text("Departure Time (12-Hour Dropdown):");
        lbl_time->fontSize(11.5f).bold().color(0xFF94A3B8);

        TimePickerOptions pop_opts;
        pop_opts.mode = TimePickerMode::InputPopup;
        pop_opts.format = TimeFormat::TwelveHour;
        pop_opts.initial_time = flight_time_;
        pop_opts.on_time_selected = [this](const TimeVal& t) {
            flight_time_ = t;
            hud_msg_ = "Flight Departure Time set to: " + t.format12h();
            setState([] {});
        };

        auto pop_picker = timePicker(pop_opts);

        std::vector<WidgetPtr> left_items = {
            form_title,
            lbl_fno, fno_box,
            lbl_time, pop_picker
        };
        auto left_col = column(left_items);
        left_col->gap(StyleValue::point(10.0f));

        auto left_card = container(left_col);
        left_card->color(0xFF0F172A)
                 .border(0xFF334155, 1.0f)
                 .borderRadius(12.0f)
                 .paddingAll(20.0f)
                 .width(380.0f);

        // ── Right Column: Server Backup Scheduler (24h Inline + Seconds)
        auto sched_title = text("⚙️ Automated Server Backup (24h Military + Sec)");
        sched_title->fontSize(15.0f).bold().color(0xFF10B981);

        TimePickerOptions inline_opts;
        inline_opts.mode = TimePickerMode::Inline;
        inline_opts.format = TimeFormat::TwentyFourHour;
        inline_opts.show_seconds = true;
        inline_opts.initial_time = backup_time_;
        inline_opts.on_time_selected = [this](const TimeVal& t) {
            backup_time_ = t;
            hud_msg_ = "Server Cron Scheduled at: " + t.format24h(true) + " UTC";
            setState([] {});
        };

        auto inline_picker = timePicker(inline_opts);

        std::vector<WidgetPtr> right_items = {sched_title, inline_picker};
        auto right_col = column(right_items);
        right_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto right_card = container(right_col);
        right_card->color(0xFF0F172A)
                  .border(0xFF334155, 1.0f)
                  .borderRadius(12.0f)
                  .paddingAll(20.0f)
                  .width(400.0f);

        // ── Side-by-Side Main Sections ────────────────────────────────
        std::vector<WidgetPtr> sections = {left_card, right_card};
        auto sections_row = row(sections);
        sections_row->gap(StyleValue::point(24.0f))
                    .justifyContent(Justify::Center)
                    .alignItems(Align::Start);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(804.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, sections_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(20.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class TimePickerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TimePickerDemoState>();
    }
    std::string_view typeName() const override { return "TimePickerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced TimePicker Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced TimePicker Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<TimePickerDemoApp>(), config);
}
