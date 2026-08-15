#include "enki/widgets/table.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderTable
// ════════════════════════════════════════════════════════════════

void RenderTable::setBorder(const TableBorder& b) { border_ = b; markNeedsPaint(); }

void RenderTable::paintBorders(PaintContext& ctx) {
    Rect bounds = Rect::fromPointSize(ctx.offset, size_);

    auto draw_border_line = [&](const Border& b, Point from, Point to) {
        if (b.width <= 0.0f || (b.color >> 24) == 0) return;
        Paint p;
        p.setColor(b.color);
        p.setStrokeWidth(b.width);
        ctx.canvas.drawLine(from, to, p);
    };

    // Outer borders
    draw_border_line(border_.top,    {bounds.x, bounds.y}, {bounds.x + bounds.width, bounds.y});
    draw_border_line(border_.bottom, {bounds.x, bounds.y + bounds.height}, {bounds.x + bounds.width, bounds.y + bounds.height});
    draw_border_line(border_.left,   {bounds.x, bounds.y}, {bounds.x, bounds.y + bounds.height});
    draw_border_line(border_.right,  {bounds.x + bounds.width, bounds.y}, {bounds.x + bounds.width, bounds.y + bounds.height});

    // Inner horizontal dividers — walk row children
    if (children_.empty() || !children_[0]) return;
    auto* col = children_[0];
    
    float y = bounds.y;
    for (int ri = 0; ri < (int)col->childCount() - 1; ++ri) {
        auto* row_child = col->children()[ri];
        if (!row_child) continue;
        y += row_child->size().height;
        draw_border_line(border_.horizontal_inside,
                         {bounds.x, y}, {bounds.x + bounds.width, y});
    }

    // Inner vertical dividers — walk first row's cell children for x positions
    if (col->childCount() > 0 && col->children()[0]) {
        float x = bounds.x;
        auto* first_row = col->children()[0];
        
        // If the row was wrapped in a decoration container, its child is the actual flex row
        if (first_row->childCount() == 1) {
            first_row = first_row->children()[0];
        }

        for (int ci = 0; ci < (int)first_row->childCount() - 1; ++ci) {
            auto* cell = first_row->children()[ci];
            if (!cell) continue;
            x += cell->size().width;
            draw_border_line(border_.vertical_inside,
                             {x, bounds.y}, {x, bounds.y + bounds.height});
        }
    }
}

void RenderTable::paint(PaintContext& ctx) {
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = ctx.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }
    paintBorders(ctx);
}

bool RenderTable::hitTestChildren(HitTestResult& result, Point localPoint) {
    for (auto* child : children_) {
        if (child && child->hitTest(result, localPoint - child->offset()))
            return true;
    }
    return false;
}

// ════════════════════════════════════════════════════════════════
// Table Widget — createRenderObject / updateRenderObject
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> TableBorderWidget::createRenderObject(BuildContext& ctx) {
    auto rt = std::make_unique<RenderTable>();
    rt->setBorder(border);
    return rt;
}

void TableBorderWidget::updateRenderObject(BuildContext& ctx, RenderObject& ro) {
    auto& rt = static_cast<RenderTable&>(ro);
    rt.setBorder(border);
}

WidgetPtr Table::build(BuildContext& ctx) {
    std::vector<WidgetPtr> row_widgets;
    row_widgets.reserve(rows.size());

    for (const auto& table_row : rows) {
        std::vector<WidgetPtr> cell_widgets;
        cell_widgets.reserve(table_row.cells.size());

        for (size_t col = 0; col < table_row.cells.size(); ++col) {
            auto cell_content = table_row.cells[col];
            if (!cell_content) {
                cell_content = std::make_shared<Container>();
            }
            
            // Find column width
            TableColumnWidth col_width = default_column_width;
            for (const auto& pair : column_widths_map) {
                if (pair.first == static_cast<int>(col)) {
                    col_width = pair.second;
                    break;
                }
            }

            auto flex_item = std::make_shared<FlexItem>(cell_content);
            
            // Apply width
            if (std::holds_alternative<FixedColumnWidth>(col_width)) {
                float w = std::get<FixedColumnWidth>(col_width).width;
                flex_item->flexBasis(StyleValue::point(w));
                flex_item->flexGrow(0.0f);
                flex_item->flexShrink(0.0f);
            } else if (std::holds_alternative<FlexColumnWidth>(col_width)) {
                float f = std::get<FlexColumnWidth>(col_width).flex_factor;
                flex_item->flexBasis(StyleValue::percent(0.0f));
                flex_item->flexGrow(f);
                flex_item->flexShrink(1.0f);
            } else if (std::holds_alternative<MinMaxColumnWidth>(col_width)) {
                auto minmax = std::get<MinMaxColumnWidth>(col_width);
                flex_item->minWidth(StyleValue::point(minmax.min_width));
                flex_item->maxWidth(StyleValue::point(minmax.max_width));
                flex_item->flexBasis(StyleValue::autoValue());
                flex_item->flexGrow(1.0f);
            } else if (std::holds_alternative<IntrinsicColumnWidth>(col_width)) {
                float f = std::get<IntrinsicColumnWidth>(col_width).flex_factor;
                flex_item->flexBasis(StyleValue::autoValue());
                flex_item->flexGrow(f);
            }
            
            cell_widgets.push_back(flex_item);
        }

        auto row_widget = row(std::move(cell_widgets));
        // Apply vertical alignment to the row
        auto align = table_row.vertical_alignment != TableCellVerticalAlignment::Middle 
                        ? table_row.vertical_alignment : default_vertical_alignment;
                        
        if (align == TableCellVerticalAlignment::Top) {
            row_widget->alignItems(Align::Start);
        } else if (align == TableCellVerticalAlignment::Middle) {
            row_widget->alignItems(Align::Center);
        } else if (align == TableCellVerticalAlignment::Bottom) {
            row_widget->alignItems(Align::End);
        } else if (align == TableCellVerticalAlignment::Fill) {
            row_widget->alignItems(Align::Stretch);
        }

        WidgetPtr final_row = row_widget;
        if (table_row.decoration.has_value()) {
            auto dec = container(row_widget);
            dec->decoration = *table_row.decoration;
            final_row = dec;
        }
        
        row_widgets.push_back(final_row);
    }

    auto col_widget = column(std::move(row_widgets));
    col_widget->width(StyleValue::percent(100.0f));
    
    return std::make_shared<TableBorderWidget>(table_border, col_widget);
}

} // namespace enki
