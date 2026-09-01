# AnimatedSwitcher

> An implicit animation widget that cross-fades and smoothly transitions between different child widgets whenever the current child changes (differentiated by its `Key`).

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedSwitcherWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedSwitcher` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedSwitcher(props)`
- **Builder Types**: `enki::TransitionBuilder`, `enki::LayoutBuilder`

---

## Overview

`AnimatedSwitcher` detects when its child widget is replaced with a new widget (either of a different runtime type or bearing a distinct `Key`). Instead of instantaneously swapping the widgets on screen, `AnimatedSwitcher` keeps the previous child visible and smoothly animates its exit while simultaneously fading in the new child.

---

## C++ API Definition

### Builder Type Aliases & Declarative Struct
```cpp
namespace enki {

using TransitionBuilder = std::function<WidgetPtr(WidgetPtr child, float animation_progress)>;
using LayoutBuilder     = std::function<WidgetPtr(WidgetPtr current_child, const std::vector<WidgetPtr>& previous_children)>;

struct AnimatedSwitcher {
    WidgetPtr                                child              = nullptr;
    std::chrono::milliseconds                duration           = std::chrono::milliseconds(300);
    std::optional<std::chrono::milliseconds> reverse_duration   = std::nullopt;
    const Curve*                             switch_in_curve    = &Curves::linear;
    const Curve*                             switch_out_curve   = &Curves::linear;
    TransitionBuilder                        transition_builder = nullptr;
    LayoutBuilder                            layout_builder     = nullptr;
    Key                                      key                = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedSwitcherWidget> animatedSwitcher(AnimatedSwitcher props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Active child widget (must use a distinct `Key` to trigger transitions). |
| `duration` | `milliseconds` | `300ms` | Transition time for incoming and outgoing children. |
| `reverse_duration` | `optional<milliseconds>`| `nullopt` | Optional custom duration for outgoing children. |
| `switch_in_curve` | `const Curve*` | `&Curves::linear` | Easing curve applied to incoming child. |
| `switch_out_curve`| `const Curve*` | `&Curves::linear` | Easing curve applied to outgoing child. |
| `transition_builder`| `TransitionBuilder` | `nullptr` | Custom transition function (defaults to cross-fade opacity). |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Cross-Fading Between Keyed Subviews
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class SwitcherViewState : public State {
    int current_tab_ = 0;

public:
    WidgetPtr build(BuildContext&) override {
        WidgetPtr activeContent;

        if (current_tab_ == 0) {
            activeContent = container({
                .key = Key::string("tab_profile"), // Unique Key is essential
                .color = 0xFF1E1B4B,
                .border_radius = BorderRadius::circular(10.0f),
                .padding = EdgeInsets::all(16.0f),
                .child = text("👤 User Profile View", { .color = 0xFFFFFFFF })
            });
        } else {
            activeContent = container({
                .key = Key::string("tab_settings"), // Unique Key is essential
                .color = 0xFF064E3B,
                .border_radius = BorderRadius::circular(10.0f),
                .padding = EdgeInsets::all(16.0f),
                .child = text("⚙️ Settings View", { .color = 0xFFFFFFFF })
            });
        }

        return column({
            .gap = 12_px,
            .children = {
                button(text("Switch Tab"), [this] {
                    setState([this] { current_tab_ = (current_tab_ + 1) % 2; });
                }),
                animatedSwitcher({
                    .child = activeContent,
                    .duration = std::chrono::milliseconds(350),
                    .switch_in_curve = &Curves::easeIn,
                    .switch_out_curve = &Curves::easeOut
                })
            }
        });
    }
};
```

---

## See Also
- [**AnimatedOpacity**](./animated_opacity.md) — Single-widget alpha fading.
- [**AnimatedSlide**](./animated_slide.md) — Positional translation.
- [**TabView**](../Navigation/tab_view.md) — Full tab navigation view.
