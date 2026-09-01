# GridView

> A two-dimensional scrollable grid widget supporting fixed column counts, auto-responsive max-extent sizing, aspect ratio constraints, and lazy cell builders.

- **Header File**: `#include "enki/widgets/grid_view.hpp"`
- **C++ Class**: `enki::GridViewWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::GridView` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::GridViewProps`
- **Sizing Delegates**: `enki::SliverGridDelegateFixedCount`, `enki::SliverGridDelegateMaxExtent`

---

## Overview

`GridView` arranges elements into a 2D layout. Rather than computing pixel positions manually, Enki delegates cell calculations to the **Anu Layout Engine** using `flex_wrap: Wrap` and dynamic `flex_basis` percentage calculation.

GridView supports two cell distribution strategies:
1. **Fixed Column Count (`SliverGridDelegateFixedCount`)**: Enforces an exact number of columns (e.g. 3 columns), distributing remaining width evenly.
2. **Responsive Max-Extent (`SliverGridDelegateMaxExtent`)**: Defines a maximum cell width (e.g. 180px). The grid automatically adds or removes columns based on viewport resize, making it naturally responsive.

---

## C++ API Definition

### Sizing Delegates
```cpp
namespace enki {

/// Fixed number of columns/rows across the cross axis
struct SliverGridDelegateFixedCount {
    int                  cross_axis_count   = 2;    ///< Number of columns (vertical)
    float                main_axis_spacing  = 0.0f; ///< Vertical gap between rows
    float                cross_axis_spacing = 0.0f; ///< Horizontal gap between columns
    float                child_aspect_ratio = 1.0f; ///< width / height per cell
    std::optional<float> main_axis_extent;          ///< Optional fixed cell height override

    SliverGridDelegateFixedCount(int count = 2,
                                 float main_spacing = 0.0f,
                                 float cross_spacing = 0.0f,
                                 float aspect_ratio = 1.0f);
};

/// Responsive grid sizing based on maximum cell width
struct SliverGridDelegateMaxExtent {
    float                max_cross_axis_extent = 200.0f; ///< Maximum cell width in pixels
    float                main_axis_spacing     = 0.0f;
    float                cross_axis_spacing    = 0.0f;
    float                child_aspect_ratio    = 1.0f;
    std::optional<float> main_axis_extent;

    SliverGridDelegateMaxExtent(float max_extent = 200.0f,
                                float main_spacing = 0.0f,
                                float cross_spacing = 0.0f,
                                float aspect_ratio = 1.0f);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct GridView {
    Key                                  key                     = Key::none();
    std::vector<WidgetPtr>               items                   = {};
    int                                  item_count              = 0;
    std::function<WidgetPtr(int index)>  item_builder            = nullptr;

    SliverGridDelegateFixedCount         fixed_delegate          = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent          max_delegate;
    bool                                 use_max_extent_delegate = false;

    Axis                                 direction               = Axis::Vertical;
    ScrollPhysics                        scroll_physics          = ScrollPhysics::Clamped;
    float                                scroll_speed            = 50.0f;
    EdgeInsets                           list_padding            = EdgeInsets{};
    bool                                 shrink_wrap             = false;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `fixed_delegate` | `SliverGridDelegateFixedCount` | `2 cols, 0 gap` | Fixed column count delegate configuration. |
| `max_delegate` | `SliverGridDelegateMaxExtent` | `200px max` | Responsive width delegate configuration. |
| `use_max_extent_delegate` | `bool` | `false` | When true, activates `max_delegate` instead of `fixed_delegate`. |
| `item_count` | `int` | `0` | Total cell count when using `item_builder`. |
| `item_builder` | `Function(int)` | `nullptr` | Lazy factory lambda invoked for each visible cell. |
| `list_padding` | `EdgeInsets` | `{}` | Inset padding around the entire grid. |
| `shrink_wrap` | `bool` | `false` | Sizes grid height to its children when nested inside flex containers. |

---

## Code Examples (From `widgets_demo/grid_view_demo/main.cpp`)

### 1. Fixed 3-Column Photo Gallery (Aspect Ratio 1:1)
```cpp
#include "enki/widgets/grid_view.hpp"
#include "enki/widgets/grid_tile.hpp"

using namespace enki;

WidgetPtr buildFixedPhotoGrid(const std::vector<WidgetPtr>& photos) {
    return GridView {
        .item_count = static_cast<int>(photos.size()),
        .item_builder = [&photos](int index) -> WidgetPtr {
            return GridTile {
                .child = photos[index],
                .footer = GridTileBar {
                    .title = text("Photo #" + std::to_string(index + 1)),
                }
            };
        },
        .fixed_delegate = SliverGridDelegateFixedCount(
            3,      // 3 columns
            8.0f,   // 8px vertical row gap
            8.0f,   // 8px horizontal column gap
            1.0f    // Square aspect ratio (1:1)
        ),
        .list_padding = EdgeInsets::all(8.0f),
    };
}
```

### 2. Auto-Responsive Card Grid
```cpp
WidgetPtr buildResponsiveDashboard(int totalCards) {
    return GridView {
        .item_count = totalCards,
        .item_builder = [](int i) -> WidgetPtr {
            return buildDashboardCard(i);
        },
        .use_max_extent_delegate = true,
        .max_delegate = SliverGridDelegateMaxExtent(
            180.0f, // Each card is at most 180px wide
            12.0f,  // 12px main spacing
            12.0f,  // 12px cross spacing
            4.0f / 3.0f // 4:3 aspect ratio
        ),
        .list_padding = EdgeInsets::all(16.0f),
    };
}
```

---

## See Also
- [**GridTile**](./grid_tile.md) — Layered cell container with title/subtitle scrim overlay.
- [**ListView**](./list_view.md) — 1D linear list view.
- [**ScrollView**](./scroll_view.md) — Base scrolling viewport.
