# AnimatedScale

> An implicit animation widget that smoothly interpolates its child's scale transform relative to a configurable alignment anchor whenever the target `scale` value changes.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedScaleWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedScale` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedScale(props)`

---

## Overview

`AnimatedScale` animates scale factor transforms on its child widget without triggering layout relayout passes in parent containers. By configuring `alignment`, developers can anchor the scale origin to the center (`Alignment::Center`), bottom-right corner, or top-left edge. It is ideal for interactive hover zooms, pop-up modal entrances, and button press feedback.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct AnimatedScale {
    float                     scale     = 1.0f;                                  ///< Target scale factor (e.g. 1.0f, 1.35f, 0.0f)
    Alignment                 alignment = Alignment::Center;                     ///< Transformation anchor point
    std::chrono::milliseconds duration  = std::chrono::milliseconds(300);         ///< Transition duration
    const Curve*              curve     = &Curves::linear;                       ///< Easing curve (e.g. &Curves::elasticOut)
    std::function<void()>     on_end    = nullptr;
    WidgetPtr                 child     = nullptr;
    Key                       key       = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedScaleWidget> animatedScale(AnimatedScale props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `scale` | `float` | `1.0f` | Target scale multiplier (e.g. `1.2f` for 120% zoom, `0.0f` to shrink away). |
| `alignment` | `Alignment` | `Alignment::Center`| Transformation origin pivot point. |
| `duration` | `milliseconds` | `300ms` | Animation duration. |
| `curve` | `const Curve*` | `&Curves::linear` | Easing curve (e.g. `&Curves::elasticOut`). |
| `on_end` | `function<void()>` | `nullptr` | Completion callback. |
| `child` | `WidgetPtr` | `nullptr` | Child widget receiving the scale transform. |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Spring-Loaded Elastic Scale Box
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildElasticScaleCard(bool isHovered) {
    return animatedScale({
        .scale = isHovered ? 1.25f : 1.0f,
        .alignment = Alignment::Center,
        .duration = std::chrono::milliseconds(350),
        .curve = &Curves::elasticOut, // Bouncy spring settlement
        .child = container({
            .color = 0xFF581C87,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFFC084FC, 1.5f),
            .padding = EdgeInsets::all(16.0f),
            .child = text("Hover to Zoom!", { .color = 0xFFF3E8FF })
        })
    });
}
```

---

## See Also
- [**AnimatedRotation**](./animated_rotation.md) — Rotational transform animations.
- [**AnimatedSlide**](./animated_slide.md) — Fractional translation offsets.
- [**AnimatedOpacity**](./animated_opacity.md) — Alpha fading animations.
