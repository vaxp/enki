# ClipRRect

> A visual clipping widget that clips its child to a smooth, anti-aliased rounded rectangle (RRect) defined by a `BorderRadius`.

- **Header File**: `#include "enki/widgets/clip.hpp"`
- **C++ Class**: `enki::ClipRRectWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ClipRRect` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::clipRRect(props)`
- **Radius Class**: `enki::BorderRadius`

---

## Overview

`ClipRRect` (Clip Rounded Rect) applies an anti-aliased rounded rectangular clip to any child widget hierarchy. It is frequently combined with `Image`, `BackdropFilter` (for frosted glass cards), and nested scrollable containers to ensure child backgrounds and content do not spill outside the rounded card corners.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct ClipRRect {
    BorderRadius border_radius = BorderRadius::zero(); ///< Corner rounding radius
    Clip         clip_behavior = Clip::AntiAlias;      ///< Anti-aliasing quality
    WidgetPtr    child         = nullptr;
    Key          key           = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ClipRRectWidget> clipRRect(const ClipRRect& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `border_radius` | `BorderRadius` | `BorderRadius::zero()` | Corner curvature (e.g. `BorderRadius::circular(16.0f)`). |
| `child` | `WidgetPtr` | `nullptr` | Child widget clipped to the rounded boundary. |
| `clip_behavior` | `Clip` | `Clip::AntiAlias` | Edge rendering mode (`AntiAlias`, `HardEdge`, `AntiAliasWithSaveLayer`). |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Rounded Card with Anti-Aliased Corners
```cpp
#include "enki/widgets/clip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildRoundedCard() {
    return clipRRect({
        .border_radius = BorderRadius::circular(20.0f),
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .color = 0xFF10B981, // Emerald green
            .align = Alignment::Center,
            .width = StyleValue::point(140.0f),
            .height = StyleValue::point(80.0f),
            .child = text("Rounded Clip", {
                .color = 0xFFFFFFFF,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

### 2. Pairing with BackdropFilter for Frosted Glass Cards
```cpp
auto frostedGlassCard = clipRRect({
    .border_radius = BorderRadius::circular(16.0f),
    .child = backdropFilter({
        .filter = ImageFilter::blur(16.0f, 16.0f),
        .child = container({
            .color = 0x251E293B,
            .border = Border(0x6638BDF8, 1.5f),
            .padding = EdgeInsets::all(16.0f),
            .child = text("Frosted Content", { .color = 0xFFFFFFFF })
        })
    })
});
```

---

## See Also
- [**ClipRect**](./clip_rect.md) — Standard rectangular clipping.
- [**ClipOval**](./clip_oval.md) — Circular and elliptical clipping.
- [**BackdropFilter**](./backdrop_filter.md) — Frosted glass blur effect.
