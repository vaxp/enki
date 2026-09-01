# DatePicker

> An advanced calendar picker widget supporting dropdown popups, inline cards, single date and range selection, fast year/month jumps, and quick presets.

- **Header File**: `#include "enki/widgets/date_picker.hpp"`
- **C++ Class**: `enki::DatePickerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Structs**: `enki::DatePicker`, `enki::DateRangePicker` (convert implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DatePickerProps`
- **Controller**: `enki::DatePickerController`
- **Value Models**: `enki::DateVal`, `enki::DateRangeVal`
- **Enums**: `enki::DatePickerMode`, `enki::DatePickerSelectionMode`

---

## Overview

`DatePicker` provides calendar-based date input in two operational modes:
1. **InputPopup**: A clean text input field that opens a floating calendar dropdown on click.
2. **Inline**: An embedded persistent calendar card.

It supports both single date selection and continuous date range highlighting (`DateRangePicker`), along with quick preset buttons ("Today", "This Weekend", "Next Week").

---

## C++ API Definition

### Data Models (`DateVal` & `DateRangeVal`)
```cpp
namespace enki {

struct DateVal {
    int year  = 2026;
    int month = 8;     // 1 - 12
    int day   = 19;    // 1 - 31

    constexpr DateVal() = default;
    constexpr DateVal(int y, int m, int d) : year(y), month(m), day(d) {}

    [[nodiscard]] std::string formatIso() const;        // e.g. "2026-08-19"
    [[nodiscard]] std::string formatFormatted() const;  // e.g. "Aug 19, 2026"

    bool operator==(const DateVal& o) const;
    bool operator<(const DateVal& o) const;
    bool operator<=(const DateVal& o) const;
};

struct DateRangeVal {
    std::optional<DateVal> start;
    std::optional<DateVal> end;

    [[nodiscard]] bool isComplete() const;
    [[nodiscard]] bool contains(const DateVal& d) const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class DatePickerMode {
    InputPopup,     ///< Input field with click-to-open dropdown popup overlay
    Inline          ///< Self-contained embedded calendar card
};

enum class DatePickerSelectionMode {
    Single,         ///< Pick single date
    Range           ///< Pick start and end dates
};

struct DatePicker {
    WidgetPtr                                body                = nullptr;
    std::shared_ptr<DatePickerController>    controller          = nullptr;
    DatePickerMode                           mode                = DatePickerMode::InputPopup;
    DatePickerSelectionMode                  selection_mode      = DatePickerSelectionMode::Single;

    DateVal                                  initial_date        = {2026, 8, 19};
    DateRangeVal                             initial_range;

    std::optional<DateVal>                   min_date;
    std::optional<DateVal>                   max_date;
    bool                                     disable_weekends    = false;

    std::string                              placeholder         = "Select a date...";
    bool                                     show_quick_presets  = true;
    bool                                     show_action_buttons = true;

    Color                                    background_color    = 0xFF1E293B;
    Color                                    border_color        = 0xFF334155;
    Color                                    active_color        = 0xFF0284C7;
    Color                                    highlight_color     = 0xFF38BDF8;
    Color                                    range_fill_color    = 0x440284C7;

    std::function<void(const DateVal&)>      on_date_selected    = nullptr;
    std::function<void(const DateRangeVal&)> on_range_selected   = nullptr;
    std::function<bool(const DateVal&)>      is_date_disabled    = nullptr;

    operator WidgetPtr() const;
};

struct DateRangePicker : public DatePicker {
    DateRangePicker() {
        selection_mode = DatePickerSelectionMode::Range;
    }
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `mode` | `DatePickerMode` | `InputPopup` | Display style (`InputPopup` or `Inline`). |
| `selection_mode` | `DatePickerSelectionMode`| `Single` | Single date or date range selection. |
| `initial_date` | `DateVal` | `{2026, 8, 19}` | Default focused date. |
| `min_date` / `max_date` | `std::optional<DateVal>` | `nullopt` | Selectable calendar constraints. |
| `disable_weekends` | `bool` | `false` | Disables Saturday/Sunday selection when true. |
| `show_quick_presets`| `bool` | `true` | Shows quick jump chips at top ("Today", "Tomorrow"). |
| `on_date_selected` | `std::function<void(const DateVal&)>` | `nullptr` | Callback when a single date is chosen. |
| `on_range_selected`| `std::function<void(const DateRangeVal&)>` | `nullptr` | Callback when a date range is chosen. |

---

## Code Examples (From `widgets_demo/date_picker_demo/main.cpp`)

### 1. Dropdown DatePicker
```cpp
#include "enki/widgets/date_picker.hpp"

using namespace enki;

auto departurePicker = DatePicker {
    .mode = DatePickerMode::InputPopup,
    .selection_mode = DatePickerSelectionMode::Single,
    .initial_date = DateVal{2026, 9, 1},
    .on_date_selected = [](const DateVal& d) {
        std::cout << "Selected: " << d.formatFormatted() << "\n";
    }
};
```

### 2. Inline Hotel Reservation RangePicker
```cpp
auto hotelRangePicker = DatePicker {
    .mode = DatePickerMode::Inline,
    .selection_mode = DatePickerSelectionMode::Range,
    .show_quick_presets = true,
    .on_range_selected = [](const DateRangeVal& r) {
        if (r.isComplete()) {
            std::cout << "Booked from " << r.start->formatIso() 
                      << " to " << r.end->formatIso() << "\n";
        }
    }
};
```

---

## See Also
- [**TimePicker**](./time_picker.md) — Clock time picker.
- [**TextField**](./text_field.md) — Raw text entry.
