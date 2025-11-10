# ListTile

> A standardized list row widget implementing Material/Desktop design standards with dedicated `leading`, `title`, `subtitle`, and `trailing` slots, interactive hover/press states, and selection styling.

- **Header File**: `#include "enki/widgets/list_tile.hpp"`
- **C++ Class**: `enki::RenderListTile` (inherits from `enki::RenderBox`)
- **Declarative Struct**: `enki::ListTile` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ListTileProps`
- **Enum**: `enki::VisualDensity` (`Standard`, `Compact`, `Comfortable`)

---

## Overview

`ListTile` is the standard building block for linear lists, settings panes, sidebars, and navigation drawers. It arranges four primary content slots horizontally:
```
┌────────────────────────────────────────────────────────────┐
│ [Leading]   Title Text                          [Trailing] │
│             Subtitle secondary text                        │
└────────────────────────────────────────────────────────────┘
```
It handles pointer states (hover tint, active press highlight, ripple splash), tap gestures (`on_tap`, `on_long_press`, `on_secondary_tap`), and selection background highlighting.

---

## C++ API Definition

### Visual Density Enum
```cpp
namespace enki {

enum class VisualDensity {
    Standard,     ///< Standard height (56px single-line / 72px two-line)
    Compact,      ///< Dense compact display (48px height)
    Comfortable   ///< Spacious padding for touch/large monitors
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ListTile {
    Key                   key                 = Key::none();

    // Slots
    WidgetPtr             leading             = nullptr;
    WidgetPtr             title               = nullptr;
    WidgetPtr             subtitle            = nullptr;
    WidgetPtr             trailing            = nullptr;

    // State & Selection
    bool                  selected            = false;
    bool                  enabled             = true;
    bool                  autofocus           = false;

    // Callbacks
    std::function<void()> on_tap              = nullptr;
    std::function<void()> on_long_press       = nullptr;
    std::function<void()> on_secondary_tap   = nullptr;

    // Sizing & Density
    VisualDensity         visual_density      = VisualDensity::Standard;
    EdgeInsets            content_padding     = EdgeInsets::symmetric(0.0f, 16.0f);
    float                 leading_gap         = 16.0f;
    float                 trailing_gap        = 8.0f;

    // Colors & Highlighting
    Color                 tile_color          = Colors::Transparent;
    Color                 hover_color         = 0x0DFFFFFF; // 5% white tint
    Color                 pressed_color       = 0x1AFFFFFF; // 10% white tint
    Color                 selected_color      = 0x1A2563EB; // Light primary blue tint
    Color                 focus_color         = 0x1A2563EB;
    Color                 disabled_color      = 0x40808080;

    BorderRadius          shape               = BorderRadius::zero();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `leading` | `WidgetPtr` | `nullptr` | Leftmost slot (icon, avatar, or checkbox). |
| `title` | `WidgetPtr` | `nullptr` | Primary text label. |
| `subtitle` | `WidgetPtr` | `nullptr` | Secondary description displayed beneath `title`. |
| `trailing` | `WidgetPtr` | `nullptr` | Rightmost slot (badge, chevron, switch, or button). |
| `selected` | `bool` | `false` | When true, renders with `selected_color` background. |
| `enabled` | `bool` | `true` | Controls interactivity and pointer cursor (`SystemCursor::Pointer`). |
| `on_tap` | `Function()` | `nullptr` | Click/tap handler. |
| `on_secondary_tap`| `Function()` | `nullptr` | Right-click handler (e.g. for opening context menus). |
| `visual_density` | `VisualDensity` | `Standard` | Sizing preset (`Standard`, `Compact`, `Comfortable`). |
| `hover_color` | `Color` | `0x0DFFFFFF` | Highlight overlay color when mouse hovers over the tile. |
| `selected_color` | `Color` | `0x1A2563EB` | Active fill color when `selected == true`. |

---

## Code Examples (From `widgets_demo/list_tile_demo/main.cpp`)

### 1. Standard Settings Option Tile
```cpp
#include "enki/widgets/list_tile.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildAccountTile() {
    return ListTile {
        .leading = icon(Icons::AccountCircle, { .color = 0xFF38BDF8 }),
        .title = text("User Profile", {
            .color = 0xFFFFFFFF,
            .font_size = 15.0f,
            .font_weight = FontWeight::SemiBold
        }),
        .subtitle = text("Manage security, sessions, and billing", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f
        }),
        .trailing = text("›", { .color = 0xFF64748B, .font_size = 20.0f }),
        .on_tap = []() {
            // Navigate to Profile View
        }
    };
}
```

### 2. Compact Selectable Item for File Trees / Sidebars
```cpp
WidgetPtr buildSidebarItem(const std::string& name, bool isCurrent, std::function<void()> onSelect) {
    return ListTile {
        .visual_density = VisualDensity::Compact,
        .shape = BorderRadius::circular(6.0f),
        .selected = isCurrent,
        .selected_color = 0x2238BDF8,
        .leading = icon(Icons::Folder, { .color = isCurrent ? 0xFF38BDF8 : 0xFF94A3B8 }),
        .title = text(name, {
            .color = isCurrent ? 0xFFFFFFFF : 0xFFCBD5E1,
            .font_size = 13.5f
        }),
        .on_tap = std::move(onSelect),
    };
}
```

---

## See Also
- [**ListView**](./list_view.md) — Scrolling parent host for multiple tiles.
- [**Avatar**](../Basic%20UI/avatar.md) — Commonly used in the `leading` slot.
- [**Badge**](../Basic%20UI/badge.md) — Commonly used in the `trailing` slot.
