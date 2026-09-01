# Spinner

> An advanced rotating loading spinner widget supporting multiple visual design paradigms (Apple-style Spokes, Material Orbiting Dots, Dual Arcs, and SkSL Custom Shaders).

- **Header File**: `#include "enki/widgets/spinner.hpp"`
- **C++ Class**: `enki::SpinnerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Spinner` (converts implicitly to `WidgetPtr`)
- **Style Enum**: `enki::SpinnerStyle` (`Spokes`, `OrbitDots`, `DualArc`, `CustomShader`)

---

## Overview

`Spinner` indicates ongoing asynchronous processing when progress cannot be measured numerically. Rather than forcing a single design aesthetic, Enki supports four distinct visual styles:
1. **`Spokes`**: Apple / macOS style radiating ticks that sequentially fade in opacity.
2. **`OrbitDots`**: Google Material / Windows Fluent style dots orbiting along a circular path.
3. **`DualArc`**: High-tech dual counter-rotating glowing arcs.
4. **`CustomShader`**: Developer-provided SkSL procedural shader code rendered directly on the GPU.

---

## C++ API Definition

### `SpinnerStyle` Enum
```cpp
namespace enki {

enum class SpinnerStyle {
    Spokes,       ///< Apple style radiating ticks that fade in sequence
    OrbitDots,    ///< Material / Fluent style orbiting dots
    DualArc,      ///< Dual counter-rotating glowing arcs
    CustomShader  ///< Direct developer SkSL procedural shader injection
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Spinner {
    SpinnerStyle       style           = SpinnerStyle::Spokes;

    float              size            = 36.0f;          ///< Bounding square diameter
    Color              color           = 0xFF3B82F6;     ///< Primary indicator color
    std::vector<Color> gradient_colors = {};             ///< Multi-color sequence

    // Spokes parameters (for SpinnerStyle::Spokes)
    int                spoke_count     = 12;
    float              spoke_width     = 3.0f;
    float              spoke_length    = 8.0f;

    // OrbitDots parameters (for SpinnerStyle::OrbitDots)
    int                dot_count       = 5;
    float              dot_size        = 6.0f;

    // Animation & Effects
    float              rotation_speed  = 1.0f;           ///< Multiplier for rotation cadence
    Color              glow_color      = 0x00000000;
    float              glow_blur       = 0.0f;

    std::string        custom_shader   = "";             ///< SkSL procedural code
    WidgetPtr          child           = nullptr;        ///< Center icon or logo

    Key                key             = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `style` | `SpinnerStyle` | `Spokes` | Visual animation paradigm (`Spokes`, `OrbitDots`, `DualArc`, `CustomShader`). |
| `size` | `float` | `36.0f` | Width and height in pixels. |
| `color` | `Color` | `0xFF3B82F6` | Primary color applied to spokes, dots, or arcs. |
| `rotation_speed`| `float` | `1.0f` | Cadence multiplier (`1.0` = normal, `2.0` = twice as fast). |
| `spoke_count` | `int` | `12` | Number of radial ticks in `Spokes` mode. |
| `dot_count` | `int` | `5` | Number of orbiting balls in `OrbitDots` mode. |
| `glow_blur` | `float` | `0.0f` | Radius of neon bloom blur around the spinner. |
| `child` | `WidgetPtr` | `nullptr` | Stationary center widget surrounded by the spinner. |

---

## Code Examples (From `widgets_demo/spinner_demo/main.cpp`)

### 1. Apple-Style Spokes Spinner
```cpp
#include "enki/widgets/spinner.hpp"

using namespace enki;

auto macSpinner = Spinner {
    .style = SpinnerStyle::Spokes,
    .size = 32.0f,
    .color = 0xFF94A3B8,
    .spoke_count = 12,
    .spoke_width = 2.5f
};
```

### 2. Material-Style Orbiting Dots
```cpp
auto materialSpinner = Spinner {
    .style = SpinnerStyle::OrbitDots,
    .size = 40.0f,
    .color = 0xFF10B981, // Emerald green
    .dot_count = 6,
    .dot_size = 6.0f,
    .rotation_speed = 1.2f
};
```

### 3. Dual Counter-Rotating Neon Arcs
```cpp
auto neonDualArc = Spinner {
    .style = SpinnerStyle::DualArc,
    .size = 54.0f,
    .color = 0xFF38BDF8,
    .glow_color = 0x8038BDF8,
    .glow_blur = 14.0f,
    .rotation_speed = 1.5f
};
```

---

## See Also
- [**ProgressRing**](./progress_ring.md) — Circular determinate progress ring.
- [**ProgressBar**](./progress_bar.md) — Linear progress bar.
- [**LoadingOverlay**](./loading_overlay.md) — Scoped loading screen using spinners.
