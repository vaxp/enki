# Expanded

> A widget that expands a child of a `Row`, `Column`, or `Flexbox` to fill available remaining space along the main axis.

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::FlexItem>`
- **Props Struct**: `enki::ExpandedProps`
- **Underlying Mechanism**: `FlexItem` with `flex_grow = flex`, `flex_shrink = 1.0f`, `flex_basis = 0_px`

---

## Overview

When placed as an immediate child of a `Row` or `Column`, `Expanded` consumes the remaining free space along the flex container's main axis. If multiple `Expanded` widgets are present, the remaining space is divided among them proportionally according to their `flex` factor values.

Because `flex_basis` is set to `0.0f`, the child is forced to size based strictly on its flex proportion rather than its intrinsic dimensions.

---

## C++ API Definition

### Props Struct
```cpp
namespace enki {

struct ExpandedProps {
    float flex = 1.0f;
    WidgetPtr child = nullptr;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<FlexItem> expanded(const ExpandedProps& props = {});
inline std::shared_ptr<FlexItem> expanded(ExpandedProps&& props);
inline std::shared_ptr<FlexItem> expanded(WidgetPtr child, float flex = 1.0f);

} // namespace enki
```

---

## Properties Reference (`ExpandedProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `flex` | `float` | `1.0f` | The flex growth factor determining the proportion of leftover space allocated to this child. |
| `child` | `WidgetPtr` | `nullptr` | The child widget to be expanded. |

---

## Technical Mechanics (Anu Flexbox Mapping)

When `expanded(child, flex)` is called, Enki constructs a `FlexItem` with the following `FlexboxStyle`:
- `flex_grow = flex` (default `1.0f`)
- `flex_shrink = 1.0f`
- `flex_basis = StyleValue::point(0.0f)`

This translates directly to Anu Layout Engine:
```c
ANUNodeStyleSetFlexGrow(node, flex);
ANUNodeStyleSetFlexShrink(node, 1.0f);
ANUNodeStyleSetFlexBasis(node, 0.0f);
```

---

## Code Examples

### 1. Two Columns Sharing Space (1:2 Proportion)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr buildSplitView() {
    return row({
        .width = 600_px,
        .children = {
            // Sidebar takes 1/3 of remaining space (200px)
            expanded(sidebarWidget, 1.0f),
            // Content pane takes 2/3 of remaining space (400px)
            expanded(contentWidget, 2.0f),
        }
    });
}
```

### 2. Designated Initializers Syntax
```cpp
auto flexChild = expanded({
    .flex = 3.0f,
    .child = container({ .color = 0xFF3B82F6 }),
});
```

### 3. Fixed Item Alongside Expanded Item (Master-Detail Pattern)
```cpp
auto masterDetail = row({
    .children = {
        container({ .width = 250_px, .child = navigationList }), // Fixed 250px
        expanded(detailView),                                    // Takes all remaining width
    }
});
```

---

## See Also
- [**Flexible**](./flexible.md) — Sizing flexibility with `flex_basis = auto`.
- [**Spacer**](./spacer.md) — Empty expanded item to push neighboring widgets apart.
- [**Row**](./row.md) & [**Column**](./column.md) — Host flex containers.
