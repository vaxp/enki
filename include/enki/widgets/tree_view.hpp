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
/// Architecture:
///   TreeView (StatefulWidget)
///     └── build() → ScrollView
///                     └── Column (FlexDirection::Column)
///                           ├── _buildNode(root[0], depth=0)
///                           │     ├── TreeNodeRow (Row: indent + arrow + icon + label + trailing)
///                           │     └── if expanded → Column of children (recursed)
///                           ├── _buildNode(root[1], depth=0)
///                           └── ...
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
// TreeView Widget
// ════════════════════════════════════════════════════════════════

/// @brief A fully-featured hierarchical tree widget for desktop environments.
///
/// Usage:
/// @code
///   treeView({
///       TreeNodeData("root", icon(Icons::Folder), text("Documents"), {
///           TreeNodeData("sub1", icon(Icons::InsertDriveFile), text("Resume.pdf")),
///           TreeNodeData("sub2", icon(Icons::InsertDriveFile), text("Cover.docx")),
///       }).expand(),
///       TreeNodeData("root2", icon(Icons::Folder), text("Downloads")),
///   })
///   ->onNodeSelected([](const std::string& id){ /* ... */ })
///   ->showLines(true)
///   ->multiSelect(true);
/// @endcode
class TreeView : public StatefulWidget {
public:
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

    /// Fired when a node is expanded. `id` = the expanded node's id.
    std::function<void(const std::string& id)> on_node_expanded;

    /// Fired when a node is collapsed.
    std::function<void(const std::string& id)> on_node_collapsed;

    /// Fired when a node is selected (single-select).
    std::function<void(const std::string& id)> on_node_selected;

    /// Fired when the multi-select set changes.
    std::function<void(const std::set<std::string>& ids)> on_selection_changed;

    /// Fired when a checkable node's check state changes.
    std::function<void(const std::string& id, bool checked)> on_node_checked;

    /// Called when a node with no loaded children is first expanded.
    /// The implementation should load children and call setState via the
    /// returned callback (or rebuild the tree externally).
    std::function<void(const std::string& id,
                        std::function<void(std::vector<TreeNodeData>)> resolve)>
        on_children_requested;

    /// Called on right-click / secondary tap of a node.
    std::function<void(const std::string& id, Point global_position)> on_node_context_menu;

    /// Called when a drag starts on a node (for Drag & Drop integration).
    std::function<void(const std::string& id)> on_node_drag_start;

    /// Custom node builder — if set, overrides the default node rendering entirely.
    /// Receives: node data, depth, isExpanded, isSelected, isHovered.
    std::function<WidgetPtr(const TreeNodeData&, int depth,
                             bool expanded, bool selected, bool hovered)> node_builder;

    // ─────────────────────────────────────────────────────────
    TreeView() = default;
    explicit TreeView(std::vector<TreeNodeData> nodes) : nodes(std::move(nodes)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    TreeView& showLines(bool s = true)        { tree_theme.show_lines = s; return *this; }
    TreeView& indentWidth(float w)            { tree_theme.indent_width = w; return *this; }
    TreeView& nodeHeight(float h)             { tree_theme.node_height = h; return *this; }
    TreeView& multiSelect(bool m = true)      { multi_select = m; return *this; }
    TreeView& selectedId(std::string id)      { selected_node_id = std::move(id); return *this; }
    TreeView& toggleOnLabel(bool v = true)    { toggle_on_label = v; return *this; }
    TreeView& toggleOnArrow(bool v = true)    { toggle_on_arrow = v; return *this; }
    TreeView& theme(TreeViewTheme t)          { this->tree_theme = std::move(t); return *this; }
    TreeView& padding(EdgeInsets p)           { this->list_padding = p; return *this; }
    TreeView& paddingAll(float p)             { this->list_padding = EdgeInsets::all(p); return *this; }
    TreeView& physics(ScrollPhysics p)        { this->scroll_physics = p; return *this; }

    TreeView& onNodeSelected(std::function<void(const std::string&)> cb) {
        on_node_selected = std::move(cb);
        return *this;
    }
    TreeView& onNodeExpanded(std::function<void(const std::string&)> cb) {
        on_node_expanded = std::move(cb);
        return *this;
    }
    TreeView& onNodeCollapsed(std::function<void(const std::string&)> cb) {
        on_node_collapsed = std::move(cb);
        return *this;
    }
    TreeView& onSelectionChanged(std::function<void(const std::set<std::string>&)> cb) {
        on_selection_changed = std::move(cb);
        return *this;
    }
    TreeView& onNodeChecked(std::function<void(const std::string&, bool)> cb) {
        on_node_checked = std::move(cb);
        return *this;
    }
    TreeView& onContextMenu(std::function<void(const std::string&, Point)> cb) {
        on_node_context_menu = std::move(cb);
        return *this;
    }
    TreeView& onChildrenRequested(
        std::function<void(const std::string&, std::function<void(std::vector<TreeNodeData>)>)> cb) {
        on_children_requested = std::move(cb);
        return *this;
    }
    TreeView& nodeBuilder(
        std::function<WidgetPtr(const TreeNodeData&, int, bool, bool, bool)> cb) {
        node_builder = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TreeView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<TreeView> treeView(std::vector<TreeNodeData> nodes) {
    return std::make_shared<TreeView>(std::move(nodes));
}

inline std::shared_ptr<TreeView> treeView() {
    return std::make_shared<TreeView>();
}

} // namespace enki
