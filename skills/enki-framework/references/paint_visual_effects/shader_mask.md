# ShaderMask

> A visual effect widget that applies a procedural Skia `Shader` (such as linear or radial gradients, perlin noise, or texture sweeps) as an alpha mask over its child widget's rendered output.

- **Header File**: `#include "enki/widgets/paint_effects.hpp"`
- **C++ Class**: `enki::ShaderMaskWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ShaderMask` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::shaderMask(props)`
- **Shader Callback**: `ShaderCallback = std::function<std::shared_ptr<Shader>(Rect bounds)>`
- **Blend Mode Enum**: `enki::BlendMode` (`SrcIn`, `Modulate`, `Multiply`, `Screen`, etc.)

---

## Overview

`ShaderMask` intercepts child rendering and composites a procedural Skia `Shader` over the child using a specified `BlendMode`. When paired with `BlendMode::SrcIn`, the child widget's alpha channel serves as the stencil through which the gradient is painted—making it the definitive technique for **gradient text titles**, glowing icons, and alpha-faded edge masks.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

using ShaderCallback = std::function<std::shared_ptr<Shader>(Rect bounds)>;

struct ShaderMask {
    ShaderCallback shader_callback = nullptr;             ///< Function returning Shader given child layout bounds
    BlendMode      blend_mode      = BlendMode::Modulate; ///< Skia blending mode (SrcIn is ideal for gradient text)
    WidgetPtr      child           = nullptr;
    Key            key             = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ShaderMaskWidget> shaderMask(const ShaderMask& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `shader_callback` | `ShaderCallback` | `nullptr` | Lambda taking `Rect bounds` and returning a compiled `Shader` (e.g. `Gradient::linear`). |
| `blend_mode` | `BlendMode` | `BlendMode::Modulate` | Compositing formula applied between shader and child (`SrcIn`, `Modulate`, etc.). |
| `child` | `WidgetPtr` | `nullptr` | Content to be masked (e.g. a `Text` or `Icon` widget). |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Multi-Color Gradient Text Header
```cpp
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/text.hpp"
#include "enki/rendering/paint.hpp"

using namespace enki;

WidgetPtr buildGradientTitle(const std::string& titleText) {
    return shaderMask({
        .shader_callback = [](Rect bounds) {
            // Generate horizontal linear gradient spanning text bounds
            return Gradient::linear(
                {bounds.x, bounds.y},
                {bounds.x + bounds.width, bounds.y},
                {0xFF38BDF8, 0xFF818CF8, 0xFFEC4899, 0xFFF59E0B} // Sky -> Indigo -> Pink -> Amber
            );
        },
        .blend_mode = BlendMode::SrcIn, // Retains text glyph shape, fills with gradient
        .child = text(titleText, {
            .color = 0xFFFFFFFF, // Color is replaced by gradient via SrcIn
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold
        })
    });
}
```

### 2. Vertical Fade-Out Mask for Scrollable Containers
```cpp
auto fadeOutBottom = shaderMask({
    .shader_callback = [](Rect bounds) {
        return Gradient::linear(
            {bounds.x, bounds.y + bounds.height * 0.8f},
            {bounds.x, bounds.y + bounds.height},
            {0xFFFFFFFF, 0x00FFFFFF} // Opaque white fading to fully transparent
        );
    },
    .blend_mode = BlendMode::DstIn,
    .child = myLongTextParagraph
});
```

---

## See Also
- [**ColorFiltered**](./color_filtered.md) — Matrix-based color manipulation.
- [**DecoratedBox**](./decorated_box.md) — Background/foreground decorations.
- [**BackdropFilter**](./backdrop_filter.md) — Background frosted glass blur.
