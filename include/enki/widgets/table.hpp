#pragma once
/// @file table.hpp
/// @brief Table widget — a fixed-layout grid of cells with configurable column widths,
///        row decorations, borders, and vertical alignment.
///
/// Features:
///   - FixedColumn, FlexColumn, MinMaxColumn width strategies (Anu flex_basis / flex_grow).
///   - Per-row BoxDecoration (background, border, highlight).
///   - TableBorder for inner/outer borders (horizontal dividers, vertical dividers).
///   - Per-cell vertical alignment (Top, Center, Bottom, Fill/Baseline).
///   - TextDirection-aware (RTL support).
///   - Optional header row with distinct styling.
///   - Horizontal scrolling for wide tables.
///   - All Anu-driven: each row is a Flexbox Row, columns are FlexItem nodes,
///     TableBorder lines are drawn by the RenderTable during paint.
///
/// Architecture:
///   TableWidget (StatelessWidget)
///     └── TableBorderWidget (SingleChildRenderObjectWidget)
///           └── RenderTable
///                 ├── [Row 0] → Anu Flexbox Row → [Cell 0] [Cell 1] ... [Cell N]
///                 ├── [Row 1] → Anu Flexbox Row → ...
///                 └── ...
///   (RenderTable draws border lines in paint phase using Canvas::drawLine)
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/widgets/container.hpp"  // BoxDecoration
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Column Width Strategies
// ════════════════════════════════════════════════════════════════

/// @brief Fixed pixel width for a column.
struct FixedColumnWidth {
    float width;
    explicit FixedColumnWidth(float w) : width(w) {}
};

/// @brief Flexible column that grows/shrinks to fill available space.
/// Anu receives flex_grow = flex_factor, flex_shrink = flex_factor.
struct FlexColumnWidth {
    float flex_factor = 1.0f;
    explicit FlexColumnWidth(float f = 1.0f) : flex_factor(f) {}
};

/// @brief Column with min and max pixel width constraints.
/// Anu receives min_width + max_width on the FlexItem node.
struct MinMaxColumnWidth {
    float min_width;
    float max_width;
    MinMaxColumnWidth(float min, float max) : min_width(min), max_width(max) {}
};

/// @brief Intrinsic column — sized to its widest cell content.
/// Anu receives width = auto on the FlexItem.
struct IntrinsicColumnWidth {
    float flex_factor = 0.0f;   ///< If > 0, distributes remaining space.
    explicit IntrinsicColumnWidth(float f = 0.0f) : flex_factor(f) {}
};

/// @brief Variant type for column width specification.
using TableColumnWidth = std::variant<
    FixedColumnWidth,
    FlexColumnWidth,
    MinMaxColumnWidth,
    IntrinsicColumnWidth
>;

// ════════════════════════════════════════════════════════════════
// TableCellVerticalAlignment
// ════════════════════════════════════════════════════════════════

enum class TableCellVerticalAlignment {
    Top,        ///< Align cell content to the top.
    Middle,     ///< Center cell content vertically.
    Bottom,     ///< Align cell content to the bottom.
    Fill,       ///< Stretch cell content to fill the row height.
    Intrinsic,  ///< Use the cell's natural height.
};

// ════════════════════════════════════════════════════════════════
// TableBorder
// ════════════════════════════════════════════════════════════════

/// @brief Specifies the borders of a Table (outer frame + inner dividers).
struct TableBorder {
    Border top;
    Border right;
    Border bottom;
    Border left;
    Border horizontal_inside;  ///< Dividers between rows.
    Border vertical_inside;    ///< Dividers between columns.
    BorderRadius border_radius = BorderRadius::zero();

    constexpr TableBorder() = default;

    /// Uniform border on all sides and inner dividers.
    static TableBorder all(Color color = 0x33FFFFFF, float width = 1.0f) {
        Border b(color, width);
        TableBorder tb;
        tb.top = tb.right = tb.bottom = tb.left = b;
        tb.horizontal_inside = tb.vertical_inside = b;
        return tb;
    }

    /// Only outer border, no inner dividers.
    static TableBorder outline(Color color = 0x33FFFFFF, float width = 1.0f,
                                BorderRadius radius = BorderRadius::zero()) {
        Border b(color, width);
        TableBorder tb;
        tb.top = tb.right = tb.bottom = tb.left = b;
        tb.border_radius = radius;
        return tb;
    }

    /// Only horizontal (row) dividers.
    static TableBorder symmetric(Color color = 0x33FFFFFF, float width = 1.0f) {
        TableBorder tb;
        tb.horizontal_inside = Border(color, width);
        return tb;
    }

