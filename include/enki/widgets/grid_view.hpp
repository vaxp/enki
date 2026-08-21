#pragma once
/// @file grid_view.hpp
/// @brief GridView widget — a scrollable grid of uniformly or variably sized cells.
///
/// Features:
///   - Two grid delegation modes:
///       a) FixedCrossAxisCount: a fixed number of columns/rows across the cross axis.
///       b) MaxCrossAxisExtent: cells are automatically sized to fit within a maximum
///          extent — making the grid responsive to available space.
///   - Static items or lazy builder (itemCount + itemBuilder) for large datasets.
///   - mainAxisSpacing and crossAxisSpacing via Anu gap properties.
///   - childAspectRatio to control item proportions.
///   - mainAxisExtent for fixed-size cells in the main axis.
///   - Scrolls vertically (default) or horizontally.
///   - shrinkWrap for embedding inside Column/Row.
///   - padding around grid content.
///   - All layout via Anu — flex_wrap + flex_basis handle grid math.
///
/// Architecture:
///   GridView (StatefulWidget)
///     └── build() → ScrollView (Overflow::Scroll, direction)
///                     └── Flexbox (FlexWrap::Wrap, gap = crossAxisSpacing)
///                           ├── FlexItem (flex_basis = cellWidth) → item[0]
///                           ├── FlexItem → item[1]
///                           └── ...
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/list_view.hpp"   // ScrollPhysics
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Grid Delegate — describes cell sizing strategy
// ════════════════════════════════════════════════════════════════

/// @brief Abstract base for grid sizing strategies.
struct GridDelegate {
    virtual ~GridDelegate() = default;
};

/// @brief Grid with a fixed number of cells in the cross axis.
///
/// Each cell occupies `1 / crossAxisCount` of the available cross-axis space,
/// minus spacing. Anu receives `flex_basis = (100% - gaps) / count` as percent.
struct SliverGridDelegateFixedCount : public GridDelegate {
    int   cross_axis_count = 2;       ///< Number of columns (vertical) or rows (horizontal).
    float main_axis_spacing = 0.0f;   ///< Gap between rows (vertical) / columns (horizontal).
    float cross_axis_spacing = 0.0f;  ///< Gap between columns (vertical) / rows (horizontal).
    float child_aspect_ratio = 1.0f;  ///< width / height per cell.
    /// If set, overrides aspect ratio and fixes the main-axis size in pixels.
    std::optional<float> main_axis_extent;

    SliverGridDelegateFixedCount() = default;
    explicit SliverGridDelegateFixedCount(int count,
                                          float main_spacing = 0.0f,
                                          float cross_spacing = 0.0f,
                                          float aspect_ratio = 1.0f)
        : cross_axis_count(count),
          main_axis_spacing(main_spacing),
          cross_axis_spacing(cross_spacing),
          child_aspect_ratio(aspect_ratio) {}
};

/// @brief Grid where each cell has a maximum cross-axis extent.
///
/// The number of columns is computed as `floor(availableWidth / maxExtent)`,
/// then cells are stretched to fill evenly. This enables responsive grids.
struct SliverGridDelegateMaxExtent : public GridDelegate {
    float max_cross_axis_extent = 200.0f; ///< Maximum cell width (vertical grid).
    float main_axis_spacing = 0.0f;
    float cross_axis_spacing = 0.0f;
    float child_aspect_ratio = 1.0f;
    std::optional<float> main_axis_extent;

    SliverGridDelegateMaxExtent() = default;
    explicit SliverGridDelegateMaxExtent(float max_extent,
                                         float main_spacing = 0.0f,
                                         float cross_spacing = 0.0f,
                                         float aspect_ratio = 1.0f)
        : max_cross_axis_extent(max_extent),
          main_axis_spacing(main_spacing),
          cross_axis_spacing(cross_spacing),
          child_aspect_ratio(aspect_ratio) {}
};

// ════════════════════════════════════════════════════════════════
// GridView Widget
// ════════════════════════════════════════════════════════════════

