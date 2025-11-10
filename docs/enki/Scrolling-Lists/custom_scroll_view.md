# CustomScrollView

> A scroll viewport widget that creates custom scroll effects by accepting an ordered sequence of sliver widgets (such as collapsible headers, linear lists, grids, and box adapters) within a single coordinated scroll axis.

- **Header File**: `#include "enki/widgets/sliver.hpp"`
- **C++ Class**: `enki::CustomScrollViewWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::CustomScrollView` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::CustomScrollViewProps`
- **Factory Helpers**: `enki::customScrollView(props)`, `customScrollView(slivers)`
- **Enums**: `enki::Axis`, `enki::ScrollPhysics`

---

## Overview

Unlike standard `ScrollView` or `ListView` (which host a single layout model), `CustomScrollView` provides a unified scroll viewport capable of coordinating multiple distinct scrolling areas known as **Slivers**. It enables seamless transitions where a collapsible `SliverAppBar` shrinks as the user scrolls, followed immediately by a `SliverGrid`, followed by a `SliverList`—all sharing the exact same scroll physics and scrollbar.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct CustomScrollView {
    Key                               key            = Key::none();
    std::vector<WidgetPtr>            slivers        = {};            ///< Sequence of sliver widgets
    std::vector<WidgetPtr>            children       = {};            ///< Convenience alias for slivers
    Axis                              direction      = Axis::Vertical;
    ScrollPhysics                     scroll_physics = ScrollPhysics::Clamped;
    float                             scroll_speed   = 50.0f;
    bool                              show_scrollbar = true;
    bool                              shrink_wrap    = false;
    bool                              reverse        = false;
    float                             cache_extent   = 0.0f;
    std::function<void(float offset)> on_scroll      = nullptr;

    operator WidgetPtr() const;
};

inline std::shared_ptr<CustomScrollViewWidget> customScrollView(CustomScrollViewProps props = {});
inline std::shared_ptr<CustomScrollViewWidget> customScrollView(std::vector<WidgetPtr> slivers);
inline std::shared_ptr<CustomScrollViewWidget> customScrollView(std::initializer_list<WidgetPtr> slivers);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `slivers` | `vector<WidgetPtr>` | `{}` | List of sliver widgets rendered along the scroll axis. |
| `direction` | `Axis` | `Axis::Vertical` | Primary scrolling orientation (`Vertical` or `Horizontal`). |
| `scroll_physics` | `ScrollPhysics` | `Clamped` | Edge resistance behavior (`Clamped`, `Bouncing`, `NeverScrollable`). |
| `scroll_speed` | `float` | `50.0f` | Mouse wheel scroll step distance in pixels. |
| `show_scrollbar`| `bool` | `true` | Renders a responsive interactive scrollbar track. |
| `shrink_wrap` | `bool` | `false` | Fits the viewport tightly to the total sliver extent. |
| `on_scroll` | `function<void(float)>`| `nullptr` | Callback dispatched with the current scroll offset. |

---

## Code Examples (From `widgets_demo/sliver_demo/main.cpp`)

### 1. Assembling a Coordinated Slivers Viewport
```cpp
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildCustomScrollWorkspace() {
    return CustomScrollView {
        .direction = Axis::Vertical,
        .scroll_physics = ScrollPhysics::Clamped,
        .slivers = {
            // 1. Collapsible Pinned App Bar
            SliverAppBar {
                .title = text("Dashboard Overview", { .color = 0xFFFFFFFF, .font_weight = FontWeight::Bold }),
                .expanded_height = 180.0f,
                .collapsed_height = 56.0f,
                .pinned = true,
                .background_color = 0xFF1E1B4B
            },

            // 2. Normal Box Widget wrapped in a Sliver Adapter
            SliverToBoxAdapter {
                .child = container({
                    .color = 0xFF0F172A,
                    .padding = EdgeInsets::all(16.0f),
                    .child = text("Featured Categories", { .color = 0xFF94A3B8, .font_size = 14.0f })
                })
            },

            // 3. Virtual Linear List Sliver
            SliverList {
                .item_count = 20,
                .item_builder = [](int idx) -> WidgetPtr {
                    return container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(8.0f),
                        .padding = EdgeInsets::all(14.0f),
                        .child = text("Item #" + std::to_string(idx + 1), { .color = 0xFFFFFFFF })
                    });
                },
                .separator_builder = [](int idx) { return sizedBox(0.0f, 6.0f); }
            }
        }
    };
}
```

---

## See Also
- [**SliverAppBar**](./sliver_app_bar.md) — Collapsible pinned header sliver.
- [**SliverList**](./sliver_list.md) — 1D linear list sliver.
- [**SliverGrid**](./sliver_grid.md) — 2D responsive grid sliver.
- [**ScrollView**](./scroll_view.md) — Simple non-sliver scrollable container.
