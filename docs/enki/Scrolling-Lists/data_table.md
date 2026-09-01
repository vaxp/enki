# DataTable

> An enterprise-grade data grid widget for desktop applications, featuring column sorting indicators, per-row and "select-all" checkbox selection, numeric alignments, editable cell badges, and alternating row colors.

- **Header File**: `#include "enki/widgets/data_table.hpp"`
- **C++ Class**: `enki::DataTableWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::DataTable` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DataTableProps`
- **Data Models**: `enki::DataColumn`, `enki::DataRow`, `enki::DataCell`
- **Visual Theme**: `enki::DataTableTheme`

---

## Overview

`DataTable` provides an interactive data grid designed for desktop workflows (e.g. employee directories, financial ledgers, database viewers, admin dashboards). It layers rich interactive behaviors atop tabular layouts:
1. **Interactive Column Sorting**: Clickable column headers display animated ascending/descending arrows and invoke `on_sort(index, ascending)`.
2. **Batch Row Selection**: Built-in leading checkbox column with tri-state "Select All" header support.
3. **Numeric Column Alignment**: Right-aligns numbers and decimals automatically with `.asNumeric()`.
4. **Editable Cells**: `DataCell` supports click/double-tap hooks and edit pencil icons (`.showEdit(true)`).
5. **Horizontal Scrolling**: Wide data sets automatically scroll horizontally via an integrated viewport.

---

## C++ API Definition

### Data Models (`DataColumn`, `DataRow`, `DataCell`)
```cpp
namespace enki {

struct DataColumn {
    WidgetPtr   label;                   ///< Header widget
    std::string tooltip;                 ///< Hover tooltip text
    bool        sortable     = false;
    bool        numeric      = false;    ///< Right-aligns column header & cells
    float       column_width = 0.0f;     ///< Fixed px width (if 0, flex grows)
    float       flex_factor  = 1.0f;

    std::function<void(int column_index, bool ascending)> on_sort;

    DataColumn(WidgetPtr label, bool sortable = false, std::function<void(int, bool)> on_sort = nullptr);
    DataColumn& withTooltip(std::string t);
    DataColumn& asNumeric();
    DataColumn& fixedWidth(float w);
    DataColumn& flex(float f);
    DataColumn& onSort(std::function<void(int, bool)> cb);
};

struct DataCell {
    WidgetPtr child;
    bool      show_edit_icon = false;
    bool      placeholder    = false;

    std::function<void()> on_tap;
    std::function<void()> on_double_tap;

    explicit DataCell(WidgetPtr child, bool edit_icon = false);
    DataCell& showEdit(bool v = true);
    DataCell& asPlaceholder(bool v = true);
    DataCell& onTap(std::function<void()> cb);
    DataCell& onDoubleTap(std::function<void()> cb);
};

struct DataRow {
    std::vector<DataCell>              cells;
    bool                               selected = false;
    std::function<void(bool selected)> on_select_changed;
    std::function<void()>              on_tap;

    explicit DataRow(std::vector<DataCell> cells);
    DataRow& select(bool s);
    DataRow& onSelectChanged(std::function<void(bool)> cb);
    DataRow& onTap(std::function<void()> cb);
};

} // namespace enki
```

### Visual Theme (`DataTableTheme`)
```cpp
namespace enki {

struct DataTableTheme {
    float heading_row_height   = 56.0f;
    float data_row_height      = 48.0f;
    float column_spacing       = 56.0f;
    float horizontal_margin    = 24.0f;
    float divider_thickness    = 1.0f;

    Color heading_row_color    = 0xFF1E2937;
    Color heading_text_color   = 0xFFB0C4D8;
    Color data_row_color       = Colors::Transparent;
    Color data_row_alt_color   = 0xFF0D1117; ///< Alternating zebra row tint
    Color selected_row_color   = 0x1A2563EB; ///< Highlight for checked rows
    Color sort_arrow_color     = 0xFFB0C4D8;

    bool  show_checkbox_column = true;
    bool  use_alternating_rows = false;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct DataTable {
    Key                                    key               = Key::none();
    std::vector<DataColumn>                columns;
    std::vector<DataRow>                   rows;

    std::optional<int>                     sort_column_index = std::nullopt;
    bool                                   sort_ascending    = true;

    std::function<void(bool all_selected)> on_select_all     = nullptr;
    std::function<void(int row_index)>     on_row_tap        = nullptr;

    DataTableTheme                         theme;
    bool                                   horizontal_scroll = true;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `columns` | `std::vector<DataColumn>` | `{}` | Header definitions specifying sortability, numeric alignment, and width. |
| `rows` | `std::vector<DataRow>` | `{}` | Row records containing individual `DataCell` elements. |
| `sort_column_index` | `optional<int>` | `nullopt` | Index of the active sorted column. |
| `sort_ascending` | `bool` | `true` | Sort direction (`true` for ascending, `false` for descending). |
| `on_select_all` | `Function(bool)` | `nullptr` | Callback when the header master checkbox is clicked. |
| `theme` | `DataTableTheme` | `{}` | Theme configuration (zebra striping, row heights, checkbox visibility). |
| `horizontal_scroll` | `bool` | `true` | Enables horizontal scrolling for tables wider than the window. |

---

## Code Examples (From `widgets_demo/data_table_demo/main.cpp`)

### 1. Interactive Employee Directory with Sort & Checkboxes
```cpp
#include "enki/widgets/data_table.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildEmployeeTable(int currentSortCol, bool isAscending, auto onSortCallback) {
    std::vector<DataColumn> cols = {
        DataColumn(text("Employee Name"), true, onSortCallback).flex(2.0f),
        DataColumn(text("Department"), true, onSortCallback).flex(1.5f),
        DataColumn(text("Annual Salary"), true, onSortCallback).asNumeric().fixedWidth(140.0f),
        DataColumn(text("Status")).fixedWidth(100.0f),
    };

    std::vector<DataRow> rows = {
        DataRow({
            DataCell(text("Alice Johnson")),
            DataCell(text("Engineering")),
            DataCell(text("$115,000")),
            DataCell(text("Active", { .color = 0xFF10B981 })),
        }).select(false),
        DataRow({
            DataCell(text("Bob Martinez")),
            DataCell(text("UI/UX Design")),
            DataCell(text("$92,000")),
            DataCell(text("Active", { .color = 0xFF10B981 })),
        }).select(true),
    };

    DataTableTheme theme;
    theme.use_alternating_rows = true;
    theme.data_row_alt_color   = 0xFF161B22;
    theme.heading_row_color    = 0xFF1E2937;
    theme.show_checkbox_column = true;

    return DataTable {
        .columns = std::move(cols),
        .rows = std::move(rows),
        .sort_column_index = currentSortCol,
        .sort_ascending = isAscending,
        .theme = theme,
    };
}
```

---

## See Also
- [**Table**](./table.md) — Base layout table with flexible column widths.
- [**Checkbox**](../Input%20Forms/checkbox.md) — Underlying selection control.
- [**ScrollView**](./scroll_view.md) — Viewport wrapping the table horizontally.
