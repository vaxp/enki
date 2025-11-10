# DragTarget

> A drop zone widget that receives data from `Draggable` widgets, featuring dynamic visual hover state builders, acceptance filtering, and typed payload extraction.

- **Header File**: `#include "enki/widgets/draggable.hpp"`
- **C++ Class**: `enki::DragTargetWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::DragTarget` (converts implicitly to `WidgetPtr`)
- **Builder Signature**: `TargetBuilder = function<WidgetPtr(BuildContext&, bool is_hovered, const std::any& data)>`

---

## Overview

`DragTarget` defines an area that can accept a dropped `Draggable`. Through its dynamic `builder` function, the widget receives real-time updates regarding whether an active drag is currently hovering over its boundary (`is_hovered`) and the candidate data being dragged. This allows the target to render responsive visual cues (e.g. green highlight borders, dashed drop placeholders) before the user releases the pointer.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct DragTarget {
    Key                                       key            = Key::none();
    std::string                               accepted_tag   = ""; ///< Restricts acceptance to matching Draggable tags

    using TargetBuilder = std::function<WidgetPtr(BuildContext& context,
                                                  bool is_hovered,
                                                  const std::any& candidate_data)>;
    TargetBuilder                             builder        = nullptr;

    // Validation & Acceptance Callbacks
    std::function<bool(const std::any& data)> on_will_accept = nullptr; ///< Validate payload eligibility
    std::function<void(const std::any& data)> on_accept      = nullptr; ///< Process the accepted drop
    std::function<void()>                     on_leave       = nullptr; ///< Cursor exited target bounds

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `accepted_tag` | `std::string` | `""` | Tag filter string. If specified, only `Draggable` widgets with this tag trigger hover states. |
| `builder` | `TargetBuilder` | `nullptr` | Renders the target UI dynamically based on `is_hovered` and `candidate_data`. |
| `on_will_accept`| `function<bool(const any&)>`| `nullptr`| Optional predicate returning `true` if the item can be dropped here. |
| `on_accept` | `function<void(const any&)>`| `nullptr`| Callback executed when a valid payload is dropped onto the target. |
| `on_leave` | `function<void()>` | `nullptr` | Callback executed when a dragged item leaves the target without dropping. |

---

## Code Examples (From `widgets_demo/gesture_suite_demo/main.cpp`)

### 1. Kanban Column Drop Zone with Hover Feedback
```cpp
#include "enki/widgets/draggable.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildKanbanColumn(const std::string& columnId,
                            const std::vector<WidgetPtr>& cardWidgets,
                            std::function<void(const std::string&)> onTaskDropped) {
    return DragTarget {
        .accepted_tag = "kanban_task",
        .builder = [cardWidgets](BuildContext&, bool is_hovered, const std::any&) -> WidgetPtr {
            return container({
                // Dynamically tint emerald green on drag hover
                .color = is_hovered ? 0x2210B981 : 0xFF0F172A,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(is_hovered ? 0xFF10B981 : 0xFF334155, is_hovered ? 2.0f : 1.0f),
                .width = StyleValue::point(260.0f),
                .padding = EdgeInsets::all(12.0f),
                .child = column({
                    .gap = 8_px,
                    .children = cardWidgets
                })
            });
        },
        .on_accept = [onTaskDropped](const std::any& data) {
            try {
                std::string taskId = std::any_cast<std::string>(data);
                onTaskDropped(taskId);
            } catch (const std::bad_any_cast& e) {
                std::cerr << "Invalid drag payload: " << e.what() << "\n";
            }
        }
    };
}
```

---

## See Also
- [**Draggable**](./draggable.md) — The source draggable widget and payload holder.
- [**Dismissible**](./dismissible.md) — Swipe-to-dismiss gesture container.
- [**GestureDetector**](./gesture_detector.md) — Low-level 2D pan/drag recognizers.
