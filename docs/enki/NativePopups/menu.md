# Menu & MenuBar

> A desktop application menu system supporting top-level horizontal menu bars, cascading submenus, checkable toggle items, mutually exclusive radio groups, keyboard shortcut hints, and custom styling.

- **Header File**: `#include "enki/widgets/menu.hpp"`
- **C++ Classes**: `enki::MenuBarWidget`, `enki::MenuWidget` (inherit from `enki::StatefulWidget`)
- **Declarative Structs**: `enki::MenuBar`, `enki::Menu` (convert implicitly to `WidgetPtr`)
- **Item Descriptor**: `enki::MenuItem`
- **Category Model**: `enki::MenuEntry`
- **Options Struct**: `enki::MenuOptions`
- **Item Types**: `enki::MenuItemType` (`Action`, `Checkbox`, `Radio`, `Submenu`, `Divider`)

---

## Overview

`MenuBar` provides the standard desktop application menu bar spanning the top of windows (e.g. `File`, `Edit`, `View`, `Help`). Clicking any category spawns an unclipped `NativePopup` dropdown menu surface. `Menu` offers a standalone anchor wrapper that attaches a menu popup to any custom widget (such as an overflow `⋮` button or action pill).

---

## C++ API Definition

### `MenuItem` Factory Methods
```cpp
namespace enki {

enum class MenuItemType {
    Action,    ///< Standard clickable action
    Checkbox,  ///< Boolean toggle with checkmark indicator
    Radio,     ///< Mutually exclusive radio group option
    Submenu,   ///< Cascading flyout child menu
    Divider    ///< Horizontal separator line
};

class MenuItem {
public:
    static MenuItem action(std::string label, std::function<void()> on_selected,
                           std::string icon = "", std::string shortcut = "", bool enabled = true);

    static MenuItem checkbox(std::string label, bool checked, std::function<void(bool)> on_toggle,
                             std::string shortcut = "", bool enabled = true);

    static MenuItem radio(std::string label, int group, int value, bool checked,
                          std::function<void()> on_selected, bool enabled = true);

    static MenuItem submenu(std::string label, std::vector<MenuItem> items,
                            std::string icon = "", bool enabled = true);

    static MenuItem divider();
};

} // namespace enki
```

### `MenuEntry` & `MenuOptions`
```cpp
namespace enki {

struct MenuEntry {
    std::string           label;
    std::vector<MenuItem> items;
    bool                  enabled = true;

    MenuEntry(std::string label, std::vector<MenuItem> items, bool enabled = true);
};

struct MenuOptions {
    Color      background_color = 0xFA1E293B; ///< Dark slate menu surface (ARGB)
    Color      border_color     = 0xFF334155;
    Color      item_hover_color = 0xFF334155;
    Color      text_color       = 0xFFF1F5F9;
    Color      text_sec_color   = 0xFF94A3B8; ///< Shortcut text color
    Color      accent_color     = 0xFF38BDF8; ///< Checkmark / radio dot color
    Color      disabled_color   = 0xFF64748B;

    float      border_width     = 1.0f;
    float      border_radius    = 8.0f;
    float      elevation        = 12.0f;
    float      min_width        = 190.0f;
    EdgeInsets padding          = EdgeInsets::all(6.0f);
    bool       auto_dismiss     = true;
};

} // namespace enki
```

### Declarative Structs (C++20 Designated Initializers)
```cpp
namespace enki {

struct MenuBar {
    std::vector<MenuEntry> entries;
    MenuOptions            options = MenuOptions();
    Key                    key     = Key::none();

    operator WidgetPtr() const;
};

struct Menu {
    WidgetPtr              child;
    std::vector<MenuItem>  items;
    MenuOptions            options = MenuOptions();
    Key                    key     = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Code Examples (From `widgets_demo/menu_demo/main.cpp`)

### 1. Desktop Application MenuBar
```cpp
#include "enki/widgets/menu.hpp"

using namespace enki;

WidgetPtr buildAppMenuBar() {
    // Cascading Export Submenu
    std::vector<MenuItem> exportSubitems = {
        MenuItem::action("PDF Document (.pdf)", []{ /* Export PDF */ }, "📄"),
        MenuItem::action("PNG Image (.png)",     []{ /* Export PNG */ }, "🖼"),
        MenuItem::action("SVG Vector (.svg)",    []{ /* Export SVG */ }, "📐"),
    };

    // File Category
    std::vector<MenuItem> fileItems = {
        MenuItem::action("New File", []{ /* New */ }, "➕", "Ctrl+N"),
        MenuItem::action("Open File...", []{ /* Open */ }, "📂", "Ctrl+O"),
        MenuItem::action("Save", []{ /* Save */ }, "💾", "Ctrl+S"),
        MenuItem::divider(),
        MenuItem::submenu("Export As", std::move(exportSubitems), "🚀"),
        MenuItem::divider(),
        MenuItem::action("Exit", []{ /* Exit */ }, "🚪", "Ctrl+Q"),
    };

    // View Category with Checkbox
    std::vector<MenuItem> viewItems = {
        MenuItem::checkbox("Show Status Bar", true, [](bool checked){ /* Toggle */ }, "Ctrl+/"),
        MenuItem::checkbox("Show Mini Map",  false, [](bool checked){ /* Toggle */ }),
    };

    return MenuBar {
        .entries = {
            MenuEntry("File", std::move(fileItems)),
            MenuEntry("View", std::move(viewItems)),
        },
        .options = {
            .background_color = 0xFA0F172A,
            .accent_color = 0xFF38BDF8,
        }
    };
}
```

### 2. Standalone Anchor Menu (Overflow Button)
```cpp
#include "enki/widgets/button.hpp"

WidgetPtr buildOverflowMenu() {
    return Menu {
        .child = button(text("Options ▾"), []{}),
        .items = {
            MenuItem::action("Refresh Data", []{ /* Refresh */ }, "🔄"),
            MenuItem::action("Duplicate",    []{ /* Duplicate */ }, "📋"),
            MenuItem::divider(),
            MenuItem::action("Archive",      []{ /* Archive */ }, "📦"),
        }
    };
}
```

---

## See Also
- [**ContextMenu**](./context_menu.md) — Right-click popup menu on widgets.
- [**Popup**](./popup.md) — Base native floating surface engine.
