# AnimatedSlide

> An implicit animation widget that smoothly translates its child's position by a fractional offset relative to the child's own layout bounds whenever the target `offset` changes.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedSlideWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedSlide` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedSlide(props)`
- **Offset Point**: `enki::Point = {dx, dy}` (measured in fractions of child dimensions)

---

## Overview

`AnimatedSlide` performs relative positional translation. The `offset` coordinates are expressed as fractions of the child's size:
- `Point{0.0f, 0.0f}`: Normal layout position.
- `Point{1.0f, 0.0f}`: Shifted completely to the right by 100% of its width.
- `Point{0.0f, -1.0f}`: Shifted completely upwards by 100% of its height.

Because translation does not affect the physical layout constraints of surrounding widgets, it is the standard technique for slide-in notification banners, drawer menus, and off-screen dismissals.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct AnimatedSlide {
    Point                     offset   = {0.0f, 0.0f};                          ///< Fractional offset (dx, dy)
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);         ///< Slide animation duration
    const Curve*              curve    = &Curves::linear;                       ///< Easing curve (e.g. &Curves::easeOut)
    std::function<void()>     on_end   = nullptr;
    WidgetPtr                 child    = nullptr;
    Key                       key      = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedSlideWidget> animatedSlide(AnimatedSlide props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `offset` | `Point` | `{0.0f, 0.0f}` | Fractional shift vector (`{1.0, 0}` is 100% right, `{0, -1.0}` is 100% up). |
| `duration` | `milliseconds` | `300ms` | Animation duration. |
| `curve` | `const Curve*` | `&Curves::linear` | Easing curve (e.g. `&Curves::easeInOut`). |
| `on_end` | `function<void()>` | `nullptr` | Completion callback. |
| `child` | `WidgetPtr` | `nullptr` | Content to slide. |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Slide-in Notification Toast
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildToastBanner(bool isVisible) {
    // Wrap in a container with clip_content = true to hide off-screen state
    return container({
        .clip_content = true,
        .height = StyleValue::point(52.0f),
        .child = animatedSlide({
            // Off-screen above (-1.5) when hidden, resting at (0, 0) when visible
            .offset = isVisible ? Point{0.0f, 0.0f} : Point{0.0f, -1.5f},
            .duration = std::chrono::milliseconds(350),
            .curve = &Curves::easeInOut,
            .child = container({
                .color = 0xFF0E7490,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF22D3EE, 1.5f),
                .padding = EdgeInsets::symmetric(10.0f, 16.0f),
                .child = text("🚀 New message received!", { .color = 0xFFFFFFFF })
            })
        })
    });
}
```

---

## See Also
- [**SlideTransition**](./slide_transition.md) — Controller-driven explicit slide animation.
- [**AnimatedContainer**](./animated_container.md) — Morphs physical dimensions and padding.
- [**AnimatedSwitcher**](./animated_switcher.md) — Cross-fades or slides between changing child widgets.
