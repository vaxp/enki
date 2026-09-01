# AnimatedRotation

> An implicit animation widget that smoothly interpolates its child's rotation around a configurable alignment pivot whenever the target `turns` value changes.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedRotationWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedRotation` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedRotation(props)`

---

## Overview

`AnimatedRotation` applies a hardware 2D rotation matrix to its child widget based on a fractional turn count, where **`1.0f turn = 360° (2π radians)`**. Changing `turns` from `0.0f` to `0.25f` smoothly rotates the child by 90 degrees clockwise. This widget is commonly used for rotating accordion expand/collapse chevron icons, spinning refresh glyphs, and wheel animations.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct AnimatedRotation {
    float                     turns     = 0.0f;                                  ///< Rotation in turns (1.0f = 360 degrees)
    Alignment                 alignment = Alignment::Center;                     ///< Pivot point of rotation
    std::chrono::milliseconds duration  = std::chrono::milliseconds(300);         ///< Animation duration
    const Curve*              curve     = &Curves::linear;                       ///< Easing curve (e.g. &Curves::bounceOut)
    std::function<void()>     on_end    = nullptr;
    WidgetPtr                 child     = nullptr;
    Key                       key       = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedRotationWidget> animatedRotation(AnimatedRotation props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `turns` | `float` | `0.0f` | Rotation count (`0.25f` = 90°, `0.5f` = 180°, `1.0f` = 360°). |
| `alignment` | `Alignment` | `Alignment::Center`| Point around which rotation occurs. |
| `duration` | `milliseconds` | `300ms` | Transition duration. |
| `curve` | `const Curve*` | `&Curves::linear` | Interpolation curve (e.g. `&Curves::easeInOut`, `&Curves::bounceOut`). |
| `on_end` | `function<void()>` | `nullptr` | Completion callback. |
| `child` | `WidgetPtr` | `nullptr` | Content to be rotated. |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Rotating Chevron Icon for Expandable Sections
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildRotatingChevron(bool isExpanded, std::function<void()> onToggle) {
    return button({
        .child = row({
            .gap = 8_px,
            .children = {
                text("Section Details"),
                animatedRotation({
                    .turns = isExpanded ? 0.5f : 0.0f, // Rotates 180° when expanded
                    .duration = std::chrono::milliseconds(250),
                    .curve = &Curves::easeInOut,
                    .child = text("⌄", { .font_size = 14.0f })
                })
            }
        }),
        .on_pressed = onToggle
    });
}
```

---

## See Also
- [**AnimatedScale**](./animated_scale.md) — Scale transformations.
- [**AnimatedSlide**](./animated_slide.md) — Fractional translation offsets.
- [**Accordion**](../Advanced%20%20Data%20UI/accordion.md) — Collapsible sections with rotating chevrons.
