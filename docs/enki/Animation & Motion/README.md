# Enki Animation & Motion Suite

> Declarative C++20 implicit and explicit animation widgets, hardware-accelerated property tweening, physical interpolation curves, and widget cross-fading.

The **Animation & Motion** subsystem provides fluid, 60+ FPS visual transitions for user interfaces in the Enki GUI Framework. The suite is divided into two intuitive paradigms: **Implicit Animations** (which automatically interpolate between old and new property values whenever the widget rebuilds with changed state) and **Explicit Animations** (which are precisely controlled via an `AnimationController`).

---

## Architectural Paradigms

### 1. Implicit Animations (Fire-and-Forget)
Implicit widgets manage their own internal animation controllers and tickers. When a property value (such as `opacity`, `scale`, or container dimensions) changes during a reactive `setState()` rebuild, the widget automatically animates smoothly from its current value to the target value over the specified `duration` using the provided `Curve`:
- `AnimatedOpacity`
- `AnimatedContainer`
- `AnimatedScale`
- `AnimatedRotation`
- `AnimatedSlide`
- `AnimatedSwitcher`

### 2. Explicit Transitions (Controller-Driven)
Explicit transitions do not manage internal tickers. Instead, they accept an external `std::shared_ptr<AnimationController>` and a target range (such as `.begin` and `.end` points), allowing complex orchestration, choreography, looping, or gesture-driven scrubbing:
- `SlideTransition`

---

## Supported Physical Curves (`enki/animation/curves.hpp`)

All motion widgets accept a pointer to a `Curve`:
- `Curves::linear` — Constant velocity.
- `Curves::easeIn`, `Curves::easeOut`, `Curves::easeInOut` — Standard cubic bezier easing.
- `Curves::fastOutSlowIn` — Natural material design acceleration and deceleration.
- `Curves::elasticOut` — Spring-loaded overshoot and settling.
- `Curves::bounceOut` — Gravitational bounce settling.

---

## Widget Catalog (Animation & Motion)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**AnimatedOpacity**](./animated_opacity.md) | `struct AnimatedOpacity`, `animatedOpacity()` | `<enki/widgets/motion.hpp>` | Smoothly animates opacity fading via GPU saveLayer alpha blending. |
| 2 | [**AnimatedContainer**](./animated_container.md) | `struct AnimatedContainer`, `animatedContainer()` | `<enki/widgets/motion.hpp>` | Automatically morphs size, colors, borders, radius, and insets. |
| 3 | [**AnimatedScale**](./animated_scale.md) | `struct AnimatedScale`, `animatedScale()` | `<enki/widgets/motion.hpp>` | Smooth scale transformations centered around an `Alignment` pivot. |
| 4 | [**AnimatedRotation**](./animated_rotation.md) | `struct AnimatedRotation`, `animatedRotation()` | `<enki/widgets/motion.hpp>` | Smooth rotation animation driven by turn count (`1.0f = 360°`). |
| 5 | [**AnimatedSlide**](./animated_slide.md) | `struct AnimatedSlide`, `animatedSlide()` | `<enki/widgets/motion.hpp>` | Translates child position relative to its layout bounding box. |
| 6 | [**AnimatedSwitcher**](./animated_switcher.md) | `struct AnimatedSwitcher`, `animatedSwitcher()` | `<enki/widgets/motion.hpp>` | Cross-fades and transitions between changing child widgets. |
| 7 | [**SlideTransition**](./slide_transition.md) | `struct SlideTransition`, `slideTransition()` | `<enki/widgets/motion.hpp>` | Low-level positional slide driven by an explicit `AnimationController`. |

---

## Quick Example (Interactive Expand & Morph Card)

```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class MorphingCardState : public State {
    bool is_expanded_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        return animatedContainer({
            .color = is_expanded_ ? 0xFF059669 : 0xFF1D4ED8, // Emerald <-> Blue
            .border_radius = is_expanded_ ? BorderRadius::circular(24.0f) : BorderRadius::circular(8.0f),
            .width = is_expanded_ ? StyleValue::point(320.0f) : StyleValue::point(200.0f),
            .height = is_expanded_ ? StyleValue::point(140.0f) : StyleValue::point(60.0f),
            .duration = std::chrono::milliseconds(450),
            .curve = &Curves::fastOutSlowIn,
            .child = button(text(is_expanded_ ? "Shrink Card" : "Expand Card", { .color = 0xFFFFFFFF }), [this] {
                setState([this] { is_expanded_ = !is_expanded_; });
            })
        });
    }
};
```
