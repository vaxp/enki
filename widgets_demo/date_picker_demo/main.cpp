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
#include <format>

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
                            text("Advanced DatePicker & DateRangePicker Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("Enterprise date input (Category 3. Input / Forms), Input Popup & Inline modes, Date Range strip highlighting, and Year/Month fast jumps", { .color = 0xFF94A3B8, .font_size = 13.0f })
                        }
                    }),

                    // ── Side-by-Side Main Sections ────────────────────────────────
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Start,
                        .gap = StyleValue::point(24.0f),
                        .children = {
                            // ── Left Column: Form with Input Popup DatePicker ─────────────
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(380.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("✈️ Flight Booking Details", { .color = 0xFF38BDF8, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        
                                        text("Destination:", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(8.0f),
                                            .border = Border(0xFF334155, 1.0f),
                                            .width = StyleValue::point(340.0f),
                                            .padding = StyleInsets::symmetric(8.0f, 12.0f),
                                            .child = TextField { .controller = dest_ctrl_, .hint_text = "Enter Destination..." }
                                        }),

                                        text("Passengers & Class:", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(8.0f),
                                            .border = Border(0xFF334155, 1.0f),
                                            .width = StyleValue::point(340.0f),
                                            .padding = StyleInsets::symmetric(8.0f, 12.0f),
                                            .child = TextField { .controller = pass_ctrl_, .hint_text = "Enter Passengers..." }
                                        }),

                                        text("Departure Date (Single Date Dropdown):", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        DatePicker {
                                            .mode = DatePickerMode::InputPopup,
                                            .selection_mode = DatePickerSelectionMode::Single,
                                            .initial_date = selected_single_date_,
                                            .on_date_selected = [this](const DateVal& d) {
                                                selected_single_date_ = d;
                                                hud_msg_ = "Selected Departure Date: " + d.formatFormatted() + " (" + d.formatIso() + ")";
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            }),

                            // ── Right Column: Inline DateRangePicker ───────────────────────
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
                                        text("🏨 Hotel Reservation Period (Inline DateRangePicker)", { .color = 0xFF10B981, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        DatePicker {
                                            .mode = DatePickerMode::Inline,
                                            .selection_mode = DatePickerSelectionMode::Range,
                                            .initial_range = selected_range_,
                                            .on_range_selected = [this](const DateRangeVal& r) {
                                                selected_range_ = r;
                                                if (r.start && r.end) {
                                                    hud_msg_ = "Hotel Reserved: " + r.start->formatFormatted() + " ➔ " + r.end->formatFormatted();
                                                } else if (r.start) {
                                                    hud_msg_ = "Check-in selected: " + r.start->formatFormatted() + ". Now click Check-out date.";
                                                }
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
