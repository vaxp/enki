# TabBar

> An animated horizontal tab strip widget featuring a smooth sliding underline indicator, icon/label support, badge notifications, and hover feedback.

- **Header File**: `#include "enki/widgets/tab_bar.hpp"`
- **C++ Class**: `enki::TabBarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::TabBar` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::TabBarOptions`
- **Item Descriptor**: `enki::TabItem`
- **Render Object**: `enki::RenderTabBar` (inherits from `enki::RenderBox`)

---

## Overview

`TabBar` provides horizontal top-level navigation between parallel views. As the user clicks tabs, `RenderTabBar` smoothly animates the indicator underline position (`indicator_x`) and width (`indicator_w`) using an internal lerp interpolation. Each tab can host an icon, a text label, and an optional notification badge.

---

## C++ API Definition

### `TabItem` Descriptor
```cpp
namespace enki {

struct TabItem {
    std::string label;
    IconData    icon;   ///< Icon glyph data (e.g. Icons::Material::explore())
    std::string badge;  ///< Badge counter/string (empty string = no badge)
};

} // namespace enki
```

### Configuration Options (`TabBarOptions`)
```cpp
namespace enki {

struct TabBarOptions {
    Color background_color = 0xFF1E293B;
    Color active_color     = 0xFF818CF8; // Violet 400
    Color inactive_color   = 0xFF64748B; // Slate 500
    Color indicator_color  = 0xFF818CF8;
    Color hover_color      = 0x1A818CF8;
    Color badge_color      = 0xFFEF4444; // Red 500
    Color badge_text_color = 0xFFFFFFFF;

    float tab_height       = 48.0f;
    float indicator_height = 3.0f;
    float indicator_radius = 2.0f;
    float label_font_size  = 13.0f;
    float icon_font_size   = 18.0f;
    float item_min_width   = 80.0f;
    float padding_h        = 16.0f;
    float gap              = 6.0f;       ///< Spacing between icon and label

    bool  show_icons       = true;
    bool  show_labels      = true;

    constexpr bool operator==(const TabBarOptions&) const = default;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct TabBar {
    Key                      key            = Key::none();
    std::vector<TabItem>     tabs;
    int                      selected_index = 0;
    std::function<void(int)> on_tab_changed = nullptr;
    TabBarOptions            options        = {};

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `tabs` | `std::vector<TabItem>` | `{}` | The list of tab descriptors (labels, icons, and badges). |
| `selected_index` | `int` | `0` | 0-indexed currently active tab. |
| `on_tab_changed` | `std::function<void(int)>` | `nullptr` | Callback invoked when user clicks a new tab. |
| `options` | `TabBarOptions` | `{}` | Sizing, typography, and color styling configuration. |

---

## Code Examples (From `widgets_demo/tab_bar_demo/main.cpp`)

### 1. Basic TabBar with Icons and Badges
```cpp
#include "enki/widgets/tab_bar.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildNavigationTabs(int activeIndex, std::function<void(int)> onTabSwitched) {
    return TabBar {
        .tabs = {
            {"Explore",  Icons::Material::explore(),       ""},
            {"Library",  Icons::Material::library_books(), ""},
            {"Inbox",    Icons::Material::chat(),          "3"}, // With red badge "3"
            {"Settings", Icons::Material::settings(),      ""},
        },
        .selected_index = activeIndex,
        .on_tab_changed = std::move(onTabSwitched),
    };
}
```

### 2. Custom Colored Compact TabBar
```cpp
auto customTabBar = TabBar {
    .tabs = {
        {"Overview", {}, ""},
        {"Metrics",  {}, ""},
        {"Alerts",   {}, "12"},
    },
    .selected_index = 0,
    .options = {
        .active_color = 0xFF38BDF8,     // Sky blue
        .indicator_color = 0xFF38BDF8,
        .indicator_height = 2.0f,
        .tab_height = 40.0f,
        .show_icons = false,
    }
};
```

---

## See Also
- [**TabView**](./tab_view.md) — The complementary container displaying the selected tab's page.
- [**NavigationBar**](./navigation_bar.md) — High-level navigation bar with floating dock and top header styles.
