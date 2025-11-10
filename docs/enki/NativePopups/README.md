# Enki Native Popups Suite

> True hardware-accelerated, compositor-level native floating surfaces built on the `NativePopup` architecture, supporting unbounded cross-window positioning, subsurfaces, desktop menu bars, context menus, tooltips, popovers, and native file pickers.

The **Native Popups** category represents one of Enki's most powerful desktop capabilities. Rather than rendering popups as constrained layers inside the application's widget tree (which are clipped by the window's boundaries), Enki leverages its native shell architecture (`NativePopup`) to create dedicated windowing surfaces (e.g. Wayland `xdg_popup` / subsurfaces and X11 override-redirect windows).

---

## Architectural Advantage: Native Popups vs In-Tree Overlays

```
┌────────────────────────────────────────────────────────────┐
│                    Desktop Workspace                       │
│                                                            │
│   ┌───────────────────────────────┐                        │
│   │ Application Window            │                        │
│   │                               │                        │
│   │   [Button]                    │                        │
│   │      │                        │                        │
│   └──────┼────────────────────────┘                        │
│          ▼                                                 │
│   ┌───────────────────────────────┐                        │
│   │ Native Popup / Menu / Tooltip │ ◄── Floats FREELY      │
│   │ (Unclipped Native Surface)    │     outside window     │
│   └───────────────────────────────┘     boundaries!        │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

- **Unbounded Floating**: Tooltips, context menus, and popovers can overflow beyond the physical window boundaries onto the desktop or neighboring displays without clipping.
- **Independent Buffer Presentation**: Powered by Skia with its own render loop, providing pristine anti-aliased shadows, rounded corners, and custom SkSL shaders.
- **Compositor Grab & Dismiss**: Native click-outside detection and dismissal handled accurately by the windowing server.

---

## Widget Catalog (Native Popups)

| # | Widget / Feature | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Popup**](./popup.md) | `struct Popup`, `class PopupWidget` | `<enki/widgets/popup.hpp>` | Universal native floating surface with 15 placement modes and programmatic controller. |
| 2 | [**Menu**](./menu.md) | `struct MenuBar`, `struct Menu` | `<enki/widgets/menu.hpp>` | Desktop application menu bar, dropdowns, cascading submenus, and checkable items. |
| 3 | [**ContextMenu**](./context_menu.md) | `struct ContextMenu`, `contextMenuItem` | `<enki/widgets/context_menu.hpp>` | Secondary-click (right click) context menu with icons, shortcuts, and divider lines. |
| 4 | [**Tooltip**](./tooltip.md) | `class Tooltip`, `tooltip()` | `<enki/widgets/tooltip.hpp>` | Floating tooltip card with auto-positioning, hover delays, and pointer tail arrows. |
| 5 | [**Popover**](./popover.md) | `class Popover`, `popover()` | `<enki/widgets/popover.hpp>` | Interactive floating card with arrow pointer, anchored to any widget or trigger. |
| 6 | [**FilePicker**](./file_picker.md) | `struct FilePicker`, `FilePicker::show` | `<enki/widgets/file_picker.hpp>` | Desktop filesystem browser for open file, open multiple, select folder, and save file. |

---

## Quick Example (Context Menu + Tooltip)

```cpp
#include "enki/widgets/context_menu.hpp"
#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildDesktopItem() {
    // 1. Tooltip attached to a card
    auto card = tooltip(
        container({
            .color = 0xFF1E293B,
            .padding = EdgeInsets::all(16.0f),
            .child = text("Right click me for menu")
        }),
        "Click or right-click to inspect options",
        { .position = TooltipPosition::Top }
    );

    // 2. Wrap with native ContextMenu
    return ContextMenu {
        .child = card,
        .items = {
            contextMenuItem("Open Item", []{ /* Open action */ }, "Enter"),
            contextMenuItem("Rename", []{ /* Rename action */ }, "F2"),
            contextMenuDivider(),
            contextMenuItem("Delete", []{ /* Delete action */ }, "Del", nullptr, false, true),
        }
    };
}
```
