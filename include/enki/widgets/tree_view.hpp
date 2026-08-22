#pragma once
/// @file tree_view.hpp
/// @brief TreeView widget — a hierarchical, expandable tree for desktop file browsers,
///        settings trees, org charts, and any nested data structure.
///
/// Features:
///   - Arbitrary depth tree with expand / collapse animation.
///   - Single or multi-selection with keyboard navigation.
///   - Custom node builder for complete visual control per node.
///   - Built-in default node appearance (icon + label + arrow) that matches
///     the ENKI desktop aesthetic.
///   - Connector lines between nodes (toggle via showLines).
///   - Configurable indent width per depth level.
///   - Lazy child loading: `on_children_requested` callback for async subtrees.
///   - Drag & drop integration hooks (on_node_drag_start, on_node_drop).
///   - Checkable nodes for multi-select with tri-state (indeterminate) support.
///   - Context menu hook per node.
///   - All layout delegated to Anu — the tree is a Column of padded rows.
///   - Integrated in a ScrollView for overflow content.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/list_view.hpp"   // ScrollPhysics
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// TreeNodeData — declarative node descriptor
// ════════════════════════════════════════════════════════════════

/// @brief Describes a single node in the tree, including its children.
///
/// Nodes are identified by a string `id`. IDs must be unique across the
/// entire tree for correct expand/select state tracking.
struct TreeNodeData {
    std::string id;                         ///< Unique identifier for this node.
    WidgetPtr   label;                      ///< Required: displayed label widget.
    WidgetPtr   leading_icon;               ///< Optional: icon to the left of the label.
    WidgetPtr   trailing;                   ///< Optional: widget on the right (badge, action).

    std::vector<TreeNodeData> children;     ///< Child nodes (empty = leaf node).

    bool initially_expanded = false;        ///< Whether the node starts expanded.
    bool selectable  = true;                ///< Whether the node can be selected.
    bool checkable   = false;               ///< Whether the node shows a checkbox.
    bool checked     = false;               ///< Initial checkbox state.
    bool loading     = false;               ///< Show a loading spinner instead of children.
    bool disabled    = false;               ///< Grayed out and non-interactive.

    /// Optional: user payload. Not owned; caller manages lifetime.
    void* user_data = nullptr;

    TreeNodeData() = default;

    TreeNodeData(std::string id, WidgetPtr label,
                 std::vector<TreeNodeData> children = {})
        : id(std::move(id)), label(std::move(label)), children(std::move(children)) {}

    TreeNodeData(std::string id, WidgetPtr icon, WidgetPtr label,
                 std::vector<TreeNodeData> children = {})
        : id(std::move(id)), label(std::move(label)), leading_icon(std::move(icon)),
          children(std::move(children)) {}

    /// Fluent setters
    TreeNodeData& expand(bool v = true)      { initially_expanded = v; return *this; }
    TreeNodeData& withTrailing(WidgetPtr w)  { trailing = std::move(w); return *this; }
    TreeNodeData& withCheck(bool v = false)  { checkable = true; checked = v; return *this; }
    TreeNodeData& disable(bool v = true)     { disabled = v; return *this; }
    TreeNodeData& setLoading(bool v = true)  { loading = v; return *this; }
    TreeNodeData& setUserData(void* d)       { user_data = d; return *this; }
};

// ════════════════════════════════════════════════════════════════
// TreeViewTheme — visual configuration
// ════════════════════════════════════════════════════════════════

/// @brief Visual configuration for the TreeView.
struct TreeViewTheme {
    // ── Indentation ────────────────────────────────────────────
    float indent_width = 20.0f;     ///< Pixels added per depth level.
    float node_height  = 32.0f;     ///< Default height of each tree row.

    // ── Colors ─────────────────────────────────────────────────
    Color hover_color    = 0x0DFFFFFF;   ///< 5% white overlay on hover.
    Color selected_color = 0x1A2563EB;   ///< 10% primary on selected.
    Color focused_color  = 0x1A58A6FF;   ///< Focus ring / highlight.
    Color line_color     = 0x33FFFFFF;   ///< Color of connector lines.
    Color arrow_color    = 0xFFAAAAAA;   ///< Expand/collapse arrow color.
    Color disabled_color = 0x60808080;   ///< Disabled node overlay.

    // ── Lines ──────────────────────────────────────────────────
    bool  show_lines  = false;      ///< Draw connector lines between nodes.
    float line_width  = 1.0f;

    // ── Arrow ──────────────────────────────────────────────────
    float arrow_size  = 12.0f;
    bool  animate_arrow = true;     ///< Rotate arrow on expand/collapse.

    // ── Padding ────────────────────────────────────────────────
    float row_padding_horizontal = 4.0f;
    float row_padding_vertical   = 2.0f;
    float icon_gap   = 8.0f;   ///< Gap between icon and label.
    float leading_gap = 4.0f;  ///< Gap between indent/arrow and leading icon.
    float trailing_gap = 8.0f; ///< Gap before trailing widget.

