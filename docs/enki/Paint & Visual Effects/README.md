# Enki Paint & Visual Effects Suite

> Hardware-accelerated Skia rendering primitives, anti-aliased geometry clipping, frosted glassmorphism blur filters, procedural shader masks, and color matrix transformations.

The **Paint & Visual Effects** subsystem leverages Enki's direct Skia GPU rasterizer to perform high-performance visual clipping, real-time background blurring, gradient shader masking, and color filtering with zero latency overhead. These primitives allow developers to create modern interfaces—such as glassmorphic sidebars, glowing gradient text headers, circular avatars, and custom vector shapes.

---

## Architectural Highlights

- **Anti-Aliased Geometry Clipping**: `ClipRect`, `ClipRRect`, `ClipOval`, and `ClipPath` push hardware clip bounds onto the Skia canvas stack (`Canvas::clipRect`, `Canvas::clipRRect`, `Canvas::clipPath`) using `Clip::AntiAlias` or `Clip::HardEdge`.
- **Frosted Glass BackdropFilter**: `BackdropFilter` applies GPU-driven convolution filters (like Gaussian `ImageFilter::blur(16.0f, 16.0f)`) to the visual content rendered *behind* the widget before painting its own foreground.
- **Dynamic Shader Masks**: `ShaderMask` intercepts child rendering passes and multiplies the child's alpha channel with procedural Skia shaders (such as `Gradient::linear` or `Gradient::radial`).
- **Hardware Color Transforms**: `ColorFiltered` evaluates 4x5 color matrices on entire subtrees, enabling instant grayscale, sepia, inversion, and tinting effects.

---

## Widget Catalog (Paint & Visual Effects)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**ClipRect**](./clip_rect.md) | `struct ClipRect`, `clipRect()` | `<enki/widgets/clip.hpp>` | Clips its child to an axis-aligned rectangular boundary. |
| 2 | [**ClipRRect**](./clip_rrect.md) | `struct ClipRRect`, `clipRRect()` | `<enki/widgets/clip.hpp>` | Clips its child to a smooth rounded rectangular boundary (`BorderRadius`). |
| 3 | [**ClipOval**](./clip_oval.md) | `struct ClipOval`, `clipOval()` | `<enki/widgets/clip.hpp>` | Clips its child to an inscribed ellipse or perfect circle. |
| 4 | [**ClipPath**](./clip_path.md) | `struct ClipPath`, `clipPath()` | `<enki/widgets/clip.hpp>` | Clips its child to an arbitrary vector `Path` using a `CustomClipper` callback. |
| 5 | [**BackdropFilter**](./backdrop_filter.md) | `struct BackdropFilter`, `backdropFilter()` | `<enki/widgets/paint_effects.hpp>` | Applies an `ImageFilter` (Gaussian blur) to content rendered behind the widget. |
| 6 | [**DecoratedBox**](./decorated_box.md) | `struct DecoratedBox`, `decoratedBox()` | `<enki/widgets/paint_effects.hpp>` | Paints a `BoxDecoration` (gradient, borders, shadows, images) behind or in front of a child. |
| 7 | [**ShaderMask**](./shader_mask.md) | `struct ShaderMask`, `shaderMask()` | `<enki/widgets/paint_effects.hpp>` | Applies a procedural gradient or image shader as a mask over child rendering. |
| 8 | [**ColorFiltered**](./color_filtered.md) | `struct ColorFiltered`, `colorFiltered()` | `<enki/widgets/paint_effects.hpp>` | Applies color matrix filters (grayscale, sepia, invert, tint) to its child. |

---

## Quick Example (Glassmorphism Card with Gradient Mask)

```cpp
#include "enki/widgets/clip.hpp"
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildFrostedGlassCard() {
    // 1. Vibrant Gradient Title masked over Text
    auto titleText = shaderMask({
        .shader_callback = [](Rect bounds) {
            return Gradient::linear(
                {bounds.x, bounds.y},
                {bounds.x + bounds.width, bounds.y},
                {0xFF38BDF8, 0xFF818CF8, 0xFFEC4899}
            );
        },
        .blend_mode = BlendMode::SrcIn,
        .child = text("Frosted Glass Card", { .font_size = 18.0f, .font_weight = FontWeight::Bold })
    });

    // 2. Rounded Container with Frosted Glass BackdropFilter
    return clipRRect({
        .border_radius = BorderRadius::circular(16.0f),
        .child = backdropFilter({
            .filter = ImageFilter::blur(16.0f, 16.0f),
            .child = container({
                .color = 0x251E293B, // Translucent slate
                .border = Border(0x6638BDF8, 1.5f), // Glowing border
                .border_radius = BorderRadius::circular(16.0f),
                .padding = EdgeInsets::all(20.0f),
                .child = column({
                    .gap = 8_px,
                    .children = {
                        titleText,
                        text("Real-time GPU blur with anti-aliased rounded clip.", {
                            .color = 0xFFE2E8F0,
                            .font_size = 13.0f
                        })
                    }
                })
            })
        })
    });
}
```
