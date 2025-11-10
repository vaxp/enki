# Draggable

> An ultra-high-performance drag-and-drop source widget for Enki, featuring zero-rebuild 600+ FPS floating feedback overlays, typed `std::any` payloads, and semantic tag filtering.

- **Header File**: `#include "enki/widgets/draggable.hpp"`
- **C++ Class**: `enki::DraggableWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Draggable` (converts implicitly to `WidgetPtr`)
- **Global Session**: `enki::DragSession`, `enki::DragManager`

---

## Overview

`Draggable` makes any widget in your application draggable across the screen. When a drag gesture starts, Enki transfers rendering of the `feedback` widget to the top-level **DragOverlay**, painting directly into Skia at full display refresh rate (600+ FPS) without triggering expensive layout re-calculations on the page.

It carries an arbitrary payload via `std::any data` and a filtering `tag` string that ensures only compatible `DragTarget` widgets can accept the drop.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Draggable {
    Key                   key                 = Key::none();
    std::string           tag                 = "";           ///< Semantic tag for target matching
    std::string           preview_label       = "";           ///< Accessibility / HUD preview label
    std::any              data;                               ///< Arbitrary payload dropped into DragTarget

    WidgetPtr             child               = nullptr;      ///< Normal resting child widget
    WidgetPtr             feedback            = nullptr;      ///< Floating widget following the cursor
    WidgetPtr             child_when_dragging = nullptr;      ///< Ghost placeholder in original location

    // Lifecycle Callbacks
    std::function<void()> on_drag_started     = nullptr;
    std::function<void()> on_drag_end         = nullptr;
    std::function<void()> on_drag_completed   = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Global Session & Manager
```cpp
namespace enki {

struct DragSession {
    bool        is_active       = false;
    std::string tag             = "";
    std::string preview_label   = "";
    std::any    data;
    Point       current_pointer = {0.0f, 0.0f};
    WidgetPtr   feedback;
};

class DragManager {
public:
    static DragManager& instance();
    DragSession session;

    void startDrag(std::string tag, std::any data, WidgetPtr feedback, Point start_pos, std::string preview_label = "");
    void updatePointer(Point p);
    void endDrag();
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `tag` | `std::string` | `""` | Type tag matched against `DragTarget.accepted_tag`. |
| `data` | `std::any` | `{}` | Payload object passed into the target's `on_accept` callback. |
| `child` | `WidgetPtr` | `nullptr` | The normal widget rendered when not dragging. |
| `feedback` | `WidgetPtr` | `nullptr` | Floating card painted under the cursor during drag operations. |
| `child_when_dragging`| `WidgetPtr` | `nullptr` | Optional placeholder widget shown in the original position while dragging. |
| `on_drag_started` | `function<void()>` | `nullptr` | Fired when pointer movement exceeds drag threshold. |
| `on_drag_completed` | `function<void()>` | `nullptr` | Fired when dropped successfully into an accepting `DragTarget`. |
| `on_drag_end` | `function<void()>` | `nullptr` | Fired when the drag terminates (accepted or cancelled). |

---

## Code Examples (From `widgets_demo/gesture_suite_demo/main.cpp`)

### 1. Kanban Card with Floating Feedback
```cpp
#include "enki/widgets/draggable.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

struct TaskItem {
    std::string id;
    std::string title;
};

WidgetPtr buildDraggableTask(const TaskItem& task) {
    // Normal Card
    auto normalCard = container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(0xFF334155, 1.0f),
        .padding = EdgeInsets::all(12.0f),
        .child = text(task.title, { .color = 0xFFFFFFFF })
    });

    // Elevated Floating Feedback with cyan highlight border
    auto feedbackCard = container({
        .color = 0xF01E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(0xFF38BDF8, 2.0f), // Neon cyan border
        .padding = EdgeInsets::all(14.0f),
        .child = text(task.title, { .color = 0xFF38BDF8, .font_weight = FontWeight::Bold })
    });

    // Dimmed ghost placeholder
    auto ghostPlaceholder = container({
        .color = 0x331E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(0x33334155, 1.0f),
        .padding = EdgeInsets::all(12.0f)
    });

    return Draggable {
        .tag = "kanban_task",
        .preview_label = task.title,
        .data = task.id, // Store task id as payload
        .child = normalCard,
        .feedback = feedbackCard,
        .child_when_dragging = ghostPlaceholder,
        .on_drag_started = [] { std::cout << "Drag started!\n"; },
        .on_drag_completed = [] { std::cout << "Task successfully transferred!\n"; }
    };
}
```

---

## See Also
- [**DragTarget**](./drag_target.md) — The receiving drop zone for `Draggable` payloads.
- [**GestureDetector**](./gesture_detector.md) — Raw 2D pan/drag gesture handling.
- [**Dismissible**](./dismissible.md) — Swipe-to-dismiss row interactions.
