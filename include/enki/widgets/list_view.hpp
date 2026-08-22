#pragma once
/// @file list_view.hpp
/// @brief ListView widget — a scrollable, optionally-virtualized vertical/horizontal list.
///
/// Features:
///   - Two build modes:
///       a) Static mode:   `items` vector of WidgetPtrs — simple, straightforward.
///       b) Builder mode:  `item_count` + `item_builder` — lazy construction, suitable for
///          large datasets (thousands of rows). Only visible items are built.
///   - Optional `separator_builder` for custom dividers between items.
///   - Full ScrollView integration (direction, scroll speed, overscroll clamping).
///   - `shrink_wrap`: if true, ListView sizes itself to its children (useful inside Column).
///   - `padding` around the list content.
///   - Selection support: single-select with `on_item_selected` callback.
///   - `physics` enum for controlling overscroll behavior.
///   - Primary / non-primary scroll views for nested scroll coordination.
///   - All layout via Anu — zero manual dimension calculations.
///
/// Architecture:
///   ListViewWidget (StatefulWidget)
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
// ListViewProps
// ════════════════════════════════════════════════════════════════

struct ListViewProps {
    Key key = Key::none();
    std::vector<WidgetPtr> items;
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    std::function<WidgetPtr(int index)> separator_builder = nullptr;
    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;
    EdgeInsets list_padding = EdgeInsets{};
    bool shrink_wrap = false;
    bool is_reversed = false;
    bool primary = true;
    std::optional<int> selected_index = std::nullopt;
    std::function<void(int index)> on_item_selected = nullptr;
    std::function<void()> on_scroll_start = nullptr;
    std::function<void(float offset)> on_scroll = nullptr;
    std::function<void()> on_scroll_end = nullptr;
};

// ════════════════════════════════════════════════════════════════
// ListViewWidget
// ════════════════════════════════════════════════════════════════

class ListViewWidget : public StatefulWidget {
public:
    ListViewProps props;

    ListViewWidget() = default;
    explicit ListViewWidget(ListViewProps p) : StatefulWidget(p.key), props(std::move(p)) {}
    ListViewWidget(Key key, ListViewProps p) : StatefulWidget(std::move(key)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ListView"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative ListView Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct ListView {
    Key key = Key::none();
    std::vector<WidgetPtr> items = {};
    std::vector<WidgetPtr> children = {}; // alias
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    std::function<WidgetPtr(int index)> separator_builder = nullptr;
    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;
    EdgeInsets list_padding = EdgeInsets{};
    EdgeInsets padding = EdgeInsets{}; // alias
    bool shrink_wrap = false;
    bool is_reversed = false;
    bool reverse = false; // alias
    bool primary = true;
    std::optional<int> selected_index = std::nullopt;
    std::function<void(int index)> on_item_selected = nullptr;
    std::function<void()> on_scroll_start = nullptr;
    std::function<void(float offset)> on_scroll = nullptr;
    std::function<void()> on_scroll_end = nullptr;

    operator WidgetPtr() const {
        ListViewProps p;
        p.key = key;
        p.items = !children.empty() ? children : items;
        p.item_count = item_count;
        p.item_builder = item_builder;
        p.separator_builder = separator_builder;
        p.direction = direction;
        p.scroll_physics = scroll_physics;
        p.scroll_speed = scroll_speed;
        p.list_padding = (padding != EdgeInsets{}) ? padding : list_padding;
        p.shrink_wrap = shrink_wrap;
        p.is_reversed = reverse || is_reversed;
        p.primary = primary;
        p.selected_index = selected_index;
        p.on_item_selected = on_item_selected;
        p.on_scroll_start = on_scroll_start;
        p.on_scroll = on_scroll;
        p.on_scroll_end = on_scroll_end;
        return std::make_shared<ListViewWidget>(key, std::move(p));
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<ListViewWidget> listView(ListViewProps props = {}) {
    return std::make_shared<ListViewWidget>(std::move(props));
}

inline std::shared_ptr<ListViewWidget> listView(std::vector<WidgetPtr> items) {
    ListViewProps props;
    props.items = std::move(items);
    return std::make_shared<ListViewWidget>(std::move(props));
}

inline std::shared_ptr<ListViewWidget> listView(std::initializer_list<WidgetPtr> items) {
    ListViewProps props;
    props.items = std::vector<WidgetPtr>(items);
    return std::make_shared<ListViewWidget>(std::move(props));
}

inline std::shared_ptr<ListViewWidget> listView(int count, std::function<WidgetPtr(int)> builder) {
    ListViewProps props;
    props.item_count = count;
    props.item_builder = std::move(builder);
    return std::make_shared<ListViewWidget>(std::move(props));
}

inline std::shared_ptr<ListViewWidget> listViewSeparated(
    std::vector<WidgetPtr> items,
    std::function<WidgetPtr(int)> separator)
{
    ListViewProps props;
    props.items = std::move(items);
    props.separator_builder = std::move(separator);
    return std::make_shared<ListViewWidget>(std::move(props));
}

} // namespace enki
