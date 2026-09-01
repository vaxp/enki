# SliverList

> A linear 1D list sliver widget that displays child items sequentially along the main scroll axis, supporting static items, lazy on-demand builders, and custom item separators.

- **Header File**: `#include "enki/widgets/sliver.hpp"`
- **C++ Class**: `enki::SliverListWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::SliverList` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::SliverListProps`
- **Factory Helpers**: `enki::sliverList()`, `enki::sliverListCount()`, `enki::sliverListSeparated()`

---

## Overview

`SliverList` provides the functionality of a `ListView` adapted specifically for placement inside a `CustomScrollView`. It manages linear streams of content and supports lazy virtualization via `item_builder` (instantiating only the items currently intersecting the viewport plus cache extent) as well as custom dividers via `separator_builder`.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct SliverList {
    Key                                 key               = Key::none();
    std::vector<WidgetPtr>              items             = {};        ///< Explicit list of child widgets
    std::vector<WidgetPtr>              children          = {};        ///< Convenience alias for items
    int                                 item_count        = 0;         ///< Total count when using builders
    std::function<WidgetPtr(int index)> item_builder      = nullptr;   ///< Lazy item factory
    std::function<WidgetPtr(int index)> separator_builder = nullptr;   ///< Divider widget factory
    EdgeInsets                          padding           = EdgeInsets{};

    operator WidgetPtr() const;
};

inline std::shared_ptr<SliverListWidget> sliverList(SliverListProps props = {});
inline std::shared_ptr<SliverListWidget> sliverList(std::vector<WidgetPtr> items);
inline std::shared_ptr<SliverListWidget> sliverList(std::initializer_list<WidgetPtr> items);
inline std::shared_ptr<SliverListWidget> sliverListCount(int count, std::function<WidgetPtr(int index)> builder);
inline std::shared_ptr<SliverListWidget> sliverListSeparated(
    int count,
    std::function<WidgetPtr(int index)> builder,
    std::function<WidgetPtr(int index)> separator
);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `vector<WidgetPtr>` | `{}` | Explicit static children list. |
| `item_count` | `int` | `0` | Total number of items when using `item_builder`. |
| `item_builder` | `function<WidgetPtr(int)>`| `nullptr`| Factory callback invoked to construct each item lazily by index. |
| `separator_builder`| `function<WidgetPtr(int)>`| `nullptr`| Optional callback returning divider widgets between items. |
| `padding` | `EdgeInsets` | `EdgeInsets{}`| Outer padding surrounding the sliver list. |

---

## Code Examples (From `widgets_demo/sliver_demo/main.cpp`)

### 1. Lazy Virtualized SliverList with Separators
```cpp
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildActivityFeedSliver() {
    return SliverList {
        .item_count = 50,
        .item_builder = [](int idx) -> WidgetPtr {
            return container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(8.0f),
                .padding = EdgeInsets::all(14.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = 12_px,
                    .children = {
                        text("#" + std::to_string(idx + 1), { .color = 0xFF38BDF8, .font_weight = FontWeight::Bold }),
                        text("System audit log entry recorded.", { .color = 0xFFCBD5E1 })
                    }
                })
            });
        },
        .separator_builder = [](int idx) {
            return sizedBox(0.0f, 6.0f); // 6px vertical spacing
        }
    };
}
```

---

## See Also
- [**CustomScrollView**](./custom_scroll_view.md) — Viewport coordinating multiple slivers.
- [**SliverGrid**](./sliver_grid.md) — 2D grid sliver.
- [**ListView**](./list_view.md) — Self-contained linear list view.
- [**ListTile**](./list_tile.md) — Standardized row tile.
