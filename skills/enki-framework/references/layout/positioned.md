# Positioned

> A widget that controls where a child of a `Stack` is positioned along the X and Y axes using absolute coordinates.

- **Header File**: `#include "enki/widgets/stack.hpp"`
- **C++ Class**: `enki::PositionedWidget`
- **Declarative Struct**: `enki::Positioned` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::PositionedProps`
- **Render Object**: `enki::RenderPositioned`
- **Underlying Mechanism**: Absolute positioning within Anu (`ANUPositionTypeAbsolute`)

---

## Overview

`Positioned` must be placed as an immediate child of a `Stack`. It sets the child's position type to `ANUPositionTypeAbsolute` in Anu and applies explicit offsets from the stack's edges (`top`, `right`, `bottom`, `left`, `start`, `end`) or explicit dimensions (`width`, `height`).

---

## C++ API Definition

### Declarative Struct
```cpp
namespace enki {

struct Positioned {
    Key                       key   = Key::none();
    WidgetPtr                 child = nullptr;

    std::optional<StyleValue> top;
    std::optional<StyleValue> right;
    std::optional<StyleValue> bottom;
    std::optional<StyleValue> left;
    std::optional<StyleValue> start;
    std::optional<StyleValue> end;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;

    operator WidgetPtr() const;

    /// Creates a Positioned widget that fills the entire stack with optional edge insets.
    static std::shared_ptr<PositionedWidget> fill(WidgetPtr child,
                                                  float left = 0.0f, float top = 0.0f,
                                                  float right = 0.0f, float bottom = 0.0f);

    /// Creates a Positioned widget from a Rect.
    static std::shared_ptr<PositionedWidget> fromRect(WidgetPtr child, const Rect& rect);

    /// Creates a directional Positioned widget (RTL-aware start/end).
    static std::shared_ptr<PositionedWidget> directional(WidgetPtr child,
                                                         float top = 0.0f, float end = 0.0f,
                                                         float bottom = 0.0f, float start = 0.0f);
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<PositionedWidget> positioned(WidgetPtr child);
inline std::shared_ptr<PositionedWidget> positioned(float top, float right, float bottom, float left, WidgetPtr child);
inline std::shared_ptr<PositionedWidget> positioned(PositionedProps props);
inline std::shared_ptr<PositionedWidget> positioned(PositionedProps props, WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The child widget to be placed at the target coordinates. |
| `top` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the top edge of the stack. |
| `right` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the right edge of the stack. |
| `bottom` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the bottom edge of the stack. |
| `left` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the left edge of the stack. |
| `start` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the leading edge (left in LTR, right in RTL). |
| `end` | `std::optional<StyleValue>` | `std::nullopt` | Distance from the trailing edge (right in LTR, left in RTL). |
| `width` | `std::optional<StyleValue>` | `std::nullopt` | Fixed width of the positioned child. |
| `height` | `std::optional<StyleValue>` | `std::nullopt` | Fixed height of the positioned child. |
| `key` | `Key` | `Key::none()` | Identifier used during reconciliation. |

---

## Code Examples

### 1. Designated Initializers (Badge on Card)
```cpp
#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildNotificationCard() {
    return Stack {
        .width = 200_px,
        .height = 120_px,
        .children = {
            baseCardWidget,
            Positioned {
                .top = -6_px,
                .right = -6_px,
                .child = container({
                    .color = 0xFFEF4444,
                    .border_radius = BorderRadius::circular(10.0f),
                    .padding = StyleInsets::symmetric(2_px, 6_px),
                    .child = text("3", { .font_size = 11.0f }),
                })
            }
        }
    };
}
```

### 2. `Positioned::fill()` Helper
```cpp
// Fills the entire parent Stack with 10px margins on all edges
auto filledLayer = Positioned::fill(contentWidget, 10.0f, 10.0f, 10.0f, 10.0f);
```

### 3. Coordinate Helper Function
```cpp
// (top, right, bottom, left, child)
auto pinnedAction = positioned(15.0f, 15.0f, 0.0f, 0.0f, closeButton);
```

---

## See Also
- [**Stack**](./stack.md) — The parent container for `Positioned` children.
- [**Align**](./align.md) — Proportional and anchor-based alignment.
