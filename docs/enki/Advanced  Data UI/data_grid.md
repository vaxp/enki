# DataGrid

> An enterprise-grade data table widget supporting column resizing, multi-column sorting, column pinning, cell progress bars, badges, pagination, quick live filtering, summary footers, and CSV export.

- **Header File**: `#include "enki/widgets/data_grid.hpp"`
- **C++ Class**: `enki::DataGridWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::DataGrid` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DataGridProps`
- **Controller**: `enki::DataGridController`
- **Descriptive Structs**: `enki::DataGridColumn`, `enki::DataGridRow`, `enki::DataGridCell`
- **Enums**: `enki::DataGridAlign`, `enki::DataGridPin`, `enki::DataGridSortDirection`, `enki::DataGridSelectionMode`, `enki::DataGridCellType`

---

## Overview

`DataGrid` provides high-density tabular presentation for enterprise workstations and dashboards. Users can sort columns by clicking headers, drag column borders to resize them, pin columns to the left or right, paginate large datasets, filter rows via search queries, and view summary aggregate calculations.

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class DataGridAlign {
    Left,
    Center,
    Right
};

enum class DataGridPin {
    None,
    Left,
    Right
};

enum class DataGridSortDirection {
    None,
    Ascending,
    Descending
};

enum class DataGridSelectionMode {
    None,
    RowSingle,
    RowMultiple,
    CellRange
};

enum class DataGridCellType {
    Text,
    Number,
    Badge,
    Progress,
    Custom
};

} // namespace enki
```

### Column & Row Descriptors
```cpp
namespace enki {

struct DataGridColumn {
    std::string      key         = "";             ///< Unique key/field identifier
    std::string      title       = "";             ///< Header text
    float            width       = 120.0f;         ///< Pixel width
    float            min_width   = 50.0f;
    float            max_width   = 600.0f;

    bool             sortable    = true;           ///< Click header to sort
    bool             resizable   = true;           ///< Drag border to resize
    bool             filterable  = true;           ///< Include in quick-filter search
    bool             visible     = true;

    DataGridAlign    align       = DataGridAlign::Left;
    DataGridPin      pin         = DataGridPin::None;
    DataGridCellType cell_type   = DataGridCellType::Text;

    DataGridColumn() = default;
    DataGridColumn(std::string k, std::string t, float w = 120.0f, DataGridAlign a = DataGridAlign::Left);
};

struct DataGridRow {
    std::string id = "";
    std::map<std::string, DataGridCell> cells;
    bool selected = false;

    DataGridRow() = default;
    explicit DataGridRow(std::string row_id);

    DataGridRow& set(const std::string& key, const std::string& val);
    DataGridRow& setBadge(const std::string& key, const std::string& val, Color bg, Color fg);
    DataGridRow& setProgress(const std::string& key, float prog, const std::string& label);
    [[nodiscard]] const std::string& get(const std::string& key) const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct DataGrid {
    Key                                     key                 = Key::none();
    std::shared_ptr<DataGridController>     controller          = nullptr;

    DataGridSelectionMode                   selection_mode      = DataGridSelectionMode::RowSingle;
    bool                                    show_header         = true;
    bool                                    show_row_numbers    = false;
    bool                                    show_pagination     = true;
    bool                                    show_quick_filter   = false;
    bool                                    show_summary_footer = false;
    bool                                    zebra_stripes       = true;

    float                                   header_height       = 40.0f;
    float                                   row_height          = 36.0f;
    float                                   border_radius       = 8.0f;

    Color                                   background_color    = 0xFF0F172A;
    Color                                   header_bg_color     = 0xFF1E293B;
    Color                                   row_bg_color        = 0xFF0F172A;
    Color                                   zebra_row_bg_color  = 0xFF172033;
    Color                                   row_hover_color     = 0xFF1E293B;
    Color                                   row_selected_color  = 0x4038BDF8;

    // Callbacks
    std::function<void(const std::set<std::string>&)> on_selection_changed = nullptr;
    std::function<void(const std::string& row_id)>    on_row_tap            = nullptr;
    std::function<void(const std::string& col, DataGridSortDirection dir)> on_sort_changed = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `controller` | `shared_ptr<DataGridController>`| `nullptr`| Manages columns, rows, sorting, filtering, and pagination. |
| `selection_mode` | `DataGridSelectionMode` | `RowSingle` | Row selection behavior (`None`, `RowSingle`, `RowMultiple`). |
| `zebra_stripes` | `bool` | `true` | Alternates row background colors for improved legibility. |
| `show_pagination`| `bool` | `true` | Renders bottom page navigation controls. |
| `show_quick_filter`| `bool`| `false` | Shows integrated live search bar. |
| `show_summary_footer`| `bool`| `false` | Renders summary row with calculated aggregates. |

---

## Code Examples (From `widgets_demo/data_grid_demo/main.cpp`)

### 1. Initializing Columns, Rows, and DataGrid
```cpp
#include "enki/widgets/data_grid.hpp"

using namespace enki;

WidgetPtr buildEmployeeDataGrid() {
    auto gridCtrl = std::make_shared<DataGridController>();

    // 1. Define Columns
    std::vector<DataGridColumn> cols = {
        DataGridColumn("id", "ID", 70.0f, DataGridAlign::Center),
        DataGridColumn("name", "Employee Name", 170.0f, DataGridAlign::Left),
        DataGridColumn("dept", "Department", 130.0f, DataGridAlign::Left),
    };

    // Badge Column
    DataGridColumn colStatus("status", "Status", 110.0f, DataGridAlign::Center);
    colStatus.cell_type = DataGridCellType::Badge;
    cols.push_back(colStatus);

    // Progress Column
    DataGridColumn colPerf("perf", "Performance", 140.0f, DataGridAlign::Left);
    colPerf.cell_type = DataGridCellType::Progress;
    cols.push_back(colPerf);

    gridCtrl->setColumns(cols);

    // 2. Add Rows
    DataGridRow r1("emp_101");
    r1.set("id", "#101")
      .set("name", "Sarah Jenkins")
      .set("dept", "Engineering")
      .setBadge("status", "ACTIVE", 0x2E10B981, 0xFF10B981)
      .setProgress("perf", 0.94f, "94% Exceptional");
    gridCtrl->addRow(r1);

    // 3. Render Widget
    return DataGrid {
        .controller = gridCtrl,
        .selection_mode = DataGridSelectionMode::RowSingle,
        .zebra_stripes = true,
        .show_pagination = true,
        .on_row_tap = [](const std::string& row_id) {
            std::cout << "Selected Employee: " << row_id << "\n";
        }
    };
}
```

---

## See Also
- [**Table**](../Scrolling-Lists/table.md) — Simple static layout table.
- [**DataTable**](../Scrolling-Lists/data_table.md) — Material-style data table.
- [**SplitView**](./split_view.md) — Resizable dual-pane container.
