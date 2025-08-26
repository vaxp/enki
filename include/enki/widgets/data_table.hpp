#pragma once
/// @file data_table.hpp
/// @brief DataTable widget — a production-grade data grid for desktop applications.
///
/// Features:
///   - Column headers with sort indicators (ascending/descending arrows).
///   - Per-row selection with checkboxes (single or multi-select).
///   - "Select all" header checkbox with indeterminate (tri-state) support.
///   - Sortable columns with `onSort` callback.
///   - Numeric column alignment (right-aligned labels and cells).
///   - Per-cell edit icon and onTap/onDoubleTap callbacks.
///   - Alternating row colors, configurable heading/data row heights.
///   - Horizontal scrolling for wide tables via ScrollView.
///   - Configurable column spacing, horizontal margin, divider thickness.
///   - `showBottomBorder` for the last row.
///   - Optional footer row for summaries/totals.
///   - `onRowTap`, `onRowSecondaryTap` for row-level interaction.
///   - All layout via Anu — column widths via flex, no manual pixel math.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/widgets/table.hpp"     // TableBorder, TableCellVerticalAlignment
#include "enki/widgets/list_view.hpp" // ScrollPhysics
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// DataColumn
// ════════════════════════════════════════════════════════════════

/// @brief Describes a column in the DataTable header.
struct DataColumn {
    WidgetPtr   label;                   ///< Required: header label widget.
    std::string tooltip;                 ///< Hover tooltip text (empty = no tooltip).
    bool        sortable    = false;     ///< Whether clicking this header triggers sorting.
    bool        numeric     = false;     ///< If true, header and cells are right-aligned.
    float       column_width = 0.0f;     ///< If > 0, fixed pixel width. Else flex grows.
    float       flex_factor  = 1.0f;    ///< Anu flex_grow factor if column_width == 0.

    /// Called when the user clicks the sort header.
    /// @param column_index  Index of this column in the DataTable.
    /// @param ascending     true = ascending order requested.
    std::function<void(int column_index, bool ascending)> on_sort;

    DataColumn() = default;
    explicit DataColumn(WidgetPtr label) : label(std::move(label)) {}
    DataColumn(WidgetPtr label, bool sortable, std::function<void(int, bool)> on_sort = nullptr)
        : label(std::move(label)), sortable(sortable), on_sort(std::move(on_sort)) {}

    DataColumn& withTooltip(std::string t) { tooltip = std::move(t); return *this; }
    DataColumn& asNumeric()                { numeric = true; return *this; }
    DataColumn& fixedWidth(float w)        { column_width = w; return *this; }
    DataColumn& flex(float f)              { flex_factor = f; return *this; }
    DataColumn& onSort(std::function<void(int, bool)> cb) {
        sortable = true;
        on_sort = std::move(cb);
        return *this;
    }
};

// ════════════════════════════════════════════════════════════════
// DataCell
// ════════════════════════════════════════════════════════════════

/// @brief A single cell within a DataRow.
struct DataCell {
    WidgetPtr child;                    ///< Required: cell content.
    bool      show_edit_icon = false;   ///< If true, shows a small edit pencil icon.
    bool      placeholder    = false;   ///< Renders content at reduced opacity.

    std::function<void()>       on_tap;
    std::function<void()>       on_double_tap;
    std::function<void()>       on_long_press;

    DataCell() = default;
    explicit DataCell(WidgetPtr child) : child(std::move(child)) {}
    DataCell(WidgetPtr child, bool edit_icon)
        : child(std::move(child)), show_edit_icon(edit_icon) {}

    DataCell& showEdit(bool v = true)       { show_edit_icon = v; return *this; }
    DataCell& asPlaceholder(bool v = true)  { placeholder = v; return *this; }
    DataCell& onTap(std::function<void()> cb)       { on_tap = std::move(cb); return *this; }
    DataCell& onDoubleTap(std::function<void()> cb) { on_double_tap = std::move(cb); return *this; }
};

// ════════════════════════════════════════════════════════════════
// DataRow
// ════════════════════════════════════════════════════════════════

/// @brief A single data row in the DataTable.
struct DataRow {
    std::vector<DataCell> cells;         ///< One DataCell per column.

    // ── Selection ──────────────────────────────────────────────
    bool selected = false;
    std::function<void(bool selected)> on_select_changed;

    // ── Interaction ────────────────────────────────────────────
    std::function<void()> on_tap;
    std::function<void()> on_secondary_tap;
    std::function<void()> on_long_press;

    // ── Visual overrides ───────────────────────────────────────
    std::optional<Color>          color;          ///< Per-row background override.
    std::optional<float>          height;         ///< Per-row height override.
    std::optional<BoxDecoration>  decoration;     ///< Full decoration override.

    DataRow() = default;
    explicit DataRow(std::vector<DataCell> cells) : cells(std::move(cells)) {}

    DataRow& select(bool s)     { selected = s; return *this; }
    DataRow& rowColor(Color c)  { color = c; return *this; }
    DataRow& rowHeight(float h) { height = h; return *this; }
    DataRow& onSelectChanged(std::function<void(bool)> cb) {
        on_select_changed = std::move(cb);
        return *this;
    }
    DataRow& onTap(std::function<void()> cb)          { on_tap = std::move(cb); return *this; }
    DataRow& onSecondaryTap(std::function<void()> cb) { on_secondary_tap = std::move(cb); return *this; }
    DataRow& onLongPress(std::function<void()> cb)    { on_long_press = std::move(cb); return *this; }
};

