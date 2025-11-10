# Skeleton

> A shimmer-animated placeholder widget rendered while asynchronous data loads, presenting content-aware shapes (rectangles, lines of text, and avatars) to minimize perceived wait times.

- **Header File**: `#include "enki/widgets/feedback_status.hpp"`
- **C++ Class**: `enki::SkeletonWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Skeleton` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::SkeletonProps`
- **Factory Helpers**: `enki::skeleton()`, `enki::skeletonRect()`, `enki::skeletonCircle()`, `enki::skeletonText()`
- **Shape Enum**: `enki::SkeletonShape` (`Rectangle`, `Circle`)

---

## Overview

`Skeleton` displays an animated gradient sweep across placeholder geometry to signal that data is actively being fetched. Instead of static grey blocks, Enki's skeleton engine paints a continuous, GPU-accelerated linear gradient that sweeps from left to right using `base_color` and `highlight_color`. It can either wrap an existing widget tree (masking its structure while loading) or render standalone geometric shapes via convenience factories.

---

## C++ API Definition

### `SkeletonShape` Enum & Declarative Struct
```cpp
namespace enki {

enum class SkeletonShape {
    Rectangle,
    Circle
};

struct Skeleton {
    Key                       key             = Key::none();
    WidgetPtr                 child           = nullptr;                        ///< Optional content to mask
    bool                      enabled         = true;                           ///< Shimmer animation toggle
    Color                     base_color      = 0xFF1E293B;                     ///< Background fill color
    Color                     highlight_color = 0xFF334155;                     ///< Sweeping gradient highlight
    std::chrono::milliseconds duration        = std::chrono::milliseconds(1200);///< Cycle repeat time
    std::optional<StyleValue> width           = std::nullopt;
    std::optional<StyleValue> height          = std::nullopt;
    BorderRadius              border_radius   = BorderRadius::circular(4.0f);
    SkeletonShape             shape           = SkeletonShape::Rectangle;

    operator WidgetPtr() const;
};

inline WidgetPtr skeleton(const SkeletonProps& props = {});
inline WidgetPtr skeletonRect(float width, float height, float border_radius = 4.0f,
                              Color base = 0xFF1E293B, Color highlight = 0xFF334155);
inline WidgetPtr skeletonCircle(float diameter, Color base = 0xFF1E293B, Color highlight = 0xFF334155);
inline WidgetPtr skeletonText(float width = 120.0f, float height = 14.0f,
                              Color base = 0xFF1E293B, Color highlight = 0xFF334155);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `enabled` | `bool` | `true` | When true, renders the animated shimmer shader sweep. |
| `base_color` | `Color` | `0xFF1E293B` | Base tone for unhighlighted placeholder regions. |
| `highlight_color` | `Color` | `0xFF334155` | Luminant tone applied to the sweeping light band. |
| `duration` | `milliseconds` | `1200ms` | Duration for one full sweep from left to right. |
| `shape` | `SkeletonShape` | `Rectangle` | Geometry boundary (`Rectangle` or `Circle`). |
| `border_radius` | `BorderRadius` | `4.0f` | Corner rounding when `shape == Rectangle`. |

---

## Code Examples (From `widgets_demo/feedback_status_demo/main.cpp`)

### 1. Composing an Asynchronous User Profile Skeleton
```cpp
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildUserProfilePlaceholder() {
    return container({
        .color = 0xFF0F172A,
        .border_radius = BorderRadius::circular(12.0f),
        .border = Border(0xFF334155, 1.0f),
        .padding = EdgeInsets::all(16.0f),
        .child = row({
            .align_items = Align::Center,
            .gap = 16_px,
            .children = {
                // Circular Avatar Placeholder
                skeletonCircle(56.0f, 0xFF1E293B, 0xFF475569),

                // Multi-line Text Info Placeholder
                expanded(column({
                    .gap = 10_px,
                    .children = {
                        skeletonRect(180.0f, 16.0f, 4.0f, 0xFF1E293B, 0xFF475569),
                        skeletonRect(280.0f, 12.0f, 4.0f, 0xFF1E293B, 0xFF475569),
                        row({
                            .gap = 8_px,
                            .children = {
                                skeletonRect(70.0f, 22.0f, 6.0f, 0xFF1E293B, 0xFF475569),
                                skeletonRect(90.0f, 22.0f, 6.0f, 0xFF1E293B, 0xFF475569),
                            }
                        })
                    }
                }))
            }
        })
    });
}
```

---

## See Also
- [**LoadingOverlay**](./loading_overlay.md) — Modal loading veil with spinner.
- [**ProgressBar**](./progress_bar.md) — Deterministic or looping progress track.
- [**Pulse**](./pulse.md) — Concentric looping status beacon.
