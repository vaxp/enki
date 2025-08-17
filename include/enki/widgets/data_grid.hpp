#pragma once
/// @file data_grid.hpp
/// @brief Advanced DataGrid widget for ENKI Framework.
/// Supports interactive column resizing, multi-column sorting, column pinning,
/// selection modes, pagination, live search filter, summary footers, and CSV export.

#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include "enki/core/types.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <sstream>

namespace enki {

/// Column horizontal alignment
enum class DataGridAlign {
    Left,
    Center,
    Right
};

/// Column pinning / frozen behavior
enum class DataGridPin {
    None,
    Left,
    Right
};

/// Sort direction
enum class DataGridSortDirection {
    None,
    Ascending,
    Descending
};

/// Selection model
enum class DataGridSelectionMode {
    None,
    RowSingle,
    RowMultiple,
    CellRange
};

/// Cell content type
enum class DataGridCellType {
    Text,
    Number,
    Badge,
    Progress,
    Custom
};

/// ════════════════════════════════════════════════════════════════
/// DataGrid Column Descriptor
/// ════════════════════════════════════════════════════════════════

struct DataGridColumn {
    std::string key = "";             ///< Unique key/field identifier
    std::string title = "";           ///< Display title in header
    float width = 120.0f;             ///< Current pixel width
    float min_width = 50.0f;          ///< Minimum allowed width during drag resize
    float max_width = 600.0f;         ///< Maximum allowed width during drag resize
    float flex_factor = 0.0f;         ///< Flex growth if width == 0

    bool sortable = true;             ///< Click to sort
    bool resizable = true;            ///< Drag edge to resize
    bool filterable = true;           ///< Searchable
    bool visible = true;              ///< Column visibility

    DataGridAlign align = DataGridAlign::Left;
    DataGridPin pin = DataGridPin::None;
    DataGridCellType cell_type = DataGridCellType::Text;

    std::function<std::string(const std::string& raw_val)> formatter;

    DataGridColumn() = default;
    DataGridColumn(std::string k, std::string t, float w = 120.0f, DataGridAlign a = DataGridAlign::Left)
        : key(std::move(k)), title(std::move(t)), width(w), align(a) {}
};

/// ════════════════════════════════════════════════════════════════
/// DataGrid Cell & Row Data
/// ════════════════════════════════════════════════════════════════

struct DataGridCell {
    std::string value = "";
    std::string display_text = "";
    Color badge_bg = 0;
    Color badge_fg = 0;
    float progress = 0.0f; // 0.0 to 1.0

    DataGridCell() = default;
    DataGridCell(std::string v) : value(v), display_text(std::move(v)) {}
    DataGridCell(std::string v, std::string disp) : value(std::move(v)), display_text(std::move(disp)) {}
    DataGridCell(std::string v, Color bg, Color fg) : value(v), display_text(std::move(v)), badge_bg(bg), badge_fg(fg) {}
    DataGridCell(float prog, std::string disp) : display_text(std::move(disp)), progress(prog) {}
};

struct DataGridRow {
    std::string id = "";
    std::map<std::string, DataGridCell> cells;
    bool selected = false;
    Color background_color = 0;

    DataGridRow() = default;
    DataGridRow(std::string row_id) : id(std::move(row_id)) {}

    DataGridRow& set(const std::string& key, const std::string& val) {
        cells[key] = DataGridCell(val);
        return *this;
    }
    DataGridRow& setBadge(const std::string& key, const std::string& val, Color bg, Color fg) {
        cells[key] = DataGridCell(val, bg, fg);
        return *this;
    }
    DataGridRow& setProgress(const std::string& key, float prog, const std::string& label) {
        cells[key] = DataGridCell(prog, label);
        return *this;
    }
    [[nodiscard]] const std::string& get(const std::string& key) const {
        auto it = cells.find(key);
        if (it != cells.end()) return it->second.value;
        static const std::string s_empty = "";
        return s_empty;
    }
};

/// Sort descriptor
struct DataGridSortRule {
    std::string column_key;
    DataGridSortDirection direction = DataGridSortDirection::Ascending;
};

/// ════════════════════════════════════════════════════════════════
/// DataGrid Controller
/// ════════════════════════════════════════════════════════════════

class DataGridController {
private:
    std::vector<DataGridColumn> columns_;
    std::vector<DataGridRow> raw_rows_;
    std::vector<size_t> filtered_row_indices_;

