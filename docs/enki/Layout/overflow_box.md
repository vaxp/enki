# OverflowBox

> A layout widget that imposes different constraints on its child than it receives from its parent, allowing the child to overflow parent boundaries visually and interactively.

- **Header File**: `#include "enki/widgets/overflow_box.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class OverflowBoxWidget`, `struct OverflowBoxProps`, `struct OverflowBox`
- **Helper Function**: `overflowBox(...)`

---

## Overview

`OverflowBox` allows a child widget to render beyond the bounds of its parent container. In standard Flexbox containers, children are rigidly clamped to parent boundaries. With `OverflowBox`, you can unbind constraints, enabling badges, protruding avatars, floating indicator icons, and decorative background artwork to hang outside their parent card.

### Key Architectural Behaviors:
- **Constraint Decoupling**: The parent container retains its own fixed dimensions, while the child inside `OverflowBox` is given custom sizing constraints (`min_width`, `max_width`, `min_height`, `max_height`).
- **9-Anchor Alignment Engine**: Child positioning is calculated geometrically according to `alignment` (`Center`, `TopRight`, `BottomLeft`, etc.), permitting both positive and negative visual offsets.
- **Interactive Hit Testing**: In `Clip::None` mode, pointer click and hover events work accurately outside the parent container boundaries. When `Clip::HardEdge` or `Clip::AntiAlias` is selected, clipping and hit-testing are strictly confined to parent bounds.

---

## C++ API Definition

### Header: `<enki/widgets/overflow_box.hpp>`

```cpp
namespace enki {

struct OverflowBoxProps {
    Key                  key           = Key::none();
    Alignment            alignment     = Alignment::Center;
    std::optional<float> min_width     = std::nullopt;
    std::optional<float> max_width     = std::nullopt;
    std::optional<float> min_height    = std::nullopt;
    std::optional<float> max_height    = std::nullopt;
    Clip                 clip_behavior = Clip::None;
    WidgetPtr            child         = nullptr;
};

struct OverflowBox {
    Key                  key           = Key::none();
    Alignment            alignment     = Alignment::Center;
    std::optional<float> min_width     = std::nullopt;
    std::optional<float> max_width     = std::nullopt;
    std::optional<float> min_height    = std::nullopt;
    std::optional<float> max_height    = std::nullopt;
    Clip                 clip_behavior = Clip::None;
    WidgetPtr            child         = nullptr;

    operator WidgetPtr() const;
};

// Declarative factory helper:
std::shared_ptr<OverflowBoxWidget> overflowBox(const OverflowBoxProps& props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and performance diffing. |
| `alignment` | `Alignment` | `Alignment::Center` | Anchor position of the overflowing child relative to parent bounds. |
| `min_width` | `std::optional<float>` | `std::nullopt` | Minimum width constraint imposed on the child. |
| `max_width` | `std::optional<float>` | `std::nullopt` | Maximum width constraint imposed on the child. |
| `min_height` | `std::optional<float>` | `std::nullopt` | Minimum height constraint imposed on the child. |
| `max_height` | `std::optional<float>` | `std::nullopt` | Maximum height constraint imposed on the child. |
| `clip_behavior` | `Clip` | `Clip::None` | Clipping mode: `Clip::None` (free visual overflow), `Clip::HardEdge`, `Clip::AntiAlias`. |
| `child` | `WidgetPtr` | `nullptr` | The child widget permitted to overflow. |

---

## Real Code Examples

### 1. Interactive Overhanging Profile Badge (From `widgets_demo/overflow_box_demo/main.cpp`)
A live status badge and avatar protrude outside the boundaries of a profile card:

```cpp
#include "enki/widgets/overflow_box.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildCardWithOverhangingBadge(WidgetPtr cardContent) {
    // 1. Protruding badge anchored at TopRight corner
    auto overhanging_badge = overflowBox({
        .key           = Key::string("protruding_badge_ofb"),
        .alignment     = Alignment::TopRight,
        .min_width     = 100.0f,
        .max_width     = 100.0f,
        .min_height    = 30.0f,
        .max_height    = 30.0f,
        .clip_behavior = Clip::None,
        .child         = gestureDetector({
            .key   = Key::string("badge_gd"),
            .child = container({
                .color         = 0xFFDC2626, // Red
                .border_radius = BorderRadius::circular(15.0f),
                .border        = Border(0xFFFCA5A5, 2.0f),
                .align         = Alignment::Center,
                .padding       = StyleInsets::symmetric(4.0f, 10.0f),
                .child         = text({
                    .text        = "● LIVE VIP",
                    .color       = 0xFFFFFFFF,
                    .font_size   = 10.0f,
                    .font_weight = FontWeight::Bold,
                    .key         = Key::string("badge_txt"),
                }),
                .key = Key::string("badge_box"),
            }),
            .on_tap = []() { /* badge tapped outside card */ },
        }),
    });

    // 2. Fixed parent container
    return container({
        .color         = 0xFF0F172A,
        .border_radius = BorderRadius::circular(14.0f),
        .border        = Border(0xFF1E293B, 1.0f),
        .width         = StyleValue::point(260.0f),
        .height        = StyleValue::point(180.0f),
        .child         = overhanging_badge,
        .key           = Key::string("profile_card_box"),
    });
}
```

### 2. Sandbox Inspection Container (From `widgets_demo/overflow_box_demo/main.cpp`)
```cpp
auto sandbox_overflow = overflowBox({
    .key           = Key::string("sandbox_ofb"),
    .alignment     = Alignment::Center,
    .min_width     = 180.0f,
    .max_width     = 180.0f,
    .min_height    = 180.0f,
    .max_height    = 180.0f,
    .clip_behavior = Clip::None,
    .child         = container({
        .color         = 0xD98B5CF6,
        .border_radius = BorderRadius::circular(12.0f),
        .border        = Border(0xFFC084FC, 2.0f),
        .width         = StyleValue::point(180.0f),
        .height        = StyleValue::point(180.0f),
        .child         = text("180x180 Content in 120x120 Parent"),
    }),
});
```

---

## See Also
- [**LimitedBox**](./limited_box.md) — Clamping dimensions only when unconstrained.
- [**Stack**](./stack.md) — Multi-layered positioning container.
- [**Positioned**](./positioned.md) — Absolute positioning within a stack.
