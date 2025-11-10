# Ripple

> A tactile Material Design ink-ripple feedback widget that animates an expanding circular radial wave outward from the exact cursor coordinate where a click or touch tap occurred.

- **Header File**: `#include "enki/widgets/feedback_status.hpp"`
- **C++ Class**: `enki::RippleWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Ripple` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::RippleProps`
- **Factory Helpers**: `enki::ripple()`

---

## Overview

`Ripple` wraps any arbitrary child widget (such as a card, list item, or custom button) with physical touch responsiveness. When clicked:
1. It captures the pointer contact coordinates `(x, y)` relative to the widget's bounds.
2. It animates an expanding circular wave that scales outward from that exact origin point while smoothly fading out its opacity.
3. If `clip_ripple = true`, the ink wave is automatically clipped to match the widget's `border_radius`.
4. It triggers the `on_tap` callback cleanly without interfering with lower-level event dispatch.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct Ripple {
    Key                       key           = Key::none();
    WidgetPtr                 child         = nullptr;                         ///< Interactive child widget
    Color                     color         = 0x33FFFFFF;                      ///< Ripple wave tint
    BorderRadius              border_radius = BorderRadius::zero();            ///< Clipping boundary radius
    bool                      clip_ripple   = true;                            ///< Mask wave to border_radius
    std::chrono::milliseconds duration      = std::chrono::milliseconds(350);  ///< Wave expansion time
    std::function<void()>     on_tap        = nullptr;                         ///< Tap callback

    operator WidgetPtr() const;
};

inline WidgetPtr ripple(const RippleProps& props);
inline WidgetPtr ripple(
    WidgetPtr child,
    std::function<void()> on_tap = nullptr,
    Color color = 0x33FFFFFF,
    BorderRadius radius = BorderRadius::zero()
);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The target visual element receiving the ripple effect. |
| `color` | `Color` | `0x33FFFFFF` | Highlight color and initial opacity of the expanding wave. |
| `border_radius` | `BorderRadius` | `zero()` | Corner curvature matching child boundaries. |
| `clip_ripple` | `bool` | `true` | When true, restricts ripple expansion within `border_radius`. |
| `duration` | `milliseconds` | `350ms` | Duration of the expansion and fade-out animation. |
| `on_tap` | `function<void()>`| `nullptr`| Callback dispatched upon mouse click release or touch. |

---

## Code Examples (From `widgets_demo/feedback_status_demo/main.cpp`)

### 1. Interactive Ripple Action Card
```cpp
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildInteractiveCard() {
    return ripple({
        .color = 0x55FFFFFF,
        .border_radius = BorderRadius::circular(10.0f),
        .duration = std::chrono::milliseconds(300),
        .on_tap = [] {
            std::cout << "Card clicked with ink wave feedback!\n";
        },
        .child = container({
            .color = 0xFF0284C7,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = EdgeInsets::all(18.0f),
            .align = Alignment::Center,
            .child = text("Tap to Trigger Ink Wave", {
                .color = 0xFFFFFFFF,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

---

## See Also
- [**Button**](../Basic%20UI/button.md) — Standard clickable push button.
- [**GestureDetector**](../Gestures-Interaction/gesture_detector.md) — Low-level tap, double-tap, and pan detection.
- [**Pulse**](./pulse.md) — Looping beacon indicator animation.