    std::vector<DataGridSortRule> sort_rules_;
    std::string global_filter_ = "";
    std::set<std::string> selected_row_ids_;

    int current_page_ = 0;
    int page_size_ = 25; // 0 for all/no pagination

    void applyFilterAndSort();

public:
    DataGridController() = default;
    DataGridController(std::vector<DataGridColumn> cols, std::vector<DataGridRow> rows)
        : columns_(std::move(cols)), raw_rows_(std::move(rows)) {
        applyFilterAndSort();
    }

    // Columns Management
    [[nodiscard]] const std::vector<DataGridColumn>& getColumns() const { return columns_; }
    std::vector<DataGridColumn>& getColumnsMut() { return columns_; }
    void setColumns(std::vector<DataGridColumn> cols) { columns_ = std::move(cols); }
    void setColumnWidth(size_t col_idx, float w) {
        if (col_idx < columns_.size()) {
            columns_[col_idx].width = std::clamp(w, columns_[col_idx].min_width, columns_[col_idx].max_width);
        }
    }

    // Rows Management
    void setRows(std::vector<DataGridRow> rows) {
        raw_rows_ = std::move(rows);
        applyFilterAndSort();
    }
    void addRow(DataGridRow row) {
        raw_rows_.push_back(std::move(row));
        applyFilterAndSort();
    }
    void removeRow(const std::string& row_id) {
        raw_rows_.erase(std::remove_if(raw_rows_.begin(), raw_rows_.end(),
            [&](const DataGridRow& r) { return r.id == row_id; }), raw_rows_.end());
        selected_row_ids_.erase(row_id);
        applyFilterAndSort();
    }
    [[nodiscard]] const std::vector<DataGridRow>& getRawRows() const { return raw_rows_; }
    [[nodiscard]] size_t getTotalFilteredCount() const { return filtered_row_indices_.size(); }
    [[nodiscard]] const std::vector<size_t>& getFilteredIndices() const { return filtered_row_indices_; }

    // Sorting
    void toggleSort(const std::string& col_key, bool multi_sort = false);
    [[nodiscard]] DataGridSortDirection getSortDirection(const std::string& col_key) const;
    [[nodiscard]] int getSortPriority(const std::string& col_key) const;

    // Filtering
    void setGlobalFilter(std::string_view filter) {
        global_filter_ = std::string(filter);
        current_page_ = 0;
        applyFilterAndSort();
    }
    [[nodiscard]] const std::string& getGlobalFilter() const { return global_filter_; }

    // Selection
    void selectRow(const std::string& row_id, bool select = true, bool clear_others = false);
    void toggleRowSelection(const std::string& row_id, bool clear_others = false);
    void selectAll(bool select = true);
    [[nodiscard]] bool isRowSelected(const std::string& row_id) const;
    [[nodiscard]] bool isAllSelected() const;
    [[nodiscard]] bool isPartiallySelected() const;
    [[nodiscard]] const std::set<std::string>& getSelectedRowIds() const { return selected_row_ids_; }

    // Pagination
    [[nodiscard]] int getCurrentPage() const { return current_page_; }
    [[nodiscard]] int getPageSize() const { return page_size_; }
    [[nodiscard]] int getTotalPages() const;
    void setPage(int page);
    void setPageSize(int size) {
        page_size_ = size;
        current_page_ = 0;
    }
    void nextPage() { setPage(current_page_ + 1); }
    void prevPage() { setPage(current_page_ - 1); }

