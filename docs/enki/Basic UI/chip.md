# Chip

> Compact interactive tokens used for action triggers, multi-selection filters, single-choice segments, and deletable input tags.

- **Header File**: `#include "enki/widgets/chip.hpp"`
- **C++ Class**: `enki::ChipWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Chip` (converts implicitly to `WidgetPtr`)
- **Group Struct**: `enki::ChipGroup` (`enki::ChipGroupWidget`)
- **Props Structs**: `enki::ChipProps`, `enki::ChipGroupProps`

---

## Overview

`Chip` represents small interactive elements such as filters, contacts, or tags. Enki supports five standard chip types (`Action`, `Filter`, `Choice`, `Input`, `Status`) across three visual variants (`Filled`, `Outlined`, `Elevated`) and three sizes (`Small`, `Medium`, `Large`).

The companion `ChipGroup` orchestrates lists of chips with uniform spacing and single-choice / multi-filter selection states.

---

## C++ API Definition

### Declarative Struct (`Chip`)
```cpp
namespace enki {

struct Chip {
    Key          key              = Key::none();
    ChipType     type             = ChipType::Action;
    ChipVariant  variant          = ChipVariant::Filled;
    ChipSize     size             = ChipSize::Medium;

    std::string  label            = "";
    std::string  avatar_icon      = "";              // Leading emoji or character
    WidgetPtr    leading          = nullptr;         // Custom leading widget
    WidgetPtr    trailing         = nullptr;        // Custom trailing widget

    bool         selected         = false;
    bool         enabled          = true;
    bool         deletable        = false;           // Shows trailing 'X' remove button
    bool         pulsing_dot      = false;           // Live animated status dot

    // Styling Colors
    Color        background_color = 0xFF1E293B;      // Slate 800
    Color        selected_color   = 0xFF0284C7;      // Sky 600
    Color        border_color     = 0xFF334155;      // Slate 700
    Color        text_color       = 0xFFFFFFFF;
    Color        status_color     = 0xFF10B981;      // Emerald 500

    // Callbacks
    std::function<void()>              on_tap      = nullptr;
    std::function<void(bool selected)> on_selected = nullptr;
    std::function<void()>              on_deleted  = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Declarative Struct (`ChipGroup`)
```cpp
namespace enki {

struct ChipGroup {
    Key                            key               = Key::none();
    std::vector<WidgetPtr>         chips;
    bool                           single_choice     = false;
    float                          gap               = 8.0f;
    std::function<void(int index)> on_choice_changed = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Chip Enums

### `ChipType`
- `ChipType::Action` — Triggers an immediate action when tapped (`on_tap`).
- `ChipType::Filter` — Toggles a selected checkmark state (`on_selected`).
- `ChipType::Choice` — Mutually exclusive option within a segmented group.
- `ChipType::Input` — Represents an entered entity (e.g. email recipient) with an `X` delete button (`on_deleted`).
- `ChipType::Status` — Shows an active live status dot with pulsing animations.

### `ChipVariant`
- `ChipVariant::Filled` — Solid background fill.
- `ChipVariant::Outlined` — Transparent background with solid border.
- `ChipVariant::Elevated` — Solid background with soft drop shadow.

### `ChipSize`
- `ChipSize::Small` — `24px` height with compact 11px font.
- `ChipSize::Medium` — `32px` height with standard 13px font.
- `ChipSize::Large` — `40px` height with prominent 15px font.

---

## Code Examples (From `widgets_demo/chip_demo/main.cpp`)

### 1. Multi-Select Filter Chips
```cpp
#include "enki/widgets/chip.hpp"

using namespace enki;

WidgetPtr buildFilterChip(const std::string& name, bool isSelected) {
    return Chip {
        .type = ChipType::Filter,
        .label = name,
        .selected = isSelected,
        .on_selected = [](bool selected) {
            // Update filter state
        },
    };
}
```

### 2. Single-Choice Priority Selector
```cpp
auto priorityGroup = ChipGroup {
    .gap = 8.0f,
    .chips = {
        Chip { .type = ChipType::Choice, .label = "Low", .selected = false },
        Chip { .type = ChipType::Choice, .label = "Medium", .selected = true },
        Chip { .type = ChipType::Choice, .label = "High", .selected = false, .selected_color = 0xFFDC2626 },
    }
};
```

### 3. Deletable Tag Input Chip
```cpp
auto emailTag = Chip {
    .type = ChipType::Input,
    .label = "alex@enki.dev",
    .avatar_icon = "👤",
    .deletable = true,
    .on_deleted = []() {
        // Remove tag from recipient list
    }
};
```

---

## See Also
- [**Badge**](./badge.md) — Unclickable or overlay status indicators.
- [**Button**](./button.md) — Standard full-sized interactive buttons.
- [**Wrap**](../Layout/wrap.md) — Used as the parent container to wrap chip lists.
