# Badge

> A badge widget that overlays a notification dot, numeric counter, or custom status label onto any child widget.

- **Header File**: `#include "enki/widgets/badge.hpp"`
- **C++ Class**: `enki::BadgeWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::Badge` (converts implicitly to `WidgetPtr`)
- **Underlying Composition**: Single-child wrapper with an internal `Stack` overlay

---

## Overview

`Badge` is designed for notification icons, inbox counters, and presence indicators. It positions an indicator relative to its child based on an `alignment` anchor (default `TopRight`), with fine-tuned positioning via an `offset` point.

If `label` is omitted, it renders as a sleek status dot sized by `size`. If `label` is provided, it expands into a rounded pill containing the label widget.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Badge {
    Key          key           = Key::none();
    WidgetPtr    child         = nullptr;
    WidgetPtr    label         = nullptr;

    Color        bg_color      = 0xFFEF4444; // Default Red
    Alignment    alignment     = Alignment::TopRight;
    Point        offset        = {0.0f, 0.0f}; // dx, dy fine-tuning
    float        size          = 12.0f; // Dot diameter when label is null
    StyleInsets  padding       = StyleInsets::symmetric(2.0f, 6.0f);
    BorderRadius border_radius = BorderRadius::circular(10.0f);

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The target widget over which the badge floats (e.g. an icon or avatar). |
| `label` | `WidgetPtr` | `nullptr` | Optional label widget (e.g. `text("5")`). If null, renders as a solid status dot. |
| `bg_color` | `Color` | `0xFFEF4444` (Red) | Background fill color of the badge. |
| `alignment` | `Alignment` | `Alignment::TopRight` | Corner anchor relative to child (`TopRight`, `TopLeft`, `BottomRight`, etc.). |
| `offset` | `Point` | `{0.0f, 0.0f}` | Fine-tuning positional translation vector `{dx, dy}` in pixels. |
| `size` | `float` | `12.0f` | Diameter in pixels when used as an unlabelled status dot. |
| `padding` | `StyleInsets` | `2px vert, 6px horiz` | Internal padding around the `label` text. |
| `border_radius` | `BorderRadius` | `circular(10.0f)` | Corner radius of the badge container. |

---

## Code Examples (From `widgets_demo/badge_demo/main.cpp`)

### 1. Unread Message Counter on Bell Icon
```cpp
#include "enki/widgets/badge.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildNotificationBell() {
    return Badge {
        .child = icon(Icons::Notifications, 28.0f),
        .label = text("9+", {
            .color = 0xFFFFFFFF,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        }),
        .bg_color = 0xFFEF4444, // Bright Red
        .offset = {4.0f, -4.0f}, // Slightly nudge outwards
    };
}
```

### 2. Status Indicator Dot
```cpp
auto activeCart = Badge {
    .child = icon(Icons::ShoppingCart, 24.0f),
    .size = 10.0f,
    .bg_color = 0xFF10B981, // Green dot
    .offset = {2.0f, -2.0f},
};
```

---

## See Also
- [**Avatar**](./avatar.md) — For user profile pictures with built-in online indicators.
- [**IconButton**](./icon_button.md) — Often wrapped inside a `Badge`.
