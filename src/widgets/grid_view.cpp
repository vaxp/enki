#include "enki/widgets/grid_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/state/state.hpp"

namespace enki {

class GridViewState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const GridViewWidget*>(widget());

        // ── Gather items ───────────────────────────────────────
        std::vector<WidgetPtr> raw_items;
        if (w->props.item_builder) {
            for (int i = 0; i < w->props.item_count; ++i)
                raw_items.push_back(w->props.item_builder(i));
        } else {
            raw_items = w->props.items;
        }

        // ── Compute cell sizing via Anu flex_basis ─────────────
        // For a fixed-count grid of N columns, each cell gets flex_basis = 100%/N
        // with mainAxis size controlled by aspect_ratio.
        // For max-extent grids, we use flex_basis = max_extent with flex_grow/shrink.
        // ALL math happens in Anu — we only pass the StyleValue.

        float row_gap = w->props.use_max_extent_delegate
                        ? w->props.max_delegate.main_axis_spacing
                        : w->props.fixed_delegate.main_axis_spacing;
        float col_gap = w->props.use_max_extent_delegate
                        ? w->props.max_delegate.cross_axis_spacing
                        : w->props.fixed_delegate.cross_axis_spacing;

        std::vector<WidgetPtr> cells;
        cells.reserve(raw_items.size());

        for (auto& item : raw_items) {
            auto padded_item = container(item);
            padded_item->padding(EdgeInsets::symmetric(row_gap / 2.0f, col_gap / 2.0f));

            auto cell = std::make_shared<FlexItem>(padded_item);

            if (w->props.use_max_extent_delegate) {
                // Max-extent: set basis to max extent, let Anu distribute
                cell->flexBasis(StyleValue::point(w->props.max_delegate.max_cross_axis_extent));
                cell->flexGrow(1.0f);
                cell->flexShrink(0.0f);
                if (w->props.max_delegate.main_axis_extent.has_value()) {
                    if (w->props.direction == Axis::Vertical)
                        cell->height(StyleValue::point(*w->props.max_delegate.main_axis_extent));
                    else
                        cell->width(StyleValue::point(*w->props.max_delegate.main_axis_extent));
                } else {
                    // Use aspect ratio
                    cell->aspectRatio(w->props.max_delegate.child_aspect_ratio);
                }
            } else {
                // Fixed count: basis = 100% / count (as percent), Anu does the rest
                float basis_pct = 100.0f / static_cast<float>(w->props.fixed_delegate.cross_axis_count);
                cell->flexBasis(StyleValue::percent(basis_pct));
                cell->flexGrow(0.0f);
                cell->flexShrink(0.0f);
                if (w->props.fixed_delegate.main_axis_extent.has_value()) {
                    if (w->props.direction == Axis::Vertical)
                        cell->height(StyleValue::point(*w->props.fixed_delegate.main_axis_extent));
                    else
                        cell->width(StyleValue::point(*w->props.fixed_delegate.main_axis_extent));
                } else {
                    cell->aspectRatio(w->props.fixed_delegate.child_aspect_ratio);
                }
            }
            cells.push_back(cell);
        }

        // ── Build wrap flexbox ─────────────────────────────────
        auto grid = std::make_shared<Wrap>(std::move(cells));

        if (w->props.direction == Axis::Vertical) {
            grid->width(StyleValue::percent(100.0f));
            grid->flexShrink(0.0f);
        } else {
            grid->flexDirection(FlexDirection::Column);
            grid->flexWrap(FlexWrap::Wrap);
            grid->height(StyleValue::percent(100.0f));
            grid->flexShrink(0.0f);
        }

        // Apply padding
        WidgetPtr content;
        auto current_padding = w->props.list_padding;
        EdgeInsets adjusted_padding = {
            std::max(0.0f, current_padding.top - row_gap / 2.0f),
            std::max(0.0f, current_padding.right - col_gap / 2.0f),
            std::max(0.0f, current_padding.bottom - row_gap / 2.0f),
            std::max(0.0f, current_padding.left - col_gap / 2.0f)
        };

        if (adjusted_padding != EdgeInsets{}) {
            auto pc = container(grid);
            pc->padding(adjusted_padding);
            content = pc;
        } else {
            content = grid;
        }

        if (w->props.shrink_wrap) return content;

        // ── Wrap in ScrollView ─────────────────────────────────
        ScrollOptions opts;
        opts.direction    = w->props.direction;
        opts.scroll_speed = w->props.scroll_speed;
        opts.show_scrollbar = true;
        opts.clamp_overscroll = (w->props.scroll_physics == ScrollPhysics::Clamped);

        return scrollView(opts, content);
    }
};

std::unique_ptr<State> GridViewWidget::createState() {
    return std::make_unique<GridViewState>();
}

} // namespace enki
