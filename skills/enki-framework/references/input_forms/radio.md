# Radio

> A material-style radio button widget for mutually exclusive single selection within a group.

- **Header File**: `#include "enki/widgets/radio.hpp"`
- **C++ Class**: `enki::RadioWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Radio` (converts implicitly to `WidgetPtr`)
- **Selection Matching**: Checked when `value == group_value`

---

## Overview

`Radio` buttons let users select exactly one option from a set. In Enki, each `Radio` is assigned an integer `value`. It checks itself against the parent's current `group_value`. When clicked, it passes its `value` to the `on_changed` callback so the state can update `group_value`.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Radio {
    int                      value          = 0;
    int                      group_value    = 0;
    std::function<void(int)> on_changed     = nullptr;
    
    float                    size           = 20.0f; // Outer diameter
    float                    inner_size     = 10.0f; // Inner dot diameter
    float                    border_width   = 2.0f;
    
    Color                    active_color   = 0xFF2563EB; // Primary Blue
    Color                    inactive_color = 0xFF64748B; // Slate 500
    Color                    hover_color    = 0xFF3B82F6;
    Color                    bg_color       = 0x00000000;
    
    bool                     disabled       = false;
    Color                    disabled_color = 0xFF94A3B8;
    Key                      key            = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `int` | `0` | The distinct identifier value this radio represents. |
| `group_value` | `int` | `0` | The currently selected value of the radio group. |
| `on_changed` | `std::function<void(int)>` | `nullptr` | Callback invoked with `value` when this radio is clicked. |
| `size` | `float` | `20.0f` | Outer ring diameter in pixels. |
| `inner_size` | `float` | `10.0f` | Filled inner circle diameter when active. |
| `active_color` | `Color` | `0xFF2563EB` | Color of outer ring and inner dot when selected. |
| `inactive_color`| `Color` | `0xFF64748B` | Border color when unselected. |
| `disabled` | `bool` | `false` | Disables interaction and applies `disabled_color`. |

---

## Code Examples (From `widgets_demo/radio_demo/main.cpp`)

### 1. Mutually Exclusive Option Group
```cpp
#include "enki/widgets/radio.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSubscriptionSelector(int selectedTier, std::function<void(int)> onSelect) {
    auto buildOption = [&](int id, const std::string& name) {
        return row({
            .align_items = Align::Center,
            .gap = 12_px,
            .children = {
                Radio {
                    .value = id,
                    .group_value = selectedTier,
                    .on_changed = onSelect,
                },
                text(name, { .color = 0xFFF1F5F9 }),
            }
        });
    };

    return column({
        .gap = 8_px,
        .children = {
            buildOption(1, "Free Tier"),
            buildOption(2, "Pro Developer ($19/mo)"),
            buildOption(3, "Enterprise Scaler ($99/mo)"),
        }
    });
}
```

---

## See Also
- [**Checkbox**](./checkbox.md) — Multi-selection boolean toggles.
- [**Switch**](./switch.md) — Single sliding on/off setting.
- [**ComboBox**](./combo_box.md) — Dropdown single/multi selector for larger lists.
