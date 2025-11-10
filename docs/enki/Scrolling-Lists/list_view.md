# ListView

> A linear scrollable list widget supporting static item vectors, lazy on-demand item builders, custom separator widgets, item selection callbacks, and configurable scroll physics.

- **Header File**: `#include "enki/widgets/list_view.hpp"`
- **C++ Class**: `enki::ListViewWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ListView` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ListViewProps`
- **Enum**: `enki::ScrollPhysics` (`Clamped`, `Bouncing`, `Never`, `Inherited`)
- **Factory Helpers**: `enki::listView(...)`, `enki::listViewSeparated(...)`

---

## Overview

`ListView` is the standard widget for presenting linear vertical or horizontal lists. It supports two primary paradigms:
1. **Static Vector Mode**: Pass a list of pre-constructed `items` or `children`. Ideal for short, static option lists or settings screens.
2. **Lazy Builder Mode**: Provide an `item_count` and an `item_builder` lambda `(int index) -> WidgetPtr`. Only visible/required items are instantiated, allowing lists with thousands of items to run at 600+ FPS without memory overhead.

It also supports custom dividers via `separator_builder`, row selection tracking with `selected_index`, and embedding inside other flex layouts via `shrink_wrap = true`.

---

## C++ API Definition

### Scroll Physics Enum
```cpp
namespace enki {

enum class ScrollPhysics {
    Clamped,   ///< Clamp scroll at boundaries — no bounce (default for desktop)
    Bouncing,  ///< Allow a spring-like bounce at the boundaries
    Never,     ///< Disable all scrolling
    Inherited  ///< Inherit the physics from a parent scrollable
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ListView {
    Key                                 key               = Key::none();
    std::vector<WidgetPtr>              items             = {};
    std::vector<WidgetPtr>              children          = {}; // Alias for items
    int                                 item_count        = 0;
    std::function<WidgetPtr(int index)> item_builder      = nullptr;
    std::function<WidgetPtr(int index)> separator_builder = nullptr;

    Axis                                direction         = Axis::Vertical;
    ScrollPhysics                       scroll_physics    = ScrollPhysics::Clamped;
    float                               scroll_speed      = 50.0f;
    EdgeInsets                          list_padding      = EdgeInsets{};
    EdgeInsets                          padding           = EdgeInsets{}; // Alias
    bool                                shrink_wrap       = false;
    bool                                is_reversed       = false;
    bool                                reverse           = false; // Alias
    bool                                primary           = true;

    std::optional<int>                  selected_index    = std::nullopt;
    std::function<void(int index)>      on_item_selected  = nullptr;

    std::function<void()>               on_scroll_start   = nullptr;
    std::function<void(float offset)>   on_scroll         = nullptr;
    std::function<void()>               on_scroll_end     = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<ListViewWidget> listView(ListViewProps props = {});
inline std::shared_ptr<ListViewWidget> listView(std::vector<WidgetPtr> items);
inline std::shared_ptr<ListViewWidget> listView(std::initializer_list<WidgetPtr> items);
inline std::shared_ptr<ListViewWidget> listView(int count, std::function<WidgetPtr(int)> builder);
inline std::shared_ptr<ListViewWidget> listViewSeparated(
    std::vector<WidgetPtr> items,
    std::function<WidgetPtr(int)> separator);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` / `children` | `std::vector<WidgetPtr>` | `{}` | Pre-constructed widgets for static list mode. |
| `item_count` | `int` | `0` | Total number of items when using `item_builder`. |
| `item_builder` | `Function(int)` | `nullptr` | Lazy factory lambda producing the widget at `index`. |
| `separator_builder` | `Function(int)` | `nullptr` | Factory lambda inserting dividers between items. |
| `direction` | `Axis` | `Axis::Vertical` | Scrolling orientation (`Vertical` or `Horizontal`). |
| `scroll_physics` | `ScrollPhysics` | `Clamped` | Boundary bounce behavior (`Clamped`, `Bouncing`, `Never`). |
| `shrink_wrap` | `bool` | `false` | When true, sizes itself to its children's height (essential inside `Column`). |
| `selected_index` | `optional<int>` | `nullopt` | Currently highlighted item index. |
| `on_item_selected` | `Function(int)` | `nullptr` | Callback triggered when user clicks an item. |

---

## Code Examples (From `widgets_demo/list_view_demo/main.cpp`)

### 1. Lazy Item Builder for Large Datasets (e.g. 5,000 Rows)
```cpp
#include "enki/widgets/list_view.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr buildLargeFeed() {
    return ListView {
        .item_count = 5000,
        .item_builder = [](int i) -> WidgetPtr {
            return container({
                .color = (i % 2 == 0) ? 0xFF0D1117 : 0xFF161B22,
                .padding = EdgeInsets::symmetric(12.0f, 16.0f),
                .child = text("Log Event #" + std::to_string(i + 1), {
                    .color = 0xFFE2E8F0,
                    .font_size = 14.0f
                })
            });
        }
    };
}
```

### 2. Separated List with Dividers
```cpp
#include "enki/widgets/divider.hpp"

WidgetPtr buildSettingsList() {
    return ListView {
        .item_count = 10,
        .item_builder = [](int i) -> WidgetPtr {
            return text("Setting Parameter #" + std::to_string(i + 1));
        },
        .separator_builder = [](int) -> WidgetPtr {
            return Divider {
                .height = 1.0f,
                .color = 0x1AFFFFFF,
                .indent = 16.0f,
            };
        }
    };
}
```

---

## See Also
- [**ScrollView**](./scroll_view.md) — Base scrolling viewport.
- [**ListTile**](./list_tile.md) — Standardized row widget for list items.
- [**GridView**](./grid_view.md) — Multi-column 2D grid list.
