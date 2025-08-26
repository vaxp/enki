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
// GridView Widget Implementation
// ════════════════════════════════════════════════════════════════

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

class GridViewWidget : public StatefulWidget {
public:
    GridViewProps props;

    GridViewWidget() = default;
    explicit GridViewWidget(GridViewProps p) : StatefulWidget(p.key), props(std::move(p)) {}
    GridViewWidget(Key k, GridViewProps p) : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "GridView"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct GridView {
    Key key = Key::none();
    std::vector<WidgetPtr> items;
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;

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

    operator WidgetPtr() const {
        GridViewProps p;
        p.key = key;
        p.items = items;
        p.item_count = item_count;
        p.item_builder = item_builder;
        p.fixed_delegate = fixed_delegate;
        p.max_delegate = max_delegate;
        p.use_max_extent_delegate = use_max_extent_delegate;
        p.direction = direction;
        p.scroll_physics = scroll_physics;
        p.scroll_speed = scroll_speed;
        p.list_padding = list_padding;
        p.shrink_wrap = shrink_wrap;
        p.reverse = reverse;
        p.primary = primary;
        return std::make_shared<GridViewWidget>(key, std::move(p));
    }
};

} // namespace enki