    // ── Shape ──────────────────────────────────────────────────
    BorderRadius row_radius = BorderRadius::circular(4.0f);

    constexpr bool operator==(const TreeViewTheme&) const = default;
};

// ════════════════════════════════════════════════════════════════
// TreeView Props
// ════════════════════════════════════════════════════════════════

struct TreeViewProps {
    Key key = Key::none();
    std::vector<TreeNodeData> nodes;   ///< Root-level tree nodes.

    // ── Selection ──────────────────────────────────────────────
    bool multi_select = false;
    std::optional<std::string> selected_node_id;   ///< Initial selection (single-select).
    std::set<std::string>      selected_node_ids;  ///< Initial selection (multi-select).

    // ── Toggle behavior ────────────────────────────────────────
    bool toggle_on_arrow  = true;   ///< Click the arrow to expand/collapse.
    bool toggle_on_label  = false;  ///< Click anywhere on the row to expand/collapse.

    // ── Visual ─────────────────────────────────────────────────
    TreeViewTheme tree_theme;

    // ── Scroll ─────────────────────────────────────────────────
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float         scroll_speed = 50.0f;
    EdgeInsets    list_padding = EdgeInsets{};

    // ── Callbacks ──────────────────────────────────────────────
    std::function<void(const std::string& id)> on_node_expanded = nullptr;
    std::function<void(const std::string& id)> on_node_collapsed = nullptr;
    std::function<void(const std::string& id)> on_node_selected = nullptr;
    std::function<void(const std::set<std::string>& ids)> on_selection_changed = nullptr;
    std::function<void(const std::string& id, bool checked)> on_node_checked = nullptr;
    std::function<void(const std::string& id,
                        std::function<void(std::vector<TreeNodeData>)> resolve)>
        on_children_requested = nullptr;
    std::function<void(const std::string& id, Point global_position)> on_node_context_menu = nullptr;
    std::function<void(const std::string& id)> on_node_drag_start = nullptr;
    std::function<WidgetPtr(const TreeNodeData&, int depth,
                             bool expanded, bool selected, bool hovered)> node_builder = nullptr;
};

// ════════════════════════════════════════════════════════════════
// TreeView Widget Implementation
// ════════════════════════════════════════════════════════════════

class TreeViewWidget : public StatefulWidget {
public:
    TreeViewProps props;

    TreeViewWidget() = default;
    explicit TreeViewWidget(TreeViewProps p) : props(std::move(p)) {}
    TreeViewWidget(Key key, TreeViewProps p) : StatefulWidget(std::move(key)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TreeView"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct TreeView {
    Key key = Key::none();
    std::vector<TreeNodeData> nodes;

    bool multi_select = false;
    std::optional<std::string> selected_node_id;
    std::set<std::string>      selected_node_ids;

    bool toggle_on_arrow  = true;
    bool toggle_on_label  = false;

    TreeViewTheme tree_theme;

    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float         scroll_speed = 50.0f;
    EdgeInsets    list_padding = EdgeInsets{};

    std::function<void(const std::string& id)> on_node_expanded = nullptr;
    std::function<void(const std::string& id)> on_node_collapsed = nullptr;
    std::function<void(const std::string& id)> on_node_selected = nullptr;
    std::function<void(const std::set<std::string>& ids)> on_selection_changed = nullptr;
    std::function<void(const std::string& id, bool checked)> on_node_checked = nullptr;
    std::function<void(const std::string& id,
                        std::function<void(std::vector<TreeNodeData>)> resolve)>
        on_children_requested = nullptr;
    std::function<void(const std::string& id, Point global_position)> on_node_context_menu = nullptr;
    std::function<void(const std::string& id)> on_node_drag_start = nullptr;
    std::function<WidgetPtr(const TreeNodeData&, int depth,
                             bool expanded, bool selected, bool hovered)> node_builder = nullptr;

    operator WidgetPtr() const {
        TreeViewProps p;
        p.key = key;
        p.nodes = nodes;
        p.multi_select = multi_select;
        p.selected_node_id = selected_node_id;
        p.selected_node_ids = selected_node_ids;
        p.toggle_on_arrow = toggle_on_arrow;
        p.toggle_on_label = toggle_on_label;
        p.tree_theme = tree_theme;
        p.scroll_physics = scroll_physics;
        p.scroll_speed = scroll_speed;
        p.list_padding = list_padding;
        p.on_node_expanded = on_node_expanded;
        p.on_node_collapsed = on_node_collapsed;
        p.on_node_selected = on_node_selected;
        p.on_selection_changed = on_selection_changed;
        p.on_node_checked = on_node_checked;
        p.on_children_requested = on_children_requested;
        p.on_node_context_menu = on_node_context_menu;
        p.on_node_drag_start = on_node_drag_start;
        p.node_builder = node_builder;

        return std::make_shared<TreeViewWidget>(key, std::move(p));
    }
};

} // namespace enki
