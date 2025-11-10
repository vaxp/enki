# Enki Advanced & Data UI Suite

> Professional enterprise data grids, chronological timelines, interactive calendars, swiper carousels, expandable accordions, multi-step expansion panels, draggable split views, and freeform resizable panels.

The **Advanced / Data UI** category brings enterprise-grade presentation, scheduling, and workspace layout management to the Enki GUI Framework. Built for complex data-dense workflows, developer tools, and rich dashboards, these widgets incorporate column resizing, multi-column sorting, drag handles, animation controllers, and stateful controllers.

---

## Widget Catalog (Advanced / Data UI)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**DataGrid**](./data_grid.md) | `struct DataGrid`, `DataGridColumn` | `<enki/widgets/data_grid.hpp>` | High-performance enterprise table with column pinning, sorting, resizing, pagination, and CSV export. |
| 2 | [**Timeline**](./timeline.md) | `struct Timeline`, `TimelineItem` | `<enki/widgets/timeline.hpp>` | Chronological event stream and multi-stage stepper with zig-zag alternate alignments. |
| 3 | [**Calendar**](./calendar.md) | `struct Calendar`, `CalendarEvent` | `<enki/widgets/calendar.hpp>` | Month/year scheduling calendar with single/range selection, event dot markers, and agenda views. |
| 4 | [**Carousel**](./carousel.md) | `struct Carousel`, `CarouselController` | `<enki/widgets/carousel.hpp>` | Full-featured slider with autoplay, pause-on-hover, infinite looping, and dot indicators. |
| 5 | [**Accordion**](./accordion.md) | `struct Accordion`, `AccordionItem` | `<enki/widgets/accordion.hpp>` | Collapsible sections supporting single/multiple modes and Bordered, Separated, or Flush styles. |
| 6 | [**ExpansionPanel**](./expansion_panel.md) | `struct ExpansionPanelList`, `ExpansionPanelItem` | `<enki/widgets/expansion_panel.hpp>` | Stepper and workflow panel list with dynamic elevation gaps and footer action bars. |
| 7 | [**SplitView**](./split_view.md) | `struct SplitView`, `SplitViewOptions` | `<enki/widgets/split_view.hpp>` | Horizontal/vertical draggable pane divider with minimum constraints and snap-to-collapse. |
| 8 | [**ResizablePanel**](./resizable_panel.md) | `struct ResizablePanel`, `ResizablePanelController` | `<enki/widgets/resizable_panel.hpp>` | Floating tool window with multi-edge/corner drag resizing and drag-to-move header. |

---

## Architectural Highlights

- **Data Virtualization & High Performance**: `DataGrid` efficiently manages large tabular datasets with zebra striping, custom formatters, badge cells, and progress bars.
- **Precision Drag Resizing**: `SplitView` and `ResizablePanel` provide sub-pixel mouse dragging, cursor feedback (`SystemCursor::ResizeLeftRight` / `ResizeUpDown`), and boundary collision handling.
- **Workflow State Machines**: `Timeline` and `ExpansionPanel` seamlessly track multi-step asynchronous deployment pipelines, order statuses, and checkout flows.

---

## Quick Example (IDE Split Workspace with DataGrid)

```cpp
#include "enki/widgets/split_view.hpp"
#include "enki/widgets/data_grid.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr buildWorkspaceView() {
    auto gridCtrl = std::make_shared<DataGridController>();

    // Left pane: Project Tree
    auto sidebar = container({
        .color = 0xFF0F172A,
        .padding = EdgeInsets::all(16.0f),
        .child = text("📁 Workspace Project Explorer", { .color = 0xFF94A3B8 })
    });

    // Right pane: Server Cluster DataGrid
    auto table = DataGrid {
        .controller = gridCtrl,
        .zebra_stripes = true,
        .show_pagination = true,
        .show_quick_filter = true
    };

    return SplitView {
        .leading = sidebar,
        .trailing = table,
        .options = {
            .orientation = SplitOrientation::Horizontal,
            .initial_ratio = 0.25f,
            .min_leading_size = 180.0f,
            .min_trailing_size = 400.0f
        }
    };
}
```
