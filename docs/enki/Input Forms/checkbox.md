# Checkbox

> A material-style binary toggle box widget used for boolean settings, terms agreement, and multi-selection lists.

- **Header File**: `#include "enki/widgets/checkbox.hpp"`
- **C++ Class**: `enki::CheckboxWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Checkbox` (converts implicitly to `WidgetPtr`)
- **Underlying Mechanism**: Skia hardware-accelerated canvas checkmark drawing with hover transitions

---

## Overview

`Checkbox` represents a two-state boolean selection (`true` / `false`). When active, it fills with `active_color` and draws a crisp white checkmark path. It natively handles hover highlighting, cursor pointer changes, and disabled states.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Checkbox {
    bool                      value              = false;
    std::function<void(bool)> on_changed         = nullptr;
    
    float                     size               = 18.0f; // Square bounding size
    float                     border_width       = 2.0f;
    float                     border_radius      = 4.0f;  // Rounded corners
    
    Color                     active_color       = 0xFF2563EB; // Primary Blue
    Color                     check_color        = 0xFFFFFFFF; // White checkmark
    
    Color                     border_color       = 0xFF363B42; // Unchecked border
    Color                     hover_border_color = 0xFF58A6FF;
    Color                     inactive_bg_color  = 0x00000000;
    
    bool                      disabled           = false;
    Color                     disabled_color     = 0xFF475569;
    Key                       key                = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `bool` | `false` | Current checked state. |
| `on_changed` | `std::function<void(bool)>` | `nullptr` | Callback invoked when user toggles the checkbox. Automatically disables the widget if `nullptr`. |
| `size` | `float` | `18.0f` | Width and height of the square checkbox box in pixels. |
| `border_radius` | `float` | `4.0f` | Curvature radius for box corners. |
| `active_color` | `Color` | `0xFF2563EB` | Background fill color when checked. |
| `check_color` | `Color` | `0xFFFFFFFF` | Color of the drawn checkmark tick. |
| `border_color` | `Color` | `0xFF363B42` | Border color when unchecked. |
| `hover_border_color`| `Color` | `0xFF58A6FF` | Border outline color when hovered. |
| `disabled` | `bool` | `false` | When true, ignores user clicks and renders with `disabled_color`. |

---

## Code Examples (From `widgets_demo/checkbox_demo/main.cpp`)

### 1. Stateful Checkbox with Label
```cpp
#include "enki/widgets/checkbox.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildRememberMeOption(bool isChecked, std::function<void(bool)> onToggle) {
    return row({
        .align_items = Align::Center,
        .gap = 10_px,
        .children = {
            Checkbox {
                .value = isChecked,
                .on_changed = std::move(onToggle),
            },
            text("Remember me on this machine", { .color = 0xFFCBD5E1 }),
        }
    });
}
```

### 2. Custom Colored Checkbox
```cpp
auto emeraldCheckbox = Checkbox {
    .value = true,
    .active_color = 0xFF10B981, // Emerald 500
    .on_changed = [](bool val) { /* ... */ },
};
```

---

## See Also
- [**Switch**](./switch.md) — Sliding pill toggle for feature activation.
- [**Radio**](./radio.md) — Single-selection mutually exclusive option.
- [**FormField**](./form_field.md) — `CheckboxFormField` with validation error support.
