# GestureDetector

> The primary gesture detection widget in Enki, providing taps, double taps, secondary right-clicks, long presses, 2D panning, scrolling, hover events, and system cursor switching.

- **Header File**: `#include "enki/widgets/gesture_detector.hpp"`
- **C++ Class**: `enki::GestureDetector` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Props Struct**: `enki::GestureDetectorProps`
- **Factory Helper**: `enki::gestureDetector(props)`
- **Render Object**: `enki::RenderGestureDetector`
- **Hit Test Enum**: `enki::HitTestBehavior` (`DeferToChild`, `Opaque`, `Translucent`)

---

## Overview

`GestureDetector` is the fundamental building block for interactive components in Enki. It wraps a single child widget and routes pointer down, move, and up events through internal recognizers (`TapGestureRecognizer`, `LongPressGestureRecognizer`, `PanGestureRecognizer`). It features non-blocking gesture arena resolution, configurable hit testing rules, and direct integration with the platform cursor manager.

---

## C++ API Definition

### `HitTestBehavior` Enum
```cpp
namespace enki {

enum class HitTestBehavior {
    /// Only target when the child widget is directly hit.
    /// Hits in empty space within bounds are ignored.
    DeferToChild,

    /// Targets can receive hits across their entire bounding box,
    /// and prevent objects visually behind them from receiving hits.
    Opaque,

    /// Targets can receive hits across their entire bounding box,
    /// but also allow targets visually behind them to receive hits.
    Translucent
};

} // namespace enki
```

### `GestureDetectorProps` Struct
```cpp
namespace enki {

struct GestureDetectorProps {
    Key                       key                   = Key::none();
    WidgetPtr                 child                 = nullptr;

    HitTestBehavior           hit_test_behavior     = HitTestBehavior::DeferToChild;
    SystemCursor              cursor_type           = SystemCursor::Default;

    // ── Tap Callbacks ──────────────────────────────────────────
    GestureTapCallback        on_tap                = nullptr;
    GestureTapDownCallback    on_tap_down           = nullptr;
    GestureTapUpCallback      on_tap_up             = nullptr;
    GestureTapCancelCallback  on_tap_cancel         = nullptr;

    // ── Secondary (Right-Click) Callbacks ──────────────────────
    GestureTapCallback        on_secondary_tap      = nullptr;
    GestureTapDownCallback    on_secondary_tap_down = nullptr;
    GestureTapUpCallback      on_secondary_tap_up   = nullptr;

    // ── Double Tap Callbacks ───────────────────────────────────
    GestureTapCallback        on_double_tap         = nullptr;
    GestureTapDownCallback    on_double_tap_down    = nullptr;
    GestureTapCancelCallback  on_double_tap_cancel  = nullptr;

    // ── Long Press Callbacks ───────────────────────────────────
    GestureLongPressCallback      on_long_press       = nullptr;
    GestureLongPressStartCallback on_long_press_start = nullptr;
    GestureLongPressMoveCallback  on_long_press_move  = nullptr;
    GestureLongPressEndCallback   on_long_press_end   = nullptr;

    // ── Pan / Drag Callbacks ───────────────────────────────────
    GestureDragStartCallback  on_pan_start          = nullptr;
    GestureDragUpdateCallback on_pan_update         = nullptr;
    GestureDragEndCallback    on_pan_end            = nullptr;
    GestureDragCancelCallback on_pan_cancel         = nullptr;

    // ── Hover & Scroll Callbacks ───────────────────────────────
    GestureHoverCallback      on_hover_enter        = nullptr;
    GestureHoverCallback      on_hover_exit         = nullptr;
    GestureHoverCallback      on_hover_move         = nullptr;
    GestureScrollCallback     on_scroll             = nullptr;
};

// Convenient Factory Function
inline std::shared_ptr<GestureDetector> gestureDetector(const GestureDetectorProps& props = {});
inline std::shared_ptr<GestureDetector> gestureDetector(Key key, GestureDetectorProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Subtree that receives visual rendering and gesture hits. |
| `cursor_type` | `SystemCursor` | `Default` | Platform cursor icon when mouse hovers over widget bounds. |
| `hit_test_behavior`| `HitTestBehavior` | `DeferToChild` | Hit-test boundary rule (`DeferToChild`, `Opaque`, `Translucent`). |
| `on_tap` | `function<void()>` | `nullptr` | Fired when primary click/touch completes without dragging. |
| `on_double_tap`| `function<void()>` | `nullptr` | Fired when two successive taps occur within timeout. |
| `on_secondary_tap`| `function<void()>` | `nullptr` | Fired on secondary button (mouse right-click) release. |
| `on_long_press`| `function<void()>` | `nullptr` | Fired when pointer is held stationary beyond threshold (~500ms). |
| `on_pan_update`| `function<void(DragUpdateDetails)>`| `nullptr`| Continuous 2D position delta and displacement callback. |
| `on_scroll` | `function<void(float, float)>`| `nullptr`| Fired when mouse wheel rotates with delta `dx` and `dy`. |

---

## Code Examples (From `widgets_demo/gesture_demo/main.cpp`)

### 1. Multi-Action Interactive Card
```cpp
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildGestureCard(std::function<void(const std::string&)> logAction) {
    return gestureDetector({
        .cursor_type = SystemCursor::Pointer,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .on_tap = [logAction] {
            logAction("Single Tap Detected");
        },
        .on_double_tap = [logAction] {
            logAction("Double Tap Detected");
        },
        .on_secondary_tap = [logAction] {
            logAction("Right-Click Context Tap");
        },
        .on_long_press = [logAction] {
            logAction("Long Press Threshold Triggered");
        },
        .child = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = EdgeInsets::all(20.0f),
            .child = text("Tap, Double-Tap, Right-Click, or Hold Me!", {
                .color = 0xFFFFFFFF,
                .font_weight = FontWeight::Bold
            })
        })
    });
}
```

### 2. 2D Pan / Dragging Surface
```cpp
auto canvasSurface = gestureDetector({
    .hit_test_behavior = HitTestBehavior::Opaque,
    .on_pan_start = [](const DragStartDetails& d) {
        std::cout << "Pan began at: " << d.local_position.x << ", " << d.local_position.y << "\n";
    },
    .on_pan_update = [](const DragUpdateDetails& d) {
        std::cout << "Delta: " << d.delta.x << ", " << d.delta.y << "\n";
    },
    .on_pan_end = [](const DragEndDetails& d) {
        std::cout << "Pan finished with velocity: " << d.velocity.x << ", " << d.velocity.y << "\n";
    },
    .child = myDrawingCanvasWidget
});
```

---

## See Also
- [**Draggable**](./draggable.md) — High-performance drag-and-drop source.
- [**Dismissible**](./dismissible.md) — Swipe-to-dismiss row containers.
- [**LongPress**](./long_press.md) — Detailed guide to long-press recognizers and details.
- [**MouseRegion**](./mouse_region.md) — Cursor styling and pointer routing.
