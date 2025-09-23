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
        auto t = text("📅 Daily Agenda & Schedule", {
            .color = 0xFF38BDF8,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto d_lbl = text("Events for " + active_date_.toString() + ":", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        std::vector<CalendarEvent> day_events;
        for (const auto& ev : sample_events_) {
            if (ev.date == active_date_) day_events.push_back(ev);
        }

        std::vector<WidgetPtr> event_items = {t, d_lbl};

        if (day_events.empty()) {
            auto empty_txt = text("No scheduled events for this day.", {
                .color = 0xFF64748B,
                .font_size = 12.5f,
            });

            auto empty_box = container({
                .padding = StyleInsets::symmetric(16.0f, 0.0f),
                .child = empty_txt,
            });
            event_items.push_back(empty_box);
        } else {
            for (const auto& ev : day_events) {
                auto ev_tit = text(ev.title, {
                    .color = 0xFFFFFFFF,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                });

                auto ev_time = text(ev.time_str, {
                    .color = ev.color,
                    .font_size = 11.5f,
                });

                auto ev_desc = text(ev.description, {
                    .color = 0xFF94A3B8,
                    .font_size = 11.5f,
                });

                auto ev_col = column({
                    .gap = StyleValue::point(3.0f),
                    .children = {ev_tit, ev_time, ev_desc},
                });

                auto ev_card = container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(8.0f),
                    .border = Border(ev.color, 1.0f),
                    .width = StyleValue::percent(100.0f),
                    .padding = StyleInsets::all(12.0f),
                    .child = ev_col,
                });

                event_items.push_back(ev_card);
            }
        }

        auto agenda_col = column({
            .gap = StyleValue::point(10.0f),
            .width = StyleValue::percent(100.0f),
            .children = event_items,
        });

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .box_shadow = { BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f) },
            .width = StyleValue::point(360.0f),
            .padding = StyleInsets::all(18.0f),
            .child = agenda_col,
        });
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
        auto title = text("Advanced Calendar & Scheduling Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Date picker (Category 10. Advanced / Data UI), month navigation, Single/Range modes, and event markers", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub},
        });

        // ── Mode Control Bar ──────────────────────────────────────────
        auto makePill = [](std::string label, bool active, std::function<void()> cb) -> WidgetPtr {
            auto b = container({
                .color = active ? 0xFF0284C7 : 0xFF0F172A,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 14.0f),
                .child = text(std::move(label), {
                    .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
                }),
            });

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [cb](const TapUpDetails&) {
                    if (cb) cb();
                },
            });
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

        auto ctrl_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(10.0f),
            .children = {pill_single, pill_range, btn_add_ev},
        });

        // ── Calendar Widget ───────────────────────────────────────────
        WidgetPtr cal_widget = Calendar {
            .events = sample_events_,
            .controller = calendar_ctrl_,
            .selection_mode = current_mode_,
            .width = 380.0f,
            .on_date_selected = [this](CalendarDate d) {
                active_date_ = d;
                hud_msg_ = "Selected Date: " + d.toString();
                setState([] {});
            },
            .on_range_selected = [this](CalendarDate s, CalendarDate e) {
                hud_msg_ = "Selected Range: " + s.toString() + " ➔ " + e.toString();
                setState([] {});
            },
        };

        // ── Agenda Panel ──────────────────────────────────────────────
        auto agenda_widget = buildAgendaPanel();

        auto main_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(20.0f),
            .children = {cal_widget, agenda_widget},
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(760.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = {
                    text("💡 " + hud_msg_, {
                        .color = 0xFF38BDF8,
                        .font_size = 12.5f,
                    }),
                },
            }),
        });

        // ── Assemble Page Body ────────────────────────────────────────
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {title_col, ctrl_row, main_row, hud_box},
            }),
        });
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
