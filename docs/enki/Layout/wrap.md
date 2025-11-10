# Wrap

> A flex layout widget that displays its children in multiple horizontal or vertical runs, wrapping overflowing children onto the next line.

- **Header File**: `#include "enki/widgets/flexbox.hpp"`
- **C++ Class**: `enki::Wrap` (inherits from `enki::Flexbox`)
- **Props Type**: `enki::WrapProps` (alias for `enki::FlexboxProps`)
- **Underlying Engine**: Anu Layout Engine (`ANUWrapWrap`)

---

## Overview

Unlike `Row` (which keeps all children on a single line unless explicitly configured otherwise), `Wrap` defaults to `FlexDirection::Row` with `FlexWrap::Wrap` enabled. When child widgets exceed the available container width, they break automatically onto the next line. It is ideal for tag clouds, chip groups, tool button palettes, and responsive card grids.

---

## C++ API Definition

### Class Declaration
```cpp
namespace enki {

class Wrap : public Flexbox {
public:
    Wrap() {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    explicit Wrap(const FlexboxProps& props);
    explicit Wrap(FlexboxProps&& props);
    Wrap(Key k, FlexboxStyle s, std::vector<WidgetPtr> children);

    [[nodiscard]] std::string_view typeName() const override { return "Wrap"; }
};

using WrapProps = FlexboxProps;

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<Wrap> wrap(const FlexboxProps& props = {});
inline std::shared_ptr<Wrap> wrap(FlexboxProps&& props);
inline std::shared_ptr<Wrap> wrap(Key key, FlexboxProps props);

} // namespace enki
```

---

## Properties Reference (`WrapProps` / `FlexboxProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `children` | `std::vector<WidgetPtr>` | `{}` | The list of widgets to flow and wrap. |
| `gap` | `std::optional<StyleValue>` | `std::nullopt` | Spacing applied uniformly between items and wrapped lines. |
| `column_gap` | `std::optional<StyleValue>` | `std::nullopt` | Horizontal spacing between items on the same line. |
| `row_gap` | `std::optional<StyleValue>` | `std::nullopt` | Vertical spacing between consecutive wrapped lines. |
| `justify_content` | `std::optional<Justify>` | `Justify::Start` | Main-axis alignment of items along each line. |
| `align_items` | `std::optional<Align>` | `Align::Stretch` | Cross-axis alignment of items within a single line. |
| `align_content` | `std::optional<Align>` | `Align::Start` | Cross-axis alignment of the entire wrapped lines block within the container. |
| `flex_wrap` | `std::optional<FlexWrap>` | `FlexWrap::Wrap` | Wrap behavior (`Wrap`, `WrapReverse`, or `NoWrap`). |
| `width` | `std::optional<StyleValue>` | `auto_val` | Width constraint of the wrap container. |
| `padding` | `std::optional<StyleInsets>` | `0` | Inner padding around the entire wrapped content. |
| `key` | `Key` | `Key::none()` | Identifier used during reconciliation. |

---

## Code Examples

### 1. Chip / Tag Cloud Group (Real-world pattern)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/chip.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildFilterChips() {
    return wrap({
        .column_gap = 8_px,
        .row_gap = 10_px,
        .children = {
            chip("C++20"),
            chip("Anu Engine"),
            chip("Skia 2D"),
            chip("Wayland"),
            chip("X11"),
            chip("Linux Desktop"),
            chip("GUI Framework"),
            chip("High Performance"),
        }
    });
}
```

### 2. Wrap with Custom Row & Column Gaps
```cpp
auto tagContainer = wrap({
    .width = 400_px,
    .column_gap = 12_px,
    .row_gap = 16_px,
    .justify_content = Justify::SpaceBetween,
    .children = { ... }
});
```

---

## See Also
- [**Row**](./row.md) — Single-line horizontal layout.
- [**Column**](./column.md) — Single-line vertical layout.
