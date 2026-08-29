#include "enki/widgets/sliver.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/state/state.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. SliverToBoxAdapter Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr SliverToBoxAdapterWidget::build(BuildContext& ctx) {
    if (!child) {
        return sizedBox(0.0f, 0.0f);
    }
    return child;
}

// ════════════════════════════════════════════════════════════════
// 2. SliverPadding Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr SliverPaddingWidget::build(BuildContext& ctx) {
    if (!sliver) {
        return sizedBox(0.0f, 0.0f);
    }
    if (padding == EdgeInsets{}) {
        return sliver;
    }
    return container({
        .padding = StyleInsets::only(padding.top, padding.right, padding.bottom, padding.left),
        .child = sliver,
    });
}

// ════════════════════════════════════════════════════════════════
// 3. SliverList Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr SliverListWidget::build(BuildContext& ctx) {
    std::vector<WidgetPtr> children;

    auto add_item = [&](int idx, WidgetPtr item) {
        if (!item) return;
        // Optional separator before this item (except the very first)
        if (idx > 0 && props.separator_builder) {
            if (auto sep = props.separator_builder(idx - 1)) {
                children.push_back(sep);
            }
        }
        children.push_back(item);
    };

    if (props.item_builder) {
        for (int i = 0; i < props.item_count; ++i) {
            add_item(i, props.item_builder(i));
        }
    } else {
        for (int i = 0; i < static_cast<int>(props.items.size()); ++i) {
            add_item(i, props.items[i]);
        }
    }

    WidgetPtr col = column({
        .flex_shrink = 0.0f,
        .width = StyleValue::percent(100.0f),
        .children = std::move(children),
    });

    if (props.padding != EdgeInsets{}) {
        return container({
            .padding = StyleInsets::only(props.padding.top, props.padding.right, props.padding.bottom, props.padding.left),
            .child = col,
        });
    }

    return col;
}

// ════════════════════════════════════════════════════════════════
// 4. SliverGrid Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr SliverGridWidget::build(BuildContext& ctx) {
    std::vector<WidgetPtr> raw_items;
    if (props.item_builder) {
        for (int i = 0; i < props.item_count; ++i) {
            if (auto itm = props.item_builder(i)) {
                raw_items.push_back(itm);
            }
        }
    } else {
        raw_items = props.items;
    }

    float row_gap = props.use_max_extent_delegate
                    ? props.max_delegate.main_axis_spacing
                    : props.fixed_delegate.main_axis_spacing;
    float col_gap = props.use_max_extent_delegate
                    ? props.max_delegate.cross_axis_spacing
                    : props.fixed_delegate.cross_axis_spacing;

    std::vector<WidgetPtr> cells;
    cells.reserve(raw_items.size());

    for (auto& item : raw_items) {
        auto padded_item = container({
            .padding = StyleInsets::symmetric(row_gap / 2.0f, col_gap / 2.0f),
            .child = item,
        });

        if (props.use_max_extent_delegate) {
            float h_val = props.max_delegate.main_axis_extent.has_value()
                          ? *props.max_delegate.main_axis_extent : 0.0f;
            bool has_h = props.max_delegate.main_axis_extent.has_value();

            FlexItemProps cell_props;
            cell_props.flex_grow = 1.0f;
            cell_props.flex_shrink = 0.0f;
            cell_props.flex_basis = StyleValue::point(props.max_delegate.max_cross_axis_extent);
            if (has_h) {
                cell_props.height = StyleValue::point(h_val);
            } else {
                cell_props.aspect_ratio = props.max_delegate.child_aspect_ratio;
            }
            cell_props.child = padded_item;
            cells.push_back(flexItem(cell_props));
        } else {
            int count = std::max(1, props.fixed_delegate.cross_axis_count);
            float basis_pct = 100.0f / static_cast<float>(count);
            bool has_h = props.fixed_delegate.main_axis_extent.has_value();
            float h_val = has_h ? *props.fixed_delegate.main_axis_extent : 0.0f;

            FlexItemProps cell_props;
            cell_props.flex_grow = 0.0f;
            cell_props.flex_shrink = 0.0f;
            cell_props.flex_basis = StyleValue::percent(basis_pct);
            if (has_h) {
                cell_props.height = StyleValue::point(h_val);
            } else {
                cell_props.aspect_ratio = props.fixed_delegate.child_aspect_ratio;
            }
            cell_props.child = padded_item;
            cells.push_back(flexItem(cell_props));
        }
    }

    WidgetPtr grid = wrap({
        .flex_shrink = 0.0f,
        .width = StyleValue::percent(100.0f),
        .children = std::move(cells),
    });

    if (props.padding != EdgeInsets{}) {
        return container({
            .padding = StyleInsets::only(props.padding.top, props.padding.right, props.padding.bottom, props.padding.left),
            .child = grid,
        });
    }

    return grid;
}

// ════════════════════════════════════════════════════════════════
// 5. SliverAppBar State Implementation
// ════════════════════════════════════════════════════════════════

