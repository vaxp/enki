# RangeSlider

> An interactive dual-thumb track slider widget for selecting minimum and maximum interval bounds without crossing.

- **Header File**: `#include "enki/widgets/range_slider.hpp"`
- **C++ Class**: `enki::RangeSliderWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::RangeSlider` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::RangeSliderProps`
- **Callback**: `using RangeSliderCallback = std::function<void(float start, float end)>`

---

## Overview

`RangeSlider` allows users to select a numeric range `[start, end]` (such as a price filter `[$20 - $80]` or working hours `[9h - 17h]`). It renders two draggable circular thumbs on a single horizontal track, ensuring the start thumb never crosses past the end thumb.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

using RangeSliderCallback = std::function<void(float start, float end)>;

struct RangeSlider {
    Key                 key              = Key::none();
    float               start_value      = 0.0f;
    float               end_value        = 1.0f;
    RangeSliderCallback on_change        = nullptr;

    Color               active_color     = 0xFF3B82F6; // Blue 500
    Color               inactive_color   = 0xFFCBD5E1; // Slate 300
    Color               thumb_color      = 0xFFFFFFFF; // White
    float               track_height     = 4.0f;
    float               thumb_radius     = 10.0f;
    float               min_value        = 0.0f;
    float               max_value        = 1.0f;
    
    // Shadow properties for thumb elevation
    Color               shadow_color     = 0x40000000;
    float               shadow_blur      = 4.0f;
    float               shadow_offset_dy = 2.0f;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `start_value` | `float` | `0.0f` | Current lower bound value of the selected interval. |
| `end_value` | `float` | `1.0f` | Current upper bound value of the selected interval. |
| `on_change` | `RangeSliderCallback` | `nullptr` | Callback receiving `(start, end)` when either thumb is dragged. |
| `min_value` | `float` | `0.0f` | Minimum scale boundary. |
| `max_value` | `float` | `1.0f` | Maximum scale boundary. |
| `active_color` | `Color` | `0xFF3B82F6` | Color of the highlighted track segment between the two thumbs. |
| `inactive_color`| `Color` | `0xFFCBD5E1` | Color of the track outside the selected interval. |
| `thumb_radius` | `float` | `10.0f` | Size of both circular thumb handles. |

---

## Code Examples (From `widgets_demo/rangeslider_demo/main.cpp`)

### 1. Price Range Filter with Dynamic Feedback
```cpp
#include "enki/widgets/range_slider.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildPriceFilter(float minPrice, float maxPrice,
                           std::function<void(float, float)> onPriceChanged) {
    return row({
        .align_items = Align::Center,
        .gap = 16_px,
        .children = {
            text("Price Range:"),
            expanded(RangeSlider {
                .start_value = minPrice,
                .end_value = maxPrice,
                .min_value = 0.0f,
                .max_value = 500.0f,
                .on_change = std::move(onPriceChanged),
            })
        }
    });
}
```

---

## See Also
- [**Slider**](./slider.md) — Single-value track slider.
- [**NumberField**](./number_field.md) — Exact numeric input.
