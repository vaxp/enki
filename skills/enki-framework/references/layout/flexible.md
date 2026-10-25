# Flexible

> A widget that controls how a child of a `Row`, `Column`, or `Flexbox` flexes while preserving its intrinsic size basis (`flex_basis = auto`).

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::FlexItem>`
- **Props Struct**: `enki::ExpandedProps`
- **Underlying Mechanism**: `FlexItem` with `flex_grow = flex`, `flex_shrink = 1.0f`, `flex_basis = StyleValue::autoValue()`

---

## Overview

While `Expanded` forces a child widget to strictly fill all allocated space by zeroing its flex basis (`flex_basis = 0`), `Flexible` gives the child flexibility to size according to its own intrinsic content size (`flex_basis = auto`) while participating in flex grow/shrink distribution.

---

## C++ API Definition

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<FlexItem> flexible(const ExpandedProps& props = {});
inline std::shared_ptr<FlexItem> flexible(ExpandedProps&& props);
inline std::shared_ptr<FlexItem> flexible(WidgetPtr child, float flex = 1.0f);

} // namespace enki
```

---

## Properties Reference (`ExpandedProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `flex` | `float` | `1.0f` | Flex factor determining growth priority when extra space exists. |
| `child` | `WidgetPtr` | `nullptr` | The child widget to wrap. |

---

## Difference Between `Expanded` and `Flexible`

| Feature | `Expanded` | `Flexible` |
|---|---|---|
| **Flex Basis** | `StyleValue::point(0.0f)` | `StyleValue::autoValue()` |
| **Child Intrinsic Size** | Ignored; widget is forced to expand | Respected; widget can be smaller than its maximum flex allotment |
| **Common Use Case** | Strict equal grids, master-detail panes, full width fills | Text labels that can wrap/shrink, flexible buttons |

---

## Code Examples

### 1. Flexible Text that Shrinks or Expands
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildFlexibleRow() {
    return row({
        .children = {
            // Long title shrinks or grows gracefully without pushing the button off-screen
            flexible(text("Very long file name or document description that might overflow..."), 1.0f),
            button({ .child = text("Open") }),
        }
    });
}
```

### 2. Designated Initializer Syntax
```cpp
auto flexItemWidget = flexible({
    .flex = 2.0f,
    .child = myWidget,
});
```

---

## See Also
- [**Expanded**](./expanded.md) — Forces child to fill remaining space.
- [**Spacer**](./spacer.md) — Empty flex space consumer.
- [**Row**](./row.md) & [**Column**](./column.md) — Parent flex containers.
