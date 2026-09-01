# IconButton

> A specialized, compact button tailored for vector icons, featuring transparent resting backgrounds, rounded hover highlights, and click ripples.

- **Header File**: `#include "enki/widgets/icon_button.hpp"`
- **C++ Class**: `enki::IconButtonWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::IconButton` (converts implicitly to `WidgetPtr`)
- **Underlying Mechanism**: Composes an interactive `Button` optimized for icon-only action bars

---

## Overview

`IconButton` is the standard control for toolbars, window control buttons, modal close triggers, and floating actions. By default, it has a transparent background (`0x00000000`) and lights up with a subtle hover color when the cursor passes over it.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct IconButton {
    Key            key            = Key::none();
    WidgetPtr      icon           = nullptr;
    ButtonCallback on_pressed     = nullptr;
    bool           disabled       = false;

    // Interactive State Colors
    Color          normal_color   = 0x00000000; // Transparent
    Color          hover_color    = 0x1A000000; // Light translucent tint on hover
    Color          pressed_color  = 0x33000000; // Darker translucent tint on click
    Color          disabled_color = 0x00000000;
    
    // Geometry & Padding
    float          size           = 48.0f;      // Square bounding box (48x48)
    EdgeInsets     padding        = EdgeInsets::all(8.0f);
    
    // Ripple Effect
    bool           enable_ripple  = true;
    Color          ripple_color   = 0x40000000;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `icon` | `WidgetPtr` | `nullptr` | The target icon widget to display inside the button. |
| `on_pressed` | `ButtonCallback` | `nullptr` | Callback invoked on click. Disables the button if `nullptr`. |
| `size` | `float` | `48.0f` | Width and height of the clickable bounding box (provides a 48px touch/click target). |
| `hover_color` | `Color` | `0x1A000000` | Highlight background color displayed when hovered. |
| `pressed_color` | `Color` | `0x33000000` | Background color displayed while clicked. |
| `enable_ripple` | `bool` | `true` | Enables visual circular ripple feedback on click. |
| `padding` | `EdgeInsets` | `all(8.0f)` | Inset padding between the outer bounding box and icon glyph. |

---

## Code Examples (From `widgets_demo/icon_button_demo/main.cpp`)

### 1. Navigation Back Button
```cpp
#include "enki/widgets/icon_button.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildBackButton() {
    return IconButton {
        .icon = icon(Icons::ArrowBack, 24.0f, 0xFFE2E8F0),
        .on_pressed = []() {
            // Navigate back
        },
    };
}
```

### 2. Favorite Toggle with Custom Tinted Hover
```cpp
auto favoriteButton = IconButton {
    .icon = icon(Icons::Favorite, 24.0f, 0xFFEF4444),
    .hover_color = 0x33EF4444, // Red translucent highlight
    .on_pressed = []() { /* Toggle like */ },
};
```

### 3. Compact Toolbar Button
```cpp
auto compactClose = IconButton {
    .size = 32.0f,
    .padding = EdgeInsets::all(4.0f),
    .icon = icon(Icons::Close, 18.0f),
    .on_pressed = []() { /* Close */ },
};
```

---

## See Also
- [**Icon**](./icon.md) — The vector icon glyph placed inside the button.
- [**Button**](./button.md) — Full-sized text/content button.
- [**FloatingActionButton**](./floating_action_button.md) — Floating elevated action button.
