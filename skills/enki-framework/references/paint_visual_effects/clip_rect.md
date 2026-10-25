# ClipRect

> A visual clipping widget that constrains its child widget's painting pass to its own axis-aligned rectangular layout boundaries.

- **Header File**: `#include "enki/widgets/clip.hpp"`
- **C++ Class**: `enki::ClipRectWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ClipRect` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::clipRect(props)`
- **Enum**: `enki::Clip` (`None`, `HardEdge`, `AntiAlias`, `AntiAliasWithSaveLayer`)

---

## Overview

`ClipRect` prevents child widgets from painting outside their allocated bounding box. In Enki, layout widgets allow overflow by default for performance; wrapping overflowing content (such as scrolling containers, animating sliders, or zooming images) inside a `ClipRect` ensures that visual artifacts do not leak into neighbouring elements.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct ClipRect {
    Clip      clip_behavior = Clip::AntiAlias; ///< Edge anti-aliasing strategy
    WidgetPtr child         = nullptr;         ///< Content to be clipped
    Key       key           = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ClipRectWidget> clipRect(const ClipRect& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The child widget whose painting will be clipped. |
| `clip_behavior` | `Clip` | `Clip::AntiAlias` | Edge rendering behavior (`HardEdge`, `AntiAlias`, `AntiAliasWithSaveLayer`). |
| `key` | `Key` | `Key::none()` | Optional widget reconciliation key. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Simple Rectangular Clipping
```cpp
#include "enki/widgets/clip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildClippedBox() {
    return clipRect({
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .color = 0xFF3B82F6,
            .align = Alignment::Center,
            .width = StyleValue::point(110.0f),
            .height = StyleValue::point(75.0f),
            .child = text("ClipRect", {
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

---

## See Also
- [**ClipRRect**](./clip_rrect.md) — Rounded rectangle clipping with configurable radii.
- [**ClipOval**](./clip_oval.md) — Circular and elliptical clipping.
- [**ClipPath**](./clip_path.md) — Arbitrary vector geometry clipping.
