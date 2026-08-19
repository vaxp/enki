/// @file main.cpp
/// @brief ENKI Advanced DatePicker & DateRangePicker Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/date_picker.hpp"
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

class DatePickerDemoState : public State {
private:
    std::string hud_msg_ = "Click the input field on the left to open the DatePicker dropdown, or pick dates in the inline RangePicker on the right.";

    DateVal selected_single_date_ = {2026, 8, 19};
    DateRangeVal selected_range_ = {DateVal{2026, 8, 19}, DateVal{2026, 8, 26}};

    std::shared_ptr<TextFieldController> dest_ctrl_;
    std::shared_ptr<TextFieldController> pass_ctrl_;

public:
    void initState() override {
        State::initState();
        dest_ctrl_ = std::make_shared<TextFieldController>("Tokyo Haneda (HND)");
        pass_ctrl_ = std::make_shared<TextFieldController>("2 Adults, 1 Child");
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced DatePicker & DateRangePicker Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise date input (Category 3. Input / Forms), Input Popup & Inline modes, Date Range strip highlighting, and Year/Month fast jumps");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Left Column: Form with Input Popup DatePicker ─────────────
        auto form_title = text("✈️ Flight Booking Details");
        form_title->fontSize(15.0f).bold().color(0xFF38BDF8);

        auto lbl_dest = text("Destination:");
        lbl_dest->fontSize(11.5f).bold().color(0xFF94A3B8);

        TextFieldOptions dest_opts;
        dest_opts.hint_text = "Enter Destination...";
        auto dest_field = std::make_shared<TextField>(dest_ctrl_, dest_opts);
        auto dest_box = container(dest_field);
        dest_box->color(0xFF1E293B).border(0xFF334155, 1.0f).borderRadius(8.0f).paddingSymmetric(8.0f, 12.0f).width(340.0f);

        auto lbl_pass = text("Passengers & Class:");
        lbl_pass->fontSize(11.5f).bold().color(0xFF94A3B8);

        TextFieldOptions pass_opts;
        pass_opts.hint_text = "Enter Passengers...";
        auto pass_field = std::make_shared<TextField>(pass_ctrl_, pass_opts);
        auto pass_box = container(pass_field);
        pass_box->color(0xFF1E293B).border(0xFF334155, 1.0f).borderRadius(8.0f).paddingSymmetric(8.0f, 12.0f).width(340.0f);

        auto label_dep = text("Departure Date (Single Date Dropdown):");
        label_dep->fontSize(11.5f).bold().color(0xFF94A3B8);

        DatePickerOptions popup_opts;
        popup_opts.mode = DatePickerMode::InputPopup;
        popup_opts.selection_mode = DatePickerSelectionMode::Single;
        popup_opts.initial_date = selected_single_date_;
        popup_opts.on_date_selected = [this](const DateVal& d) {
            selected_single_date_ = d;
            hud_msg_ = "Selected Departure Date: " + d.formatFormatted() + " (" + d.formatIso() + ")";
            setState([] {});
        };

        auto single_date_widget = datePicker(popup_opts);

        std::vector<WidgetPtr> form_items = {
            form_title,
            lbl_dest, dest_box,
            lbl_pass, pass_box,
            label_dep, single_date_widget
        };
        auto form_col = column(form_items);
        form_col->gap(StyleValue::point(10.0f));

        auto form_card = container(form_col);
        form_card->color(0xFF0F172A)
                 .border(0xFF334155, 1.0f)
                 .borderRadius(12.0f)
                 .paddingAll(20.0f)
                 .width(380.0f);

        // ── Right Column: Inline DateRangePicker ───────────────────────
        auto range_title = text("🏨 Hotel Reservation Period (Inline DateRangePicker)");
        range_title->fontSize(15.0f).bold().color(0xFF10B981);

        DatePickerOptions range_opts;
        range_opts.mode = DatePickerMode::Inline;
        range_opts.selection_mode = DatePickerSelectionMode::Range;
        range_opts.initial_range = selected_range_;
        range_opts.on_range_selected = [this](const DateRangeVal& r) {
            selected_range_ = r;
            if (r.start && r.end) {
                hud_msg_ = "Hotel Reserved: " + r.start->formatFormatted() + " ➔ " + r.end->formatFormatted();
            } else if (r.start) {
                hud_msg_ = "Check-in selected: " + r.start->formatFormatted() + ". Now click Check-out date.";
            }
            setState([] {});
        };

        auto inline_picker = dateRangePicker(range_opts);

        std::vector<WidgetPtr> range_items = {range_title, inline_picker};
        auto range_col = column(range_items);
        range_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto range_card = container(range_col);
        range_card->color(0xFF0F172A)
                  .border(0xFF334155, 1.0f)
                  .borderRadius(12.0f)
                  .paddingAll(20.0f)
                  .width(400.0f);

        // ── Side-by-Side Main Sections ────────────────────────────────
        std::vector<WidgetPtr> sections = {form_card, range_card};
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

class DatePickerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<DatePickerDemoState>();
    }
    std::string_view typeName() const override { return "DatePickerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced DatePicker Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced DatePicker Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DatePickerDemoApp>(), config);
}
