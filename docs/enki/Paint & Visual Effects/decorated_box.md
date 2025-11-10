# DecoratedBox

> A visual decoration widget that paints a `BoxDecoration` (linear/radial gradients, solid fills, borders, drop shadows, or background shapes) either behind or in front of a child widget.

- **Header File**: `#include "enki/widgets/paint_effects.hpp"`
- **C++ Class**: `enki::DecoratedBoxWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::DecoratedBox` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::decoratedBox(props)`
- **Enum**: `enki::DecorationPosition` (`Background`, `Foreground`)
- **Decoration Class**: `enki::BoxDecoration`

---

## Overview

`DecoratedBox` applies graphical styling directly to the layout bounds of its child. Unlike `Container` (which also manages sizing, padding, and alignment), `DecoratedBox` is a lightweight, dedicated render widget focused solely on painting. The `position` parameter allows applying decorations either beneath the child (`DecorationPosition::Background`) or overlaying on top of it (`DecorationPosition::Foreground`).

---

## C++ API Definition

### `DecorationPosition` Enum & Declarative Struct
```cpp
namespace enki {

enum class DecorationPosition {
    Background, ///< Paints decoration behind child
    Foreground  ///< Paints decoration on top of child (useful for overlays or glass tints)
};

struct DecoratedBox {
    BoxDecoration      decoration = {};                          ///< Color, gradient, borders, shadows
    DecorationPosition position   = DecorationPosition::Background;
    WidgetPtr          child      = nullptr;
    Key                key        = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<DecoratedBoxWidget> decoratedBox(const DecoratedBox& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `decoration` | `BoxDecoration` | `{}` | Visual style descriptor (color, border, radius, shadow, gradient). |
| `position` | `DecorationPosition` | `Background` | Paint order relative to child (`Background` or `Foreground`). |
| `child` | `WidgetPtr` | `nullptr` | Content to be wrapped by the decoration. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Elevated Gradient Card with Outer Drop Shadow
```cpp
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildElevatedGradientBox() {
    BoxDecoration boxDec;
    boxDec.color = 0xFF1E293B;
    boxDec.border_radius = BorderRadius::circular(12.0f);
    boxDec.border = Border(0xFF38BDF8, 1.5f);

    return decoratedBox({
        .decoration = boxDec,
        .position = DecorationPosition::Background,
        .child = container({
            .padding = EdgeInsets::all(20.0f),
            .child = text("Decorated Box Content", {
                .color = 0xFFFFFFFF,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

---

## See Also
- [**Container**](../Layout/container.md) — Comprehensive convenience container combining layout with decoration.
- [**BackdropFilter**](./backdrop_filter.md) — Frosted glass blur filter.
- [**ShaderMask**](./shader_mask.md) — Gradient and texture masking over child pixels.
