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
        return container({
            .color = placeholder ? std::optional<Color>(0x00000000) : std::nullopt,
            .align = numeric ? Alignment::CenterRight : Alignment::CenterLeft,
            .height = StyleValue::point(h),
            .padding = StyleInsets::symmetric(0.0f, col_spacing * 0.5f),
            .child = child,
        });
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

            auto cb_cell = container({
                .align = Alignment::Center,
                .width = StyleValue::point(theme.heading_row_height * 0.5f),
                .height = StyleValue::point(theme.heading_row_height),
                .padding = StyleInsets::symmetric(0.0f, theme.checkbox_h_margin),
                .child = cb_ptr,
            });
            cells.push_back(cb_cell);
        }

        // Column headers
        for (int ci = 0; ci < (int)w->props.columns.size(); ++ci) {
            const auto& col = w->props.columns[ci];

            std::vector<WidgetPtr> header_row_children;

            auto lbl = flexItem({
                .flex_grow = 1.0f,
                .flex_shrink = 1.0f,
                .child = col.label,
            });
            header_row_children.push_back(lbl);

            // Sort arrow
            if (col.sortable && w->props.sort_column_index.has_value() &&
                *w->props.sort_column_index == ci) {
                header_row_children.push_back(
                    buildSortArrow(w->props.sort_ascending, theme.sort_arrow_color, theme.sort_arrow_size));
            }

            auto header_content = row({
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .children = std::move(header_row_children),
            });

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

            float cell_w = col.column_width > 0 ? col.column_width : 0.0f;
            auto cell_wrap = container({
                .align = col.numeric ? Alignment::CenterRight : Alignment::CenterLeft,
                .height = StyleValue::point(theme.heading_row_height),
                .padding = StyleInsets::symmetric(0.0f, theme.column_spacing * 0.5f),
                .child = header_cell_content,
            });

            auto cell_fi = flexItem({
                .flex_grow = (cell_w > 0.0f) ? 0.0f : col.flex_factor,
                .flex_shrink = (cell_w > 0.0f) ? 0.0f : 1.0f,
                .width = (cell_w > 0.0f) ? std::optional<StyleValue>(StyleValue::point(cell_w)) : std::nullopt,
                .child = cell_wrap,
            });

            cells.push_back(cell_fi);
        }

        auto header_row = row({
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = std::move(cells),
        });

        auto header_wrap = container({
            .color = theme.heading_row_color,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(0.0f, theme.horizontal_margin),
            .child = header_row,
        });
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

            auto cb_cell = container({
                .align = Alignment::Center,
                .width = StyleValue::point(theme.heading_row_height * 0.5f),
                .height = StyleValue::point(dr.height.value_or(theme.data_row_height)),
                .padding = StyleInsets::symmetric(0.0f, theme.checkbox_h_margin),
                .child = cb_ptr,
            });
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

            float cell_w = col.column_width > 0 ? col.column_width : 0.0f;
            auto cell_wrap = container({
                .color = dc.placeholder ? std::optional<Color>(0x40FFFFFF) : std::nullopt,
                .align = col.numeric ? Alignment::CenterRight : Alignment::CenterLeft,
                .height = StyleValue::point(dr.height.value_or(theme.data_row_height)),
                .padding = StyleInsets::symmetric(0.0f, theme.column_spacing * 0.5f),
                .child = cell_content,
            });

            auto cell_fi = flexItem({
                .flex_grow = (cell_w > 0.0f) ? 0.0f : col.flex_factor,
                .flex_shrink = (cell_w > 0.0f) ? 0.0f : 1.0f,
                .width = (cell_w > 0.0f) ? std::optional<StyleValue>(StyleValue::point(cell_w)) : std::nullopt,
                .child = cell_wrap,
            });
            cells.push_back(cell_fi);
        }

        auto data_row = row({
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = std::move(cells),
        });

        // Row background
        Color row_bg = dr.color.value_or(
            (theme.use_alternating_rows && ri % 2 == 1) ? theme.data_row_alt_color
                                                         : theme.data_row_color);
        if (is_selected) row_bg = theme.selected_row_color;

        auto row_wrap = container({
            .color = row_bg,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(0.0f, theme.horizontal_margin),
            .child = data_row,
        });

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
            auto hdiv = container({
                .color = theme.divider_color,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::point(theme.divider_thickness),
            });
            col_children.push_back(hdiv);
        }

        // Data rows with dividers
        for (int ri = 0; ri < (int)w->props.rows.size(); ++ri) {
            col_children.push_back(buildDataRow(ri, w));

            bool last_row = (ri == (int)w->props.rows.size() - 1);
            if (!last_row || theme.show_bottom_border) {
                auto div = container({
                    .color = theme.divider_color,
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(theme.divider_thickness),
                });
                col_children.push_back(div);
            }
        }

        auto table_col = column({
            .flex_shrink = 0.0f,
            .width = StyleValue::percent(100.0f),
            .children = std::move(col_children),
        });

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
