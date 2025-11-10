# HoverRegion & Pointer Tracking

> Boundary hover event detection in Enki, providing cursor entry, movement, and exit callbacks for dynamic hover styles, micro-animations, and tooltips.

- **Header File**: `#include "enki/widgets/gesture_detector.hpp"`
- **Host Widget**: `enki::GestureDetector`
- **Render Object**: `enki::RenderGestureDetector`
- **Callback Aliases**: `GestureHoverCallback` (`std::function<void(const PointerEvent&)>`)

---

## Overview

A **HoverRegion** tracks when the platform's mouse cursor crosses into, moves within, or exits a widget's bounding geometry. In Enki, hover detection is baked into `GestureDetector`. When a pointer moves across the screen, the render tree performs hit testing:
- **`on_hover_enter`**: Invoked precisely once when the cursor moves from outside the widget's bounds to inside.
- **`on_hover_move`**: Invoked continuously as the cursor travels within the widget's bounds (providing local coordinates).
- **`on_hover_exit`**: Invoked when the cursor leaves the widget's boundary.

---

## C++ API Definition

### `GestureDetector` Hover Callbacks
```cpp
namespace enki {

struct GestureDetectorProps {
    // ...
    GestureHoverCallback on_hover_enter = nullptr; ///< Cursor entered bounding box
    GestureHoverCallback on_hover_exit  = nullptr; ///< Cursor left bounding box
    GestureHoverCallback on_hover_move  = nullptr; ///< Cursor moved within bounds
};

} // namespace enki
```

### Event Payload (`PointerEvent`)
```cpp
namespace enki {

struct PointerEvent {
    Point       position;       ///< Global window coordinates
    Point       local_position; ///< Local widget coordinates
    MouseButton button;
    int         modifiers;
    double      timestamp;
};

using GestureHoverCallback = std::function<void(const PointerEvent&)>;

} // namespace enki
```

---

## Code Examples (From `widgets_demo/gesture_demo/main.cpp`)

### 1. Dynamic Hover Card with Smooth State Transitions
```cpp
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class HoverableCardState : public State {
    bool is_hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        return gestureDetector({
            .cursor_type = SystemCursor::Pointer,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .on_hover_enter = [this](const PointerEvent&) {
                is_hovered_ = true;
                setState([]{});
            },
            .on_hover_exit = [this](const PointerEvent&) {
                is_hovered_ = false;
                setState([]{});
            },
            .child = container({
                // Dynamic styling based on hover state
                .color = is_hovered_ ? 0xFF20263E : 0xFF181C2E,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(is_hovered_ ? 0xFF38BDF8 : 0xFF334155, 1.0f),
                .padding = EdgeInsets::all(16.0f),
                .child = text("Hover Over This Card", {
                    .color = is_hovered_ ? 0xFF38BDF8 : 0xFFE2E8F0,
                    .font_weight = is_hovered_ ? FontWeight::Bold : FontWeight::Normal
                })
            })
        });
    }
};
```

---

## See Also
- [**MouseRegion**](./mouse_region.md) — System cursor icon switching and scroll wheel routing.
- [**GestureDetector**](./gesture_detector.md) — Unified gesture recognizer widget.
- [**Tooltip**](../NativePopups/tooltip.md) — Contextual balloon popups triggered on hover.
