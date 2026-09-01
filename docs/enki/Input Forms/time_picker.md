# TimePicker

> A clock and time selector widget supporting 12-hour AM/PM and 24-hour military formats, seconds precision, steppers, and quick time presets.

- **Header File**: `#include "enki/widgets/time_picker.hpp"`
- **C++ Class**: `enki::TimePickerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::TimePicker` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::TimePickerProps`
- **Controller**: `enki::TimePickerController`
- **Value Model**: `enki::TimeVal`
- **Enums**: `enki::TimePickerMode`, `enki::TimeFormat`

---

## Overview

`TimePicker` provides an intuitive clock selector for scheduling appointments, setting reminders, or configuring alarm triggers. It supports:
1. **Time Formats**: 12-hour (with AM/PM pills) or 24-hour (military 00:00 - 23:59).
2. **Display Modes**: `InputPopup` (dropdown field) or `Inline` (embedded card).
3. **Precision**: Minute step intervals (`minute_step = 5` or `15`) and optional seconds column.

---

## C++ API Definition

### Data Model (`TimeVal`)
```cpp
namespace enki {

struct TimeVal {
    int  hour   = 8;     // 0 - 23 or 1 - 12
    int  minute = 30;    // 0 - 59
    int  second = 0;     // 0 - 59
    bool is_pm  = true;  // For 12-hour format

    constexpr TimeVal() = default;
    constexpr TimeVal(int h, int m, int s = 0, bool pm = false);

    [[nodiscard]] std::string format12h(bool show_seconds = false) const; // e.g. "08:30 PM"
    [[nodiscard]] std::string format24h(bool show_seconds = false) const; // e.g. "20:30:00"

    bool operator==(const TimeVal& o) const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class TimePickerMode {
    InputPopup,     ///< Input field with dropdown popup
    Inline          ///< Self-contained embedded card
};

enum class TimeFormat {
    TwelveHour,     ///< 12-hour format with AM/PM
    TwentyFourHour  ///< 24-hour military format
};

struct TimePicker {
    std::shared_ptr<TimePickerController> controller         = nullptr;
    WidgetPtr                             body               = nullptr;

    TimePickerMode                        mode               = TimePickerMode::InputPopup;
    TimeFormat                            format             = TimeFormat::TwelveHour;

    TimeVal                               initial_time       = {8, 30, 0, true};
    bool                                  show_seconds       = false;
    int                                   minute_step        = 1;
    bool                                  show_quick_presets = true;
    std::string                           placeholder        = "Select time...";

    Color                                 background_color   = 0xFF1E293B;
    Color                                 border_color       = 0xFF334155;
    Color                                 active_color       = 0xFF0284C7;

    std::function<void(const TimeVal&)>   on_time_selected   = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `format` | `TimeFormat` | `TwelveHour` | Time display mode (`TwelveHour` or `TwentyFourHour`). |
| `mode` | `TimePickerMode` | `InputPopup` | Display mode (`InputPopup` or `Inline`). |
| `initial_time` | `TimeVal` | `{8, 30, 0, true}` | Starting time displayed on the clock. |
| `show_seconds` | `bool` | `false` | Enables a third numeric column for seconds. |
| `minute_step` | `int` | `1` | Increment step between selectable minutes (e.g. `5` or `15`). |
| `show_quick_presets`| `bool` | `true` | Displays quick chips ("Morning 09:00", "Noon 12:00", "Now"). |
| `on_time_selected` | `std::function<void(const TimeVal&)>` | `nullptr` | Callback triggered when the user picks a time. |

---

## Code Examples (From `widgets_demo/time_picker_demo/main.cpp`)

### 1. 12-Hour Dropdown TimePicker
```cpp
#include "enki/widgets/time_picker.hpp"

using namespace enki;

auto appointmentTime = TimePicker {
    .mode = TimePickerMode::InputPopup,
    .format = TimeFormat::TwelveHour,
    .initial_time = TimeVal{10, 30, 0, false}, // 10:30 AM
    .minute_step = 15,
    .on_time_selected = [](const TimeVal& t) {
        std::cout << "Time set to: " << t.format12h() << "\n";
    }
};
```

### 2. 24-Hour Military Time with Seconds
```cpp
auto serverBackupTime = TimePicker {
    .mode = TimePickerMode::Inline,
    .format = TimeFormat::TwentyFourHour,
    .show_seconds = true,
    .initial_time = TimeVal{23, 59, 50},
    .on_time_selected = [](const TimeVal& t) {
        std::cout << "Backup scheduled at: " << t.format24h(true) << "\n";
    }
};
```

---

## See Also
- [**DatePicker**](./date_picker.md) — Calendar date selector.
- [**NumberField**](./number_field.md) — Stepper numeric entry.
