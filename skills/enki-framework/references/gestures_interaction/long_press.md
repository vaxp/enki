# LongPress

> Touch and mouse pointer hold gesture detection in Enki, providing start, continuous movement, and release coordinate streams resolved through the Gesture Arena.

- **Header Files**: `#include "enki/widgets/gesture_detector.hpp"`, `#include "enki/gestures/gesture_types.hpp"`
- **Internal Recognizer**: `enki::LongPressGestureRecognizer` (in `<enki/gestures/recognizer.hpp>`)
- **Detail Structs**: `LongPressStartDetails`, `LongPressMoveUpdateDetails`, `LongPressEndDetails`

---

## Overview

A **LongPress** gesture occurs when a primary pointer (mouse button, touch finger, or stylus) presses down and remains stationary within a small tolerance window (touch slop) for an established duration threshold (approx. 500ms).

In Enki, long presses are recognized via `LongPressGestureRecognizer` and exposed declaratively on `GestureDetector`. If the pointer moves significantly before the timer expires, the **Gesture Arena** awards victory to the `PanGestureRecognizer` instead and cancels the pending long press.

---

## C++ API Definition

### Event Detail Structs
```cpp
namespace enki {

/// @brief Details when a long-press duration threshold is reached.
struct LongPressStartDetails {
    Point  global_position;  ///< Coordinates relative to the application window
    Point  local_position;   ///< Coordinates relative to the widget's top-left
    double timestamp = 0.0;
};

/// @brief Details during movement while in long-press state.
struct LongPressMoveUpdateDetails {
    Point  global_position;
    Point  local_position;
    Point  delta;            ///< Offset displacement since last event
    double timestamp = 0.0;
};

/// @brief Details when pointer is released after a long-press.
struct LongPressEndDetails {
    Point  global_position;
    Point  local_position;
    double timestamp = 0.0;
};

// Callback Aliases
using GestureLongPressCallback      = std::function<void()>;
using GestureLongPressStartCallback = std::function<void(const LongPressStartDetails&)>;
using GestureLongPressMoveCallback  = std::function<void(const LongPressMoveUpdateDetails&)>;
using GestureLongPressEndCallback   = std::function<void(const LongPressEndDetails&)>;

} // namespace enki
```

### Integration with `GestureDetector`
```cpp
namespace enki {

struct GestureDetectorProps {
    // ...
    GestureLongPressCallback      on_long_press       = nullptr;
    GestureLongPressStartCallback on_long_press_start = nullptr;
    GestureLongPressMoveCallback  on_long_press_move  = nullptr;
    GestureLongPressEndCallback   on_long_press_end   = nullptr;
};

} // namespace enki
```

---

## Properties & Callbacks Reference

| Callback | Signature | Description |
|---|---|---|
| `on_long_press` | `void()` | Simple zero-argument notification when hold threshold is achieved. |
| `on_long_press_start`| `void(LongPressStartDetails)` | Fired at the moment the long press timer triggers, with touch coordinates. |
| `on_long_press_move` | `void(LongPressMoveUpdateDetails)`| Continuous updates if the user slides their finger/mouse after holding. |
| `on_long_press_end` | `void(LongPressEndDetails)` | Fired when the user finally releases their pointer after a long press. |

---

## Code Examples (From `widgets_demo/gesture_demo/main.cpp`)

### 1. Contextual Action Sheet on Long Press
```cpp
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildHoldToInspectCard(const std::string& fileName) {
    return gestureDetector({
        .hit_test_behavior = HitTestBehavior::Opaque,
        .on_long_press_start = [fileName](const LongPressStartDetails& details) {
            std::cout << "Long press triggered on '" << fileName << "' at local ("
                      << details.local_position.x << ", " << details.local_position.y << ")\n";
            // Present context popup / preview sheet at cursor location
        },
        .on_long_press_end = [](const LongPressEndDetails&) {
            std::cout << "Long press released.\n";
        },
        .child = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = EdgeInsets::all(16.0f),
            .child = text("Hold to Inspect: " + fileName, { .color = 0xFFFFFFFF })
        })
    });
}
```

---

## See Also
- [**GestureDetector**](./gesture_detector.md) — Complete gesture recognition widget.
- [**ContextMenu**](../NativePopups/context_menu.md) — OS-level context menu commonly triggered on long-press.
- [**Draggable**](./draggable.md) — Drag operations initiated by tap-and-drag.
