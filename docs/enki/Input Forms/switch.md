# Switch

> A modern pill-shaped toggle switch widget for binary on/off settings and feature toggles.

- **Header File**: `#include "enki/widgets/switch.hpp"`
- **C++ Class**: `enki::SwitchWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Switch` (converts implicitly to `WidgetPtr`)
- **Underlying Engine**: Skia 2D anti-aliased pill capsule drawing with smooth sliding thumb animations

---

## Overview

`Switch` is the preferred control for enabling/disabling application settings (e.g. "Dark Mode", "Notifications", "Hardware Acceleration"). It features an iOS/Material-style rounded capsule track with a circular sliding thumb and distinct active and inactive color palettes.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Switch {
    bool                      value                 = false;
    std::function<void(bool)> on_changed            = nullptr;
    
    float                     width                 = 44.0f; // Track width
    float                     height                = 24.0f; // Track height
    float                     thumb_padding         = 2.0f;  // Inset padding for thumb
    
    // Active Colors
    Color                     active_color          = 0xFF34C759; // Green (iOS/Material)
    Color                     active_thumb_color    = 0xFFFFFFFF; // White
    
    // Inactive Colors
    Color                     inactive_color        = 0xFFE5E5EA; // Slate light track
    Color                     inactive_thumb_color  = 0xFFFFFFFF;
    
    // Hover Highlights
    Color                     hover_color           = 0xFF28A745;
    Color                     hover_inactive_color  = 0xFFD1D1D6;
    
    // Disabled State
    bool                      disabled              = false;
    Color                     disabled_color        = 0xFFF2F2F7;
    Color                     disabled_thumb_color  = 0xFFE5E5EA;
    Key                       key                   = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `bool` | `false` | The current toggle position (`true` for On, `false` for Off). |
| `on_changed` | `std::function<void(bool)>` | `nullptr` | Callback invoked when user slides or clicks the switch. |
| `width` | `float` | `44.0f` | Total capsule width in logical pixels. |
| `height` | `float` | `24.0f` | Capsule height in logical pixels. |
| `thumb_padding` | `float` | `2.0f` | Spacing between thumb circle and track boundary. |
| `active_color` | `Color` | `0xFF34C759` | Background track fill when in the On position. |
| `inactive_color`| `Color` | `0xFFE5E5EA` | Background track fill when in the Off position. |
| `disabled` | `bool` | `false` | Prevents user interaction when true. |

---

## Code Examples (From `widgets_demo/switch_demo/main.cpp`)

### 1. Feature Setting Toggle
```cpp
#include "enki/widgets/switch.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildDarkModeSetting(bool isDark, std::function<void(bool)> onToggle) {
    return row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .width = 100_pct,
        .children = {
            text("Enable Hardware Acceleration", { .color = 0xFFF8FAFC }),
            Switch {
                .value = isDark,
                .on_changed = std::move(onToggle),
                .active_color = 0xFF38BDF8, // Custom sky-blue track
            }
        }
    });
}
```

---

## See Also
- [**Checkbox**](./checkbox.md) — Square checkmark toggle.
- [**Radio**](./radio.md) — Grouped single-selection options.
