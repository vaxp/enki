# ProgressBar

> A linear progress indicator widget supporting determinate fraction values (0.0 to 1.0), animated indeterminate shimmer sweeps, multi-stop gradient fills, neon glow effects, and custom procedural SkSL shaders.

- **Header File**: `#include "enki/widgets/progress_bar.hpp"`
- **C++ Class**: `enki::ProgressBarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ProgressBar` (converts implicitly to `WidgetPtr`)

---

## Overview

`ProgressBar` provides visual feedback regarding task progression along a linear horizontal bar. In **determinate mode**, it fills the track proportionally according to its `value` property (clamped between `0.0f` and `1.0f`). In **indeterminate mode** (`indeterminate = true`), it continuously sweeps an animated light pulse across the track. Multi-stop color gradients and outer glow blurs can be configured with zero performance penalty via direct Skia drawing.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ProgressBar {
    float              value            = 0.0f;           ///< Progress fraction (0.0f to 1.0f)
    float              height           = 8.0f;           ///< Track height in pixels
    float              border_radius    = 4.0f;           ///< Track corner radius
    float              min_width        = 100.0f;

    Color              background_color = 0xFF1E293B;     ///< Track empty background (Slate 800)
    Color              progress_color   = 0xFF3B82F6;     ///< Filled bar color (Blue 500)
    std::vector<Color> gradient_colors  = {};             ///< Multi-stop gradient (overrides progress_color)

    Color              glow_color       = 0x00000000;     ///< Outer glow bloom tint
    float              glow_blur        = 0.0f;           ///< Blur radius of glow effect

    bool               indeterminate    = false;          ///< When true, animates continuous sweep
    std::string        custom_shader    = "";             ///< Optional SkSL runtime shader

    bool               show_label       = false;          ///< Display numeric percentage text
    std::string        label_format     = "{percent}%";   ///< String interpolation template

    Key                key              = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `float` | `0.0f` | Current fraction completed (`0.0f` = 0%, `1.0f` = 100%). |
| `height` | `float` | `8.0f` | Thickness of the linear bar in pixels. |
| `border_radius` | `float` | `4.0f` | Corner rounding of both track and progress pill. |
| `progress_color` | `Color` | `0xFF3B82F6` | Solid fill color for the completed portion. |
| `gradient_colors` | `vector<Color>` | `{}` | Sequence of colors forming a smooth horizontal linear gradient. |
| `glow_color` | `Color` | `0x00000000` | Color of the outer neon aura bloom. |
| `glow_blur` | `float` | `0.0f` | Blur radius of the glow effect. |
| `indeterminate` | `bool` | `false` | Enables autonomous sweeping animation for unknown duration tasks. |
| `show_label` | `bool` | `false` | Renders a right-aligned percentage readout automatically. |
| `custom_shader` | `string` | `""` | Raw SkSL shader program evaluated per-pixel on the filled area. |

---

## Code Examples (From `widgets_demo/progress_demo/main.cpp`)

### 1. Determinate Progress Bar with Numeric Label
```cpp
#include "enki/widgets/progress_bar.hpp"

using namespace enki;

WidgetPtr buildDownloadBar(float progressFraction) {
    return ProgressBar {
        .value = progressFraction,
        .height = 14.0f,
        .border_radius = 7.0f,
        .progress_color = 0xFF10B981, // Emerald green
        .show_label = true,
        .label_format = "{percent}% Completed"
    };
}
```

### 2. Multi-Color Gradient with Neon Glow
```cpp
auto neonProgressBar = ProgressBar {
    .value = 0.75f,
    .height = 12.0f,
    .border_radius = 6.0f,
    .gradient_colors = {0xFFEC4899, 0xFF8B5CF6, 0xFF38BDF8}, // Pink -> Purple -> Cyan
    .glow_color = 0x80EC4899,
    .glow_blur = 12.0f
};
```

### 3. Indeterminate Continuous Sweep
```cpp
auto busyProgressBar = ProgressBar {
    .height = 6.0f,
    .indeterminate = true,
    .progress_color = 0xFF38BDF8,
    .background_color = 0xFF0F172A
};
```

---

## See Also
- [**ProgressRing**](./progress_ring.md) — Circular arc progress ring.
- [**Spinner**](./spinner.md) — Rotating spoke, dot, and arc spinners.
- [**LoadingOverlay**](./loading_overlay.md) — Modal busy screen with integrated progress bars.
