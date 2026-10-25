# Card

> A material-style elevated surface widget featuring background coloring, rounded corners, drop shadows, and border outlines.

- **Header File**: `#include "enki/widgets/card.hpp"`
- **C++ Class**: `enki::CardWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::Card` (converts implicitly to `WidgetPtr`)
- **Underlying Composition**: Specialized `Container` with automated `BoxDecoration` and `BoxShadow` based on `elevation`

---

## Overview

`Card` is used to group related information and actions into visually distinct modules. It provides sensible Material Design defaults (dark slate background `0xFF1E293B`, `12px` rounded corners, and soft drop shadows modulated by the `elevation` property).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Card {
    Color                 color         = 0xFF1E293B;                       // Background fill
    Color                 shadow_color  = 0x40000000;                       // Soft black drop shadow
    float                 elevation     = 8.0f;                             // Elevation controlling blur & offset
    BorderRadius          border_radius = BorderRadius::circular(12.0f);    // Rounded corners
    std::optional<Border> border        = std::nullopt;                     // Optional stroke border
    StyleInsets           margin        = StyleInsets::all(4.0f);           // Margin around the card
    StyleInsets           padding       = StyleInsets::all(0.0f);           // Inner padding
    WidgetPtr             child         = nullptr;
    Key                   key           = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content widget displayed inside the card. |
| `color` | `Color` | `0xFF1E293B` | Surface background color (32-bit ARGB). |
| `elevation` | `float` | `8.0f` | Visual height off the page; dynamically computes shadow blur and Y-offset. |
| `shadow_color` | `Color` | `0x40000000` | 25% black shadow color. |
| `border_radius` | `BorderRadius` | `circular(12.0f)` | Curvature radius of all four corners. |
| `border` | `std::optional<Border>` | `std::nullopt` | Optional outline stroke (e.g. `Border(0xFF334155, 1.0f)`). |
| `padding` | `StyleInsets` | `all(0.0f)` | Padding between the card boundaries and child widget. |
| `margin` | `StyleInsets` | `all(4.0f)` | Margin spacing outside the card boundary. |

---

## Code Examples (From `widgets_demo/card_demo/main.cpp`)

### 1. Elevated Card with Border
```cpp
#include "enki/widgets/card.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildMetricCard() {
    return Card {
        .color = 0xFF1E293B,
        .elevation = 16.0f,
        .border_radius = BorderRadius::circular(16.0f),
        .border = Border(0xFF38BDF8, 1.5f), // Sky-blue border
        .padding = StyleInsets::all(24_px),
        .child = text("Active Workstation Session", {
            .font_size = 16.0f,
            .font_weight = FontWeight::SemiBold,
        })
    };
}
```

### 2. User Profile Card
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/avatar.hpp"
#include "enki/widgets/button.hpp"

auto profileCard = Card {
    .elevation = 10.0f,
    .padding = StyleInsets::all(16_px),
    .child = row({
        .align_items = Align::Center,
        .gap = 12_px,
        .children = {
            Avatar { .initials = "JD", .background_color = 0xFF4FD1C5 },
            column({
                .children = {
                    text("Jane Doe", { .font_weight = FontWeight::Bold }),
                    text("Software Engineer", { .color = 0xFFA0AEC0, .font_size = 12.0f }),
                }
            }),
            spacer(),
            button(text("Follow"), [](){ /* ... */ }),
        }
    })
};
```

---

## See Also
- [**Container**](../Layout/container.md) — The underlying box model supporting custom gradients and complex decorations.
- [**Divider**](./divider.md) — Separator for sections inside a card.
