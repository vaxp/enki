# Container

> A comprehensive visual decoration and layout box combining Skia background styling (colors, gradients, borders, shadows, rounded corners) with full Anu Flexbox geometry and constraints.

- **Header File**: `#include "enki/widgets/container.hpp"`
- **C++ Class**: `enki::ContainerWidget`
- **Declarative Struct**: `enki::Container` (converts implicitly to `WidgetPtr`)
- **Props Type**: `enki::ContainerProps` (alias for `enki::Container`)
- **Render Object**: `enki::RenderDecoratedBox`
- **Underlying Engine**: Skia 2D rendering + Anu Layout Engine

---

## Overview

`Container` is the most versatile single-child widget in Enki. It combines visual painting (`BoxDecoration`) with layout styling (`FlexboxStyle`). You can use it to configure background fills, linear/radial gradients, multiple drop/inner shadows, rounded corners, borders, clipping, insets (padding and margins), constraints, aspect ratios, and child alignment.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Container {
    // ── Visual Decoration ──────────────────────────────────────
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;

    // ── Child Alignment ────────────────────────────────────────
    std::optional<Alignment>      align;

    // ── Dimensions & Constraints ───────────────────────────────
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // ── Insets ─────────────────────────────────────────────────
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;
    std::optional<StyleInsets>    position;

    // ── Flexbox Item Properties ────────────────────────────────
    std::optional<float>          flex;
    std::optional<float>          flex_grow;
    std::optional<float>          flex_shrink;
    std::optional<StyleValue>     flex_basis;
    std::optional<Align>          align_self;
    std::optional<PositionType>   position_type;

    WidgetPtr                     child = nullptr;
    Key                           key   = Key::none();

    operator WidgetPtr() const;
};

using ContainerProps = Container;

} // namespace enki
```

### Factory Helper Function
```cpp
namespace enki {

inline std::shared_ptr<ContainerWidget> container(ContainerProps props = {});

} // namespace enki
```

---

## Properties Reference

### Visual Decoration
| Property | Type | Default | Description |
|---|---|---|---|
| `color` | `std::optional<Color>` | `Colors::Transparent` | Background 32-bit ARGB color (e.g. `0xFF1E293B`). |
| `gradient` | `std::optional<GradientConfig>` | `std::nullopt` | Linear or radial background gradient. |
| `border_radius` | `std::optional<BorderRadius>` | `BorderRadius::zero()` | Corner radius (`circular(r)`, `all(r)`, `only(...)`). |
| `border` | `std::optional<Border>` | `std::nullopt` | Stroke outline (`Border(Color, width)`). |
| `box_shadow` | `std::vector<BoxShadow>` | `{}` | List of drop or glow shadows (`BoxShadow::standard()`). |
| `shape` | `std::optional<BoxShape>` | `BoxShape::Rectangle` | Box shape (`BoxShape::Rectangle` or `BoxShape::Circle`). |
| `clip_content` | `std::optional<bool>` | `false` | Whether to clip the child widget to the container's rounded bounds. |

### Layout & Sizing
| Property | Type | Default | Description |
|---|---|---|---|
| `align` | `std::optional<Alignment>` | `std::nullopt` | Anchors the child inside the container (`Alignment::Center`, etc.). |
| `width` | `std::optional<StyleValue>` | `auto_val` | Explicit width (`200_px`, `50_pct`). |
| `height` | `std::optional<StyleValue>` | `auto_val` | Explicit height (`100_px`, `100_pct`). |
| `min_width` / `max_width` | `std::optional<StyleValue>` | `undefined_val` | Minimum and maximum width boundaries. |
| `min_height` / `max_height` | `std::optional<StyleValue>` | `undefined_val` | Minimum and maximum height boundaries. |
| `aspect_ratio` | `std::optional<float>` | `std::nullopt` | Enforces width-to-height proportion (e.g. `16.0f / 9.0f`). |
| `padding` | `std::optional<StyleInsets>` | `0` | Inner padding between container border and child. |
| `margin` | `std::optional<StyleInsets>` | `0` | Outer margin around the container. |
| `child` | `WidgetPtr` | `nullptr` | The nested child widget. |

---

## Code Examples

### 1. Modern Glassmorphism / Dark Theme Card (Pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildMetricCard() {
    return container({
        .color = 0xFF161D2F,
        .border_radius = BorderRadius::circular(14.0f),
        .border = Border(0x4038BDF8, 1.5f),
        .box_shadow = {
            BoxShadow(0x30000000, {0.0f, 6.0f}, 12.0f, 0.0f),
        },
        .padding = StyleInsets::all(20_px),
        .width = 280_px,
        .child = text("Active Users: 12,480", {
            .color = 0xFFF8FAFC,
            .font_size = 15.0f,
            .font_weight = FontWeight::SemiBold,
        })
    });
}
```

### 2. Circular Avatar with Gradient
```cpp
auto avatar = container({
    .shape = BoxShape::Circle,
    .gradient = GradientConfig::linear(
        {0xFF6366F1, 0xFF00E5FF},
        {0.0f, 0.0f}, {1.0f, 1.0f}
    ),
    .width = 56_px,
    .height = 56_px,
    .align = Alignment::Center,
    .child = text("JD", { .font_weight = FontWeight::Bold }),
});
```

---

## See Also
- [**SizedBox**](./sized_box.md) — Lightweight dimension-only box.
- [**Padding**](./padding.md) — Dedicated padding box wrapper.
- [**Align**](./align.md) & [**Center**](./center.md) — Alignment within containers.
- [**ConstrainedBox**](./constrained_box.md) — Min/max size bounds.
