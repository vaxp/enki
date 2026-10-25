# ProgressRing

> A circular arc progress indicator widget supporting determinate and indeterminate modes, sweep gradients, round caps, neon glow effects, and centered custom child widgets.

- **Header File**: `#include "enki/widgets/progress_ring.hpp"`
- **C++ Class**: `enki::ProgressRingWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ProgressRing` (converts implicitly to `WidgetPtr`)

---

## Overview

`ProgressRing` displays progress along a circular track. Rendered via Skia's `Canvas::drawArc`, it supports configurable stroke widths, rounded line caps (`round_cap = true`), sweeping angular gradients, and an optional **inner center widget** slot (`child`) commonly used to render numeric percentages, icons, or status badges centered within the ring.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ProgressRing {
    float              value            = 0.0f;           ///< Progress fraction (0.0f to 1.0f)
    WidgetPtr          child            = nullptr;        ///< Center child widget (text, icon, etc.)

    float              size             = 48.0f;          ///< Diameter in pixels
    float              stroke_width     = 6.0f;           ///< Ring stroke thickness
    float              start_angle      = -90.0f;         ///< Start degree (-90 = 12 o'clock)
    bool               round_cap        = true;           ///< Rounded arc endpoints

    Color              background_color = 0xFF1E293B;     ///< Track background ring
    Color              progress_color   = 0xFF3B82F6;     ///< Arc fill color
    std::vector<Color> gradient_colors  = {};             ///< Angular sweep gradient sequence

    Color              glow_color       = 0x00000000;
    float              glow_blur        = 0.0f;

    bool               indeterminate    = false;          ///< Continuous rotating spinner mode
    std::string        custom_shader    = "";             ///< Optional SkSL runtime shader

    Key                key              = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `float` | `0.0f` | Fraction completed from `0.0f` (0°) to `1.0f` (360°). |
| `child` | `WidgetPtr` | `nullptr` | Content centered inside the ring (e.g. percentage label or checkmark). |
| `size` | `float` | `48.0f` | Total diameter of the ring bounding box. |
| `stroke_width` | `float` | `6.0f` | Arc stroke thickness in pixels. |
| `start_angle` | `float` | `-90.0f` | Starting angle in degrees (`-90.0f` begins at top / 12 o'clock). |
| `round_cap` | `bool` | `true` | Rounds arc stroke ends (`PaintCap::Round`). |
| `gradient_colors`| `vector<Color>`| `{}` | Multi-color angular sweep gradient applied along the arc. |
| `indeterminate` | `bool` | `false` | When true, animates continuous rotation for busy states. |

---

## Code Examples (From `widgets_demo/progress_demo/main.cpp`)

### 1. Determinate Ring with Center Percentage Label
```cpp
#include "enki/widgets/progress_ring.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildStorageUsageRing(float usedFraction) {
    int percentage = static_cast<int>(usedFraction * 100.0f);

    return ProgressRing {
        .value = usedFraction,
        .size = 72.0f,
        .stroke_width = 7.0f,
        .progress_color = 0xFF38BDF8, // Sky blue
        .child = text(std::to_string(percentage) + "%", {
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
            .color = 0xFFFFFFFF
        })
    };
}
```

### 2. Gradient Ring with Neon Bloom
```cpp
auto neonRing = ProgressRing {
    .value = 0.85f,
    .size = 80.0f,
    .stroke_width = 8.0f,
    .gradient_colors = {0xFF10B981, 0xFF38BDF8, 0xFF818CF8}, // Emerald -> Cyan -> Purple
    .glow_color = 0x6610B981,
    .glow_blur = 12.0f,
    .round_cap = true
};
```

### 3. Indeterminate Rotating Ring
```cpp
auto loadingRing = ProgressRing {
    .size = 36.0f,
    .stroke_width = 4.0f,
    .indeterminate = true,
    .progress_color = 0xFFF59E0B // Amber
};
```

---

## See Also
- [**ProgressBar**](./progress_bar.md) — Linear horizontal progress bar.
- [**Spinner**](./spinner.md) — Rotating spoke, dot, and arc spinners.
- [**LoadingOverlay**](./loading_overlay.md) — Full-screen loading overlay with progress rings.
