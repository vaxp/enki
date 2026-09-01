# Enki Gestures & Interaction Suite

> Pointer routing pipeline, Gesture Arena disambiguation, taps, long presses, 2D panning, zero-rebuild 600+ FPS drag-and-drop, swipe-to-dismiss, hover tracking, and system cursor control.

The **Gestures / Interaction** subsystem transforms raw hardware pointer events (mouse clicks, trackpad pans, touch down/up/move, and wheel scrolls) into semantic user intent. Enki features a sophisticated **Gesture Arena** to resolve competing recognizers (e.g. distinguishing a quick single tap from a double tap, a long press, or a continuous drag), high-performance drag-and-drop overlays that bypass widget tree rebuilding, and granular hover/cursor dispatching.

---

## Architectural Architecture: Gesture Pipeline & Arena

```
Raw Pointer Event (Platform Window)
       │
       ▼
Hit Testing (`HitTestBehavior`: DeferToChild | Opaque | Translucent)
       │
       ▼
Gesture Recognizers (`Tap`, `LongPress`, `Pan`)
       │
       ▼
┌─────────────────────────────────────────────────────────────┐
│                      Gesture Arena                          │
│                                                             │
│   • Competing recognizers register interest on pointer-down │
│   • Movement beyond touch slop claims Victory for Pan       │
│   • Timer expiry without movement declares LongPress win    │
│   • Pointer-up before timeout declares Tap winner           │
└─────────────────────────────────────────────────────────────┘
       │
       ▼
Dispatch Semantic Callbacks (`on_tap`, `on_long_press`, `on_pan_update`)
```

---

## Widget & Interaction Catalog

| # | Widget / Concept | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**GestureDetector**](./gesture_detector.md) | `class GestureDetector`, `gestureDetector()` | `<enki/widgets/gesture_detector.hpp>` | Master gesture detector for taps, secondary clicks, double taps, 2D panning, and scrolling. |
| 2 | [**Draggable**](./draggable.md) | `struct Draggable`, `class DraggableWidget` | `<enki/widgets/draggable.hpp>` | Drag source with zero-rebuild floating feedback overlay and typed `std::any` payloads. |
| 3 | [**DragTarget**](./drag_target.md) | `struct DragTarget`, `class DragTargetWidget` | `<enki/widgets/draggable.hpp>` | Drop recipient target with tag filtering, hover state styling, and accept callbacks. |
| 4 | [**Dismissible**](./dismissible.md) | `struct Dismissible`, `DismissDirection` | `<enki/widgets/dismissible.hpp>` | Swipe-to-dismiss row container with bidirectional action reveals and threshold animations. |
| 5 | [**LongPress**](./long_press.md) | `LongPressGestureRecognizer`, Details structs | `<enki/gestures/gesture_types.hpp>` | Touch/click hold detection with start, continuous move, and release coordinate streams. |
| 6 | [**HoverRegion**](./hover_region.md) | `on_hover_enter`, `on_hover_exit`, `on_hover_move` | `<enki/widgets/gesture_detector.hpp>` | Pointer boundary tracking, hover styling, and cursor enter/exit state management. |
| 7 | [**MouseRegion**](./mouse_region.md) | `SystemCursor`, `cursor_type`, `on_scroll` | `<enki/widgets/gesture_detector.hpp>` | System mouse cursor switching (Pointer, Crosshair, Resize) and wheel scroll handling. |

---

## High-Performance Drag-and-Drop Architecture

Enki includes an optimized global drag system managed by `DragManager` and `DragOverlay`:
- **Zero Tree Rebuild**: Dragging does not trigger expensive layout passes or reconciliations on the main widget tree.
- **Direct Skia Canvas Render**: The dragging `feedback` preview widget is painted directly at the cursor position on the root overlay pass at 600+ FPS.
- **Type-Safe Payloads**: Arbitrary C++ objects can be passed safely via `std::any` and validated through `tag` matching.

---

## Quick Example (Unified Gesture Card)

```cpp
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr makeInteractiveCard(const std::string& label) {
    return gestureDetector({
        .cursor_type = SystemCursor::Pointer,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .on_tap = [label] {
            std::cout << "Card clicked: " << label << "\n";
        },
        .on_secondary_tap = [label] {
            std::cout << "Right-clicked (Context Menu): " << label << "\n";
        },
        .on_long_press = [label] {
            std::cout << "Long pressed: " << label << "\n";
        },
        .on_hover_enter = [](const HoverEvent&) {
            std::cout << "Mouse entered card bounds.\n";
        },
        .child = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = EdgeInsets::all(16.0f),
            .child = text(label, { .color = 0xFFF1F5F9 })
        })
    });
}
```
