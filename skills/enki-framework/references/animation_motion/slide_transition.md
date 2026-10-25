# SlideTransition

> An explicit animation widget that drives positional translation using an external `AnimationController`, interpolating between designated `begin` and `end` fractional coordinates.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::SlideTransitionWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::SlideTransition` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::slideTransition(props)`
- **Controller**: `std::shared_ptr<enki::AnimationController>`

---

## Overview

Unlike `AnimatedSlide` (which is an implicit widget managing internal state), `SlideTransition` is an **explicit** animation primitive. It does not manage its own ticker; instead, it is driven by an external `AnimationController`. This allows precise programmatic control: stopping, reversing, repeating, fling physics, or tying the slide position directly to a gesture drag event.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct SlideTransition {
    std::shared_ptr<AnimationController> position            = nullptr;             ///< Driving animation controller
    Point                                begin               = {0.0f, 0.0f};        ///< Starting fractional coordinate
    Point                                end                 = {1.0f, 0.0f};        ///< Ending fractional coordinate
    const Curve*                         curve               = &Curves::linear;     ///< Optional curve mapping
    bool                                 transform_hit_tests = true;                ///< Update hit test target positions
    WidgetPtr                            child               = nullptr;
    Key                                  key                 = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<SlideTransitionWidget> slideTransition(const SlideTransition& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `position` | `shared_ptr<AnimationController>`| `nullptr` | Driving animation controller (`0.0f` to `1.0f`). |
| `begin` | `Point` | `{0.0f, 0.0f}` | Fractional offset corresponding to controller value `0.0`. |
| `end` | `Point` | `{1.0f, 0.0f}` | Fractional offset corresponding to controller value `1.0`. |
| `curve` | `const Curve*` | `&Curves::linear` | Easing curve applied to the linear progression of `position`. |
| `transform_hit_tests`| `bool`| `true` | When true, mouse hit-testing follows the transformed visual position. |
| `child` | `WidgetPtr` | `nullptr` | Content to be shifted. |

---

## Code Examples (From `tests/widgets/test_motion.cpp` & `widgets_demo/motion_demo/main.cpp`)

### 1. Programmatic Slide-Down Drawer Driven by AnimationController
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class DrawerControllerState : public State {
    std::shared_ptr<AnimationController> anim_ctrl_;

public:
    void initState() override {
        State::initState();
        // 500ms transition controller
        anim_ctrl_ = std::make_shared<AnimationController>(std::chrono::milliseconds(500));
    }

    WidgetPtr build(BuildContext&) override {
        return column({
            .gap = 16_px,
            .children = {
                row({
                    .gap = 8_px,
                    .children = {
                        button(text("Open Drawer"),  [this] { anim_ctrl_->forward(); }),
                        button(text("Close Drawer"), [this] { anim_ctrl_->reverse(); }),
                    }
                }),
                slideTransition({
                    .position = anim_ctrl_,
                    .begin = {0.0f, -1.0f}, // Starts fully offscreen above
                    .end   = {0.0f,  0.0f}, // Slides down into normal position
                    .curve = &Curves::easeOut,
                    .transform_hit_tests = true,
                    .child = container({
                        .color = 0xFF0284C7,
                        .border_radius = BorderRadius::circular(10.0f),
                        .padding = EdgeInsets::all(16.0f),
                        .child = text("🚀 Controlled Drawer Content", { .color = 0xFFFFFFFF })
                    })
                })
            }
        });
    }
};
```

---

## See Also
- [**AnimatedSlide**](./animated_slide.md) — Implicit fire-and-forget slide animation.
- [**AnimatedSwitcher**](./animated_switcher.md) — Animated transitions between changing children.
- [**Drawer**](../Navigation/drawer.md) — Pre-built slide-out side drawer navigation.
