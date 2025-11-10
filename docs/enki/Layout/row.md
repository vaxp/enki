# Row

> A multi-child flex container that arranges its children horizontally along the X-axis.

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Class**: `enki::Row` (inherits from `enki::Flexbox`)
- **Props Type**: `enki::RowProps` (alias for `enki::FlexboxProps`)
- **Underlying Engine**: Anu Layout Engine (`ANUFlexDirectionRow`)

---

## Overview

`Row` is the primary horizontal layout building block in Enki. It sets `flex_direction` to `FlexDirection::Row` and delegates layout calculations directly to Anu. It supports full flex distribution, alignment, cross-axis stretching, RTL flow directions, gaps/gutters, and insets.

---

## C++ API Definition

### Class Declaration
```cpp
namespace enki {

class Row : public Flexbox {
public:
    Row() { style.flex_direction = FlexDirection::Row; }
    explicit Row(const FlexboxProps& props);
    explicit Row(FlexboxProps&& props);
    Row(Key k, FlexboxStyle s, std::vector<WidgetPtr> children);

    [[nodiscard]] std::string_view typeName() const override { return "Row"; }
};

using RowProps = FlexboxProps;

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<Row> row(const FlexboxProps& props = {});
inline std::shared_ptr<Row> row(FlexboxProps&& props);
inline std::shared_ptr<Row> row(Key key, FlexboxProps props);

} // namespace enki
```

---

## Properties Reference (`RowProps` / `FlexboxProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `children` | `std::vector<WidgetPtr>` | `{}` | Ordered list of child widgets to lay out horizontally. |
| `justify_content` | `std::optional<Justify>` | `Justify::Start` | Main-axis (horizontal) alignment of children. |
| `align_items` | `std::optional<Align>` | `Align::Stretch` | Cross-axis (vertical) alignment of children. |
| `gap` | `std::optional<StyleValue>` | `std::nullopt` | Uniform spacing between all children along both axes. |
| `column_gap` | `std::optional<StyleValue>` | `std::nullopt` | Horizontal spacing between consecutive children in the row. |
| `row_gap` | `std::optional<StyleValue>` | `std::nullopt` | Vertical spacing if items wrap to multiple rows. |
| `width` | `std::optional<StyleValue>` | `auto_val` | Explicit width constraint (pixels `_px`, percent `_pct`, or auto). |
| `height` | `std::optional<StyleValue>` | `auto_val` | Explicit height constraint. |
| `min_width` | `std::optional<StyleValue>` | `undefined_val` | Minimum width constraint. |
| `max_width` | `std::optional<StyleValue>` | `undefined_val` | Maximum width constraint. |
| `min_height` | `std::optional<StyleValue>` | `undefined_val` | Minimum height constraint. |
| `max_height` | `std::optional<StyleValue>` | `undefined_val` | Maximum height constraint. |
| `padding` | `std::optional<StyleInsets>` | `0` | Inner padding around the row children. |
| `margin` | `std::optional<StyleInsets>` | `0` | Outer margin around the row container. |
| `direction` | `std::optional<Direction>` | `Direction::Inherit` | Text / Layout flow direction (`LTR` or `RTL`). |
| `flex_wrap` | `std::optional<FlexWrap>` | `FlexWrap::NoWrap` | Whether items wrap onto multiple lines if exceeding width. |
| `key` | `Key` | `Key::none()` | Identifier used during element reconciliation. |

---

## Enums

### `Justify` (Main Axis Alignment)
- `Justify::Start` — Pack children toward the start of the row (left in LTR).
- `Justify::Center` — Center children along the row.
- `Justify::End` — Pack children toward the end of the row (right in LTR).
- `Justify::SpaceBetween` — Evenly distribute children; first item at start, last item at end.
- `Justify::SpaceAround` — Evenly distribute children with equal space around each item.
- `Justify::SpaceEvenly` — Evenly distribute children such that the space between any two items is equal.

### `Align` (Cross Axis Alignment)
- `Align::Start` — Align children to the top edge of the row.
- `Align::Center` — Center children vertically within the row.
- `Align::End` — Align children to the bottom edge of the row.
- `Align::Stretch` — Stretch children to match the row's height.
- `Align::Baseline` — Align children along their text baseline.

---

## Code Examples

### 1. Basic Action Toolbar with Gaps
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildToolbar() {
    return row({
        .justify_content = Justify::Start,
        .align_items = Align::Center,
        .gap = 8_px,
        .children = {
            button({ .child = text("Save") }),
            button({ .child = text("Edit") }),
            button({ .child = text("Delete") }),
        }
    });
}
```

### 2. Header Bar with Spacer (Pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildHeader() {
    return row({
        .align_items = Align::Center,
        .padding = StyleInsets::symmetric(8_px, 16_px),
        .width = 100_pct,
        .children = {
            text("Dashboard Overview", { .font_size = 16.0f, .font_weight = FontWeight::Bold }),
            spacer(), // Automatically absorbs all remaining horizontal space
            container({
                .color = 0xFF10B981,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::symmetric(4_px, 8_px),
                .child = text("Online", { .font_size = 11.0f }),
            })
        }
    });
}
```

### 3. RTL Support
```cpp
auto arabicRow = row({
    .direction = Direction::RTL,
    .gap = 12_px,
    .children = {
        icon(Icons::ArrowBack),
        text("رجوع"),
    }
});
```

---

## See Also
- [**Column**](./column.md) — Vertical flex container.
- [**Expanded**](./expanded.md) — Expand a child within a Row.
- [**Spacer**](./spacer.md) — Blank space consumer inside a Row.
- [**Wrap**](./wrap.md) — Horizontal flex layout that wraps onto multiple rows.
