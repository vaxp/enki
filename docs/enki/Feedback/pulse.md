# Pulse

> A live status beacon widget that renders looping concentric radar waves radiating outward from a solid center dot, used to visually signal live connectivity, real-time recording, or streaming.

- **Header File**: `#include "enki/widgets/feedback_status.hpp"`
- **C++ Class**: `enki::PulseWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Pulse` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::PulseProps`
- **Factory Helpers**: `enki::pulse()`
- **Curves**: `&Curves::easeOut`

---

## Overview

`Pulse` provides an eye-catching, hardware-accelerated radar beacon animation. It renders a solid central circle (`dot_radius`) and continuously radiates one or more concentric rings (`ring_count`) outward up to `max_radius`, smoothly diminishing their alpha transparency according to a timing curve (`Curves::easeOut`). It is extensively used for "Online", "Live Recording", "Server Healthy", or "System Alert" indicators.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct Pulse {
    Key                       key         = Key::none();
    WidgetPtr                 child       = nullptr;                          ///< Optional centered icon/content
    Color                     color       = 0xFF10B981;                       ///< Radiant beacon color (Emerald default)
    size_t                    ring_count  = 2;                                ///< Number of simultaneous expanding rings
    float                     max_radius  = 24.0f;                            ///< Outer boundary of expanding wave
    float                     dot_radius  = 6.0f;                             ///< Solid center circle radius
    bool                      center_dot  = true;                             ///< Whether to draw solid center dot
    std::chrono::milliseconds duration    = std::chrono::milliseconds(1500);  ///< Single cycle duration
    const Curve*              curve       = &Curves::easeOut;                 ///< Expansion easing curve

    operator WidgetPtr() const;
};

inline WidgetPtr pulse(const PulseProps& props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `color` | `Color` | `0xFF10B981` | Tone applied to center dot and expanding rings. |
| `ring_count` | `size_t` | `2` | Number of phased concentric rings animating outward. |
| `max_radius` | `float` | `24.0f` | Maximum pixel radius before a ring fades out completely. |
| `dot_radius` | `float` | `6.0f` | Radius of the stationary center dot. |
| `center_dot` | `bool` | `true` | When true, paints the anchored central beacon circle. |
| `duration` | `milliseconds` | `1500ms`| Time taken for a single wave to travel from center to `max_radius`. |
| `curve` | `const Curve*`| `&Curves::easeOut` | Easing formula governing expansion speed. |

---

## Code Examples (From `widgets_demo/feedback_status_demo/main.cpp`)

### 1. Live Status Badges (Operational vs Recording Live)
```cpp
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildLiveStatusBadge(bool isLive) {
    return container({
        .color = 0xFF0F172A,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0xFF334155, 1.0f),
        .padding = EdgeInsets::symmetric(10.0f, 14.0f),
        .child = row({
            .align_items = Align::Center,
            .gap = 10_px,
            .children = {
                // Live radar beacon
                pulse({
                    .color = isLive ? 0xFFEF4444 : 0xFF10B981, // Red when live, green when operational
                    .ring_count = 2,
                    .max_radius = 18.0f,
                    .dot_radius = 5.0f,
                    .duration = std::chrono::milliseconds(isLive ? 1000 : 1600)
                }),
                // Text status
                column({
                    .gap = 2_px,
                    .children = {
                        text(isLive ? "Recording Live" : "System Operational", {
                            .color = isLive ? 0xFFF87171 : 0xFF34D399,
                            .font_size = 13.0f,
                            .font_weight = FontWeight::Bold
                        }),
                        text(isLive ? "4K Stream 60 FPS" : "All services healthy", {
                            .color = 0xFF64748B,
                            .font_size = 11.0f
                        })
                    }
                })
            }
        })
    });
}
```

---

## See Also
- [**Skeleton**](./skeleton.md) — Shimmer loading placeholder.
- [**ProgressRing**](./progress_ring.md) — Circular progress track.
- [**Spinner**](./spinner.md) — Spinning indeterminate indicator.
