# SkiaCanvas

> A high-performance, declarative 2D drawing canvas widget for ENKI, exposing direct Skia (`SkCanvas*`) and high-level Enki `Canvas` drawing callbacks with automatic Anu Flexbox layout, repaint controllers, and layered composite rendering.

- **Header File**: `#include "enki/widgets/skia_canvas.hpp"`
- **C++ Class**: `enki::SkiaCanvasWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::SkiaCanvasProps`, `enki::SkiaCanvas` (converts implicitly to `WidgetPtr`)
- **Style Model**: `enki::SkiaCanvasStyle`
- **Render Object**: `enki::RenderSkiaCanvas` (inherits from `enki::RenderBox`)
- **Factory Helper**: `enki::skiaCanvas()`

---

## Overview

`SkiaCanvas` is the atomic 2D custom graphics widget in ENKI (equivalent to Flutter's `CustomPaint` / HTML5 Canvas). It bridges declarative UI construction with raw, GPU-accelerated immediate-mode rendering powered by Google Skia.

### Architectural Highlights

```
┌─────────────────────────────────────────────────────────────┐
│                       SkiaCanvasWidget                      │
│            (SkiaCanvasProps / SkiaCanvasStyle)              │
└──────────────────────────────┬──────────────────────────────┘
                               │
        ┌──────────────────────┴──────────────────────┐
        ▼                                             ▼
┌───────────────────────────────┐     ┌───────────────────────────────┐
│      Anu Layout Engine        │     │      Repaint Controller       │
│ - Responsive width & height   │     │ - AnimationController sync    │
│ - Min/max constraints         │     │ - Skips tree rebuild on tick  │
│ - Percentage or pixel points  │     │ - Direct markNeedsPaint()     │
└──────────────┬────────────────┘     └───────────────┬───────────────┘
               │                                      │
               └──────────────────┬───────────────────┘
                                  ▼
┌─────────────────────────────────────────────────────────────┐
│                 Layered Painting Pipeline                   │
│                                                             │
│   1. [painter / skia_painter]           (Background layer)  │
│   2. [child]                            (Subtree widgets)   │
│   3. [foreground_painter / skia_fg]     (HUD / Overlay)     │
└─────────────────────────────────────────────────────────────┘
```

- **Dual Painter API**:
  - **High-Level (`painter`)**: Uses ENKI's `Canvas&` and `Size` abstraction for clean, portable drawing code.
  - **Direct Skia (`skia_painter`)**: Direct `SkCanvas*` pointer access for advanced Skia features (custom shaders, SkPath operations, blend modes, matrix transformations, filters).
- **Three-Tier Layer Composition**: Supports background painting behind a child widget (`painter`), normal child painting, and foreground overlay painting (`foreground_painter`) on top of children (ideal for HUD reticles, glass glares, and custom borders).
- **Repaint Controller Integration**: Pass a `std::shared_ptr<AnimationController>` via `.repaint` to trigger 60/120 FPS repaints without triggering expensive widget tree rebuilds.
- **Local Origin Coordinate System**: The canvas is automatically pre-translated to `(0, 0)` at the top-left of the widget's bounds, meaning painters never have to manually offset coordinates.

---

## C++ Declarative Definition

```cpp
namespace enki {

struct SkiaCanvasProps {
    CanvasPainterCallback                painter                 = nullptr; ///< Background Canvas& painter
    SkiaDirectPainterCallback            skia_painter            = nullptr; ///< Background raw SkCanvas* painter
    CanvasPainterCallback                foreground_painter      = nullptr; ///< Foreground Canvas& painter
    SkiaDirectPainterCallback            skia_foreground_painter = nullptr; ///< Foreground raw SkCanvas* painter
    WidgetPtr                            child                   = nullptr; ///< Optional child widget
    CanvasHitTestCallback                hit_test                = nullptr; ///< Custom shape hit-test logic
    std::shared_ptr<AnimationController> repaint                 = nullptr; ///< Repaint notifier controller

    std::optional<StyleValue>            width                   = std::nullopt;
    std::optional<StyleValue>            height                  = std::nullopt;
    std::optional<StyleValue>            min_width               = std::nullopt;
    std::optional<StyleValue>            min_height              = std::nullopt;
    std::optional<StyleValue>            max_width               = std::nullopt;
    std::optional<StyleValue>            max_height              = std::nullopt;

    Clip                                 clip_behavior           = Clip::None;
    BorderRadius                         clip_radius             = BorderRadius::zero();
    bool                                 is_complex              = false;
    Key                                  key                     = Key::none();

    operator WidgetPtr() const;
};

inline WidgetPtr skiaCanvas(const SkiaCanvasProps& props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `painter` | `CanvasPainterCallback` | `nullptr` | Background painter using ENKI's `Canvas&` interface (`void(Canvas&, Size)`). |
| `skia_painter` | `SkiaDirectPainterCallback` | `nullptr` | Direct Skia painter using raw `SkCanvas*` (`void(SkCanvas*, Size)`). |
| `foreground_painter` | `CanvasPainterCallback` | `nullptr` | Overlay painter rendered on top of child widgets. |
| `skia_foreground_painter` | `SkiaDirectPainterCallback` | `nullptr` | Direct Skia overlay painter rendered on top of child widgets. |
| `child` | `WidgetPtr` | `nullptr` | Optional child widget positioned between background and foreground painters. |
| `repaint` | `shared_ptr<AnimationController>` | `nullptr` | Animation controller that drives repaints without rebuilding the element tree. |
| `width`, `height` | `optional<StyleValue>` | `nullopt` | Explicit dimension constraints (points, percent, or auto). |
| `clip_behavior` | `Clip` | `Clip::None` | Boundary clipping (`Clip::None`, `Clip::HardEdge`, `Clip::AntiAlias`). |
| `clip_radius` | `BorderRadius` | `zero()` | Corner radius applied when clipping is enabled. |
| `hit_test` | `CanvasHitTestCallback` | `nullptr` | Custom shape hit testing function `bool(Point localPoint, Size size)`. |

---

## Usage Examples

### 1. Simple Custom Shape (High-Level Canvas)

```cpp
#include "enki/widgets/skia_canvas.hpp"

using namespace enki;

WidgetPtr buildGlowCircle() {
    return skiaCanvas({
        .painter = [](Canvas& canvas, Size size) {
            Paint paint;
            paint.setColor(0xFF38BDF8);
            paint.setStyle(PaintStyle::Fill);
            paint.setAntiAlias(true);

            Point center = {size.width * 0.5f, size.height * 0.5f};
            canvas.drawCircle(center, 40.0f, paint);
        },
        .width = 120.0f,
        .height = 120.0f,
    });
}
```

### 2. Animated Radar Scanner with Raw Skia API (`SkCanvas*`)

```cpp
auto radar = skiaCanvas({
    .skia_painter = [this](SkCanvas* sk, Size size) {
        float cx = size.width * 0.5f;
        float cy = size.height * 0.5f;
        float angle = radar_controller->value() * 2.0f * 3.14159f;

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0xFF0284C7);
        p.setStrokeWidth(2.0f);
        p.setStyle(SkPaint::kStroke_Style);
        sk->drawCircle(cx, cy, 60.0f, p);

        // Sweep line
        p.setColor(0xFF38BDF8);
        sk->drawLine(cx, cy, cx + 60.0f * std::cos(angle), cy + 60.0f * std::sin(angle), p);
    },
    .repaint = radar_controller,
    .width = 160.0f,
    .height = 160.0f,
});
```

### 3. Layered Card with Foreground HUD Reticle

```cpp
auto hud_card = skiaCanvas({
    .painter = [](Canvas& canvas, Size size) {
        // Background cyber grid lines
    },
    .foreground_painter = [](Canvas& canvas, Size size) {
        // Foreground corner target brackets
    },
    .child = text("SYSTEM ONLINE"),
    .width = 240.0f,
    .height = 140.0f,
    .clip_behavior = Clip::AntiAlias,
    .clip_radius = BorderRadius::circular(12.0f),
});
```
