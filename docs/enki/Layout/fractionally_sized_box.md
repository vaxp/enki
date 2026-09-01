# FractionallySizedBox

> A layout mechanism that sizes its child to a fractional percentage of the total available parent space using `StyleValue::percent` or the `_pct` literal.

- **Header Files**: `#include "enki/widgets/container.hpp"`, `#include "enki/widgets/flexbox.hpp"`, and `#include "enki/core/types.hpp"`
- **Syntax Literal**: `_pct` (e.g. `50_pct`, `100_pct`)
- **Factory Helper**: `StyleValue::percent(float val)`
- **Underlying Engine**: Anu Layout Engine percentage dimensions (`ANUNodeStyleSetWidthPercent`, `ANUNodeStyleSetHeightPercent`)

---

## Overview

In Enki, proportional fractional sizing does not require a bulky wrapper widget. Instead, any `Container`, `flexItem`, or `Stack` natively accepts fractional percentage values via the `StyleValue::percent(val)` function or the convenient user-defined literal `_pct`.

The underlying Anu Layout Engine calculates the pixel geometry relative to the parent box dimensions during each layout pass.

---

## C++ API Definition

### Literal and StyleValue
```cpp
namespace enki {

struct StyleValue {
    enum class Unit : uint8_t {
        Undefined = 0,
        Point,
        Percent,
        Auto
    };

    static constexpr StyleValue percent(float val) { return {Unit::Percent, val}; }
};

inline namespace literals {
    constexpr StyleValue operator""_pct(long double val);
    constexpr StyleValue operator""_pct(unsigned long long val);
}

} // namespace enki
```

### Underlying Engine Mapping
```cpp
if (style.width.isPercent()) {
    ANUNodeStyleSetWidthPercent(node, style.width.value);
}
if (style.height.isPercent()) {
    ANUNodeStyleSetHeightPercent(node, style.height.value);
}
```

---

## Code Examples

### 1. Half-Screen Split Pane
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildSplitView() {
    return row({
        .width = 100_pct,
        .height = 100_pct,
        .children = {
            // Left pane: exactly 40% of parent width
            container({
                .width = 40_pct,
                .height = 100_pct,
                .color = 0xFF0F172A,
                .child = navigationSidebar,
            }),
            // Right pane: remaining 60% of parent width
            container({
                .width = 60_pct,
                .height = 100_pct,
                .color = 0xFF1E293B,
                .child = mainWorkspace,
            }),
        }
    });
}
```

### 2. Centered Modal with Percentage Width
```cpp
auto centeredModal = container({
    .width = 75_pct,   // 75% of window width
    .max_width = 800_px, // Clamped to 800px max
    .color = 0xFF161D2F,
    .border_radius = BorderRadius::circular(16.0f),
    .child = modalContent,
});
```

---

## See Also
- [**Container**](./container.md) — Base container accepting percentage dimensions.
- [**Expanded**](./expanded.md) — Dynamic flex-weighted space distribution.
- [**ConstrainedBox**](./constrained_box.md) — Boundary limits combined with percentage sizing.
