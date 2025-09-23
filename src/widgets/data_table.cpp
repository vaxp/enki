#include "enki/widgets/data_table.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/checkbox.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Helper: sort arrow widget
// ════════════════════════════════════════════════════════════════
static WidgetPtr buildSortArrow(bool ascending, Color color, float size) {
    auto arrow = text({
        .text = ascending ? " ▲" : " ▼",
        .color = color,
        .font_size = size,
    });
    return arrow;
}

// ════════════════════════════════════════════════════════════════
// DataTableState
// ════════════════════════════════════════════════════════════════

class DataTableState : public State {
    std::vector<bool> row_selected_;

    void initState() override {
        State::initState();
        auto* w = static_cast<const DataTableWidget*>(widget());
        row_selected_.resize(w->props.rows.size());
        for (int i = 0; i < (int)w->props.rows.size(); ++i)
            row_selected_[i] = w->props.rows[i].selected;
    }

    // Build a single cell widget applying padding and alignment
    WidgetPtr buildCell(const WidgetPtr& child, bool numeric,
                        bool placeholder, float h,
                        float col_spacing, const DataTableTheme& theme) {
        auto wrap = container(child);
        wrap->height(StyleValue::point(h));
        wrap->padding(EdgeInsets::symmetric(0, col_spacing * 0.5f));
        wrap->align(numeric ? Alignment::CenterRight : Alignment::CenterLeft);
        if (placeholder) {
            // Lower opacity visually — wrap in a semi-transparent container
            wrap->color(0x00000000); // transparent bg; placeholder handled by text style
        }
        return wrap;
    }

    WidgetPtr buildHeaderRow(const DataTableWidget* w) {
        const auto& theme = w->props.theme;
        std::vector<WidgetPtr> cells;

        // Checkbox column
        if (theme.show_checkbox_column) {
            bool all_sel = !row_selected_.empty() &&
                           std::all_of(row_selected_.begin(), row_selected_.end(), [](bool b){ return b; });

            auto cb = Checkbox {
                .value = all_sel,
                .on_changed = [this, w, all_sel](bool) {
                    bool new_val = !all_sel;
                    setState([this, new_val, w]() {
                        std::fill(row_selected_.begin(), row_selected_.end(), new_val);
                        if (w->props.on_select_all) w->props.on_select_all(new_val);
                    });
                },
                .active_color = theme.checkbox_color
            };
            
            auto cb_ptr = (WidgetPtr)cb;

            auto cb_cell = container(cb_ptr);
            cb_cell->width(theme.heading_row_height * 0.5f);
            cb_cell->height(StyleValue::point(theme.heading_row_height));
            cb_cell->padding(EdgeInsets::symmetric(0, theme.checkbox_h_margin));
            cb_cell->align(Alignment::Center);
            cells.push_back(cb_cell);
        }

        // Column headers
        for (int ci = 0; ci < (int)w->props.columns.size(); ++ci) {
            const auto& col = w->props.columns[ci];

            std::vector<WidgetPtr> header_row_children;

            auto lbl = std::make_shared<FlexItem>(col.label);
            lbl->flexGrow(1.0f).flexShrink(1.0f);
            header_row_children.push_back(lbl);

            // Sort arrow
            if (col.sortable && w->props.sort_column_index.has_value() &&
                *w->props.sort_column_index == ci) {
                header_row_children.push_back(
                    buildSortArrow(w->props.sort_ascending, theme.sort_arrow_color, theme.sort_arrow_size));
            }

            auto header_content = std::make_shared<Row>(std::move(header_row_children));
            header_content->alignItems(Align::Center);
            header_content->width(StyleValue::percent(100.0f));

            // Make sortable headers clickable
            WidgetPtr header_cell_content;
            if (col.sortable) {
                int idx = ci;
                bool asc = w->props.sort_ascending;
                std::optional<int> sort_idx = w->props.sort_column_index;
                auto on_sort = col.on_sort;
                header_cell_content = gestureDetector({
                    .child = header_content,
                    .hit_test_behavior = HitTestBehavior::Opaque,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = [on_sort, idx, sort_idx, asc]() {
                        if (on_sort) {
                            bool new_asc = (sort_idx.has_value() && *sort_idx == idx) ? !asc : true;
                            on_sort(idx, new_asc);
                        }
                    },
                });
            } else {
                header_cell_content = header_content;
            }

            auto cell_wrap = container(header_cell_content);
            float cell_w = col.column_width > 0 ? col.column_width : 0.0f;

            auto cell_fi = std::make_shared<FlexItem>(cell_wrap);
            if (cell_w > 0.0f) {
                cell_fi->width(StyleValue::point(cell_w));
                cell_fi->flexGrow(0.0f);
                cell_fi->flexShrink(0.0f);
            } else {
                cell_fi->flexGrow(col.flex_factor);
                cell_fi->flexShrink(1.0f);
            }
            cell_wrap->height(StyleValue::point(theme.heading_row_height));
            cell_wrap->padding(EdgeInsets::symmetric(0, theme.column_spacing * 0.5f));
            cell_wrap->align(col.numeric ? Alignment::CenterRight : Alignment::CenterLeft);

            cells.push_back(cell_fi);
        }

        auto header_row = std::make_shared<Row>(std::move(cells));
        header_row->alignItems(Align::Center);
        header_row->width(StyleValue::percent(100.0f));

        auto header_wrap = container(header_row);
        header_wrap->color(theme.heading_row_color);
        header_wrap->padding(EdgeInsets::symmetric(0, theme.horizontal_margin));
        header_wrap->width(StyleValue::percent(100.0f));
        return header_wrap;
    }

