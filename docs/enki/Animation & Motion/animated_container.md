# AnimatedContainer

> An implicit animation widget that automatically interpolates changes to its size, background colors, borders, corner radii, box shadows, and insets over time.

- **Header File**: `#include "enki/widgets/motion.hpp"`
- **C++ Class**: `enki::AnimatedContainerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::AnimatedContainer` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::animatedContainer(props)`

---

## Overview

`AnimatedContainer` is the most versatile implicit animation widget in Enki. It behaves exactly like a standard `Container`, but whenever any of its layout or visual properties change between rebuilds, it computes multi-property tweens (interpolating colors via linear sRGB mixing, dimensions in pixel or percent units, border thicknesses, and corner radii) simultaneously over a specified `duration`.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct AnimatedContainer {
    // Visual Decoration
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;

    // Child Alignment
    std::optional<Alignment>      align;

    // Dimensions & Constraints
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // Insets
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;

    // Animation Properties
    std::chrono::milliseconds    duration = std::chrono::milliseconds(300);
    const Curve*                  curve    = &Curves::linear;
    std::function<void()>         on_end   = nullptr;
    WidgetPtr                     child    = nullptr;
    Key                           key      = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedContainerWidget> animatedContainer(AnimatedContainer props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `color` | `optional<Color>` | `nullopt` | Background fill color (interpolated smoothly). |
| `border_radius` | `optional<BorderRadius>`| `nullopt` | Corner rounding radius. |
| `border` | `optional<Border>` | `nullopt` | Border stroke color and width. |
| `width`, `height`| `optional<StyleValue>` | `nullopt` | Container width/height (`StyleValue::point` or `percent`). |
| `padding`, `margin`| `optional<StyleInsets>`| `nullopt` | Internal and external spacing insets. |
| `duration` | `milliseconds` | `300ms` | Total animation time for all changing properties. |
| `curve` | `const Curve*` | `&Curves::linear`| Easing curve (e.g. `&Curves::fastOutSlowIn`). |
| `on_end` | `function<void()>` | `nullptr` | Callback fired when the container finishes animating. |

---

## Code Examples (From `widgets_demo/motion_demo/main.cpp`)

### 1. Dynamic Morphing Button Card
```cpp
#include "enki/widgets/motion.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildMorphingCard(bool isExpanded, std::function<void()> onToggle) {
    return animatedContainer({
        .color = isExpanded ? 0xFF059669 : 0xFF1D4ED8, // Green <-> Blue
        .border_radius = isExpanded ? BorderRadius::circular(24.0f) : BorderRadius::circular(6.0f),
        .border = isExpanded ? Border(0xFF34D399, 2.0f) : Border(0xFF60A5FA, 1.0f),
        .width = isExpanded ? StyleValue::percent(100.0f) : StyleValue::point(260.0f),
        .height = isExpanded ? StyleValue::point(90.0f) : StyleValue::point(55.0f),
        .padding = isExpanded ? StyleInsets::all(18.0f) : StyleInsets::all(10.0f),
        .duration = std::chrono::milliseconds(500),
        .curve = &Curves::fastOutSlowIn,
        .child = button(text(isExpanded ? "Morphed State!" : "Initial State"), onToggle)
    });
}
```

---

## See Also
- [**Container**](../Layout/container.md) — Static container layout widget.
- [**AnimatedOpacity**](./animated_opacity.md) — Fast alpha fade transitions.
- [**AnimatedScale**](./animated_scale.md) — Scale transformations.
