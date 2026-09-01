# ConstrainedBox

> A layout widget or property configuration that imposes additional minimum and maximum sizing boundaries on its child.

- **Header Files**: `#include "enki/widgets/container.hpp"` and `#include "enki/widgets/flexbox.hpp"`
- **Declarative Properties**: `.min_width`, `.max_width`, `.min_height`, `.max_height` on `Container`, `FlexItem`, and `Stack`
- **Underlying Engine**: Direct Anu Layout Engine constraint styling (`ANUNodeStyleSetMinWidth`, `ANUNodeStyleSetMaxWidth`, `ANUNodeStyleSetMinHeight`, `ANUNodeStyleSetMaxHeight`)

---

## Overview

`ConstrainedBox` enforces boundary ranges on widgets so they don't shrink smaller than a minimum size or grow larger than a maximum size. In Enki, constraints are expressed using `StyleValue` (pixels `_px` or percentages `_pct`) directly within `Container` or `flexItem`.

---

## C++ API Definition

### Constraint Properties
```cpp
// Inside ContainerProps, FlexItemProps, and StackProps:
std::optional<StyleValue> min_width;
std::optional<StyleValue> max_width;
std::optional<StyleValue> min_height;
std::optional<StyleValue> max_height;
```

### Anu Layout Engine Mapping
```cpp
if (style.min_width.isPercent()) {
    ANUNodeStyleSetMinWidthPercent(node, style.min_width.value);
} else if (style.min_width.isPoint()) {
    ANUNodeStyleSetMinWidth(node, style.min_width.value);
}

if (style.max_width.isPercent()) {
    ANUNodeStyleSetMaxWidthPercent(node, style.max_width.value);
} else if (style.max_width.isPoint()) {
    ANUNodeStyleSetMaxWidth(node, style.max_width.value);
}
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `min_width` | `std::optional<StyleValue>` | `undefined_val` | The minimum width the widget can shrink to. |
| `max_width` | `std::optional<StyleValue>` | `undefined_val` | The maximum width the widget can grow to. |
| `min_height` | `std::optional<StyleValue>` | `undefined_val` | The minimum height the widget can shrink to. |
| `max_height` | `std::optional<StyleValue>` | `undefined_val` | The maximum height the widget can grow to. |

---

## Code Examples

### 1. Responsive Dialog Card with Boundaries (Pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildAdaptiveDialog(WidgetPtr content) {
    return container({
        .min_width = 300_px,  // Never narrower than 300px on small screens
        .max_width = 600_px,  // Never wider than 600px on ultra-wide screens
        .min_height = 200_px, // Guaranteed minimum space for content
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(12.0f),
        .padding = StyleInsets::all(20_px),
        .child = content,
    });
}
```

### 2. Clamping Button Dimensions
```cpp
auto clampedButton = container({
    .min_width = 120_px, // Standard minimum click target
    .max_width = 240_px,
    .height = 44_px,
    .child = button({ .child = text("Click Me") }),
});
```

---

## See Also
- [**Container**](./container.md) — Base container hosting constraints.
- [**SizedBox**](./sized_box.md) — For exact, rigid width and height.
- [**FractionallySizedBox**](./fractionally_sized_box.md) — Relative percentage sizing.
