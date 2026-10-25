# FloatingActionButton

> A prominent, circular elevated button used for primary screen actions, featuring a deep drop shadow and ripple feedback.

- **Header File**: `#include "enki/widgets/floating_action_button.hpp"`
- **C++ Class**: `enki::FloatingActionButtonWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::FloatingActionButton` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::FloatingActionButtonProps`
- **Underlying Composition**: Specialized high-elevation `Button` with default `56px` circular geometry and `12px` drop shadow blur

---

## Overview

The `FloatingActionButton` (FAB) represents the primary action on a screen (e.g. "Create New Document", "Add Item", "Compose"). It is typically anchored at the bottom-right of a view (often positioned within a `Stack`).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct FloatingActionButton {
    Key            key              = Key::none();
    WidgetPtr      child            = nullptr;
    ButtonCallback on_pressed       = nullptr;

    // Interactive State Colors
    Color          normal_color     = 0xFF2563EB; // Primary Blue
    Color          hover_color      = 0xFF3B82F6;
    Color          pressed_color    = 0xFF1D4ED8;
    Color          disabled_color   = 0xFF475569;

    // Geometry
    float          size             = 56.0f;      // Standard FAB size (56x56)
    float          border_radius    = 28.0f;      // Fully circular (size / 2)
    EdgeInsets     padding          = EdgeInsets::all(16.0f);

    // Deep Shadow Elevation
    Color          shadow_color     = 0x60000000;
    float          shadow_blur      = 12.0f;
    float          shadow_offset_dy = 6.0f;

    // Ripple
    bool           enable_ripple    = true;
    Color          ripple_color     = 0x40FFFFFF; // Bright ripple over saturated background
    bool           disabled         = false;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content placed at the center of the FAB (usually an `Icon` or an icon + text `Row`). |
| `on_pressed` | `ButtonCallback` | `nullptr` | Click handler function. Disables the button if null. |
| `size` | `float` | `56.0f` | Diameter of the circular button in logical pixels. |
| `normal_color` | `Color` | `0xFF2563EB` | Base background color (primary blue). |
| `hover_color` | `Color` | `0xFF3B82F6` | Color when pointer hovers over the FAB. |
| `pressed_color` | `Color` | `0xFF1D4ED8` | Color while holding down mouse button. |
| `shadow_blur` | `float` | `12.0f` | Radius of the soft drop shadow. |
| `shadow_offset_dy` | `float` | `6.0f` | Vertical offset of the shadow giving high elevation. |

---

## Code Examples (From `widgets_demo/fab_demo/main.cpp`)

### 1. Standard Circular FAB
```cpp
#include "enki/widgets/floating_action_button.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildAddFAB() {
    return FloatingActionButton {
        .child = icon(Icons::Add, 24.0f, 0xFFFFFFFF),
        .on_pressed = []() {
            // Trigger item creation
        },
    };
}
```

### 2. Positioned FAB in Screen Corner (Pattern with `Stack`)
```cpp
#include "enki/widgets/stack.hpp"

auto screen = Stack {
    .fit = StackFit::Expand,
    .children = {
        mainContentList,
        Positioned {
            .bottom = 24_px,
            .right = 24_px,
            .child = FloatingActionButton {
                .child = icon(Icons::Add),
                .on_pressed = [](){ /* Create */ }
            }
        }
    }
};
```

### 3. Extended FAB (Icon + Text)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

auto extendedFAB = FloatingActionButton {
    .size = 48.0f,
    .border_radius = 24.0f,
    .padding = EdgeInsets::symmetric(12.0f, 20.0f),
    .child = row({
        .align_items = Align::Center,
        .gap = 8_px,
        .children = {
            icon(Icons::Edit, 20.0f, 0xFFFFFFFF),
            text("Compose", { .font_weight = FontWeight::Bold }),
        }
    }),
    .on_pressed = [](){ /* Compose */ }
};
```

---

## See Also
- [**Button**](./button.md) — Standard in-layout button widget.
- [**IconButton**](./icon_button.md) — Compact toolbar icon button.
- [**Positioned**](../Layout/positioned.md) — For placing the FAB at fixed screen coordinates.
