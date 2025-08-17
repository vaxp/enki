#pragma once
/// @file list_view.hpp
/// @brief ListView widget — a scrollable, optionally-virtualized vertical/horizontal list.
///
/// Features:
///   - Two build modes:
///       a) Static mode:   `items` vector of WidgetPtrs — simple, straightforward.
///       b) Builder mode:  `itemCount` + `itemBuilder` — lazy construction, suitable for
///          large datasets (thousands of rows). Only visible items are built.
///   - Optional `separatorBuilder` for custom dividers between items.
///   - Full ScrollView integration (direction, scroll speed, overscroll clamping).
///   - `shrinkWrap`: if true, ListView sizes itself to its children (useful inside Column).
///   - `padding` around the list content.
///   - Selection support: single-select with `onItemSelected` callback.
///   - `physics` enum for controlling overscroll behavior.
///   - Primary / non-primary scroll views for nested scroll coordination.
///   - All layout via Anu — zero manual dimension calculations.
///
/// Architecture:
///   ListView (StatefulWidget)
///     └── build() → ScrollView
///                     └── Column/Row (FlexDirection driven by `direction`)
///                           ├── [separator]?
///                           ├── item[0]
///                           ├── [separator]?
///                           ├── item[1]
///                           └── ...
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/scroll_view.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ScrollPhysics
// ════════════════════════════════════════════════════════════════

/// @brief Controls overscroll/bounce behavior.
enum class ScrollPhysics {
    /// Clamp scroll at boundaries — no bounce (default for desktop).
    Clamped,
    /// Allow a spring-like bounce at the boundaries.
    Bouncing,
    /// Disable all scrolling.
    Never,
    /// Inherit the physics from a parent scrollable.
    Inherited,
};

// ════════════════════════════════════════════════════════════════
// ListView
// ════════════════════════════════════════════════════════════════

/// @brief A scrollable list of widgets, built statically or lazily via a builder function.
///
/// Usage (static):
/// @code
///   listView({
///       listTile()->title(text("Item 1")),
///       listTile()->title(text("Item 2")),
///   })
///   ->padding(EdgeInsets::all(8))
///   ->onItemSelected([](int idx){ /* ... */ });
/// @endcode
///
/// Usage (builder — efficient for large data):
/// @code
///   listView(100, [&](int index) -> WidgetPtr {
///       return listTile()->title(text("Row " + std::to_string(index)));
///   })
///   ->separated([](int index) -> WidgetPtr { return divider(); });
/// @endcode
struct ListViewProps {
    Key key = Key::none();
    std::vector<WidgetPtr> items;
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder;
    std::function<WidgetPtr(int index)> separator_builder;
    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;
    EdgeInsets list_padding = EdgeInsets{};
    bool shrink_wrap = false;
    bool is_reversed = false;
    bool primary = true;
    std::optional<int> selected_index;
    std::function<void(int index)> on_item_selected;
    std::function<void()> on_scroll_start;
    std::function<void(float offset)> on_scroll;
    std::function<void()> on_scroll_end;
};

class ListView : public StatefulWidget {
public:
    ListViewProps props;

    ListView() = default;

    explicit ListView(ListViewProps p) : props(std::move(p)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    ListView& separated(std::function<WidgetPtr(int)> sep) {
        props.separator_builder = std::move(sep);
        return *this;
    }

    ListView& padding(EdgeInsets p)   { props.list_padding = p; return *this; }
    ListView& paddingAll(float p)     { props.list_padding = EdgeInsets::all(p); return *this; }
    ListView& horizontal()            { props.direction = Axis::Horizontal; return *this; }
    ListView& vertical()              { props.direction = Axis::Vertical; return *this; }
    ListView& reverse(bool r = true)  { props.is_reversed = r; return *this; }
    ListView& shrinkWrap(bool s = true) { props.shrink_wrap = s; return *this; }

    ListView& scrollSpeed(float s)   { props.scroll_speed = s; return *this; }
    ListView& physics(ScrollPhysics p) { props.scroll_physics = p; return *this; }

    ListView& selectedIndex(int idx)  { props.selected_index = idx; return *this; }
    ListView& onItemSelected(std::function<void(int)> cb) {
        props.on_item_selected = std::move(cb);
        return *this;
    }
    ListView& onScroll(std::function<void(float)> cb) {
        props.on_scroll = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ListView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// Create a static ListView from a vector of widgets.
inline std::shared_ptr<ListView> listView(ListViewProps props = {}) {
    return std::make_shared<ListView>(std::move(props));
}

/// Create a static ListView from a vector of widgets.
inline std::shared_ptr<ListView> listView(std::vector<WidgetPtr> items) {
    ListViewProps props;
    props.items = std::move(items);
    return std::make_shared<ListView>(std::move(props));
}

/// Create a static ListView from an initializer list.
inline std::shared_ptr<ListView> listView(std::initializer_list<WidgetPtr> items) {
    ListViewProps props;
    props.items = std::vector<WidgetPtr>(items);
    return std::make_shared<ListView>(std::move(props));
}

/// Create a lazy-builder ListView for large datasets.
inline std::shared_ptr<ListView> listView(int count, std::function<WidgetPtr(int)> builder) {
    ListViewProps props;
    props.item_count = count;
    props.item_builder = std::move(builder);
    return std::make_shared<ListView>(std::move(props));
}

/// Create an empty ListView for building via fluent API.
inline std::shared_ptr<ListView> listView() {
    return std::make_shared<ListView>(ListViewProps{});
}

/// Convenience: separated list with a divider between every item.
inline std::shared_ptr<ListView> listViewSeparated(
    std::vector<WidgetPtr> items,
    std::function<WidgetPtr(int)> separator)
{
    ListViewProps props;
    props.items = std::move(items);
    props.separator_builder = std::move(separator);
    return std::make_shared<ListView>(std::move(props));
}

} // namespace enki
