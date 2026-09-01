# AspectRatio

> A layout constraint that attempts to size a widget to a specific aspect ratio (width / height).

- **Header Files**: `#include "enki/widgets/container.hpp"` and `#include "enki/widgets/flexbox.hpp"`
- **Property**: `.aspect_ratio = <float>` on `Container`, `FlexItem`, and `Flexbox`
- **Underlying Engine**: Direct Anu Layout Engine integration (`ANUNodeStyleSetAspectRatio`)

---

## Overview

In Enki, aspect ratio constraints are built directly into `Container`, `FlexItem`, and `Flexbox` through the `aspect_ratio` property. It enforces a strict mathematical proportion between the width and height of the box:

$$\text{aspect\_ratio} = \frac{\text{width}}{\text{height}}$$

When either dimension is constrained (e.g., width is set to `320_px` or `100_pct`), the Anu Layout Engine calculates the dependent dimension automatically.

---

## C++ API Definition

### Property Definition
```cpp
// Within ContainerProps, FlexboxProps, and FlexItemProps:
std::optional<float> aspect_ratio;
```

### Underlying Engine Mapping
```cpp
ANUNodeStyleSetAspectRatio(node, style.aspect_ratio ? *style.aspect_ratio : ANUUndefined);
```

---

## Standard Aspect Ratio Values

| Ratio | Value | Common Application |
|---|---|---|
| `1.0f` | `1.0f` | Square avatar, icon button, profile picture. |
| `16.0f / 9.0f` | `1.7778f` | Widescreen video players, YouTube previews, hero banners. |
| `4.0f / 3.0f` | `1.3333f` | Classic camera photos, standard preview cards. |
| `21.0f / 9.0f` | `2.3333f` | Ultrawide cinematic banners. |

---

## Code Examples

### 1. Video Player Container (16:9 Widescreen)
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildVideoPreview() {
    return container({
        .width = 320_px,
        .aspect_ratio = 16.0f / 9.0f, // Height automatically computes to 180px
        .color = 0xFF000000,
        .border_radius = BorderRadius::circular(8.0f),
        .align = Alignment::Center,
        .child = text("▶ Video Stream", { .color = 0xFFFFFFFF }),
    });
}
```

### 2. Percentage Width with Aspect Ratio in a Flex Row (From `tests/widgets/test_flexbox.cpp`)
```cpp
#include "enki/widgets/flexbox.hpp"

auto mediaItem = flexItem({
    .width = 50_pct,               // Takes 50% of parent width
    .aspect_ratio = 2.0f,           // width / height = 2.0 -> height = width / 2
    .child = imageThumbnailWidget,
});
```

---

## See Also
- [**Container**](./container.md) — Visual container supporting aspect ratio.
- [**ConstrainedBox**](./constrained_box.md) — Min/max size bounds.
- [**FractionallySizedBox**](./fractionally_sized_box.md) — Percentage dimensions.
