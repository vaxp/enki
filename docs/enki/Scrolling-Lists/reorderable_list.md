# ReorderableList

> An interactive list widget delivering  direct Skia floating drag-and-drop capabilities, real-time target slot displacement previews, glowing drop line indicators, and custom drag handles.

- **Header File**: `#include "enki/widgets/reorderable_list.hpp"`
- **C++ Class**: `enki::ReorderableListWidget` (inherits from `enki::MultiChildRenderObjectWidget`)
- **Declarative Structs**: `enki::ReorderableList`, `enki::ReorderableDragHandle` (convert implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ReorderableListProps`

---

## Overview

`ReorderableList` allows users to intuitively reorder items within a vertical list by dragging rows up or down. As the user drags a card, the displaced target items smoothly slide out of the way to visualize the landing position, a glowing drop indicator line marks the target insertion boundary, and an `on_reorder(old_index, new_index)` callback is invoked on release.

Drag initiation can be triggered either across the entire row or restricted to an embedded `ReorderableDragHandle` widget (e.g. a `≡` grip icon).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ReorderableList {
    Key                                               key                  = Key::none();
    std::vector<WidgetPtr>                            children;

    float                                             item_height          = 56.0f; ///< Uniform height per item
    float                                             gap                  = 10.0f; ///< Vertical spacing between items
    float                                             width                = 480.0f;///< Fixed container width

    bool                                              show_drop_indicator  = true;  ///< Glowing drop target line
    Color                                             drop_indicator_color = 0xFF38BDF8; // Sky 400

    std::function<void(int old_index, int new_index)> on_reorder           = nullptr;

    operator WidgetPtr() const;
};

struct ReorderableDragHandle {
    Key       key   = Key::none();
    WidgetPtr child = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `children` | `std::vector<WidgetPtr>` | `{}` | The list of draggable child widgets to display. |
| `item_height` | `float` | `56.0f` | Uniform pixel height of each item in the list. |
| `gap` | `float` | `10.0f` | Vertical gap between adjacent items. |
| `width` | `float` | `480.0f` | Total width of the list bounds. |
| `show_drop_indicator`| `bool` | `true` | Renders a glowing horizontal accent bar at the proposed drop index. |
| `drop_indicator_color`| `Color` | `0xFF38BDF8` | Color of the glowing drop insertion bar. |
| `on_reorder` | `Function(old_idx, new_idx)` | `nullptr` | Callback invoked upon item release with former and new indices. |

---

## Code Examples (From `widgets_demo/reorderable_list_demo/main.cpp`)

### 1. Reorderable Task Priority Backlog
```cpp
#include "enki/widgets/reorderable_list.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

struct Task {
    std::string title;
    std::string priority;
};

class BacklogState : public State {
    std::vector<Task> tasks_ = {
        {"Direct Skia Shader Pipeline", "CRITICAL"},
        {"Wayland Buffer Presentation", "HIGH"},
        {"Anura Layout Nodes",          "CORE"},
        {"XKB Keyboard Input Manager",  "INPUT"},
    };

    WidgetPtr buildCard(const Task& t, int index) {
        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = EdgeInsets::symmetric(12.0f, 16.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .width = 100_pct,
                .children = {
                    text("#" + std::to_string(index + 1) + "  " + t.title),
                    ReorderableDragHandle { // Drag grip handle
                        .child = text("☰", { .color = 0xFF64748B, .font_size = 16.0f })
                    }
                }
            })
        });
    }

public:
    WidgetPtr build(BuildContext&) override {
        std::vector<WidgetPtr> cards;
        for (size_t i = 0; i < tasks_.size(); ++i) {
            cards.push_back(buildCard(tasks_[i], static_cast<int>(i)));
        }

        return ReorderableList {
            .children = std::move(cards),
            .item_height = 52.0f,
            .gap = 10.0f,
            .width = 540.0f,
            .on_reorder = [this](int old_idx, int new_idx) {
                auto item = tasks_[old_idx];
                tasks_.erase(tasks_.begin() + old_idx);
                tasks_.insert(tasks_.begin() + new_idx, item);
                setState([] {}); // Re-render with new order
            }
        };
    }
};
```

---

## See Also
- [**ListView**](./list_view.md) — Standard non-reorderable list view.
- [**ListTile**](./list_tile.md) — Row component frequently used within reorderable lists.
