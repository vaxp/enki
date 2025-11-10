# Spacer

> An empty, lightweight flex item that expands to absorb remaining free space between adjacent widgets in a `Row` or `Column`.

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::FlexItem>`
- **Factory Function**: `enki::spacer(float flex = 1.0f)`
- **Underlying Mechanism**: `FlexItem` with `child = nullptr`, `flex_grow = flex`, `flex_shrink = 1.0f`, `flex_basis = 0_px`

---

## Overview

`Spacer` creates an adjustable, empty space along the main axis of a `Row` or `Column`. By default, it has a flex factor of `1.0f`. Placing a `Spacer` between two widgets pushes them as far apart as possible (for instance, pushing a title to the left edge and an action button or close icon to the right edge).

Multiple spacers can be used in the same container with varying flex weights to divide available space unevenly.

---

## C++ API Definition

```cpp
namespace enki {

/// @brief Creates an empty flex item that consumes remaining space.
/// @param flex The flex growth weight (defaults to 1.0f).
inline std::shared_ptr<FlexItem> spacer(float flex = 1.0f) {
    FlexboxStyle s;
    s.flex_grow = flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(Key::none(), s, nullptr);
}

} // namespace enki
```

---

## Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `flex` | `float` | `1.0f` | Flex factor determining how much of the free space this spacer claims relative to other expanded items. |

---

## Code Examples

### 1. Title Bar Navigation (Header pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildWindowHeader() {
    return row({
        .align_items = Align::Center,
        .padding = StyleInsets::symmetric(8_px, 16_px),
        .width = 100_pct,
        .children = {
            text("Project Explorer", { .font_weight = FontWeight::Bold }),
            // Spacer pushes everything after it to the far right edge
            spacer(),
            button({ .child = text("—") }), // Minimize
            button({ .child = text("✕") }), // Close
        }
    });
}
```

### 2. Weighted Spacers (Asymmetric Spacing)
```cpp
auto spacedRow = row({
    .width = 600_px,
    .children = {
        leftWidget,
        spacer(1.0f),  // 1/3 of extra space
        middleWidget,
        spacer(2.0f),  // 2/3 of extra space
        rightWidget,
    }
});
```

---

## See Also
- [**Expanded**](./expanded.md) — Expands an actual child widget rather than empty space.
- [**SizedBox**](./sized_box.md) — Inserts fixed, non-flexing pixel gaps.
- [**Row**](./row.md) & [**Column**](./column.md) — Flex containers hosting the spacer.
