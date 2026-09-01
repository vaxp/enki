# Sidebar

> A permanent collapsible desktop side navigation panel that shares layout space with the body content, pushing and resizing adjacent views instead of obscuring them.

- **Header File**: `#include "enki/widgets/sidebar.hpp"`
- **C++ Class**: `enki::SidebarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Sidebar` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::SidebarOptions`
- **Item Descriptor**: `enki::SidebarItem`
- **Side Enum**: `enki::SidebarSide` (`Left`, `Right`)

---

## Overview

Unlike a modal `Drawer` which slides over content with a dark scrim, `Sidebar` is a permanent desktop layout component. It partitions the window into a side panel (`sidebar_content`) and main application area (`body`). When expanded (e.g. 250px), it reveals full labels and groups; when collapsed (e.g. 60px), it smoothly animates down to a compact icon rail while allowing the adjacent body view to expand horizontally.

---

## C++ API Definition

### Configuration Options (`SidebarOptions`)
```cpp
namespace enki {

enum class SidebarSide {
    Left,
    Right
};

struct SidebarOptions {
    float       expanded_width     = 240.0f;
    float       collapsed_width    = 64.0f;
    Color       background_color   = 0xFF1E293B;
    Color       border_color       = 0xFF334155;
    Color       toggle_color       = 0xFF64748B;
    Color       toggle_hover_color = 0xFF818CF8;
    float       border_width       = 1.0f;
    float       toggle_size        = 36.0f;

    SidebarSide side               = SidebarSide::Left;
    bool        collapsible        = true;
    bool        initially_expanded = true;
    bool        show_toggle_button = true;

    constexpr bool operator==(const SidebarOptions&) const = default;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Sidebar {
    Key                       key             = Key::none();
    WidgetPtr                 sidebar_content = nullptr; ///< Panel navigation items
    WidgetPtr                 body            = nullptr; ///< Main content area
    SidebarOptions            options         = {};
    std::function<void(bool)> on_toggle       = nullptr; ///< Called with new expansion state

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `sidebar_content` | `WidgetPtr` | `nullptr` | Content inside the sidebar (navigation links, logo, user account). |
| `body` | `WidgetPtr` | `nullptr` | Main application workspace (e.g. document view, editor, dashboard). |
| `options.expanded_width` | `float` | `240.0f` | Width when expanded. |
| `options.collapsed_width`| `float` | `64.0f` | Width when collapsed (icon-only mode). |
| `options.collapsible` | `bool` | `true` | Allows user to collapse the sidebar via toggle button. |
| `options.show_toggle_button`| `bool` | `true` | Renders a header toggle chevron/hamburger button. |
| `on_toggle` | `Function(bool)` | `nullptr` | Callback receiving `is_expanded` on transition. |

---

## Code Examples (From `widgets_demo/sidebar_demo/main.cpp`)

### 1. Collapsible Desktop Application Shell
```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/sidebar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildDesktopShell(WidgetPtr mainWorkspace) {
    auto navigationLinks = column({
        .padding = EdgeInsets::all(16.0f),
        .gap = 10_px,
        .children = {
            text("Explorer", { .color = 0xFFFFFFFF, .font_size = 14.0f }),
            text("Git Changes", { .color = 0xFF94A3B8, .font_size = 14.0f }),
            text("Terminal", { .color = 0xFF94A3B8, .font_size = 14.0f }),
        }
    });

    return Sidebar {
        .sidebar_content = navigationLinks,
        .body = mainWorkspace,
        .options = {
            .expanded_width = 260.0f,
            .collapsed_width = 64.0f,
            .background_color = 0xFF0F172A, // Dark slate
            .border_color = 0xFF1E293B,
            .side = SidebarSide::Left,
            .collapsible = true,
        },
        .on_toggle = [](bool expanded) {
            std::cout << "Sidebar toggled. Expanded: " << std::boolalpha << expanded << "\n";
        }
    };
}
```

---

## See Also
- [**Drawer**](./drawer.md) — Modal overlay drawer that slides above content.
- [**NavigationRail**](./navigation_rail.md) — Dedicated vertical icon rail without auto-layout pushing.
- [**ListTile**](../Scrolling-Lists/list_tile.md) — Commonly used as sidebar menu links.