// ════════════════════════════════════════════════════════════════
// DataTableTheme
// ════════════════════════════════════════════════════════════════

/// @brief Full visual configuration for the DataTable.
struct DataTableTheme {
    // ── Row Heights ────────────────────────────────────────────
    float heading_row_height = 56.0f;   ///< Height of the header row.
    float data_row_height    = 48.0f;   ///< Default height of data rows.
    float data_row_min_height = 48.0f;
    float data_row_max_height = 48.0f;

    // ── Spacing ────────────────────────────────────────────────
    float column_spacing       = 56.0f;  ///< Space between columns.
    float horizontal_margin    = 24.0f;  ///< Padding at left and right edges.
    float checkbox_h_margin    = 8.0f;   ///< Horizontal padding around checkboxes.
    float divider_thickness    = 1.0f;   ///< Thickness of row dividers.

    // ── Colors ─────────────────────────────────────────────────
    Color heading_row_color      = 0xFF1E2937;   ///< Header background.
    Color heading_text_color     = 0xFFB0C4D8;   ///< Header text color.
    Color data_row_color         = Colors::Transparent;
    Color data_row_alt_color     = 0xFF0D1117;   ///< Alternating row background.
    Color selected_row_color     = 0x1A2563EB;   ///< Selected row highlight.
    Color hover_row_color        = 0x0DFFFFFF;   ///< Hovered row highlight.
    Color divider_color          = 0x1AFFFFFF;   ///< Row divider color.
    Color sort_arrow_color       = 0xFFB0C4D8;   ///< Sort indicator arrow color.
    Color checkbox_color         = 0xFF2563EB;
    Color edit_icon_color        = 0xFFB0C4D8;
    Color placeholder_text_color = 0x60FFFFFF;

    // ── Border ─────────────────────────────────────────────────
    TableBorder border;              ///< Optional outer + inner border.
    bool show_bottom_border = false; ///< Show border below last data row.

    // ── Checkboxes ─────────────────────────────────────────────
    bool show_checkbox_column = true;  ///< Show selection checkbox column.

    // ── Sort ───────────────────────────────────────────────────
    float sort_arrow_size = 12.0f;

    // ── Alternating rows ───────────────────────────────────────
    bool use_alternating_rows = false;

    constexpr bool operator==(const DataTableTheme&) const = default;
};

// ════════════════════════════════════════════════════════════════
// DataTable Widget
// ════════════════════════════════════════════════════════════════

struct DataTableProps {
    Key key = Key::none();
    std::vector<DataColumn> columns;   ///< Column definitions (header).
    std::vector<DataRow>    rows;      ///< Data rows.

    // ── Sorting ────────────────────────────────────────────────
    std::optional<int> sort_column_index;    ///< Currently sorted column (nullopt = unsorted).
    bool               sort_ascending = true;

    // ── Selection ──────────────────────────────────────────────
    std::function<void(bool all_selected)> on_select_all;

    // ── Callbacks ──────────────────────────────────────────────
    std::function<void(int row_index)> on_row_tap;
    std::function<void(int row_index)> on_row_secondary_tap;

    // ── Visual ─────────────────────────────────────────────────
    DataTableTheme theme;

    // ── Scroll ─────────────────────────────────────────────────
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float         scroll_speed   = 50.0f;
    bool          horizontal_scroll = true;  ///< Enable horizontal scroll for wide tables.
};

class DataTableWidget : public StatefulWidget {
public:
    DataTableProps props;

    DataTableWidget() = default;
    explicit DataTableWidget(DataTableProps p) : StatefulWidget(p.key), props(std::move(p)) {}
    DataTableWidget(Key k, DataTableProps p) : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DataTable"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative DataTable Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct DataTable {
    Key key = Key::none();
    std::vector<DataColumn> columns;
    std::vector<DataRow>    rows;

    std::optional<int> sort_column_index = std::nullopt;
    bool               sort_ascending = true;

    std::function<void(bool all_selected)> on_select_all = nullptr;
    std::function<void(int row_index)> on_row_tap = nullptr;
    std::function<void(int row_index)> on_row_secondary_tap = nullptr;

    DataTableTheme theme;

    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float         scroll_speed   = 50.0f;
    bool          horizontal_scroll = true;

    operator WidgetPtr() const {
        DataTableProps p;
        p.key = key;
        p.columns = columns;
        p.rows = rows;
        p.sort_column_index = sort_column_index;
        p.sort_ascending = sort_ascending;
        p.on_select_all = on_select_all;
        p.on_row_tap = on_row_tap;
        p.on_row_secondary_tap = on_row_secondary_tap;
        p.theme = theme;
        p.scroll_physics = scroll_physics;
        p.scroll_speed = scroll_speed;
        p.horizontal_scroll = horizontal_scroll;
        return std::make_shared<DataTableWidget>(key, std::move(p));
    }
};

} // namespace enki
