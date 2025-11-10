# BackdropFilter

> A post-processing visual effect widget that applies an `ImageFilter` (such as real-time GPU Gaussian blur) to the existing pixels rendered *behind* it before painting its own child.

- **Header File**: `#include "enki/widgets/paint_effects.hpp"`
- **C++ Class**: `enki::BackdropFilterWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::BackdropFilter` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::backdropFilter(props)`
- **Filter Helper**: `enki::ImageFilter::blur(sigmaX, sigmaY)`

---

## Overview

`BackdropFilter` enables modern **Glassmorphism** and frosted glass aesthetics. Rather than applying a blur to its own child widget, it intercepts the Skia canvas buffer beneath its bounds and executes an image filter pass (typically Gaussian blur) over the background pixels. When paired with a semi-transparent child background (e.g. `0x25FFFFFF` or `0x301E293B`) and bounded by a `ClipRRect`, it produces rich frosted glass surfaces.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct BackdropFilter {
    std::shared_ptr<ImageFilter> filter     = nullptr;           ///< e.g. ImageFilter::blur(16.0f, 16.0f)
    BlendMode                    blend_mode = BlendMode::SrcOver; ///< Skia compositing blend mode
    WidgetPtr                    child      = nullptr;           ///< Foreground content rendered over blurred backdrop
    Key                          key        = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<BackdropFilterWidget> backdropFilter(const BackdropFilter& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `filter` | `shared_ptr<ImageFilter>` | `nullptr` | Skia convolution filter (e.g. `ImageFilter::blur(sigmaX, sigmaY)`). |
| `blend_mode` | `BlendMode` | `BlendMode::SrcOver`| Blending mode applied when combining filtered backdrop with child. |
| `child` | `WidgetPtr` | `nullptr` | Foreground widget rendered directly above the filtered backdrop. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Frosted Glass Panel
```cpp
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/clip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildGlassmorphicPanel() {
    return clipRRect({
        .border_radius = BorderRadius::circular(16.0f),
        .child = backdropFilter({
            // 16px horizontal and vertical GPU Gaussian blur
            .filter = ImageFilter::blur(16.0f, 16.0f),
            .child = container({
                .color = 0x251E293B, // 15% opacity slate background
                .border_radius = BorderRadius::circular(16.0f),
                .border = Border(0x6638BDF8, 1.5f), // Cyan translucent highlight border
                .padding = EdgeInsets::all(20.0f),
                .child = text("Frosted Glassmorphism", {
                    .color = 0xFFFFFFFF,
                    .font_size = 15.0f,
                    .font_weight = FontWeight::Bold
                })
            })
        })
    });
}
```

---

## See Also
- [**ClipRRect**](./clip_rrect.md) — Constrains backdrop filter boundaries to rounded cards.
- [**DecoratedBox**](./decorated_box.md) — Custom box borders, shadows, and gradients.
- [**ColorFiltered**](./color_filtered.md) — Color matrix adjustments.
