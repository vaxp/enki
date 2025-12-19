# IntrinsicHeight

> A layout widget that sizes its child to the child's maximum intrinsic (natural) height, with optional step quantization.

- **Header File**: `#include "enki/widgets/intrinsic_height.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class IntrinsicHeightWidget`, `struct IntrinsicHeightProps`, `struct IntrinsicHeight`
- **Helper Function**: `intrinsicHeight(...)`

---

## Overview

`IntrinsicHeight` measures the intrinsic (unconstrained natural) height of its child widget tree and forces the child to assume that height. When children with varying heights (such as pricing cards with different numbers of features, or text blurbs of varying lengths) are placed in a horizontal `Row`, wrapping them in `IntrinsicHeight` causes all children to stretch uniformly to match the height of the tallest child.

Crucially, this solves the classic flexbox problem where vertical dividers (`verticalDivider`) or card borders fail to span the full height because their container has an unbounded or auto height.

### Key Architectural Behaviors:
- **Tallest Item Synchronization**: During `syncLayout()`, `RenderIntrinsicHeight` computes the maximum natural height required by the children and establishes a tight height boundary for the row.
- **Vertical Divider Support**: Elements inside the row that stretch along the cross-axis (`.align_items = Align::Stretch`) now receive a concrete bounded height, allowing vertical dividers to span 100% of the row's height.
- **Step Quantization (`step_height`)**: Snaps the calculated height to multiples of a specified step value (e.g., 25px, 50px).

---

## C++ API Definition

### Header: `<enki/widgets/intrinsic_height.hpp>`

```cpp
namespace enki {

struct IntrinsicHeightProps {
    Key                  key         = Key::none();
    std::optional<float> step_height = std::nullopt;
    std::optional<float> step_width  = std::nullopt;
    WidgetPtr            child       = nullptr;
};

struct IntrinsicHeight {
    Key                  key         = Key::none();
    std::optional<float> step_height = std::nullopt;
    std::optional<float> step_width  = std::nullopt;
    WidgetPtr            child       = nullptr;

    operator WidgetPtr() const;
};

// Declarative factory helpers:
std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(const IntrinsicHeightProps& props);
std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(WidgetPtr child);
std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(float step_height, WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and efficient diffing. |
| `step_height` | `std::optional<float>` | `std::nullopt` | If non-null, snaps the computed child height to the nearest multiple of this value. |
| `step_width` | `std::optional<float>` | `std::nullopt` | If non-null, snaps the computed child width to the nearest multiple of this value. |
| `child` | `WidgetPtr` | `nullptr` | The child widget whose intrinsic height is measured and imposed. |

---

## Real Code Examples

### 1. Equal-Height Pricing Cards with Full-Span Dividers (From `widgets_demo/intrinsic_height_demo/main.cpp`)
All cards in this pricing matrix match the height of the tallest card, aligning action buttons and enabling vertical dividers to span the entire card height:

```cpp
#include "enki/widgets/intrinsic_height.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildPricingMatrix(WidgetPtr starterCard, WidgetPtr proCard, WidgetPtr enterpriseCard) {
    // Inner Row configured to stretch all items along the vertical cross-axis
    auto inner_row = row({
        .justify_content = Justify::SpaceBetween,
        .align_items     = Align::Stretch,
        .gap             = StyleValue::point(16.0f),
        .width           = StyleValue::percent(100.0f),
        .children        = {
            starterCard,
            verticalDivider({ .thickness = 1.5f, .color = 0xFF475569 }),
            proCard,
            verticalDivider({ .thickness = 1.5f, .color = 0xFF475569 }),
            enterpriseCard,
        },
        .key = Key::string("pricing_row"),
    });

    // IntrinsicHeight wraps the row, ensuring all cards and dividers match the tallest card
    return intrinsicHeight({
        .child = inner_row,
        .key   = Key::string("unified_pricing_matrix"),
    });
}
```

### 2. Snapped Step Quantization
Snaps the height of the card deck to the nearest 25px:

```cpp
auto snappedDeck = intrinsicHeight({
    .step_height = 25.0f,
    .child = row({
        .align_items = Align::Stretch,
        .children = { cardA, cardB },
    }),
});
```

---

## See Also
- [**IntrinsicWidth**](./intrinsic_width.md) — Sizing children to their natural intrinsic width.
- [**Row**](./row.md) — Horizontal flex container.
- [**Divider**](../Basic%20UI/divider.md) — Horizontal and vertical dividers.
