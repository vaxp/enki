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
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    // ── Main Page Header ──────────────────────────────────────────
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(6.0f),
                        .children = {
                            text("Advanced TimePicker Suite", {
                                .color = 0xFFFFFFFF,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold
                            }),
                            text("Enterprise time selector (Category 3. Input / Forms), 12h AM/PM & 24h Military formats, Seconds precision, Steppers, and Quick Presets", {
                                .color = 0xFF94A3B8,
                                .font_size = 13.0f
                            })
                        }
                    }),
                    
                    // ── Side-by-Side Main Sections ────────────────────────────────
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Start,
                        .gap = StyleValue::point(24.0f),
                        .children = {
                            // ── Left Column: Flight Departure Time (Input Popup) ──────────
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(380.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("✈️ Flight Departure Time", { .color = 0xFF38BDF8, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        text("Flight Number & Carrier:", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(8.0f),
                                            .border = Border(0xFF334155, 1.0f),
                                            .width = StyleValue::point(340.0f),
                                            .padding = StyleInsets::symmetric(8.0f, 12.0f),
                                            .child = TextField { .controller = flight_no_ctrl_, .hint_text = "Enter flight..." }
                                        }),
                                        text("Departure Time (12-Hour Dropdown):", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        TimePicker {
                                            .mode = TimePickerMode::InputPopup,
                                            .format = TimeFormat::TwelveHour,
                                            .initial_time = flight_time_,
                                            .on_time_selected = [this](const TimeVal& t) {
                                                flight_time_ = t;
                                                hud_msg_ = "Flight Departure Time set to: " + t.format12h();
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            }),
                            
                            // ── Right Column: Server Backup Scheduler (24h Inline + Seconds)
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(400.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(12.0f),
                                    .children = {
                                        text("⚙️ Automated Server Backup (24h Military + Sec)", { .color = 0xFF10B981, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        TimePicker {
                                            .mode = TimePickerMode::Inline,
                                            .format = TimeFormat::TwentyFourHour,
                                            .initial_time = backup_time_,
                                            .show_seconds = true,
                                            .on_time_selected = [this](const TimeVal& t) {
                                                backup_time_ = t;
                                                hud_msg_ = "Server Cron Scheduled at: " + t.format24h(true) + " UTC";
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            })
                        }
                    }),
                    
                    // ── HUD / Status Box ──────────────────────────────────────────
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(804.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = row({
                            .children = {
                                text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                            }
                        })
                    })
                }
            })
        });
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
