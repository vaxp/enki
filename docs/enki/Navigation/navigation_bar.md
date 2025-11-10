# NavigationBar

> A multi-style navigation bar widget supporting Material 3 bottom bars, macOS floating glassmorphism capsule docks, desktop top header bars, and segmented tabs with 600+ FPS sliding indicator physics.

- **Header File**: `#include "enki/widgets/navigation_bar.hpp"`
- **C++ Class**: `enki::NavigationBarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::NavigationBar` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::NavigationBarOptions`
- **Item Descriptor**: `enki::NavigationBarItem`
- **Enums**: `enki::NavigationBarStyle`, `enki::NavIndicatorStyle`, `enki::NavItemLayout`

---

## Overview

`NavigationBar` is a modern navigation component supporting four distinct architectural styles:
1. **`BottomStandard`**: Standard mobile/tablet bottom navigation bar.
2. **`FloatingPill`**: Floating capsule island with multi-layer glassmorphism shadows (macOS Dock / iOS 18 style).
3. **`TopHeader`**: Full desktop/web top navigation bar supporting leading brand logos, search bar placeholders, and trailing action buttons.
4. **`SegmentedCapsule`**: Compact segmented pill switcher.

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class NavigationBarStyle {
    BottomStandard,   ///< Material 3 style bottom bar
    FloatingPill,     ///< macOS Dock / iOS 18 floating capsule island with glassmorphism
    TopHeader,        ///< Desktop top header bar with brand, search & actions
    SegmentedCapsule  ///< Compact pill-shaped segmented tabs
};

enum class NavIndicatorStyle {
    Pill,             ///< Rounded capsule pill enclosing active item
    Underline,        ///< Glowing underline beneath active item
    Dot,              ///< Radiant glowing dot beneath active item
    Glow,             ///< Ambient gradient aura behind active item
    None              ///< Color change only without dedicated shape
};

enum class NavItemLayout {
    Vertical,         ///< Icon on top, label on bottom (Mobile standard)
    Horizontal,       ///< Icon leading on left, label on right (Desktop standard)
    IconOnly,         ///< Icon only (Compact dock)
    LabelOnly         ///< Text label only (Tab strip)
};

} // namespace enki
```

### `NavigationBarItem` Descriptor
```cpp
namespace enki {

struct NavigationBarItem {
    std::string label;
    IconData    icon;
    IconData    selected_icon;      ///< Optional active state icon
    std::string badge;              ///< Numeric count ("5", "99+") or tag text ("NEW")
    bool        dot_badge = false;  ///< Glowing notification dot
    bool        enabled   = true;
    std::string sublabel;
    std::string tooltip;
    std::string id;

    NavigationBarItem(std::string lbl, IconData ic, std::string bdg = "", bool dot = false);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct NavigationBar {
    Key                                   key               = Key::none();
    std::vector<NavigationBarItem>        items;
    int                                   selected_index    = 0;

    std::function<void(int)>              on_item_selected  = nullptr;
    std::function<void(int)>              on_item_reselect  = nullptr;
    std::function<void(std::string_view)> on_action_clicked = nullptr;

    NavigationBarOptions                  options           = {};

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `std::vector<NavigationBarItem>` | `{}` | Selectable navigation items. |
| `selected_index` | `int` | `0` | 0-indexed active item. |
| `on_item_selected` | `Function(int)` | `nullptr` | Callback fired when selecting an item. |
| `on_item_reselect` | `Function(int)` | `nullptr` | Callback fired when re-clicking the already-active item (e.g. scroll to top). |
| `on_action_clicked`| `Function(string_view)` | `nullptr` | Callback fired when desktop top header trailing actions are clicked. |
| `options.style` | `NavigationBarStyle` | `BottomStandard` | Overall navigation bar layout paradigm. |
| `options.indicator_style`| `NavIndicatorStyle` | `Pill` | Shape of the hardware-accelerated animated sliding indicator. |
| `options.item_layout`| `NavItemLayout` | `Vertical` | Internal arrangement of icon and text label. |

---

## Code Examples (From `widgets_demo/navigation_bar_demo/main.cpp`)

### 1. Desktop Top Header Bar with Brand & Actions
```cpp
#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildDesktopHeader(int activeIndex, auto onSelect) {
    return NavigationBar {
        .items = {
            {"Dashboard", Icons::Material::dashboard()},
            {"Projects",  Icons::Material::folder()},
            {"Team",      Icons::Material::people(), "3"},
            {"Settings",  Icons::Material::settings()},
        },
        .selected_index = activeIndex,
        .on_item_selected = onSelect,
        .options = {
            .style = NavigationBarStyle::TopHeader,
            .item_layout = NavItemLayout::Horizontal,
            .indicator_style = NavIndicatorStyle::Underline,
            .leading_title = "ENKI Studio",
            .show_search_placeholder = true,
            .trailing_actions = {"Docs", "GitHub"},
        }
    };
}
```

### 2. Floating macOS Dock / iOS 18 Capsule Island
```cpp
WidgetPtr buildFloatingDock(int activeIndex, auto onSelect) {
    return NavigationBar {
        .items = {
            {"Home",     Icons::Material::home()},
            {"Search",   Icons::Material::search()},
            {"Inbox",    Icons::Material::mail(), "12"},
            {"Terminal", Icons::Material::terminal()},
        },
        .selected_index = activeIndex,
        .on_item_selected = onSelect,
        .options = {
            .style = NavigationBarStyle::FloatingPill,
            .item_layout = NavItemLayout::IconOnly,
            .indicator_style = NavIndicatorStyle::Pill,
            .enable_glassmorphism = true,
            .corner_radius = 28.0f,
            .height = 56.0f,
            .width = 340.0f,
        }
    };
}
```

---

## See Also
- [**NavigationRail**](./navigation_rail.md) — Vertical side rail navigation.
- [**TabBar**](./tab_bar.md) — Lightweight tab strip.
- [**Breadcrumb**](./breadcrumb.md) — Path breadcrumb indicators.
