# Align

> A layout mechanism that positions a child widget within its parent container using 9 standard geometrical anchors, or aligns an individual flex item along the cross-axis.

- **Header Files**: `#include "enki/widgets/container.hpp"`, `#include "enki/core/types.hpp"`, and `#include "enki/widgets/flexbox.hpp"`
- **Primary Enum**: `enki::Alignment` (9-point anchor system)
- **Flex Enum**: `enki::Align` (Anu layout cross-axis alignment)

---

## Overview

Alignment in Enki is designed for zero runtime overhead. Rather than allocating redundant render nodes, alignment is configured declaratively:
1. **Inside Containers**: Set `.align = Alignment::<Anchor>` on a `Container`. It automatically computes the exact `justify_content` and `align_items` flex rules.
2. **Inside Stacks**: Set `.alignment = Alignment::<Anchor>` on a `Stack` to anchor non-positioned flow children.
3. **Inside Flex Items**: Set `.align_self = Align::<Anchor>` on a `flexItem` to override the parent's `align_items` rule.

---

## The 9-Point `Alignment` System

The `Alignment` enum defines nine standardized layout anchors:

```
┌─────────────────┬──────────────────┬─────────────────┐
│     TopLeft     │    TopCenter     │    TopRight     │
├─────────────────┼──────────────────┼─────────────────┤
│   CenterLeft    │      Center      │   CenterRight   │
├─────────────────┼──────────────────┼─────────────────┤
│   BottomLeft    │   BottomCenter   │   BottomRight   │
└─────────────────┴──────────────────┴─────────────────┘
```

### Automatic Flexbox Mapping (in `ContainerWidget`)

| `Alignment` | `justify_content` | `align_items` |
|---|---|---|
| `Alignment::TopLeft` | `Justify::Start` | `Align::Start` |
| `Alignment::TopCenter` | `Justify::Start` | `Align::Center` |
| `Alignment::TopRight` | `Justify::Start` | `Align::End` |
| `Alignment::CenterLeft` | `Justify::Center` | `Align::Start` |
| `Alignment::Center` | `Justify::Center` | `Align::Center` |
| `Alignment::CenterRight` | `Justify::Center` | `Align::End` |
| `Alignment::BottomLeft` | `Justify::End` | `Align::Start` |
| `Alignment::BottomCenter` | `Justify::End` | `Align::Center` |
| `Alignment::BottomRight` | `Justify::End` | `Align::End` |

---

## The `Align` Enum (Flex Cross-Axis)

For individual flex items or container cross-axis rules:
- `Align::Auto` — Inherit from parent flex container.
- `Align::Start` — Align to cross-axis start (top in Row, left in Column).
- `Align::Center` — Center along cross axis.
- `Align::End` — Align to cross-axis end (bottom in Row, right in Column).
- `Align::Stretch` — Stretch across entire cross axis dimension.
- `Align::Baseline` — Align along text baseline.

---

## Code Examples

### 1. Aligning a Child Inside a Container
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildBottomRightAction() {
    return container({
        .width = 100_pct,
        .height = 120_px,
        .align = Alignment::BottomRight,
        .padding = StyleInsets::all(16_px),
        .child = button({ .child = text("Proceed") }),
    });
}
```

### 2. Overriding Cross-Axis Alignment on a Single Flex Item (`align_self`)
```cpp
#include "enki/widgets/flexbox.hpp"

auto myRow = row({
    .align_items = Align::Center, // All children centered by default
    .height = 100_px,
    .children = {
        item1,
        // Item 2 overrides and aligns to the bottom
        flexItem({
            .align_self = Align::End,
            .child = item2,
        }),
        item3,
    }
});
```

---

## See Also
- [**Center**](./center.md) — Shorthand for `Alignment::Center`.
- [**Container**](./container.md) — The visual box implementing `Alignment`.
- [**Positioned**](./positioned.md) — For absolute offset-based positioning.
