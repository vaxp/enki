# Enki Layout System

> High-performance, declarative C++20 layout engine powered by **Anu Layout Engine** and **Skia**.

The Enki Layout subsystem provides a comprehensive suite of declarative widgets for structuring user interfaces in 1D (Flexbox), 2D (Wrap), and 2.5D (Stack/Z-axis multi-layered) coordinate spaces. It operates with zero-calculation tampering, delegating layout calculations directly to the high-performance Anu C engine while supporting modern C++20 designated initializers and fluent helper functions.

---

## Widget Catalog (Layout)

The Layout category comprises 16 foundational widgets:

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Row**](./row.md) | `class Row`, `row(...)` | `<enki/widgets/flexbox.hpp>` | Horizontal flex container distributing children along the X-axis. |
| 2 | [**Column**](./column.md) | `class Column`, `column(...)` | `<enki/widgets/flexbox.hpp>` | Vertical flex container distributing children along the Y-axis. |
| 3 | [**Stack**](./stack.md) | `struct Stack`, `class StackWidget`, `stack(...)` | `<enki/widgets/stack.hpp>` | Multi-layered 2.5D container stacking children along the Z-axis with reverse hit-testing. |
| 4 | [**Container**](./container.md) | `struct Container`, `class ContainerWidget`, `container(...)` | `<enki/widgets/container.hpp>` | Visual box model combining `BoxDecoration` (gradients, borders, shadows, shape) with Flexbox geometry. |
| 5 | [**SizedBox**](./sized_box.md) | `sizedBox(width, height, child)` | `<enki/widgets/container.hpp>` | Fixed-dimension box for sizing a child or creating fixed gaps between widgets. |
| 6 | [**Expanded**](./expanded.md) | `expanded(...)`, `ExpandedProps` | `<enki/widgets/flexbox.hpp>` | Expands a child to consume remaining flex space (`flex_grow = N`, `flex_basis = 0px`). |
| 7 | [**Flexible**](./flexible.md) | `flexible(...)`, `ExpandedProps` | `<enki/widgets/flexbox.hpp>` | Allows a child to size flex-growingly while respecting its intrinsic basis (`flex_basis = auto`). |
| 8 | [**Padding**](./padding.md) | `paddingBox(insets, child)`, `StyleInsets`, `EdgeInsets` | `<enki/widgets/container.hpp>` | Insets a child by specified padding on all, symmetric, or individual edges. |
| 9 | [**Align**](./align.md) | `Alignment` on `Container`, `Align::*` on `FlexItem` | `<enki/widgets/container.hpp>` | Positions a child inside its parent using 9 standard alignment anchors or flex self-alignment. |
| 10 | [**Center**](./center.md) | `centerBox(child)` | `<enki/widgets/container.hpp>` | Shorthand helper widget that centers its child horizontally and vertically. |
| 11 | [**Positioned**](./positioned.md) | `struct Positioned`, `positioned(...)` | `<enki/widgets/stack.hpp>` | Absolutely positions a child within a `Stack` via `top`, `right`, `bottom`, `left`, `width`, and `height`. |
| 12 | [**Wrap**](./wrap.md) | `class Wrap`, `wrap(...)` | `<enki/widgets/flexbox.hpp>` | Multi-line flex container that wraps overflowing children onto adjacent lines. |
| 13 | [**Spacer**](./spacer.md) | `spacer(flex = 1.0f)` | `<enki/widgets/flexbox.hpp>` | Empty, lightweight flex item that absorbs leftover space in a `Row` or `Column`. |
| 14 | [**AspectRatio**](./aspect_ratio.md) | `.aspect_ratio` on `FlexItem` & `Container` | `<enki/widgets/flexbox.hpp>` | Constrains a widget's dimensions to maintain a fixed width-to-height proportion. |
| 15 | [**ConstrainedBox**](./constrained_box.md) | `.min_width`, `.max_width`, `.min_height`, `.max_height` | `<enki/widgets/container.hpp>` | Imposes additional minimum and maximum sizing boundaries on child widgets. |
| 16 | [**FractionallySizedBox**](./fractionally_sized_box.md) | `_pct` literal & `StyleValue::percent(...)` | `<enki/widgets/container.hpp>` | Sizes a child relative to a percentage fraction of the parent container's size. |

---

## Core Layout Architecture

### 1. Three-Tree Architecture
Enki structures UI using three synchronous tree layers:
1. **Widget Tree**: Lightweight, immutable declarative descriptions (constructed per build frame).
2. **Element Tree**: State and lifecycle manager, diffing tree nodes on rebuilds.
3. **RenderObject Tree**: Retained layout and paint hierarchy. Flexbox nodes hold an `ANUNodeRef` pointer to the underlying Anu layout node, while painting is performed directly through Skia's `SkCanvas`.

### 2. Dimension Representation: `StyleValue` & Literals
Dimensions across all layout widgets accept the uniform `StyleValue` structure:

```cpp
#include "enki/core/types.hpp"

// Literals available under namespace enki::literals (or `using namespace enki;`):
100_px      // Point/Pixel dimension (StyleValue::point(100.0f))
50_pct      // Percent dimension (StyleValue::percent(50.0f))
auto_val    // Automatic sizing (StyleValue::autoValue())
```

### 3. Edge Insets: `StyleInsets` & `EdgeInsets`
Insets for padding, margins, and absolute positioning can be configured uniformly:

```cpp
StyleInsets::all(16_px)
StyleInsets::symmetric(8_px, 16_px)             // vertical, horizontal
StyleInsets::only(10_px, 20_px, 10_px, 20_px)   // top, right, bottom, left
StyleInsets::directional(10_px, 10_px, 8_px, 8_px) // top, bottom, start, end
```

---

## Getting Started Example

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"

using namespace enki;

WidgetPtr buildHeaderCard() {
    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(12.0f),
        .border = Border(0xFF334155, 1.0f),
        .padding = StyleInsets::all(16_px),
        .width = 100_pct,
        .child = row({
            .align_items = Align::Center,
            .children = {
                text("Enki Workstation", { .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                spacer(),
                container({
                    .color = 0xFF3B82F6,
                    .border_radius = BorderRadius::circular(6.0f),
                    .padding = StyleInsets::symmetric(4_px, 10_px),
                    .child = text("v0.2.0", { .font_size = 12.0f }),
                })
            }
        })
    });
}
```