    WidgetPtr buildDataRow(int ri, const DataTableWidget* w) {
        const auto& theme = w->props.theme;
        const auto& dr = w->props.rows[ri];
        bool is_selected = ri < (int)row_selected_.size() && row_selected_[ri];

        std::vector<WidgetPtr> cells;

        // Checkbox
        if (theme.show_checkbox_column) {
            auto cb = Checkbox {
                .value = is_selected,
                .on_changed = [this, ri, w](bool val) {
                    setState([this, ri, val, w]() {
                        if (ri < (int)row_selected_.size()) {
                            row_selected_[ri] = val;
                            if (w->props.rows[ri].on_select_changed) w->props.rows[ri].on_select_changed(val);
                        }
                    });
                },
                .active_color = theme.checkbox_color
            };
            auto cb_ptr = (WidgetPtr)cb;

            auto cb_cell = container(cb_ptr);
            cb_cell->width(theme.heading_row_height * 0.5f);
            cb_cell->height(StyleValue::point(dr.height.value_or(theme.data_row_height)));
            cb_cell->padding(EdgeInsets::symmetric(0, theme.checkbox_h_margin));
            cb_cell->align(Alignment::Center);
            cells.push_back(cb_cell);
        }

        // Data cells
        for (int ci = 0; ci < (int)dr.cells.size() && ci < (int)w->props.columns.size(); ++ci) {
            const auto& dc = dr.cells[ci];
            const auto& col = w->props.columns[ci];

            WidgetPtr cell_content = dc.child;
            if (dc.on_tap || dc.on_double_tap) {
                cell_content = gestureDetector({
                    .child = cell_content,
                    .hit_test_behavior = HitTestBehavior::Opaque,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = dc.on_tap,
                    .on_double_tap = dc.on_double_tap,
                });
            }

            auto cell_wrap = container(cell_content);
            float cell_w = col.column_width > 0 ? col.column_width : 0.0f;

            cell_wrap->height(StyleValue::point(dr.height.value_or(theme.data_row_height)));
            cell_wrap->padding(EdgeInsets::symmetric(0, theme.column_spacing * 0.5f));
            cell_wrap->align(col.numeric ? Alignment::CenterRight : Alignment::CenterLeft);

            if (dc.placeholder) cell_wrap->color(0x40FFFFFF);

            auto cell_fi = std::make_shared<FlexItem>(cell_wrap);
            if (cell_w > 0.0f) {
                cell_fi->width(StyleValue::point(cell_w));
                cell_fi->flexGrow(0.0f).flexShrink(0.0f);
            } else {
                cell_fi->flexGrow(col.flex_factor).flexShrink(1.0f);
            }
            cells.push_back(cell_fi);
        }

        auto data_row = std::make_shared<Row>(std::move(cells));
        data_row->alignItems(Align::Center);
        data_row->width(StyleValue::percent(100.0f));

        // Row background
        Color row_bg = dr.color.value_or(
            (theme.use_alternating_rows && ri % 2 == 1) ? theme.data_row_alt_color
                                                         : theme.data_row_color);
        if (is_selected) row_bg = theme.selected_row_color;

        auto row_wrap = container(data_row);
        row_wrap->color(row_bg);
        row_wrap->padding(EdgeInsets::symmetric(0, theme.horizontal_margin));
        row_wrap->width(StyleValue::percent(100.0f));

        // Make row tappable
        if (dr.on_tap || dr.on_select_changed || w->props.on_row_tap) {
            int row_idx = ri;
            auto row_tap = dr.on_tap;
            auto global_tap = w->props.on_row_tap;
            return gestureDetector({
                .child = row_wrap,
                .hit_test_behavior = HitTestBehavior::Opaque,
                .cursor_type = SystemCursor::Pointer,
                .on_tap = [row_tap, global_tap, row_idx]() {
                    if (row_tap)    row_tap();
                    if (global_tap) global_tap(row_idx);
                },
            });
        }

        return row_wrap;
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const DataTableWidget*>(widget());
        const auto& theme = w->props.theme;

        // Sync row_selected_ size
        while (row_selected_.size() < w->props.rows.size())
            row_selected_.push_back(false);

        std::vector<WidgetPtr> col_children;

        // Header
        col_children.push_back(buildHeaderRow(w));

        // Horizontal divider after header
        {
            auto hdiv = container();
            hdiv->color(theme.divider_color);
            hdiv->height(StyleValue::point(theme.divider_thickness));
            hdiv->width(StyleValue::percent(100.0f));
            col_children.push_back(hdiv);
        }

        // Data rows with dividers
        for (int ri = 0; ri < (int)w->props.rows.size(); ++ri) {
            col_children.push_back(buildDataRow(ri, w));

            bool last_row = (ri == (int)w->props.rows.size() - 1);
            if (!last_row || theme.show_bottom_border) {
                auto div = container();
                div->color(theme.divider_color);
                div->height(StyleValue::point(theme.divider_thickness));
                div->width(StyleValue::percent(100.0f));
                col_children.push_back(div);
            }
        }

        auto table_col = std::make_shared<Column>(std::move(col_children));
        table_col->width(StyleValue::percent(100.0f));
        table_col->flexShrink(0.0f);

        // Wrap in horizontal scroll for wide tables
        if (w->props.horizontal_scroll) {
            ScrollOptions horiz_opts;
            horiz_opts.direction = Axis::Horizontal;
            horiz_opts.show_scrollbar = true;
            horiz_opts.clamp_overscroll = true;

            auto horiz_scroll = scrollView(horiz_opts, table_col);

            ScrollOptions vert_opts;
            vert_opts.direction = Axis::Vertical;
            vert_opts.show_scrollbar = true;
            vert_opts.scroll_speed = w->props.scroll_speed;
            vert_opts.clamp_overscroll = true;

            return scrollView(vert_opts, horiz_scroll);
        }

        ScrollOptions vert_opts;
        vert_opts.direction = Axis::Vertical;
        vert_opts.show_scrollbar = true;
        vert_opts.scroll_speed = w->props.scroll_speed;
        return scrollView(vert_opts, table_col);
    }
};

std::unique_ptr<State> DataTableWidget::createState() {
    return std::make_unique<DataTableState>();
}

} // namespace enki
