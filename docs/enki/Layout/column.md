# Column

> A multi-child flex container that arranges its children vertically along the Y-axis.

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Class**: `enki::Column` (inherits from `enki::Flexbox`)
- **Props Type**: `enki::ColumnProps` (alias for `enki::FlexboxProps`)
- **Underlying Engine**: Anu Layout Engine (`ANUFlexDirectionColumn`)

---

## Overview

`Column` is the core vertical layout container in Enki. It sets `flex_direction` to `FlexDirection::Column` and distributes its child widgets from top to bottom. It supports full flex distribution, vertical main-axis alignment (`Justify`), horizontal cross-axis alignment (`Align`), vertical gaps, and constraints.

---

## C++ API Definition

### Class Declaration
```cpp
namespace enki {

class Column : public Flexbox {
public:
    Column() { style.flex_direction = FlexDirection::Column; }
    explicit Column(const FlexboxProps& props);
    explicit Column(FlexboxProps&& props);
    Column(Key k, FlexboxStyle s, std::vector<WidgetPtr> children);

    [[nodiscard]] std::string_view typeName() const override { return "Column"; }
};

using ColumnProps = FlexboxProps;

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<Column> column(const FlexboxProps& props = {});
inline std::shared_ptr<Column> column(FlexboxProps&& props);
inline std::shared_ptr<Column> column(Key key, FlexboxProps props);

} // namespace enki
```

---

## Properties Reference (`ColumnProps` / `FlexboxProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `children` | `std::vector<WidgetPtr>` | `{}` | Ordered list of child widgets laid out from top to bottom. |
| `justify_content` | `std::optional<Justify>` | `Justify::Start` | Main-axis (vertical) alignment of children. |
| `align_items` | `std::optional<Align>` | `Align::Stretch` | Cross-axis (horizontal) alignment of children. |
| `gap` | `std::optional<StyleValue>` | `std::nullopt` | Spacing between children. |
| `row_gap` | `std::optional<StyleValue>` | `std::nullopt` | Vertical spacing between consecutive children in the column. |
| `width` | `std::optional<StyleValue>` | `auto_val` | Width constraint (e.g. `300_px`, `100_pct`). |
| `height` | `std::optional<StyleValue>` | `auto_val` | Height constraint. |
| `min_width` | `std::optional<StyleValue>` | `undefined_val` | Minimum width boundary. |
| `max_width` | `std::optional<StyleValue>` | `undefined_val` | Maximum width boundary. |
| `min_height` | `std::optional<StyleValue>` | `undefined_val` | Minimum height boundary. |
| `max_height` | `std::optional<StyleValue>` | `undefined_val` | Maximum height boundary. |
| `padding` | `std::optional<StyleInsets>` | `0` | Inner padding around the column content. |
| `margin` | `std::optional<StyleInsets>` | `0` | Outer margin around the column container. |
| `key` | `Key` | `Key::none()` | Identifier used during element reconciliation. |

---

## Enums

### `Justify` (Vertical Main Axis Alignment)
- `Justify::Start` — Pack children toward the top of the column.
- `Justify::Center` — Center children vertically within the column.
- `Justify::End` — Pack children toward the bottom of the column.
- `Justify::SpaceBetween` — Distribute children evenly; top child at the top, bottom child at the bottom.
- `Justify::SpaceAround` — Distribute children with equal vertical space around each item.
- `Justify::SpaceEvenly` — Distribute children with equal spacing between items and borders.

### `Align` (Horizontal Cross Axis Alignment)
- `Align::Start` — Align children to the left edge of the column.
- `Align::Center` — Center children horizontally in the column.
- `Align::End` — Align children to the right edge of the column.
- `Align::Stretch` — Stretch children horizontally to fill the full width of the column.

---

## Code Examples

### 1. Form Card Layout (Pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildLoginForm() {
    return column({
        .align_items = Align::Stretch,
        .gap = 12_px,
        .width = 320_px,
        .children = {
            text("Sign In", { .font_size = 20.0f, .font_weight = FontWeight::Bold }),
            text("Enter your credentials below:", { .color = 0xFF94A3B8 }),
            container({ .height = 40_px, .color = 0xFF1E293B }), // Input placeholder
            container({ .height = 40_px, .color = 0xFF1E293B }),
            button({
                .child = text("Submit"),
            })
        }
    });
}
```

### 2. Centered Empty State
```cpp
auto emptyState = column({
    .justify_content = Justify::Center,
    .align_items = Align::Center,
    .height = 100_pct,
    .gap = 8_px,
    .children = {
        text("📭", { .font_size = 48.0f }),
        text("No Notifications Yet", { .font_weight = FontWeight::SemiBold }),
        text("We'll let you know when updates arrive.", { .color = 0xFF64748B }),
    }
});
```

---

## See Also
- [**Row**](./row.md) — Horizontal flex container.
- [**Expanded**](./expanded.md) — Vertically expand a child inside a Column.
- [**Spacer**](./spacer.md) — Vertical spacing consumer inside a Column.
- [**SizedBox**](./sized_box.md) — Add fixed vertical height gaps between Column items.
