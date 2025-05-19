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
/// Architecture:
///   DataTable (StatefulWidget)
///     └── build() → ScrollView (Horizontal)
///                     └── Column (FlexDirection::Column)
///                           ├── Header Row (Row: checkbox + [DataColumn headers])
///                           ├── DataRow[0]  (Row: checkbox + [DataCell content])
///                           ├── DataRow[1]
///                           └── ...
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

/// @brief A production-grade sortable data grid for desktop applications.
///
/// Usage:
/// @code
///   dataTable(
///       {
///           DataColumn(text("Name"))   .onSort([](int col, bool asc){ /* sort */ }),
///           DataColumn(text("Size"))   .asNumeric().fixedWidth(100),
///           DataColumn(text("Status")) .fixedWidth(120),
///       },
///       {
///           DataRow({ DataCell(text("file.txt")),
///                     DataCell(text("12 KB")),
///                     DataCell(text("Active")) }).onTap([]{}),
///           DataRow({ DataCell(text("photo.jpg")),
///                     DataCell(text("3.4 MB")),
///                     DataCell(text("Archived")) }).select(true),
///       }
///   )
///   ->onSelectAll([](bool){ /* ... */ })
///   ->sortColumnIndex(0)
///   ->sortAscending(true);
/// @endcode
class DataTable : public StatefulWidget {
public:
    std::vector<DataColumn> columns;   ///< Column definitions (header).
    std::vector<DataRow>    rows;      ///< Data rows.

    // ── Sorting ────────────────────────────────────────────────
    std::optional<int> sort_column_index;    ///< Currently sorted column (nullopt = unsorted).
    bool               sort_ascending = true;

    // ── Selection ──────────────────────────────────────────────
    /// Called when the "select all" header checkbox is toggled.
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

    // ─────────────────────────────────────────────────────────
    DataTable() = default;
    DataTable(std::vector<DataColumn> columns, std::vector<DataRow> rows)
        : columns(std::move(columns)), rows(std::move(rows)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    DataTable& sortColumnIndex(int idx)     { sort_column_index = idx; return *this; }
    DataTable& sortAscending(bool asc)      { sort_ascending = asc; return *this; }
    DataTable& showCheckboxColumn(bool v)   { theme.show_checkbox_column = v; return *this; }
    DataTable& headingRowHeight(float h)    { theme.heading_row_height = h; return *this; }
    DataTable& dataRowHeight(float h) {
        theme.data_row_height = theme.data_row_min_height = theme.data_row_max_height = h;
        return *this;
    }
    DataTable& columnSpacing(float s)       { theme.column_spacing = s; return *this; }
    DataTable& horizontalMargin(float m)    { theme.horizontal_margin = m; return *this; }
    DataTable& dividerThickness(float t)    { theme.divider_thickness = t; return *this; }
    DataTable& showBottomBorder(bool v)     { theme.show_bottom_border = v; return *this; }
    DataTable& alternatingRows(bool v)      { theme.use_alternating_rows = v; return *this; }
    DataTable& border(TableBorder b)        { theme.border = std::move(b); return *this; }
    DataTable& withTheme(DataTableTheme t)  { theme = std::move(t); return *this; }
    DataTable& headingRowColor(Color c)     { theme.heading_row_color = c; return *this; }
    DataTable& dataRowColor(Color c)        { theme.data_row_color = c; return *this; }
    DataTable& selectedRowColor(Color c)    { theme.selected_row_color = c; return *this; }

    DataTable& onSelectAll(std::function<void(bool)> cb) {
        on_select_all = std::move(cb);
        return *this;
    }
    DataTable& onRowTap(std::function<void(int)> cb) {
        on_row_tap = std::move(cb);
        return *this;
    }
    DataTable& onRowSecondaryTap(std::function<void(int)> cb) {
        on_row_secondary_tap = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DataTable"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<DataTable> dataTable(std::vector<DataColumn> columns,
                                             std::vector<DataRow>    rows) {
    return std::make_shared<DataTable>(std::move(columns), std::move(rows));
}

inline std::shared_ptr<DataTable> dataTable() {
    return std::make_shared<DataTable>();
}

} // namespace enki
