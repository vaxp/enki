# Visibility

> A behavioral utility widget that conditionally shows or hides its child widget, with fine-grained control over whether the child's state, size, animations, and interactivity are preserved while invisible.

- **Header File**: `#include "enki/widgets/utility.hpp"`
- **C++ Class**: `enki::VisibilityWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::Visibility` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::VisibilityProps`
- **Factory Helpers**: `enki::visibility()`

---

## Overview

`Visibility` provides a declarative mechanism to toggle visual display without dismantling the element tree:
- **Default Hiding (`visible = false`)**: The child is not painted and by default collapses to take up zero space.
- **Maintain Size (`maintain_size = true`)**: The child continues to participate in the Anu layout pass, occupying its exact layout footprint on the screen, but is completely invisible to the paint pass. This prevents disruptive layout jumping (e.g. adjacent cards shifting up and down).
- **Maintain State (`maintain_state = true`)**: Internal component state (such as text input contents, scroll positions, or stateful animations) is preserved while hidden.
- **Custom Replacement (`replacement`)**: Allows rendering an alternate widget (such as a placeholder or loading spinner) when `visible` is false.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct Visibility {
    Key       key                    = Key::none();
    WidgetPtr child                  = nullptr;         ///< Primary child to display when visible
    WidgetPtr replacement            = nullptr;         ///< Optional fallback when invisible
    bool      visible                = true;            ///< Show/hide toggle
    bool      maintain_state         = false;           ///< Preserve internal state while hidden
    bool      maintain_animation     = false;           ///< Keep tickers ticking while hidden
    bool      maintain_size          = false;           ///< Retain layout dimensions when hidden
    bool      maintain_interactivity = false;           ///< Allow clicks even when invisible

    operator WidgetPtr() const;
};

inline WidgetPtr visibility(const VisibilityProps& props);
inline WidgetPtr visibility(bool visible, WidgetPtr child, WidgetPtr replacement = nullptr);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content displayed when `visible == true`. |
| `visible` | `bool` | `true` | Whether the child widget is visible. |
| `replacement` | `WidgetPtr` | `nullptr` | Alternate widget displayed when hidden (if `maintain_size` is false). |
| `maintain_size` | `bool` | `false` | Allocates normal layout width/height even when invisible. |
| `maintain_state`| `bool` | `false` | Keeps the underlying `StatefulWidget`'s state alive while hidden. |
| `maintain_interactivity` | `bool` | `false` | When true, the hidden widget continues to receive hit tests. |

---

## Code Examples (From `widgets_demo/utility_demo/main.cpp`)

### 1. Toggling Visibility Without Layout Shift (`maintain_size = true`)
```cpp
#include "enki/widgets/utility.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildStableLayoutColumn(bool isTargetVisible) {
    return column({
        .gap = 10_px,
        .children = {
            // Upper Card
            container({
                .color = 0xFF0F172A,
                .padding = EdgeInsets::all(12.0f),
                .child = text("Upper Content Card", { .color = 0xFF94A3B8 })
            }),

            // Target Box: Hidden but occupies space so lower card doesn't jump
            visibility({
                .visible = isTargetVisible,
                .maintain_size = true,
                .maintain_state = true,
                .child = container({
                    .color = 0xFF0284C7,
                    .height = StyleValue::point(60.0f),
                    .padding = EdgeInsets::all(12.0f),
                    .child = text("Target Element", { .color = 0xFFFFFFFF })
                })
            }),

            // Lower Card
            container({
                .color = 0xFF0F172A,
                .padding = EdgeInsets::all(12.0f),
                .child = text("Lower Content Card", { .color = 0xFF94A3B8 })
            })
        }
    });
}
```

---

## See Also
- [**IgnorePointer**](./ignore_pointer.md) — Controls mouse event pass-through.
- [**AnimatedOpacity**](../Animation%20&%20Motion/animated_opacity.md) — Fades opacity with smooth animations.
- [**Container**](../Layout/container.md) — Base layout box.
