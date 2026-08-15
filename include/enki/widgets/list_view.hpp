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
class ListView : public StatefulWidget {
public:
    // ── Build mode A: static items ─────────────────────────────
    std::vector<WidgetPtr> items;

    // ── Build mode B: lazy builder ─────────────────────────────
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder;

    // ── Optional separator between items ───────────────────────
    /// Return nullptr to skip the separator at a given index.
    std::function<WidgetPtr(int index)> separator_builder;

    // ── Scroll configuration ────────────────────────────────────
    Axis       direction     = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float      scroll_speed  = 50.0f;

    // ── Layout ─────────────────────────────────────────────────
    EdgeInsets list_padding  = EdgeInsets{};
    bool       shrink_wrap   = false;   ///< Size to content height (disables infinite scroll).
    bool       is_reversed    = false;  ///< Reverse the scroll direction.
    bool       primary        = true;   ///< Whether this is the primary scroll view.

    // ── Selection ──────────────────────────────────────────────
    std::optional<int> selected_index;
    std::function<void(int index)> on_item_selected;

    // ── Callbacks ──────────────────────────────────────────────
    std::function<void()> on_scroll_start;
    std::function<void(float offset)> on_scroll;
    std::function<void()> on_scroll_end;

    // ─────────────────────────────────────────────────────────
    ListView() = default;

    /// Static list constructor.
    explicit ListView(std::vector<WidgetPtr> items) : items(std::move(items)) {}

    /// Builder-mode constructor.
    ListView(int count, std::function<WidgetPtr(int)> builder)
        : item_count(count), item_builder(std::move(builder)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    ListView& separated(std::function<WidgetPtr(int)> sep) {
        separator_builder = std::move(sep);
        return *this;
    }

    ListView& padding(EdgeInsets p)   { this->list_padding = p; return *this; }
    ListView& paddingAll(float p)     { this->list_padding = EdgeInsets::all(p); return *this; }
    ListView& horizontal()            { direction = Axis::Horizontal; return *this; }
    ListView& vertical()              { direction = Axis::Vertical; return *this; }
    ListView& reverse(bool r = true)  { this->is_reversed = r; return *this; }
    ListView& shrinkWrap(bool s = true) { shrink_wrap = s; return *this; }

    ListView& scrollSpeed(float s)   { scroll_speed = s; return *this; }
    ListView& physics(ScrollPhysics p) { this->scroll_physics = p; return *this; }

    ListView& selectedIndex(int idx)  { selected_index = idx; return *this; }
    ListView& onItemSelected(std::function<void(int)> cb) {
        on_item_selected = std::move(cb);
        return *this;
    }
    ListView& onScroll(std::function<void(float)> cb) {
        on_scroll = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ListView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// Create a static ListView from a vector of widgets.
inline std::shared_ptr<ListView> listView(std::vector<WidgetPtr> items) {
    return std::make_shared<ListView>(std::move(items));
}

/// Create a static ListView from an initializer list.
inline std::shared_ptr<ListView> listView(std::initializer_list<WidgetPtr> items) {
    return std::make_shared<ListView>(std::vector<WidgetPtr>(items));
}

/// Create a lazy-builder ListView for large datasets.
inline std::shared_ptr<ListView> listView(int count, std::function<WidgetPtr(int)> builder) {
    return std::make_shared<ListView>(count, std::move(builder));
}

/// Create an empty ListView for building via fluent API.
inline std::shared_ptr<ListView> listView() {
    return std::make_shared<ListView>();
}

/// Convenience: separated list with a divider between every item.
inline std::shared_ptr<ListView> listViewSeparated(
    std::vector<WidgetPtr> items,
    std::function<WidgetPtr(int)> separator)
{
    auto lv = std::make_shared<ListView>(std::move(items));
    lv->separator_builder = std::move(separator);
    return lv;
}

} // namespace enki
