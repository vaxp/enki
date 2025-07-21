#pragma once
/// @file calendar.hpp
/// @brief Advanced Calendar & Scheduling widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports Month/Year navigation, Single/Range date selection, event dot markers,
/// agenda sidebar integration, Today quick-jump, and CalendarController.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include <tuple>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// Calendar Date Representation
/// ════════════════════════════════════════════════════════════════

struct CalendarDate {
    int year = 2026;
    int month = 8;    ///< 1 - 12
    int day = 19;     ///< 1 - 31

    constexpr CalendarDate() = default;
    constexpr CalendarDate(int y, int m, int d) : year(y), month(m), day(d) {}

    constexpr auto toTuple() const { return std::make_tuple(year, month, day); }
    constexpr bool operator==(const CalendarDate& o) const { return toTuple() == o.toTuple(); }
    constexpr bool operator!=(const CalendarDate& o) const { return toTuple() != o.toTuple(); }
    constexpr bool operator<(const CalendarDate& o) const { return toTuple() < o.toTuple(); }
    constexpr bool operator<=(const CalendarDate& o) const { return toTuple() <= o.toTuple(); }
    constexpr bool operator>(const CalendarDate& o) const { return toTuple() > o.toTuple(); }
    constexpr bool operator>=(const CalendarDate& o) const { return toTuple() >= o.toTuple(); }

    [[nodiscard]] std::string toString() const {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
        return std::string(buf);
    }
};

/// Selection mode for Calendar
enum class CalendarSelectionMode {
    Single,     ///< Pick a single date
    Range,      ///< Pick a start date and an end date
    None        ///< Read-only calendar
};

/// ════════════════════════════════════════════════════════════════
/// Calendar Event Model
/// ════════════════════════════════════════════════════════════════

struct CalendarEvent {
    std::string id = "";
    std::string title = "";
    std::string time_str = "";       ///< e.g. "10:00 AM - 11:30 AM"
    std::string description = "";
    CalendarDate date;
    Color color = 0xFF38BDF8;         ///< Accent color (e.g. 0xFF10B981 for deploy, 0xFFEF4444 for release)
    bool is_all_day = false;

    CalendarEvent() = default;
    CalendarEvent(std::string id_, std::string title_, CalendarDate date_,
                  Color col = 0xFF38BDF8, std::string time_ = "", std::string desc_ = "")
        : id(std::move(id_)), title(std::move(title_)), time_str(std::move(time_)),
          description(std::move(desc_)), date(date_), color(col) {}
};

/// ════════════════════════════════════════════════════════════════
/// Calendar Options
/// ════════════════════════════════════════════════════════════════

struct CalendarOptions {
    CalendarSelectionMode selection_mode = CalendarSelectionMode::Single;
    CalendarDate initial_date = {2026, 8, 19};
    int first_day_of_week = 1;        ///< 0 = Sunday, 1 = Monday

    bool show_today_btn = true;
    bool show_adjacent_days = true;   ///< Dimmed days from previous/next month
    bool highlight_today = true;

    float width = 360.0f;
    float border_radius = 12.0f;

    // Styling Colors
    Color background_color    = 0xFF1E293B; // Slate 800
    Color border_color        = 0xFF334155; // Slate 700
    Color header_bg_color     = 0xFF0F172A; // Slate 900
    Color title_color         = 0xFFFFFFFF; // White
    Color weekday_color       = 0xFF94A3B8; // Slate 400
    Color day_color           = 0xFFF1F5F9; // Slate 100
    Color adjacent_day_color  = 0xFF475569; // Slate 600
    Color today_ring_color    = 0xFF38BDF8; // Sky 400
    Color selected_bg_color   = 0xFF0284C7; // Blue 600
    Color in_range_bg_color   = 0x330284C7; // Blue 20%
    Color hover_bg_color      = 0x33334155; // Slate 700 with opacity

    // Callbacks
    std::function<void(CalendarDate selected_date)> on_date_selected;
    std::function<void(CalendarDate range_start, CalendarDate range_end)> on_range_selected;
    std::function<void(int year, int month)> on_month_changed;
    std::function<void(const CalendarEvent& event)> on_event_clicked;
};

/// ════════════════════════════════════════════════════════════════
/// Calendar Controller
/// ════════════════════════════════════════════════════════════════

class CalendarController {
public:
    std::function<void(CalendarDate)> select_date_fn;
    std::function<void(CalendarDate, CalendarDate)> select_range_fn;
    std::function<void()> go_to_today_fn;
    std::function<void()> next_month_fn;
    std::function<void()> prev_month_fn;
    std::function<void(int, int)> set_month_fn;
    std::function<void(CalendarEvent)> add_event_fn;
    std::function<CalendarDate()> get_selected_date_fn;

    void selectDate(CalendarDate d) { if (select_date_fn) select_date_fn(d); }
    void selectRange(CalendarDate s, CalendarDate e) { if (select_range_fn) select_range_fn(s, e); }
    void goToToday() { if (go_to_today_fn) go_to_today_fn(); }
    void nextMonth() { if (next_month_fn) next_month_fn(); }
    void prevMonth() { if (prev_month_fn) prev_month_fn(); }
    void setMonth(int y, int m) { if (set_month_fn) set_month_fn(y, m); }
    void addEvent(CalendarEvent ev) { if (add_event_fn) add_event_fn(std::move(ev)); }
    [[nodiscard]] CalendarDate getSelectedDate() const { return get_selected_date_fn ? get_selected_date_fn() : CalendarDate{2026, 8, 19}; }
};

/// ════════════════════════════════════════════════════════════════
/// Calendar Widget
/// ════════════════════════════════════════════════════════════════

class Calendar : public StatefulWidget {
public:
    std::vector<CalendarEvent> events;
    CalendarOptions options;
    std::shared_ptr<CalendarController> controller;

    Calendar() = default;
    Calendar(std::vector<CalendarEvent> events_, CalendarOptions opts = {},
             std::shared_ptr<CalendarController> ctrl = nullptr)
        : events(std::move(events_)), options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Calendar"; }
};

inline std::shared_ptr<Calendar> calendar(
    std::vector<CalendarEvent> events = {},
    CalendarOptions options = {},
    std::shared_ptr<CalendarController> controller = nullptr) {
    return std::make_shared<Calendar>(std::move(events), std::move(options), std::move(controller));
}

} // namespace enki
