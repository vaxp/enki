# ColorFiltered

> A visual effect widget that applies a Skia `ColorFilter` (such as hardware matrix transforms for grayscale, sepia, color inversion, or color tinting) to its entire child widget subtree.

- **Header File**: `#include "enki/widgets/paint_effects.hpp"`
- **C++ Class**: `enki::ColorFilteredWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ColorFiltered` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::colorFiltered(props)`
- **Filter Class**: `enki::ColorFilter` (`grayscale()`, `sepia()`, `invert()`, `tint()`)

---

## Overview

`ColorFiltered` modifies pixel color values across its entire child subtree before rasterizing to the screen. It is evaluated directly inside the GPU pipeline using Skia's color matrix transforms, allowing developers to dynamically disable sections of the UI (by applying `ColorFilter::grayscale()`), implement night/dark mode inversions, or tint complex widgets without mutating any underlying element data.

---

## C++ API Definition

### `ColorFilter` Factory Methods
```cpp
namespace enki {

class ColorFilter {
public:
    static std::shared_ptr<ColorFilter> grayscale();
    static std::shared_ptr<ColorFilter> sepia();
    static std::shared_ptr<ColorFilter> invert();
    static std::shared_ptr<ColorFilter> tint(Color color);
    static std::shared_ptr<ColorFilter> mode(Color color, BlendMode blend_mode);
    static std::shared_ptr<ColorFilter> matrix(const float matrix_4x5[20]);
};

} // namespace enki
```

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct ColorFiltered {
    std::shared_ptr<ColorFilter> color_filter = nullptr; ///< Hardware color matrix filter
    WidgetPtr                    child        = nullptr;
    Key                          key          = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ColorFilteredWidget> colorFiltered(const ColorFiltered& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `color_filter` | `shared_ptr<ColorFilter>`| `nullptr` | Transformation filter applied to child colors. |
| `child` | `WidgetPtr` | `nullptr` | Content subtree receiving the color transformation. |
| `key` | `Key` | `Key::none()` | Optional widget reconciliation key. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Dynamic Grayscale Filter for Disabled State
```cpp
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildStateAwarePanel(WidgetPtr complexPanel, bool isLocked) {
    return colorFiltered({
        // Turn entire panel into black & white if locked
        .color_filter = isLocked ? ColorFilter::grayscale() : nullptr,
        .child = complexPanel
    });
}
```

### 2. Sepia & Color Inversion Toggles
```cpp
// Vintage Sepia Tone
auto sepiaPhoto = colorFiltered({
    .color_filter = ColorFilter::sepia(),
    .child = myImageWidget
});

// Night Vision / Invert Colors
auto invertedUi = colorFiltered({
    .color_filter = ColorFilter::invert(),
    .child = myDashboardWidget
});
```

---

## See Also
- [**ShaderMask**](./shader_mask.md) — Procedural gradient alpha mask.
- [**BackdropFilter**](./backdrop_filter.md) — Background frosted glass blur.
- [**DecoratedBox**](./decorated_box.md) — Box decoration backgrounds.