    constexpr bool operator==(const TableBorder&) const = default;
};

// ════════════════════════════════════════════════════════════════
// TableRow — a single row of cells
// ════════════════════════════════════════════════════════════════

/// @brief A row in the Table, containing one WidgetPtr per column.
struct TableRow {
    std::vector<WidgetPtr> cells = {};            ///< One widget per column. May be nullptr (empty cell).
    std::optional<BoxDecoration> decoration;      ///< Optional row background/border styling.
    TableCellVerticalAlignment   vertical_alignment = TableCellVerticalAlignment::Middle;

    TableRow() = default;
    explicit TableRow(std::vector<WidgetPtr> cells) : cells(std::move(cells)) {}
    TableRow(std::vector<WidgetPtr> cells, BoxDecoration dec)
        : cells(std::move(cells)), decoration(std::move(dec)) {}
    TableRow(std::vector<WidgetPtr> cells, BoxDecoration dec, TableCellVerticalAlignment align)
        : cells(std::move(cells)), decoration(std::move(dec)), vertical_alignment(align) {}
};

// ════════════════════════════════════════════════════════════════
// RenderTable — custom render object for table
// ════════════════════════════════════════════════════════════════

/// @brief Render object for Table. Uses Anu for per-row flex layout and
///        draws TableBorder lines in the paint phase.
class RenderTable : public RenderBox {
public:
    RenderTable() = default;
    ~RenderTable() override = default;

    void setRows(const std::vector<TableRow>& rows);
    void setColumnWidths(const std::vector<TableColumnWidth>& widths);
    void setDefaultColumnWidth(const TableColumnWidth& def);
    void setBorder(const TableBorder& border);
    void setDefaultVerticalAlignment(TableCellVerticalAlignment alignment);

    // ── RenderBox interface ────────────────────────────────────
    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;

private:
    std::vector<TableRow>        rows_;
    std::vector<TableColumnWidth> column_widths_;
    TableColumnWidth             default_column_width_ = FlexColumnWidth(1.0f);
    TableBorder                  border_;
    TableCellVerticalAlignment   default_vertical_alignment_ = TableCellVerticalAlignment::Middle;

    void paintBorders(PaintContext& ctx);
};

// ════════════════════════════════════════════════════════════════
// TableBorderWidget
// ════════════════════════════════════════════════════════════════

class TableBorderWidget : public SingleChildRenderObjectWidget {
public:
    TableBorder border;
    TableBorderWidget(TableBorder b, WidgetPtr c)
        : border(b) {
        this->child = std::move(c);
    }
    
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "TableBorderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// TableProps & TableWidget Implementation
// ════════════════════════════════════════════════════════════════

struct TableProps {
    Key key = Key::none();
    std::vector<TableRow> rows;
    std::vector<std::pair<int, TableColumnWidth>> column_widths_map; ///< Per-column overrides.
    TableColumnWidth default_column_width = FlexColumnWidth(1.0f);
    TableBorder table_border;
    TableCellVerticalAlignment default_vertical_alignment = TableCellVerticalAlignment::Middle;
};

class TableWidget : public StatelessWidget {
public:
    TableProps props;

    TableWidget() = default;
    explicit TableWidget(TableProps p) : StatelessWidget(p.key), props(std::move(p)) {}
    TableWidget(Key k, TableProps p) : StatelessWidget(std::move(k)), props(std::move(p)) {}

    WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Table"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Table Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Table {
    Key key = Key::none();
    std::vector<TableRow> rows = {};
    std::vector<std::pair<int, TableColumnWidth>> column_widths_map = {};
    std::vector<std::pair<int, TableColumnWidth>> column_widths = {};
    TableColumnWidth default_column_width = FlexColumnWidth(1.0f);
    TableBorder table_border = {};
    TableBorder border = {};
    TableCellVerticalAlignment default_vertical_alignment = TableCellVerticalAlignment::Middle;

    operator WidgetPtr() const {
        TableProps p;
        p.key = key;
        p.rows = rows;
        p.column_widths_map = !column_widths.empty() ? column_widths : column_widths_map;
        p.default_column_width = default_column_width;
        p.table_border = (table_border != TableBorder{}) ? table_border : border;
        p.default_vertical_alignment = default_vertical_alignment;
        return std::make_shared<TableWidget>(key, std::move(p));
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<TableWidget> table(TableProps props = {}) {
    auto k = props.key;
    return std::make_shared<TableWidget>(k, std::move(props));
}

inline std::shared_ptr<TableWidget> table(std::vector<TableRow> rows) {
    TableProps props;
    props.rows = std::move(rows);
    return std::make_shared<TableWidget>(std::move(props));
}

} // namespace enki
