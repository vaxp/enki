# VerticalDivider

> A thin vertical line that visually separates adjacent widgets in a `Row`, toolbar, or split panel.

- **Header File**: `#include "enki/widgets/divider.hpp"`
- **C++ Class**: `enki::VerticalDividerWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::VerticalDivider` (converts implicitly to `WidgetPtr`)
- **Helper Function**: `enki::verticalDivider(DividerProps props = {})`
- **Underlying Engine**: Skia 2D vertical line path rendering

---

## Overview

`VerticalDivider` serves as the vertical counterpart to `Divider`. When placed inside a `Row` with `.align_items = Align::Stretch`, it stretches vertically to match the height of neighboring content, drawing a clean dividing line.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct VerticalDivider {
    Key          key           = Key::none();
    float        width         = 16.0f;          // Total horizontal bounding width
    float        height        = 16.0f;          // Height (or stretches in flex)
    float        thickness     = 1.0f;           // Stroke thickness of the vertical line
    float        indent        = 0.0f;           // Top empty margin
    float        end_indent    = 0.0f;           // Bottom empty margin
    Color        color         = 0xFF334155;     // Line color

    DividerStyle style         = DividerStyle::Solid;

    float        dash_length   = 6.0f;
    float        dash_gap      = 4.0f;
    bool         gradient_fade = false;
    bool         round_caps    = false;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Function
```cpp
namespace enki {

inline std::shared_ptr<VerticalDividerWidget> verticalDivider(DividerProps props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `width` | `float` | `16.0f` | Total horizontal bounding box allocated for the vertical divider. |
| `thickness` | `float` | `1.0f` | Thickness of the drawn vertical line in pixels. |
| `indent` | `float` | `0.0f` | Empty spacing from the top edge. |
| `end_indent` | `float` | `0.0f` | Empty spacing from the bottom edge. |
| `color` | `Color` | `0xFF334155` | 32-bit ARGB line color. |
| `style` | `DividerStyle` | `DividerStyle::Solid` | Stroke style (`Solid`, `Dashed`, `Dotted`, `Gradient`). |
| `dash_length` | `float` | `6.0f` | Length of each vertical dash. |
| `dash_gap` | `float` | `4.0f` | Space between dash/dot segments. |
| `round_caps` | `bool` | `false` | Rounds the top and bottom endpoints. |

---

## Code Examples (From `widgets_demo/divider_demo/main.cpp`)

### 1. Toolbar Section Separator
```cpp
#include "enki/widgets/divider.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/icon_button.hpp"

using namespace enki;

WidgetPtr buildTextToolbar() {
    return row({
        .align_items = Align::Center,
        .gap = 4_px,
        .children = {
            IconButton { .icon = icon(Icons::FormatBold) },
            IconButton { .icon = icon(Icons::FormatItalic) },
            IconButton { .icon = icon(Icons::FormatUnderlined) },
            
            // Vertical separation line
            VerticalDivider {
                .width = 16.0f,
                .height = 20.0f,
                .thickness = 1.0f,
                .color = 0xFF475569,
            },
            
            IconButton { .icon = icon(Icons::FormatAlignLeft) },
            IconButton { .icon = icon(Icons::FormatAlignCenter) },
        }
    });
}
```

### 2. Gradient Vertical Divider Between Columns
```cpp
auto colDivider = VerticalDivider {
    .height = 100_pct,
    .thickness = 1.5f,
    .color = 0xFF38BDF8,
    .style = DividerStyle::Gradient,
};
```

---

## See Also
- [**Divider**](./divider.md) — Horizontal separating line.
- [**Row**](../Layout/row.md) — Flex container commonly hosting vertical dividers.
