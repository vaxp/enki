# IgnorePointer

> A behavioral utility widget that controls whether its child widget subtree participates in pointer hit testing, allowing mouse clicks and touches to pass directly through to widgets beneath it.

- **Header File**: `#include "enki/widgets/utility.hpp"`
- **C++ Class**: `enki::IgnorePointerWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::IgnorePointer` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::IgnorePointerProps`
- **Factory Helpers**: `enki::ignorePointer()`

---

## Overview

`IgnorePointer` alters the event-dispatch phase of Enki's rendering pipeline. When `ignoring = true`, the widget's render object (`RenderIgnorePointer`) overrides `hitTest()` to return `false`, causing all mouse clicks, hover events, wheel scrolling, and drag gestures to pass right through the child widget as if it were completely transparent to input. It is the primary tool for creating non-interactive visual watermarks, click-through decorative overlays, and disabling user interactions across entire subtrees.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct IgnorePointer {
    Key       key                = Key::none();
    WidgetPtr child              = nullptr;         ///< Content whose pointer events will be blocked/passed
    bool      ignoring           = true;            ///< When true, pointer events pass through
    bool      ignoring_semantics = false;           ///< Whether to ignore accessibility semantics

    operator WidgetPtr() const;
};

inline WidgetPtr ignorePointer(const IgnorePointerProps& props);
inline WidgetPtr ignorePointer(bool ignoring, WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content subtree wrapped by the pointer filter. |
| `ignoring` | `bool` | `true` | When `true`, mouse and touch events pass completely through to lower widgets. |
| `ignoring_semantics` | `bool` | `false` | Whether accessibility tree generators should also ignore this node. |
| `key` | `Key` | `Key::none()` | Optional widget reconciliation key. |

---

## Code Examples (From `widgets_demo/utility_demo/main.cpp`)

### 1. Click-Through Overlay in a Stack
```cpp
#include "enki/widgets/utility.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildClickThroughStack(bool enableClickThrough) {
    return stack({
        .children = {
            // Layer 0: Underlying clickable button
            container({
                .color = 0xFF0F172A,
                .padding = EdgeInsets::all(20.0f),
                .child = button(text("Click Me!"), [] {
                    std::cout << "Underlying button successfully clicked!\n";
                })
            }),

            // Layer 1: Semi-transparent visual overlay
            ignorePointer({
                .ignoring = enableClickThrough, // When true, clicking over this box triggers Layer 0
                .child = container({
                    .color = 0x608B5CF6, // Translucent purple veil
                    .padding = EdgeInsets::all(20.0f),
                    .child = text("Overlay Veil", { .color = 0xFFFFFFFF })
                })
            })
        }
    });
}
```

---

## See Also
- [**Visibility**](./visibility.md) — Controls child rendering and layout presence.
- [**MouseRegion**](../Gestures-Interaction/mouse_region.md) — Tracks mouse hover and cursor changes.
- [**GestureDetector**](../Gestures-Interaction/gesture_detector.md) — Detects taps, pans, and gestures.
