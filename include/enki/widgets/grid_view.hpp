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
class GridView : public StatefulWidget {
public:
    // ── Build mode A: static items ─────────────────────────────
    std::vector<WidgetPtr> items;

    // ── Build mode B: lazy builder ─────────────────────────────
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder;

    // ── Grid delegate (sizing strategy) ────────────────────────
    /// Defaults to a 2-column fixed-count grid.
    SliverGridDelegateFixedCount fixed_delegate  = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent  max_delegate;
    bool use_max_extent_delegate = false;  ///< Switch to max-extent mode.

    // ── Scroll ─────────────────────────────────────────────────
    Axis          direction    = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float         scroll_speed = 50.0f;

    // ── Layout ─────────────────────────────────────────────────
    EdgeInsets list_padding = EdgeInsets{};
    bool       shrink_wrap  = false;
    bool       reverse       = false;
    bool       primary       = true;

    // ─────────────────────────────────────────────────────────
    GridView() = default;

    /// Static grid — fixed column count.
    GridView(int cross_axis_count, std::vector<WidgetPtr> items)
        : items(std::move(items)) {
        fixed_delegate.cross_axis_count = cross_axis_count;
    }

    /// Builder grid — fixed column count.
    GridView(int cross_axis_count, int count, std::function<WidgetPtr(int)> builder)
        : item_count(count), item_builder(std::move(builder)) {
        fixed_delegate.cross_axis_count = cross_axis_count;
    }

    // ── Fluent Builder API ─────────────────────────────────────

    /// Switch to max-extent delegate mode.
    GridView& maxExtent(float max) {
        max_delegate.max_cross_axis_extent = max;
        use_max_extent_delegate = true;
        return *this;
    }

    GridView& crossAxisCount(int n)    { fixed_delegate.cross_axis_count = n; return *this; }
    GridView& crossAxisSpacing(float s) {
        fixed_delegate.cross_axis_spacing = s;
        max_delegate.cross_axis_spacing = s;
        return *this;
    }
    GridView& mainAxisSpacing(float s) {
        fixed_delegate.main_axis_spacing = s;
        max_delegate.main_axis_spacing = s;
        return *this;
    }
    GridView& childAspectRatio(float r) {
        fixed_delegate.child_aspect_ratio = r;
        max_delegate.child_aspect_ratio = r;
        return *this;
    }
    GridView& mainAxisExtent(float e) {
        fixed_delegate.main_axis_extent = e;
        max_delegate.main_axis_extent = e;
        return *this;
    }

    GridView& padding(EdgeInsets p)    { this->list_padding = p; return *this; }
    GridView& paddingAll(float p)      { this->list_padding = EdgeInsets::all(p); return *this; }
    GridView& horizontal()             { direction = Axis::Horizontal; return *this; }
    GridView& shrinkWrap(bool s = true){ shrink_wrap = s; return *this; }
    GridView& physics(ScrollPhysics p) { this->scroll_physics = p; return *this; }
    GridView& scrollSpeed(float s)     { scroll_speed = s; return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "GridView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// GridView with fixed column count — static items.
inline std::shared_ptr<GridView> gridView(int crossAxisCount, std::vector<WidgetPtr> items) {
    return std::make_shared<GridView>(crossAxisCount, std::move(items));
}

/// GridView with fixed column count — lazy builder.
inline std::shared_ptr<GridView> gridView(int crossAxisCount, int count,
                                           std::function<WidgetPtr(int)> builder) {
    return std::make_shared<GridView>(crossAxisCount, count, std::move(builder));
}

/// GridView with max-extent delegate — responsive, auto-columns — lazy builder.
inline std::shared_ptr<GridView> gridViewExtent(float maxExtent, int count,
                                                 std::function<WidgetPtr(int)> builder) {
    auto gv = std::make_shared<GridView>();
    gv->item_count    = count;
    gv->item_builder  = std::move(builder);
    gv->max_delegate.max_cross_axis_extent = maxExtent;
    gv->use_max_extent_delegate = true;
    return gv;
}

/// GridView with max-extent delegate — static items.
inline std::shared_ptr<GridView> gridViewExtent(float maxExtent, std::vector<WidgetPtr> items) {
    auto gv = std::make_shared<GridView>();
    gv->items = std::move(items);
    gv->max_delegate.max_cross_axis_extent = maxExtent;
    gv->use_max_extent_delegate = true;
    return gv;
}

} // namespace enki
