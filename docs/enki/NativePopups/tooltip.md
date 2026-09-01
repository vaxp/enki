# Tooltip

> A hardware-accelerated floating native tooltip surface providing plain text or rich widget content, pointer tail arrows, smart screen collision bounds, and configurable hover delays.

- **Header File**: `#include "enki/widgets/tooltip.hpp"`
- **C++ Class**: `enki::Tooltip` (inherits from `enki::StatefulWidget`)
- **Factory Helpers**: `enki::tooltip(child, message)`, `enki::tooltip(child, rich_message)`, `enki::tooltip(TooltipProps)`
- **Options Struct**: `enki::TooltipOptions`
- **Enums**: `enki::TooltipPosition`, `enki::TooltipTrigger`

---

## Overview

`Tooltip` wraps any target child widget. When hovered or pressed, it spawns a native compositor surface (`NativePopup`) adjacent to the target widget. Because it is rendered on a dedicated surface, it floats cleanly over borders, panels, and neighboring window edges without being clipped.

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class TooltipPosition {
    Auto,   ///< Automatically pick direction based on screen boundaries
    Top,    ///< Positioned above target widget
    Bottom, ///< Positioned below target widget
    Left,   ///< Positioned to the left of target widget
    Right   ///< Positioned to the right of target widget
};

enum class TooltipTrigger {
    Hover,     ///< Appears on mouse hover
    LongPress, ///< Appears on touch hold
    Tap,       ///< Toggles on click/tap
    Manual     ///< Controlled programmatically
};

} // namespace enki
```

### Options Struct (`TooltipOptions`)
```cpp
namespace enki {

struct TooltipOptions {
    Color                     background_color = 0xEE0F172A; ///< Dark translucent slate
    Color                     text_color       = 0xFFF8FAFC;
    Color                     border_color     = 0x3394A3B8;
    float                     border_width     = 1.0f;
    float                     border_radius    = 8.0f;
    EdgeInsets                padding          = EdgeInsets::symmetric(6.0f, 12.0f);
    float                     elevation        = 6.0f;

    float                     arrow_size       = 6.0f;       ///< Pointer arrow tail size
    TooltipPosition           position         = TooltipPosition::Auto;
    TooltipTrigger           trigger          = TooltipTrigger::Hover;

    std::chrono::milliseconds show_delay{400};               ///< Delay before displaying
    std::chrono::milliseconds hide_delay{150};               ///< Delay before dismissing
    bool                      interactive      = false;      ///< Keep open when pointer moves inside
    std::string               custom_shader    = "";

    float                     font_size        = 13.0f;
};

} // namespace enki
```

### Construction Helpers
```cpp
namespace enki {

// Text message helper
WidgetPtr tooltip(WidgetPtr child, std::string message, TooltipOptions options = TooltipOptions());

// Rich widget message helper
WidgetPtr tooltip(WidgetPtr child, WidgetPtr rich_message, TooltipOptions options = TooltipOptions());

// Props struct helper (Designated initializers)
WidgetPtr tooltip(TooltipProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Anchor widget hovered or clicked by the user. |
| `message` | `std::string` | `""` | Plain text tooltip label. |
| `rich_message` | `WidgetPtr` | `nullptr` | Custom widget layout (icons, multiple text rows, badges). |
| `options.position`| `TooltipPosition` | `Auto` | Relative placement orientation. |
| `options.show_delay`| `milliseconds` | `400ms` | Hover duration required before the tooltip reveals. |
| `options.arrow_size`| `float` | `6.0f` | Height and width of the pointer triangle arrow. |
| `options.interactive`| `bool` | `false` | When true, allows user to hover and select text inside tooltip. |

---

## Code Examples (From `widgets_demo/tooltip_demo/main.cpp`)

### 1. Basic Text Tooltip on Action Button
```cpp
#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSaveButton() {
    auto btn = button(text("Save Changes"), []{ /* Save */ });

    return tooltip({
        .child = btn,
        .message = "Saves pending settings to disk (Ctrl+S)",
        .options = {
            .position = TooltipPosition::Top,
            .arrow_size = 6.0f,
        }
    });
}
```

### 2. Rich Multi-line Warning Tooltip
```cpp
WidgetPtr buildDeleteButton() {
    auto dangerBtn = button(text("Delete Account"), []{ /* Delete */ });

    auto warningCard = column({
        .gap = 4_px,
        .children = {
            text("⚠️ Irreversible Action", { .font_weight = FontWeight::Bold, .color = 0xFFFCA5A5 }),
            text("All cloud data will be permanently purged.", { .font_size = 11.0f, .color = 0xFFE2E8F0 }),
        }
    });

    return tooltip({
        .child = dangerBtn,
        .rich_message = warningCard,
        .options = {
            .background_color = 0xEE7F1D1D, // Dark red
            .border_color = 0xFFEF4444,
            .position = TooltipPosition::Bottom,
        }
    });
}
```

---

## See Also
- [**Popover**](./popover.md) — Interactive floating cards with controls and buttons.
- [**Popup**](./popup.md) — Generalized unconstrained floating surfaces.