/// @brief A scrollable 2D grid of widgets.
///
/// Usage (fixed columns):
/// @code
///   gridView(3, photos.size(), [&](int i) {
///       return gridTile(
///           image(photos[i]),
///           gridTileBar()->title(text(captions[i]))
///       );
///   })
///   ->crossAxisSpacing(8)
///   ->mainAxisSpacing(8)
///   ->padding(EdgeInsets::all(8));
/// @endcode
///
/// Usage (max-extent / responsive):
/// @code
///   gridViewExtent(200.0f, cards.size(), [&](int i){
///       return cards[i];
///   })
///   ->crossAxisSpacing(12)
///   ->mainAxisSpacing(12);
/// @endcode
struct GridViewProps {
    Key key = Key::none();
    std::vector<WidgetPtr> items;
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder;

    SliverGridDelegateFixedCount fixed_delegate = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent max_delegate;
    bool use_max_extent_delegate = false;

    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;

    EdgeInsets list_padding = EdgeInsets{};
    bool shrink_wrap = false;
    bool reverse = false;
    bool primary = true;
};

class GridView : public StatefulWidget {
public:
    GridViewProps props;

    GridView() = default;
    explicit GridView(GridViewProps p) : props(std::move(p)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    GridView& maxExtent(float max) {
        props.max_delegate.max_cross_axis_extent = max;
        props.use_max_extent_delegate = true;
        return *this;
    }

    GridView& crossAxisCount(int n)    { props.fixed_delegate.cross_axis_count = n; return *this; }
    GridView& crossAxisSpacing(float s) {
        props.fixed_delegate.cross_axis_spacing = s;
        props.max_delegate.cross_axis_spacing = s;
        return *this;
    }
    GridView& mainAxisSpacing(float s) {
        props.fixed_delegate.main_axis_spacing = s;
        props.max_delegate.main_axis_spacing = s;
        return *this;
    }
    GridView& childAspectRatio(float r) {
        props.fixed_delegate.child_aspect_ratio = r;
        props.max_delegate.child_aspect_ratio = r;
        return *this;
    }
    GridView& mainAxisExtent(float e) {
        props.fixed_delegate.main_axis_extent = e;
        props.max_delegate.main_axis_extent = e;
        return *this;
    }

    GridView& padding(EdgeInsets p)    { props.list_padding = p; return *this; }
    GridView& paddingAll(float p)      { props.list_padding = EdgeInsets::all(p); return *this; }
    GridView& horizontal()             { props.direction = Axis::Horizontal; return *this; }
    GridView& shrinkWrap(bool s = true){ props.shrink_wrap = s; return *this; }
    GridView& physics(ScrollPhysics p) { props.scroll_physics = p; return *this; }
    GridView& scrollSpeed(float s)     { props.scroll_speed = s; return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "GridView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<GridView> gridView(GridViewProps props = {}) {
    return std::make_shared<GridView>(std::move(props));
}

/// GridView with fixed column count — static items.
inline std::shared_ptr<GridView> gridView(int crossAxisCount, std::vector<WidgetPtr> items) {
    GridViewProps props;
    props.fixed_delegate.cross_axis_count = crossAxisCount;
    props.items = std::move(items);
    return std::make_shared<GridView>(std::move(props));
}

/// GridView with fixed column count — lazy builder.
inline std::shared_ptr<GridView> gridView(int crossAxisCount, int count,
                                           std::function<WidgetPtr(int)> builder) {
    GridViewProps props;
    props.fixed_delegate.cross_axis_count = crossAxisCount;
    props.item_count = count;
    props.item_builder = std::move(builder);
    return std::make_shared<GridView>(std::move(props));
}

/// GridView with max-extent delegate — responsive, auto-columns — lazy builder.
inline std::shared_ptr<GridView> gridViewExtent(float maxExtent, int count,
                                                 std::function<WidgetPtr(int)> builder) {
    GridViewProps props;
    props.item_count = count;
    props.item_builder = std::move(builder);
    props.max_delegate.max_cross_axis_extent = maxExtent;
    props.use_max_extent_delegate = true;
    return std::make_shared<GridView>(std::move(props));
}

/// GridView with max-extent delegate — static items.
inline std::shared_ptr<GridView> gridViewExtent(float maxExtent, std::vector<WidgetPtr> items) {
    GridViewProps props;
    props.items = std::move(items);
    props.max_delegate.max_cross_axis_extent = maxExtent;
    props.use_max_extent_delegate = true;
    return std::make_shared<GridView>(std::move(props));
}

} // namespace enki
