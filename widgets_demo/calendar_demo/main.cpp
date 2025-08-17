/// @file main.cpp
/// @brief ENKI Advanced Calendar Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/calendar.hpp"
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

class CalendarDemoState : public State {
private:
    std::shared_ptr<CalendarController> calendar_ctrl_;
    CalendarSelectionMode current_mode_ = CalendarSelectionMode::Single;
    CalendarDate active_date_ = {2026, 8, 19};
    std::string hud_msg_ = "Click any date cell to select date or inspect scheduled events in the Agenda.";

    std::vector<CalendarEvent> sample_events_;

    // ── Build Agenda Panel for Selected Date ──────────────────────
    WidgetPtr buildAgendaPanel() {
        auto t = text("📅 Daily Agenda & Schedule");
        t->fontSize(14.5f).bold().color(0xFF38BDF8);

        auto d_lbl = text("Events for " + active_date_.toString() + ":");
        d_lbl->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<CalendarEvent> day_events;
        for (const auto& ev : sample_events_) {
            if (ev.date == active_date_) day_events.push_back(ev);
        }

        std::vector<WidgetPtr> event_items = {t, d_lbl};

        if (day_events.empty()) {
            auto empty_txt = text("No scheduled events for this day.");
            empty_txt->fontSize(12.5f).color(0xFF64748B);

            auto empty_box = container(empty_txt);
            empty_box->paddingSymmetric(16.0f, 0.0f);
            event_items.push_back(empty_box);
        } else {
            for (const auto& ev : day_events) {
                auto ev_tit = text(ev.title);
                ev_tit->fontSize(13.0f).bold().color(0xFFFFFFFF);

                auto ev_time = text(ev.time_str);
                ev_time->fontSize(11.5f).color(ev.color);

                auto ev_desc = text(ev.description);
                ev_desc->fontSize(11.5f).color(0xFF94A3B8);

                std::vector<WidgetPtr> ev_col_items = {ev_tit, ev_time, ev_desc};
                auto ev_col = column(ev_col_items);
                ev_col->gap(StyleValue::point(3.0f));

                auto ev_card = container(ev_col);
                ev_card->color(0xFF0F172A)
                       .border(ev.color, 1.0f)
                       .borderRadius(8.0f)
                       .paddingAll(12.0f)
                       .width(StyleValue::percent(100.0f));

                event_items.push_back(ev_card);
            }
        }

        auto agenda_col = column(event_items);
        agenda_col->gap(StyleValue::point(10.0f)).width(StyleValue::percent(100.0f));

        auto agenda_card = container(agenda_col);
        agenda_card->color(0xFF1E293B)
                   .border(0xFF334155, 1.0f)
                   .borderRadius(12.0f)
                   .paddingAll(18.0f)
                   .width(360.0f)
                   .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        return agenda_card;
    }

public:
    void initState() override {
        State::initState();
        calendar_ctrl_ = std::make_shared<CalendarController>();

        // Pre-populate sample cloud scheduled events
        sample_events_ = {
            CalendarEvent("ev1", "🚀 ENKI v1.0 Release Candidate", {2026, 8, 19}, 0xFF10B981,
                          "09:00 AM - 11:00 AM", "Vulkan pipeline freeze and binary tags"),
            CalendarEvent("ev2", "🛡️ SOC-2 Compliance Audit", {2026, 8, 19}, 0xFF38BDF8,
                          "02:00 PM - 03:30 PM", "Security key rotation review"),
            CalendarEvent("ev3", "⚡ GPU Cluster Maintenance", {2026, 8, 24}, 0xFFF59E0B,
                          "01:00 AM - 04:00 AM", "NVIDIA H100 firmware upgrade"),
            CalendarEvent("ev4", "🌐 Edge Mesh Sync", {2026, 8, 12}, 0xFF8B5CF6,
                          "10:00 AM - 11:00 AM", "Global DNS propagation test")
        };
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Calendar & Scheduling Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Date picker (Category 10. Advanced / Data UI), month navigation, Single/Range modes, and event markers");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Mode Control Bar ──────────────────────────────────────────
        auto makePill = [](std::string label, bool active, std::function<void()> cb) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(12.0f).color(active ? 0xFFFFFFFF : 0xFF94A3B8);
            if (active) t->bold();

            auto b = container(t);
            b->color(active ? 0xFF0284C7 : 0xFF0F172A)
             .border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(6.0f, 14.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        auto pill_single = makePill("Single Date Mode", current_mode_ == CalendarSelectionMode::Single, [this] {
            current_mode_ = CalendarSelectionMode::Single;
            hud_msg_ = "Switched to Single Date Selection Mode.";
            setState([] {});
        });

        auto pill_range = makePill("Date Range Mode", current_mode_ == CalendarSelectionMode::Range, [this] {
            current_mode_ = CalendarSelectionMode::Range;
            hud_msg_ = "Switched to Date Range Mode (Click start date, then end date).";
            setState([] {});
        });

        auto btn_add_ev = button(text("➕ Add Standup Today"), [this] {
            CalendarEvent standup("ev_new", "👥 Daily Engineering Standup", active_date_, 0xFFEC4899,
                                  "11:00 AM - 11:30 AM", "Sprint progress and blocker check");
            sample_events_.push_back(standup);
            hud_msg_ = "Added new Standup event for " + active_date_.toString();
            setState([] {});
        });

        std::vector<WidgetPtr> ctrl_items = {pill_single, pill_range, btn_add_ev};
        auto ctrl_row = row(ctrl_items);
        ctrl_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // ── Calendar Widget ───────────────────────────────────────────
        CalendarProps opts;
        opts.selection_mode = current_mode_;
        opts.width = 380.0f;
        opts.on_date_selected = [this](CalendarDate d) {
            active_date_ = d;
            hud_msg_ = "Selected Date: " + d.toString();
            setState([] {});
        };
        opts.on_range_selected = [this](CalendarDate s, CalendarDate e) {
            hud_msg_ = "Selected Range: " + s.toString() + " ➔ " + e.toString();
            setState([] {});
        };

        opts.events = sample_events_;
        opts.controller = calendar_ctrl_;
        auto cal_widget = calendar(opts);

        // ── Agenda Panel ──────────────────────────────────────────────
        auto agenda_widget = buildAgendaPanel();

        std::vector<WidgetPtr> main_row_items;
        main_row_items.push_back(cal_widget);
        main_row_items.push_back(agenda_widget);
        auto main_row = row(main_row_items);
        main_row->gap(StyleValue::point(20.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(760.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, ctrl_row, main_row, hud_box};
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

class CalendarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<CalendarDemoState>();
    }
    std::string_view typeName() const override { return "CalendarDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Calendar Component Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Calendar Component Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<CalendarDemoApp>(), config);
}
