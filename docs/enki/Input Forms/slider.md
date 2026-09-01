# Slider

> An interactive continuous track slider widget for selecting numeric values within a defined range.

- **Header File**: `#include "enki/widgets/slider.hpp"`
- **C++ Class**: `enki::SliderWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::Slider` (converts implicitly to `WidgetPtr`)
- **Render Object**: `enki::RenderSlider`
- **Underlying Mechanism**: Skia anti-aliased track and elevated thumb with drop shadow

---

## Overview

`Slider` allows users to pick a continuous value (such as audio volume, screen brightness, or playback progress) by dragging a circular thumb along a horizontal track. It divides the track into an `active_color` segment (progress) and an `inactive_color` segment.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

using SliderCallback = std::function<void(float)>;

struct Slider {
    float          value            = 0.0f;
    SliderCallback on_change        = nullptr;

    Color          active_color     = 0xFF3B82F6; // Blue 500
    Color          inactive_color   = 0xFFCBD5E1; // Slate 300
    Color          thumb_color      = 0xFFFFFFFF; // White
    float          track_height     = 4.0f;
    float          thumb_radius     = 10.0f;
    float          min_value        = 0.0f;
    float          max_value        = 1.0f;
    
    // Shadow properties for thumb elevation
    Color          shadow_color     = 0x40000000;
    float          shadow_blur      = 4.0f;
    float          shadow_offset_dy = 2.0f;
    
    Key            key              = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `float` | `0.0f` | Current numeric position on the slider. |
| `on_change` | `SliderCallback` | `nullptr` | Callback invoked with the new value continuously during dragging. |
| `min_value` | `float` | `0.0f` | Minimum bound of the range. |
| `max_value` | `float` | `1.0f` | Maximum bound of the range. |
| `track_height` | `float` | `4.0f` | Thickness of the horizontal line track. |
| `thumb_radius` | `float` | `10.0f` | Radius of the circular draggable thumb handle. |
| `active_color` | `Color` | `0xFF3B82F6` | Color of the track to the left of the thumb. |
| `inactive_color`| `Color` | `0xFFCBD5E1` | Color of the track to the right of the thumb. |
| `thumb_color` | `Color` | `0xFFFFFFFF` | Fill color of the circular thumb handle. |
| `shadow_blur` | `float` | `4.0f` | Blur radius of the drop shadow under the thumb. |

---

## Code Examples (From `widgets_demo/slider_demo/main.cpp`)

### 1. Volume Slider with Value Display
```cpp
#include "enki/widgets/slider.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildVolumeControl(float currentVol, std::function<void(float)> onVolChanged) {
    return row({
        .align_items = Align::Center,
        .gap = 16_px,
        .children = {
            text("Volume:"),
            expanded(Slider {
                .value = currentVol,
                .min_value = 0.0f,
                .max_value = 100.0f,
                .on_change = std::move(onVolChanged),
            }),
        }
    });
}
```

### 2. Custom Colored Accent Slider
```cpp
auto amberSlider = Slider {
    .value = 0.75f,
    .active_color = 0xFFF59E0B, // Amber 500
    .inactive_color = 0xFF334155, // Slate 700
    .thumb_radius = 12.0f,
    .on_change = [](float val) { /* ... */ }
};
```

---

## See Also
- [**RangeSlider**](./range_slider.md) — Dual-thumb slider for range intervals.
- [**NumberField**](./number_field.md) — Numeric input with precision and steppers.
