# Scrollbar

> The hardware-accelerated scrollbar rendering pipeline integrated into Enki's scrolling architecture (`ScrollView`, `ListView`, and `SliverView`).

- **Associated Headers**: `#include "enki/widgets/scroll_view.hpp"`, `#include "enki/widgets/list_view.hpp"`
- **Activation Property**: `show_scrollbar = true`
- **Render Implementation**: `RenderScrollView::paint` in `src/widgets/scroll_view.cpp`
- **Underlying Engine**: Skia anti-aliased rounded rectangle canvas drawing (`drawRRect`)

---

## Overview

In the Enki GUI Framework, rather than requiring verbose nested wrapper widgets, scrollbars are **integrated directly into the scrolling primitives**. Setting `.show_scrollbar = true` on any `ScrollView`, `ListView`, or `GridView` activates automatic proportional thumb calculations and renders a modern, translucent floating scrollbar track on the active scroll edge.

---

## Technical Implementation & Calculation Mechanics

When `.show_scrollbar = true`, `RenderScrollView::paint` executes the following algorithm during the paint pass:

### 1. Vertical Scrollbar (Right Edge)
```cpp
// 1. Calculate proportional thumb height
float viewport_h = size_.height;
float child_h    = viewport_h + max_scroll_y;
float thumb_h    = std::max(20.0f, (viewport_h / child_h) * viewport_h);

// 2. Map current scroll offset to viewport position
float scroll_percent = scroll_offset_y / max_scroll_y;
float thumb_y        = bounds.y + scroll_percent * (viewport_h - thumb_h);

// 3. Draw rounded pill thumb
Rect thumb_rect = { bounds.x + bounds.width - 6.0f, thumb_y, 4.0f, thumb_h };
context.canvas.drawRRect(thumb_rect, BorderRadius::circular(2.0f), thumb_paint);
```

### 2. Horizontal Scrollbar (Bottom Edge)
```cpp
float viewport_w     = size_.width;
float child_w        = viewport_w + max_scroll_x;
float thumb_w        = std::max(20.0f, (viewport_w / child_w) * viewport_w);
float scroll_percent = scroll_offset_x / max_scroll_x;
float thumb_x        = bounds.x + scroll_percent * (viewport_w - thumb_w);

Rect thumb_rect = { thumb_x, bounds.y + bounds.height - 6.0f, thumb_w, 4.0f };
context.canvas.drawRRect(thumb_rect, BorderRadius::circular(2.0f), thumb_paint);
```

---

## Specifications & Visual Constants

| Attribute | Specification | Description |
|---|---|---|
| **Thumb Thickness** | `4.0f` px | Sleek, unobtrusive modern pill bar. |
| **Edge Inset** | `6.0f` px from border | Floats cleanly within the viewport without obscuring borders. |
| **Minimum Size** | `20.0f` px | Ensures the thumb remains easily visible even in massive datasets. |
| **Corner Radius** | `2.0f` px (`BorderRadius::circular(2.0f)`) | Fully rounded pill capsule ends. |
| **Default Color** | `0x80808080` (50% translucent gray) | Neutral styling visible on both dark and light surfaces. |
| **Anti-Aliasing** | `true` | Subpixel smooth edge rendering via Skia. |

---

## Usage Examples

### 1. Enabling Scrollbars in `ScrollView`
```cpp
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

auto myScrollableDocument = ScrollView {
    .direction = Axis::Vertical,
    .show_scrollbar = true, // Activates floating scrollbar thumb
    .child = column({
        .flex_shrink = 0.0f,
        .children = { /* ... long content ... */ }
    })
};
```

### 2. Enabling Scrollbars in `ListView`
```cpp
#include "enki/widgets/list_view.hpp"

auto logFeed = ListView {
    .item_count = 1000,
    .item_builder = [](int i) { return buildLogLine(i); },
    // ScrollView inside ListView inherits ScrollOptions
    .scroll_physics = ScrollPhysics::Clamped,
};
```

---

## See Also
- [**ScrollView**](./scroll_view.md) — The parent viewport hosting the scrollbar.
- [**ListView**](./list_view.md) — High-volume linear list view.