class SliverAppBarState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const SliverAppBarWidget*>(widget());
        const auto& p = w->props;

        float exp_h = std::max(p.expanded_height, p.collapsed_height);
        float col_h = p.collapsed_height > 0.0f ? p.collapsed_height : 56.0f;

        // Toolbar Row (leading + title + actions)
        std::vector<WidgetPtr> toolbar_items;

        if (p.leading) {
            toolbar_items.push_back(container({
                .align = Alignment::Center,
                .width = StyleValue::point(48.0f),
                .height = StyleValue::point(col_h),
                .child = p.leading,
            }));
        }

        if (p.title) {
            if (p.center_title) {
                toolbar_items.push_back(container({
                    .align = Alignment::Center,
                    .height = StyleValue::point(col_h),
                    .flex_grow = 1.0f,
                    .child = p.title,
                }));
            } else {
                toolbar_items.push_back(container({
                    .align = Alignment::CenterLeft,
                    .height = StyleValue::point(col_h),
                    .padding = StyleInsets::symmetric(0.0f, 12.0f),
                    .flex_grow = 1.0f,
                    .child = p.title,
                }));
            }
        } else {
            toolbar_items.push_back(spacer());
        }

        if (!p.actions.empty()) {
            toolbar_items.push_back(row({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .height = StyleValue::point(col_h),
                .children = p.actions,
            }));
        }

        WidgetPtr toolbar = container({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(col_h),
            .padding = StyleInsets::symmetric(0.0f, 8.0f),
            .child = row({
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = std::move(toolbar_items),
            }),
        });

        // Layer stack: flexible space / background layer + toolbar layer + bottom layer
        std::vector<WidgetPtr> layers;

        // 1. Flexible Space / Background
        if (p.flexible_space) {
            PositionedProps pos_props;
            pos_props.child = p.flexible_space;
            pos_props.top = StyleValue::point(0.0f);
            pos_props.right = StyleValue::point(0.0f);
            pos_props.bottom = StyleValue::point(0.0f);
            pos_props.left = StyleValue::point(0.0f);
            layers.push_back(positioned(pos_props));
        }

        // 2. Toolbar at the top
        PositionedProps tb_pos;
        tb_pos.child = toolbar;
        tb_pos.top = StyleValue::point(0.0f);
        tb_pos.right = StyleValue::point(0.0f);
        tb_pos.left = StyleValue::point(0.0f);
        tb_pos.height = StyleValue::point(col_h);
        layers.push_back(positioned(tb_pos));

        // 3. Optional Bottom widget (e.g. TabBar)
        if (p.bottom) {
            PositionedProps btm_pos;
            btm_pos.child = p.bottom;
            btm_pos.right = StyleValue::point(0.0f);
            btm_pos.bottom = StyleValue::point(0.0f);
            btm_pos.left = StyleValue::point(0.0f);
            layers.push_back(positioned(btm_pos));
        }

        std::vector<BoxShadow> shadows;
        if (p.elevation > 0.0f) {
            shadows.push_back(BoxShadow(
                Color(0x40000000),
                Point{0.0f, p.elevation * 0.5f},
                p.elevation * 1.5f,
                0.0f,
                false
            ));
        }

        return container({
            .color = p.background_color,
            .box_shadow = std::move(shadows),
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(exp_h),
            .child = stack({
                .children = std::move(layers),
            }),
        });
    }
};

std::unique_ptr<State> SliverAppBarWidget::createState() {
    return std::make_unique<SliverAppBarState>();
}

// ════════════════════════════════════════════════════════════════
// 6. CustomScrollView State Implementation
// ════════════════════════════════════════════════════════════════

class CustomScrollViewState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const CustomScrollViewWidget*>(widget());
        const auto& p = w->props;

        // Build all slivers sequentially in a Flex Column/Row
        std::vector<WidgetPtr> sliver_children;
        sliver_children.reserve(p.slivers.size());

        for (const auto& slv : p.slivers) {
            if (slv) {
                sliver_children.push_back(slv);
            }
        }

        WidgetPtr content;
        if (p.direction == Axis::Vertical) {
            content = column({
                .flex_shrink = 0.0f,
                .width = StyleValue::percent(100.0f),
                .children = std::move(sliver_children),
            });
        } else {
            content = row({
                .flex_shrink = 0.0f,
                .height = StyleValue::percent(100.0f),
                .children = std::move(sliver_children),
            });
        }

        if (p.shrink_wrap) {
            return content;
        }

        ScrollOptions scroll_opts;
        scroll_opts.direction = p.direction;
        scroll_opts.scroll_speed = p.scroll_speed;
        scroll_opts.show_scrollbar = p.show_scrollbar;
        scroll_opts.clamp_overscroll = (p.scroll_physics == ScrollPhysics::Clamped);

        return scrollView(scroll_opts, content);
    }
};

std::unique_ptr<State> CustomScrollViewWidget::createState() {
    return std::make_unique<CustomScrollViewState>();
}

} // namespace enki
