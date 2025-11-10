# ClipOval

> A visual clipping widget that clips its child to an inscribed axis-aligned ellipse or perfect circle.

- **Header File**: `#include "enki/widgets/clip.hpp"`
- **C++ Class**: `enki::ClipOvalWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ClipOval` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::clipOval(props)`

---

## Overview

`ClipOval` inscribes an oval within the child widget's layout bounding box. If the child's width and height are equal, `ClipOval` produces a perfect circle; if unequal, it creates an ellipse with horizontal and vertical axes matching the widget's dimensions. It is the preferred primitive for circular profile avatars, round icon buttons, and circular status badges.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct ClipOval {
    Clip      clip_behavior = Clip::AntiAlias;
    WidgetPtr child         = nullptr;
    Key       key           = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ClipOvalWidget> clipOval(const ClipOval& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content clipped to the ellipse. |
| `clip_behavior` | `Clip` | `Clip::AntiAlias` | Edge rendering quality (`AntiAlias`, `HardEdge`). |
| `key` | `Key` | `Key::none()` | Optional widget reconciliation key. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Elliptical & Circular Avatar
```cpp
#include "enki/widgets/clip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildCircularBadge() {
    return clipOval({
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .color = 0xFF8B5CF6, // Purple accent
            .align = Alignment::Center,
            .width = StyleValue::point(80.0f),
            .height = StyleValue::point(80.0f), // Equal width & height = perfect circle
            .child = text("👤", { .font_size = 32.0f })
        })
    });
}
```

---

## See Also
- [**Avatar**](../Basic%20UI/avatar.md) — Pre-packaged circular avatar widget.
- [**ClipRRect**](./clip_rrect.md) — Rounded rectangle clipping.
- [**ClipPath**](./clip_path.md) — Arbitrary vector geometry clipping.
