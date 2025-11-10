# Calendar

> An interactive calendar and scheduling widget supporting single-date and date-range selection, event dot markers, agenda panel integration, month/year navigation, and today quick-jumping.

- **Header File**: `#include "enki/widgets/calendar.hpp"`
- **C++ Class**: `enki::CalendarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Calendar` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::CalendarProps`
- **Controller**: `enki::CalendarController`
- **Date Representation**: `enki::CalendarDate`
- **Event Model**: `enki::CalendarEvent`
- **Selection Enum**: `enki::CalendarSelectionMode` (`Single`, `Range`, `None`)

---

## Overview

`Calendar` provides a full-featured desktop calendar component. It computes month layout grids, highlights today's date, indicates events with colored dot badges under corresponding day numbers, and supports selecting single dates or continuous date ranges (useful for booking systems and date-range filtering).

---

## C++ API Definition

### `CalendarDate` Struct
```cpp
namespace enki {

struct CalendarDate {
    int year  = 2026;
    int month = 8;    ///< 1 to 12
    int day   = 19;   ///< 1 to 31

    constexpr CalendarDate() = default;
    constexpr CalendarDate(int y, int m, int d);

    constexpr bool operator==(const CalendarDate& o) const;
    constexpr bool operator!=(const CalendarDate& o) const;
    constexpr bool operator<(const CalendarDate& o) const;

    [[nodiscard]] std::string toString() const; ///< Formats as "YYYY-MM-DD"
};

enum class CalendarSelectionMode {
    Single,     ///< Pick a single date
    Range,      ///< Pick a start date and an end date
    None        ///< Read-only calendar
};

struct CalendarEvent {
    std::string  id          = "";
    std::string  title       = "";
    std::string  time_str    = "";   ///< e.g. "10:00 AM - 11:30 AM"
    std::string  description = "";
    CalendarDate date;
    Color        color       = 0xFF38BDF8;
    bool         is_all_day  = false;

    CalendarEvent() = default;
    CalendarEvent(std::string id, std::string title, CalendarDate date,
                  Color col = 0xFF38BDF8, std::string time = "", std::string desc = "");
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Calendar {
    Key                                 key                = Key::none();
    std::vector<CalendarEvent>          events;
    std::shared_ptr<CalendarController> controller         = nullptr;

    CalendarSelectionMode               selection_mode     = CalendarSelectionMode::Single;
    CalendarDate                        initial_date       = {2026, 8, 19};
    int                                 first_day_of_week  = 1; ///< 0 = Sunday, 1 = Monday

    bool                                show_today_btn     = true;
    bool                                show_adjacent_days = true;
    bool                                highlight_today    = true;

    float                               width              = 360.0f;
    float                               border_radius      = 12.0f;

    Color                               background_color   = 0xFF1E293B; // Slate 800
    Color                               border_color       = 0xFF334155; // Slate 700
    Color                               header_bg_color    = 0xFF0F172A; // Slate 900
    Color                               selected_bg_color  = 0xFF0284C7; // Blue 600

    // Callbacks
    std::function<void(CalendarDate)>                         on_date_selected  = nullptr;
    std::function<void(CalendarDate start, CalendarDate end)> on_range_selected = nullptr;
    std::function<void(int year, int month)>                  on_month_changed  = nullptr;
    std::function<void(const CalendarEvent&)>                 on_event_clicked  = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `events` | `vector<CalendarEvent>` | `{}` | Scheduled items rendered as colored indicator dots on days. |
| `selection_mode` | `CalendarSelectionMode` | `Single` | Mode for picking dates (`Single`, `Range`, `None`). |
| `first_day_of_week`| `int` | `1` | Starting day column (0 = Sunday, 1 = Monday). |
| `initial_date` | `CalendarDate` | `{2026, 8, 19}` | The initially viewed/selected calendar date. |
| `show_today_btn` | `bool` | `true` | Renders quick-jump "Today" button in header. |
| `show_adjacent_days`| `bool` | `true` | Displays dimmed days belonging to adjacent months. |

---

## Code Examples (From `widgets_demo/calendar_demo/main.cpp`)

### 1. Interactive Event Calendar with Date Selection
```cpp
#include "enki/widgets/calendar.hpp"

using namespace enki;

class CalendarViewState : public State {
    std::shared_ptr<CalendarController> cal_ctrl_ = std::make_shared<CalendarController>();
    CalendarDate active_date_ = {2026, 8, 19};

public:
    WidgetPtr build(BuildContext& ctx) override {
        std::vector<CalendarEvent> events = {
            CalendarEvent("e1", "Sprint Retrospective", CalendarDate(2026, 8, 19), 0xFF10B981, "10:00 AM"),
            CalendarEvent("e2", "v1.0 Production Deploy", CalendarDate(2026, 8, 22), 0xFFEF4444, "02:00 PM")
        };

        return Calendar {
            .events = std::move(events),
            .controller = cal_ctrl_,
            .selection_mode = CalendarSelectionMode::Single,
            .initial_date = active_date_,
            .on_date_selected = [this](CalendarDate selected) {
                active_date_ = selected;
                std::cout << "Selected date: " << selected.toString() << "\n";
                setState([]{});
            }
        };
    }
};
```

---

## See Also
- [**DatePicker**](../Input%20Forms/date_picker.md) — Popup dialog date selection field.
- [**Timeline**](./timeline.md) — Sequential workflow progress.
- [**DataGrid**](./data_grid.md) — Large tabular record views.
