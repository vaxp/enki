# Table

> A high-performance structured grid table widget supporting configurable fixed, flexible, and min/max column width strategies, inner/outer borders, per-row background decorations, and vertical cell alignment.

- **Header File**: `#include "enki/widgets/table.hpp"`
- **C++ Class**: `enki::TableWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::Table` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::TableProps`
- **Row Model**: `enki::TableRow`
- **Border Specifier**: `enki::TableBorder`
- **Column Strategies**: `FixedColumnWidth`, `FlexColumnWidth`, `MinMaxColumnWidth`, `IntrinsicColumnWidth`

---

## Overview

`Table` arranges widgets into a strict row-and-column tabular grid. Every row is laid out using the Anu Flexbox engine, ensuring fluid responsiveness across various display widths. Inner row/column divider borders and outer bounding borders are rendered cleanly during the paint phase using Skia `Canvas::drawLine` and `drawRRect`.

---

## Column Width Strategies

Columns can be individually customized using `TableColumnWidth` variants:

| Strategy | Definition | Behavior |
|---|---|---|
| **`FixedColumnWidth(px)`** | `FixedColumnWidth(60.0f)` | Enforces an exact, immutable pixel width. Ideal for IDs, rank numbers, or action icons. |
| **`FlexColumnWidth(factor)`**| `FlexColumnWidth(2.0f)` | Stretches proportionally to fill remaining horizontal space (`flex_grow`). |
| **`MinMaxColumnWidth(min, max)`** | `MinMaxColumnWidth(100.0f, 250.0f)` | Clamps column width within a lower and upper pixel threshold. |
| **`IntrinsicColumnWidth()`** | `IntrinsicColumnWidth()` | Automatically sizes the column to match its widest child cell. |

---

## C++ API Definition

### `TableRow` Model
```cpp
namespace enki {

enum class TableCellVerticalAlignment {
    Top,        ///< Align content to top of cell
    Middle,     ///< Center content vertically
    Bottom,     ///< Align content to bottom of cell
    Fill,       ///< Stretch cell content to fill row height
    Intrinsic   ///< Cell natural height
};

struct TableRow {
    std::vector<WidgetPtr>       cells;              ///< Widgets for each column
    std::optional<BoxDecoration> decoration;         ///< Optional row background fill/border
    TableCellVerticalAlignment   vertical_alignment = TableCellVerticalAlignment::Middle;

    TableRow() = default;
    explicit TableRow(std::vector<WidgetPtr> cells);
    TableRow(std::vector<WidgetPtr> cells, BoxDecoration dec);
    TableRow(std::vector<WidgetPtr> cells, BoxDecoration dec, TableCellVerticalAlignment align);
};

} // namespace enki
```

### `TableBorder` Builder
```cpp
namespace enki {

struct TableBorder {
    Border top, right, bottom, left;
    Border horizontal_inside;  ///< Dividers between rows
    Border vertical_inside;    ///< Dividers between columns
    BorderRadius border_radius = BorderRadius::zero();

    static TableBorder all(Color color = 0x33FFFFFF, float width = 1.0f);
    static TableBorder outline(Color color = 0x33FFFFFF, float width = 1.0f,
                               BorderRadius radius = BorderRadius::zero());
    static TableBorder symmetric(Color color = 0x33FFFFFF, float width = 1.0f);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Table {
    Key                                           key                        = Key::none();
    std::vector<TableRow>                         rows                       = {};
    std::vector<std::pair<int, TableColumnWidth>> column_widths              = {};
    TableColumnWidth                              default_column_width       = FlexColumnWidth(1.0f);
    TableBorder                                   border                     = {};
    TableCellVerticalAlignment                    default_vertical_alignment = TableCellVerticalAlignment::Middle;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `rows` | `std::vector<TableRow>` | `{}` | The sequence of table rows from top to bottom. |
| `column_widths` | `vector<pair<int, TableColumnWidth>>`| `{}` | Explicit column width strategy overrides by 0-indexed column index. |
| `default_column_width`| `TableColumnWidth` | `FlexColumnWidth(1.0f)`| Default width strategy for columns without an explicit override. |
| `border` | `TableBorder` | `{}` | Inner divider lines and outer border configuration. |
| `default_vertical_alignment`| `TableCellVerticalAlignment` | `Middle` | Default vertical positioning of content inside rows. |

---

## Code Examples (From `widgets_demo/table_demo/main.cpp`)

### 1. Tabular Programming Language Leaderboard
```cpp
#include "enki/widgets/table.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr makeCell(const std::string& str, bool isHeader = false) {
    return container({
        .padding = EdgeInsets::symmetric(10.0f, 12.0f),
        .child = text(str, {
            .color = isHeader ? 0xFF94A3B8 : 0xFFF1F5F9,
            .font_size = 13.0f,
            .font_weight = isHeader ? FontWeight::Bold : FontWeight::Normal,
        })
    });
}

WidgetPtr buildLanguagesTable() {
    std::vector<TableRow> rows;

    // Header Row with solid slate background
    rows.push_back(TableRow({
        makeCell("Rank", true),
        makeCell("Language", true),
        makeCell("Usage", true),
        makeCell("Type", true),
    }, BoxDecoration(0xFF1E293B)));

    // Data Rows
    rows.push_back(TableRow({ makeCell("1"), makeCell("C++20"), makeCell("42.5%"), makeCell("Compiled") }));
    rows.push_back(TableRow({ makeCell("2"), makeCell("Rust"),   makeCell("28.1%"), makeCell("Compiled") }));
    rows.push_back(TableRow({ makeCell("3"), makeCell("Python"), makeCell("18.4%"), makeCell("Interpreted") }));

    return Table {
        .rows = std::move(rows),
        .column_widths = {
            {0, FixedColumnWidth(60.0f)}, // 60px fixed rank column
            {1, FlexColumnWidth(2.0f)},   // Expands twice as much
            {2, FixedColumnWidth(90.0f)}, // 90px fixed usage column
            {3, FlexColumnWidth(1.0f)},
        },
        .border = TableBorder::symmetric(0x1AFFFFFF, 1.0f), // Row dividers
        .default_vertical_alignment = TableCellVerticalAlignment::Middle,
    };
}
```

---

## See Also
- [**DataTable**](./data_table.md) — Advanced interactive table with sorting and selection.
- [**ScrollView**](./scroll_view.md) — Horizontal scrolling container for wide tables.
