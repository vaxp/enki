# ScrollView

> The foundational scrolling viewport widget in Enki, driven 100% by the Anu Layout Engine (`Overflow::Scroll`) with mouse wheel, trackpad pan, touch gesture recognition, and Skia clip painting.

- **Header File**: `#include "enki/widgets/scroll_view.hpp"`
- **C++ Class**: `enki::ScrollViewWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ScrollView` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ScrollProps`
- **Render Object**: `enki::RenderScrollView` (inherits from `enki::RenderBox`)
- **Factory Helpers**: `enki::scrollView(...)`

---

## Overview

`ScrollView` wraps a single child widget (typically a `Column` or `Row`) and provides a scrollable clipping viewport. When the child exceeds the viewport's bounds in the chosen `direction`, `ScrollView` computes scroll boundaries (`max_scroll_x`, `max_scroll_y`), applies `context.canvas.clipRect(bounds)`, shifts the paint context, and optionally renders an anti-aliased scrollbar thumb.

---

## C++ API Definition

### Configuration Struct (`ScrollOptions`)
```cpp
namespace enki {

struct ScrollOptions {
    Axis  direction        = Axis::Vertical;
    bool  clamp_overscroll = true;
    bool  show_scrollbar   = false;
    float scroll_speed     = 50.0f; // Multiplier for mouse wheel delta

    constexpr bool operator==(const ScrollOptions&) const = default;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ScrollView {
    Key       key              = Key::none();
    WidgetPtr child            = nullptr;
    Axis      direction        = Axis::Vertical;
    bool      clamp_overscroll = true;
    bool      show_scrollbar   = false;
    float     scroll_speed     = 50.0f;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<ScrollViewWidget> scrollView(WidgetPtr child);
inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollOptions opt, WidgetPtr child);
inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollProps props);
inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollProps props, WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content widget to be scrolled (typically a `Column` or `Row`). |
| `direction` | `Axis` | `Axis::Vertical` | Primary scrolling axis (`Axis::Vertical` or `Axis::Horizontal`). |
| `clamp_overscroll` | `bool` | `true` | When true, prevents scroll offsets from moving beyond `[0, max_scroll]`. |
| `show_scrollbar` | `bool` | `false` | When true, draws a translucent rounded scrollbar thumb along the edge. |
| `scroll_speed` | `float` | `50.0f` | Mouse wheel delta sensitivity multiplier. |

---

## Layout Rules & Best Practices

> [!TIP]
> **Avoid Child Shrinking**: When placing a `Column` inside a vertical `ScrollView`, or a `Row` inside a horizontal `ScrollView`, always specify `.flex_shrink = 0.0f` on the child column/row so Anu allows it to expand to its full natural height/width instead of compressing.

---

## Code Examples (From `widgets_demo/scroll_demo/main.cpp`)

### 1. Vertical Scroll Container with Scrollbar
```cpp
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildVerticalDocument(const std::vector<WidgetPtr>& paragraphs) {
    auto contentColumn = column({
        .flex_shrink = 0.0f, // Essential: allows natural expansion
        .width = 100_pct,
        .gap = 12_px,
        .children = paragraphs,
    });

    return ScrollView {
        .direction = Axis::Vertical,
        .show_scrollbar = true,
        .scroll_speed = 60.0f,
        .child = contentColumn,
    };
}
```

### 2. Horizontal Card Carousel Strip
```cpp
WidgetPtr buildHorizontalCardStrip(const std::vector<WidgetPtr>& cards) {
    auto cardRow = row({
        .flex_shrink = 0.0f,
        .gap = 16_px,
        .children = cards,
    });

    return ScrollView {
        .direction = Axis::Horizontal,
        .show_scrollbar = true,
        .child = cardRow,
    };
}
```

---

## See Also
- [**ListView**](./list_view.md) — Optimized linear list with builder and item selection.
- [**GridView**](./grid_view.md) — 2D multi-column scrollable layout.
- [**Scrollbar**](./scrollbar.md) — Deep dive into scrollbar rendering mechanics.
