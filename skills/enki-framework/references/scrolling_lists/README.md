# Enki Scrolling & List Widgets

> High-performance layout containers, virtualized lists, responsive grids, expandable tree hierarchies, data tables, drag-and-drop lists, and coordinated sliver viewports for desktop applications.

The **Scrolling / Lists** category provides structural containers for displaying datasets that exceed available screen real estate. Powered by the **Anu Layout Engine** (`Overflow::Scroll`) and hardware-accelerated **Skia 2D rendering** with automatic subpixel clipping, every component in this category is built for  responsiveness with full mouse wheel, trackpad pan, and touch gesture support.

---

## Widget Catalog (Scrolling / Lists)

### Standard Scrolling Primitives
| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**ScrollView**](./scroll_view.md) | `struct ScrollView`, `scrollView(...)` | `<enki/widgets/scroll_view.hpp>` | Base scrollable viewport supporting vertical/horizontal pan, mouse wheel speed, and clipping. |
| 2 | [**ListView**](./list_view.md) | `struct ListView`, `listView(...)` | `<enki/widgets/list_view.hpp>` | Linear list supporting static items, lazy on-demand `item_builder`, separators, and physics. |
| 3 | [**GridView**](./grid_view.md) | `struct GridView`, `gridView(...)` | `<enki/widgets/grid_view.hpp>` | 2D scrollable cell grid with fixed-column counts or responsive max-extent auto-wrapping. |
| 4 | [**ListTile**](./list_tile.md) | `struct ListTile`, `listTile(...)` | `<enki/widgets/list_tile.hpp>` | Standardized list row with `leading`, `title`, `subtitle`, `trailing` slots, and hover/press effects. |
| 5 | [**GridTile**](./grid_tile.md) | `struct GridTile`, `struct GridTileBar` | `<enki/widgets/grid_tile.hpp>` | Stack-based cell container with semi-transparent title/footer scrim overlays for media galleries. |
| 6 | [**TreeView**](./tree_view.md) | `struct TreeView`, `struct TreeNodeData` | `<enki/widgets/tree_view.hpp>` | Hierarchical collapsible tree view with connector lines, multi-select, and lazy child loading. |
| 7 | [**Table**](./table.md) | `struct Table`, `struct TableRow` | `<enki/widgets/table.hpp>` | Fixed/flex column tabular grid with per-cell alignment and inner/outer border painting. |
| 8 | [**DataTable**](./data_table.md) | `struct DataTable`, `struct DataColumn` | `<enki/widgets/data_table.hpp>` | Enterprise data grid with column sort arrows, multi-row checkbox selection, and editable cells. |
| 9 | [**Scrollbar**](./scrollbar.md) | Built-in via `show_scrollbar = true` | `<enki/widgets/scroll_view.hpp>` | Integrated scrollbar thumb rendering with proportional sizing and smooth canvas clipping. |
| 10 | [**ReorderableList**](./reorderable_list.md) | `struct ReorderableList` | `<enki/widgets/reorderable_list.hpp>` | Interactive drag-and-drop reordering list with live displacement preview and drop line glowing. |

### Extended Scrolling (Sliver Subsystem)
| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 11 | [**CustomScrollView**](./custom_scroll_view.md) | `struct CustomScrollView`, `customScrollView(...)` | `<enki/widgets/sliver.hpp>` | Unified viewport coordinating multiple heterogeneous sliver components in a single scroll axis. |
| 12 | [**SliverAppBar**](./sliver_app_bar.md) | `struct SliverAppBar`, `sliverAppBar(...)` | `<enki/widgets/sliver.hpp>` | Collapsible, pinned, or floating header with flexible space, actions, and elevation. |
| 13 | [**SliverList**](./sliver_list.md) | `struct SliverList`, `sliverList(...)` | `<enki/widgets/sliver.hpp>` | Virtualized linear list sliver supporting lazy `item_builder` and dividers. |
| 14 | [**SliverGrid**](./sliver_grid.md) | `struct SliverGrid`, `sliverGrid(...)` | `<enki/widgets/sliver.hpp>` | 2D grid sliver supporting fixed cross-axis counts or responsive max-extent sizing. |

---

## Scrolling Architecture in Enki

Scrolling in Enki is divided cleanly between the layout stage and the paint stage:

```
┌─────────────────────────────────────────────────────────────┐
│                       ScrollViewWidget                      │
│                  (options: ScrollOptions)                   │
└──────────────────────────────┬──────────────────────────────┘
                               │
            ┌──────────────────┴──────────────────┐
            ▼                                     ▼
┌───────────────────────────────┐     ┌───────────────────────┐
│     Anu Layout (Flexbox)      │     │  Skia Paint Pipeline  │
│ - Content sized by Anu        │     │ - Canvas clipRect()   │
│ - max_scroll_x / max_scroll_y │     │ - Point scroll_shift  │
│ - Overflow::Scroll            │     │ - drawRRect scrollbar │
└───────────────────────────────┘     └───────────────────────┘
```

1. **Layout Pass**: Anu calculates children dimensions. The difference between child bounds and the viewport establishes `max_scroll_x` and `max_scroll_y`.
2. **Hit Testing & Input**: `RenderScrollView` captures `handlePointerScroll(dx, dy)` and touch drag gestures via `PanGestureRecognizer`.
3. **Paint Pass**: The canvas applies `context.canvas.clipRect(viewport_bounds)` and draws children shifted by `(-scroll_offset_x, -scroll_offset_y)`. If `show_scrollbar = true`, a rounded thumb is drawn along the active edge.

---

## Slivers Architecture (`<enki/widgets/sliver.hpp>`)

Slivers are portions of a scrollable area that can be composed together inside a `CustomScrollView`:

```
┌─────────────────────────────────────────────────────────────┐
│                      CustomScrollView                       │
├─────────────────────────────────────────────────────────────┤
│ 1. SliverAppBar (Collapsible & Pinned to top)               │
├─────────────────────────────────────────────────────────────┤
│ 2. SliverToBoxAdapter (Static Header / Filter Toolbar)      │
├─────────────────────────────────────────────────────────────┤
│ 3. SliverGrid (Multi-column responsive grid tiles)          │
├─────────────────────────────────────────────────────────────┤
│ 4. SliverList (Virtualized infinite lazy stream)            │
└─────────────────────────────────────────────────────────────┘
```

---

## Quick Example (CustomScrollView with SliverAppBar & SliverList)

```cpp
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSliverShowcase() {
    return CustomScrollView {
        .slivers = {
            SliverAppBar {
                .title = text("Feed Explorer", { .color = 0xFFFFFFFF, .font_weight = FontWeight::Bold }),
                .expanded_height = 160.0f,
                .collapsed_height = 56.0f,
                .pinned = true,
                .background_color = 0xFF1E1B4B
            },
            SliverList {
                .item_count = 30,
                .item_builder = [](int idx) -> WidgetPtr {
                    return container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(8.0f),
                        .padding = EdgeInsets::all(14.0f),
                        .child = text("Sliver Item #" + std::to_string(idx + 1), { .color = 0xFFFFFFFF })
                    });
                },
                .separator_builder = [](int) { return sizedBox(0.0f, 6.0f); }
            }
        }
    };
}
```
