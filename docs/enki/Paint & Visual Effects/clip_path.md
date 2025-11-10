# ClipPath

> A visual clipping widget that clips its child to an arbitrary geometric Skia vector `Path` defined by a static path or a dynamic `CustomClipper` callback.

- **Header File**: `#include "enki/widgets/clip.hpp"`
- **C++ Class**: `enki::ClipPathWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::ClipPath` (converts implicitly to `WidgetPtr`)
- **Factory Helper**: `enki::clipPath(props)`
- **Clipper Signature**: `CustomClipper = std::function<Path(Size)>`

---

## Overview

`ClipPath` allows developers to clip child widgets into any custom two-dimensional geometric shape (such as triangles, stars, diamonds, diagonal slant banners, and speech bubbles). By supplying a `CustomClipper` function, the clipping path is evaluated dynamically according to the widget's layout `Size`.

---

## C++ API Definition

### `CustomClipper` Alias & Declarative Struct
```cpp
namespace enki {

/// Custom clipper callback mapping a layout size to a geometric path
using CustomClipper = std::function<Path(Size)>;

struct ClipPath {
    CustomClipper         clipper       = nullptr;         ///< Dynamic size-aware path generator
    std::shared_ptr<Path> path          = nullptr;         ///< Optional pre-baked static path
    Clip                  clip_behavior = Clip::AntiAlias; ///< Anti-aliased rasterization
    WidgetPtr             child         = nullptr;
    Key                   key           = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<ClipPathWidget> clipPath(const ClipPath& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `clipper` | `CustomClipper` | `nullptr` | Lambda returning a Skia `Path` given the widget's render `Size`. |
| `path` | `shared_ptr<Path>` | `nullptr` | Static path instance used if `clipper` is omitted. |
| `child` | `WidgetPtr` | `nullptr` | Child widget clipped to the path contour. |
| `clip_behavior` | `Clip` | `Clip::AntiAlias` | Hardware clipping anti-aliasing quality. |

---

## Code Examples (From `widgets_demo/paint_effects_demo/main.cpp`)

### 1. Diamond-Shaped Clipping Path
```cpp
#include "enki/widgets/clip.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildDiamondBadge() {
    return clipPath({
        .clipper = [](Size sz) {
            Path p;
            p.moveTo(sz.width * 0.5f, 0.0f);            // Top center
            p.lineTo(sz.width, sz.height * 0.5f);        // Right center
            p.lineTo(sz.width * 0.5f, sz.height);        // Bottom center
            p.lineTo(0.0f, sz.height * 0.5f);            // Left center
            p.close();
            return p;
        },
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .color = 0xFFF59E0B, // Amber
            .align = Alignment::Center,
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(80.0f),
            .child = text("Diamond", {
                .color = 0xFFFFFFFF,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

### 2. Triangular Badge Clipper
```cpp
auto triangleClip = clipPath({
    .clipper = [](Size sz) {
        Path p;
        p.moveTo(sz.width * 0.5f, 0.0f); // Top apex
        p.lineTo(sz.width, sz.height);    // Bottom right
        p.lineTo(0.0f, sz.height);        // Bottom left
        p.close();
        return p;
    },
    .child = myBadgeImageWidget
});
```

---

## See Also
- [**ClipRect**](./clip_rect.md) — Rectangular clipping.
- [**ClipRRect**](./clip_rrect.md) — Rounded rectangular clipping.
- [**ClipOval**](./clip_oval.md) — Circular / elliptical clipping.
