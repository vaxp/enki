# AnimatedOpacity

> An implicit animation widget that smoothly interpolates its child's opacity over a given duration whenever the target `opacity` value changes.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedOpacityWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedOpacity` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedOpacity(props)`
- **Animation Curves**: `<enki/animation/curves.hpp>`

---

## Overview

`AnimatedOpacity` automatically manages smooth fading transitions for its child widget hierarchy. Whenever a parent widget triggers a state update (`setState()`) with a modified `opacity` value, `AnimatedOpacity` catches the delta and computes an internal tween over the specified `duration` using hardware-accelerated Skia alpha compositing (`saveLayerAlpha`).

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct AnimatedOpacity {
    float                     opacity  = 1.0f;                                  ///< Target alpha (0.0f to 1.0f)
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);         ///< Transition duration
    const Curve*              curve    = &Curves::linear;                       ///< Easing curve
    std::function<void()>     on_end   = nullptr;                               ///< Completion callback
    WidgetPtr                 child    = nullptr;
    Key                       key      = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedOpacityWidget> animatedOpacity(AnimatedOpacity props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `opacity` | `float` | `1.0f` | Target opacity (from 0.0 invisible to 1.0 fully opaque). |
| `duration` | `milliseconds` | `300ms` | Time span over which the opacity transition takes place. |
| `curve` | `const Curve*` | `&Curves::linear` | Animation easing curve (e.g. `&Curves::easeInOut`). |
| `on_end` | `function<void()>` | `nullptr` | Optional callback invoked when the opacity reaches its target. |
| `child` | `WidgetPtr` | `nullptr` | Content subtree to be faded. |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Toggle Visibility with Smooth Fade
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class FadeViewState : public State {
    bool is_visible_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        return column({
            .gap = 12_px,
            .children = {
                button(text(is_visible_ ? "Fade Out" : "Fade In"), [this] {
                    setState([this] { is_visible_ = !is_visible_; });
                }),
                animatedOpacity({
                    .opacity = is_visible_ ? 1.0f : 0.0f,
                    .duration = std::chrono::milliseconds(400),
                    .curve = &Curves::easeInOut,
                    .child = container({
                        .color = 0xFF881337,
                        .border_radius = BorderRadius::circular(10.0f),
                        .padding = EdgeInsets::all(16.0f),
                        .child = text("Smoothly fading content box!", { .color = 0xFFFFFFFF })
                    })
                })
            }
        });
    }
};
```

---

## See Also
- [**AnimatedContainer**](./animated_container.md) — Morphs size, colors, padding, and borders simultaneously.
- [**AnimatedSwitcher**](./animated_switcher.md) — Cross-fades between two distinct widgets.
- [**AnimatedScale**](./animated_scale.md) — Scales child widgets smoothly.