    // CSV Export
    [[nodiscard]] std::string exportToCsv(bool selected_only = false) const;
};

/// ════════════════════════════════════════════════════════════════
/// Configuration Options for DataGrid
/// ════════════════════════════════════════════════════════════════

struct DataGridProps {
    Key key = Key::none();
    std::shared_ptr<DataGridController> controller;

    DataGridSelectionMode selection_mode = DataGridSelectionMode::RowMultiple;
    bool show_header = true;
    bool show_row_numbers = false;
    bool show_pagination = true;
    bool show_quick_filter = true;
    bool show_summary_footer = false;
    bool show_borders = true;
    bool zebra_stripes = true;

    float header_height = 38.0f;
    float row_height = 36.0f;
    float footer_height = 36.0f;
    float pagination_height = 42.0f;

    // Styling Colors
    Color background_color    = 0xFF0F172A; // Slate 900
    Color header_bg_color     = 0xFF1E293B; // Slate 800
    Color header_text_color   = 0xFFF1F5F9; // Slate 100
    Color row_bg_color        = 0xFF0F172A; // Slate 900
    Color zebra_row_bg_color  = 0xFF172033; // Slate 850
    Color row_hover_color     = 0xFF1E293B; // Slate 800
    Color row_selected_color  = 0x4038BDF8; // Sky 400 with alpha
    Color border_color        = 0xFF334155; // Slate 700
    Color text_color          = 0xFFE2E8F0; // Slate 200
    Color sort_icon_color     = 0xFF38BDF8; // Sky 400
    Color footer_bg_color     = 0xFF1E293B; // Slate 800

    float border_radius = 8.0f;

    // Callbacks
    std::function<void(const std::set<std::string>& selected_ids)> on_selection_changed;
    std::function<void(const std::string& row_id)> on_row_tap;
    std::function<void(const std::string& row_id)> on_row_double_tap;
    std::function<void(const std::string& col_key, DataGridSortDirection dir)> on_sort_changed;
    std::function<void(int new_page)> on_page_changed;
    std::function<std::string(const std::string& col_key, const std::vector<DataGridRow>& rows)> summary_calculator;
};

/// ════════════════════════════════════════════════════════════════
/// DataGrid Widget
/// ════════════════════════════════════════════════════════════════

class DataGrid : public StatefulWidget {
public:
    DataGridProps props;

    DataGrid() = default;
    explicit DataGrid(DataGridProps p) : props(std::move(p)) {
        if (!props.controller) {
            props.controller = std::make_shared<DataGridController>();
        }
    }

    // Fluent API Chaining
    DataGrid& selectionMode(DataGridSelectionMode mode) { props.selection_mode = mode; return *this; }
    DataGrid& pagination(bool enable = true) { props.show_pagination = enable; return *this; }
    DataGrid& quickFilter(bool enable = true) { props.show_quick_filter = enable; return *this; }
    DataGrid& summaryFooter(bool enable = true) { props.show_summary_footer = enable; return *this; }
    DataGrid& zebra(bool enable = true) { props.zebra_stripes = enable; return *this; }
    DataGrid& rowHeight(float h) { props.row_height = h; return *this; }
    DataGrid& onSelectionChanged(std::function<void(const std::set<std::string>&)> cb) {
        props.on_selection_changed = std::move(cb);
        return *this;
    }
    DataGrid& onRowTap(std::function<void(const std::string&)> cb) { props.on_row_tap = std::move(cb); return *this; }
    DataGrid& onRowDoubleTap(std::function<void(const std::string&)> cb) { props.on_row_double_tap = std::move(cb); return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DataGrid"; }
};

inline std::shared_ptr<DataGrid> dataGrid(DataGridProps props = {}) {
    return std::make_shared<DataGrid>(std::move(props));
}

inline std::shared_ptr<DataGrid> dataGrid(
    std::shared_ptr<DataGridController> ctrl) {
    DataGridProps props;
    props.controller = std::move(ctrl);
    return std::make_shared<DataGrid>(std::move(props));
}

} // namespace enki
