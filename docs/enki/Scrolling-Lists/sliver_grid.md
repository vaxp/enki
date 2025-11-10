# SliverGrid

> A 2D multi-column grid sliver widget that arranges child items in a two-dimensional grid within a `CustomScrollView`, supporting fixed column counts or responsive max-extent layouts.

- **Header File**: `#include "enki/widgets/sliver.hpp"`
- **C++ Class**: `enki::SliverGridWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::SliverGrid` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::SliverGridProps`
- **Factory Helpers**: `enki::sliverGrid()`, `enki::sliverGridExtent()`
- **Delegates**: `enki::SliverGridDelegateFixedCount`, `enki::SliverGridDelegateMaxExtent`

---

## Overview

`SliverGrid` adapts the multi-column layout capabilities of `GridView` for use inside a `CustomScrollView`. Developers can either specify a fixed number of cross-axis columns (`SliverGridDelegateFixedCount`) or define a maximum item width (`SliverGridDelegateMaxExtent`) so the grid automatically recalculates the optimal number of columns based on the window's available width.

---

## C++ API Definition

### Grid Delegates & Declarative Struct
```cpp
namespace enki {

struct SliverGrid {
    Key                                 key                     = Key::none();
    std::vector<WidgetPtr>              items                   = {};
    std::vector<WidgetPtr>              children                = {}; // alias
    int                                 item_count              = 0;
    std::function<WidgetPtr(int index)> item_builder            = nullptr;
    SliverGridDelegateFixedCount        fixed_delegate          = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent         max_delegate;
    bool                                use_max_extent_delegate = false;
    EdgeInsets                          padding                 = EdgeInsets{};

    operator WidgetPtr() const;
};

inline std::shared_ptr<SliverGridWidget> sliverGrid(SliverGridProps props = {});
inline std::shared_ptr<SliverGridWidget> sliverGrid(
    int count,
    std::function<WidgetPtr(int index)> builder,
    SliverGridDelegateFixedCount delegate = SliverGridDelegateFixedCount(2)
);
inline std::shared_ptr<SliverGridWidget> sliverGridExtent(
    int count,
    std::function<WidgetPtr(int index)> builder,
    SliverGridDelegateMaxExtent delegate
);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `vector<WidgetPtr>` | `{}` | Static children list. |
| `item_count` | `int` | `0` | Total item count for builder generation. |
| `item_builder` | `function<WidgetPtr(int)>`| `nullptr`| Factory lambda building items by index. |
| `fixed_delegate` | `SliverGridDelegateFixedCount`| `2 columns` | Fixed column count delegate `(crossAxisCount, mainSpacing, crossSpacing, aspectRatio)`. |
| `max_delegate` | `SliverGridDelegateMaxExtent`| `{}` | Responsive max item width delegate. |
| `padding` | `EdgeInsets` | `EdgeInsets{}`| External padding around the grid sliver. |

---

## Code Examples (From `widgets_demo/sliver_demo/main.cpp`)

### 1. Fixed 3-Column SliverGrid
```cpp
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildShowcaseGridSliver() {
    return SliverGrid {
        .item_count = 6,
        .item_builder = [](int idx) -> WidgetPtr {
            return container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF38BDF8, 1.5f),
                .padding = EdgeInsets::all(14.0f),
                .child = text("Card #" + std::to_string(idx + 1), {
                    .color = 0xFFFFFFFF,
                    .font_weight = FontWeight::Bold
                })
            });
        },
        // 3 columns, 10px row spacing, 10px column spacing, aspect ratio 1.25
        .fixed_delegate = SliverGridDelegateFixedCount(3, 10.0f, 10.0f, 1.25f)
    };
}
```

### 2. Responsive Auto-Fitting SliverGrid
```cpp
auto responsiveGrid = SliverGrid {
    .item_count = 24,
    .item_builder = [](int idx) { return myProductCard(idx); },
    .max_delegate = SliverGridDelegateMaxExtent(220.0f, 12.0f, 12.0f, 1.0f),
    .use_max_extent_delegate = true
};
```

---

## See Also
- [**CustomScrollView**](./custom_scroll_view.md) — Viewport coordinating multiple slivers.
- [**SliverList**](./sliver_list.md) — 1D linear list sliver.
- [**GridView**](./grid_view.md) — Self-contained standard grid view.
- [**GridTile**](./grid_tile.md) — Standard grid tile with header and footer.
