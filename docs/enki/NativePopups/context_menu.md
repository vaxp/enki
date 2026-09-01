# ContextMenu

> A native secondary-click (right click) context menu widget built on NativePopup, providing nested submenus, keyboard shortcut hints, destructive danger styling, and intelligent screen boundary fitting.

- **Header File**: `#include "enki/widgets/context_menu.hpp"`
- **C++ Class**: `enki::ContextMenuWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ContextMenu` (converts implicitly to `WidgetPtr`)
- **Item Hierarchy**: `ContextMenuItemBase`, `ContextMenuItem`, `ContextMenuDivider`, `ContextMenuSubMenu`
- **Helper Functions**: `contextMenuItem()`, `contextMenuDivider()`, `contextMenuSubMenu()`
- **Options Struct**: `enki::ContextMenuOptions`

---

## Overview

`ContextMenu` wraps any child widget and intercepts secondary pointer clicks (right-click on desktop, long-press on touch devices). It spawns an unclipped native surface at the click coordinates (`NativePopup`). If the click occurs near the screen edge, Enki's boundary collision resolver automatically flips or clamps the menu to stay fully within the visible display area.

---

## C++ API Definition

### Helper Constructors
```cpp
namespace enki {

/// Creates a standard selectable context menu item
inline std::shared_ptr<ContextMenuItem> contextMenuItem(
    std::string           label,
    std::function<void()> on_selected = nullptr,
    std::string           shortcut    = "",
    WidgetPtr             icon        = nullptr,
    bool                  disabled    = false,
    bool                  danger      = false   ///< Renders text in warning red
);

/// Inserts a horizontal divider separator line
inline std::shared_ptr<ContextMenuDivider> contextMenuDivider();

/// Creates a cascading nested submenu
inline std::shared_ptr<ContextMenuSubMenu> contextMenuSubMenu(
    std::string                     label,
    std::vector<ContextMenuItemPtr> children,
    WidgetPtr                       icon     = nullptr,
    bool                            disabled = false
);

} // namespace enki
```

### Styling Options (`ContextMenuOptions`)
```cpp
namespace enki {

struct ContextMenuOptions {
    Color      background_color = 0xFA1F242C; ///< Dark slate surface (ARGB)
    Color      text_color       = 0xFFF0F6FC;
    Color      shortcut_color   = 0xFF8B949E; ///< Shortcut hint color
    Color      hover_color      = 0xFF30363D; ///< Item hover background
    Color      disabled_color   = 0xFF484F58;
    Color      danger_color     = 0xFFF85149; ///< Destructive action red
    Color      border_color     = 0xFF363B42;

    float      border_width     = 1.0f;
    float      border_radius    = 8.0f;
    EdgeInsets padding          = EdgeInsets::all(6.0f);
    float      elevation        = 10.0f;
    float      min_width        = 180.0f;
    float      max_width        = 280.0f;
    float      item_height      = 32.0f;
    float      font_size        = 13.0f;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ContextMenu {
    Key                             key   = Key::none();
    WidgetPtr                       child = nullptr;
    std::vector<ContextMenuItemPtr> items;
    ContextMenuOptions              options;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The target widget receiving right-click / long-press events. |
| `items` | `std::vector<ContextMenuItemPtr>`| `{}` | Sequence of items, dividers, and submenus. |
| `options` | `ContextMenuOptions` | `{}` | Sizing, color palette, and drop shadow configuration. |

---

## Code Examples (From `widgets_demo/context_menu_demo/main.cpp`)

### 1. File Item Context Menu with Danger Action
```cpp
#include "enki/widgets/context_menu.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildFileCard(const std::string& filename) {
    auto fileCardWidget = container({
        .color = 0xFF1E293B,
        .padding = EdgeInsets::all(16.0f),
        .child = text(filename, { .font_size = 14.0f })
    });

    return ContextMenu {
        .child = fileCardWidget,
        .items = {
            contextMenuItem("Open",   []{ /* Open */ },   "Enter"),
            contextMenuItem("Rename", []{ /* Rename */ }, "F2"),
            contextMenuDivider(),
            contextMenuItem("Copy Path", []{ /* Copy */ }, "Ctrl+Shift+C"),
            contextMenuDivider(),
            // Destructive danger item
            contextMenuItem("Move to Trash", []{ /* Delete */ }, "Del", nullptr, false, true),
        }
    };
}
```

### 2. Cascading Submenu within Context Menu
```cpp
auto gitSubmenu = contextMenuSubMenu("Git Actions", {
    contextMenuItem("Pull Changes", []{ /* Pull */ }),
    contextMenuItem("Commit...",    []{ /* Commit */ }, "Ctrl+K"),
    contextMenuItem("Push",         []{ /* Push */ }),
});

auto codeEditorMenu = ContextMenu {
    .child = myCodeEditorWidget,
    .items = {
        contextMenuItem("Cut",   []{ /* Cut */ },   "Ctrl+X"),
        contextMenuItem("Copy",  []{ /* Copy */ },  "Ctrl+C"),
        contextMenuItem("Paste", []{ /* Paste */ }, "Ctrl+V"),
        contextMenuDivider(),
        gitSubmenu,
    }
};
```

---

## See Also
- [**Menu**](./menu.md) — Top-level desktop menu bars.
- [**Popup**](./popup.md) — Generalized native floating surfaces.
- [**Popover**](./popover.md) — Arrow-anchored interactive popover cards.
