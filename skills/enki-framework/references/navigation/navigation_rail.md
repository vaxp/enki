# NavigationRail

> A vertical side navigation rail widget designed for desktop and tablet screens, supporting collapsed icon mode (72px), expanded label mode (220px), header slots, and badge counters.

- **Header File**: `#include "enki/widgets/navigation_rail.hpp"`
- **C++ Class**: `enki::NavigationRailWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::NavigationRail` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::NavigationRailOptions`
- **Item Descriptor**: `enki::NavigationRailItem`

---

## Overview

`NavigationRail` offers a compact vertical navigation bar placed along the left edge of desktop applications. Unlike a heavy sidebar, it defaults to a slim 72px rail displaying icons with active pill indicators and tooltips, while smoothly expanding to 220px to show labels and badge counters when toggled.

---

## C++ API Definition

### `NavigationRailItem` Descriptor
```cpp
namespace enki {

struct NavigationRailItem {
    std::string label;
    IconData    icon;
    std::string badge;  ///< Optional badge text (e.g. "3", "NEW")
};

} // namespace enki
```

### Configuration Options (`NavigationRailOptions`)
```cpp
namespace enki {

struct NavigationRailOptions {
    Color background_color   = 0xFF1E293B;
    Color border_color       = 0xFF334155;
    Color active_color       = 0xFF818CF8;
    Color inactive_color     = 0xFF64748B;
    Color indicator_color    = 0x1A818CF8; // Translucent active pill highlight
    Color hover_color        = 0x0FFFFFFF;
    Color badge_color        = 0xFFEF4444;
    Color badge_text_color   = 0xFFFFFFFF;

    float collapsed_width    = 72.0f;
    float expanded_width     = 220.0f;
    float item_height        = 52.0f;
    float icon_font_size     = 20.0f;
    float label_font_size    = 13.0f;
    float indicator_radius   = 12.0f;
    float header_height      = 56.0f;

    bool  initially_expanded = true;

    constexpr bool operator==(const NavigationRailOptions&) const = default;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct NavigationRail {
    Key                             key              = Key::none();
    std::vector<NavigationRailItem> items;
    int                             selected_index   = 0;
    std::function<void(int)>        on_item_selected = nullptr;
    NavigationRailOptions           options          = {};
    WidgetPtr                       header           = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `std::vector<NavigationRailItem>`| `{}` | List of navigation items with label, icon, and optional badge. |
| `selected_index` | `int` | `0` | 0-indexed currently active rail item. |
| `on_item_selected` | `std::function<void(int)>`| `nullptr` | Callback fired when a rail item is selected. |
| `header` | `WidgetPtr` | `nullptr` | Optional top slot (e.g. application logo or expand toggle button). |
| `options` | `NavigationRailOptions` | `{}` | Width thresholds, colors, and initial expansion state. |

---

## Code Examples (From `widgets_demo/navigation_rail_demo/main.cpp`)

### 1. Vertical Desktop Navigation Rail
```cpp
#include "enki/widgets/navigation_rail.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildDesktopNavigation(int currentTab, std::function<void(int)> onSelect) {
    return NavigationRail {
        .items = {
            {"Dashboard", Icons::Material::dashboard(), ""},
            {"Analytics", Icons::Material::analytics(), "2"},
            {"Users",     Icons::Material::people(),    ""},
            {"Settings",  Icons::Material::settings(),  ""},
        },
        .selected_index = currentTab,
        .on_item_selected = std::move(onSelect),
        .options = {
            .collapsed_width = 72.0f,
            .active_color = 0xFF38BDF8, // Sky 400
        }
    };
}
```

---

## See Also
- [**Sidebar**](./sidebar.md) — Permanent expandable side navigation panel.
- [**NavigationBar**](./navigation_bar.md) — Horizontal top/bottom navigation bar.
- [**Drawer**](./drawer.md) — Modal overlay navigation panel.
