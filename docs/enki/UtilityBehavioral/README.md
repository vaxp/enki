# Enki Utility & Behavioral Suite

> Fundamental structural and event-routing primitives for controlling subtree visibility, layout space preservation, and hit-testing pass-through.

The **Utility / Behavioral** subsystem provides essential layout and event management primitives that operate directly on the widget and render trees without requiring component destruction or costly tree rebuilds.

---

## Architectural Highlights

- **Non-Destructive Visibility Control**: `Visibility` toggles whether a child widget is painted on the screen. Unlike conditional C++ branching (e.g. `condition ? widget : nullptr`), which destroys the underlying `Element` and loses transient state (such as text field cursors, scroll offsets, or active controllers), `Visibility` can retain the element in the tree (`maintain_state = true`) and preserve its layout footprint (`maintain_size = true`) to prevent unwanted layout reflow.
- **Precision Hit-Testing Control**: `IgnorePointer` intercepts the pointer event dispatch pipeline. When `ignoring = true`, mouse clicks, hovering, scrolls, and drag gestures completely bypass the widget and reach whatever interactive surfaces are located underneath—ideal for HUD diagnostic overlays, watermarks, and disabled state wrappers.

---

## Widget Catalog (Utility / Behavioral)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Visibility**](./visibility.md) | `struct Visibility`, `visibility(...)` | `<enki/widgets/utility.hpp>` | Controls child rendering visibility with options to maintain size, state, and animations. |
| 2 | [**IgnorePointer**](./ignore_pointer.md) | `struct IgnorePointer`, `ignorePointer(...)` | `<enki/widgets/utility.hpp>` | Selectively absorbs or passes pointer/mouse events through to underlying widgets. |

---

## Quick Example (HUD Overlay with Click-Through & Collapsible Stats)

```cpp
#include "enki/widgets/utility.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildGameViewportWithOverlay(bool showStats, bool lockClicks) {
    // 1. Underlying interactive game canvas
    auto gameSurface = container({
        .color = 0xFF0B0F17,
        .child = button(text("Click to Interact"), [] {
            std::cout << "Game canvas clicked!\n";
        })
    });

    // 2. Translucent diagnostic overlay
    auto hudOverlay = ignorePointer({
        .ignoring = lockClicks, // When true, clicks pass right through to gameSurface
        .child = container({
            .color = 0x301E1B4B,
            .padding = EdgeInsets::all(16.0f),
            .child = visibility({
                .visible = showStats,
                .maintain_size = true, // Keeps space allocated even when hidden
                .child = text("FPS: 144 • Frame Time: 6.9ms", { .color = 0xFF38BDF8 })
            })
        })
    });

    return stack({
        .children = { gameSurface, hudOverlay }
    });
}
```
