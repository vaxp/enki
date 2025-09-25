#include "enki/widgets/grid_tile.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/tree/build_context.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// GridTileBar::build
// ════════════════════════════════════════════════════════════════

WidgetPtr GridTileBarWidget::build(BuildContext& ctx) {
    std::vector<WidgetPtr> row_children;

    if (props.leading_widget) {
        row_children.push_back(props.leading_widget);
        auto gap = container();
        gap->width(props.leading_gap);
        gap->height(StyleValue::percent(100.0f));
        row_children.push_back(gap);
    }

    // Title + subtitle
    std::vector<WidgetPtr> text_children;
    if (props.title_widget)    text_children.push_back(props.title_widget);
    if (props.subtitle_widget) text_children.push_back(props.subtitle_widget);

    WidgetPtr text_col;
    if (text_children.size() == 1) {
        text_col = text_children[0];
    } else if (!text_children.empty()) {
        text_col = column({
            .gap = StyleValue::point(2.0f),
            .children = std::move(text_children),
        });
    }

    if (text_col) {
        auto expanded_item = flexItem({
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .child = text_col,
        });
        row_children.push_back(expanded_item);
    }

    if (props.trailing_widget) {
        auto gap = container();
        gap->width(props.leading_gap);
        gap->height(StyleValue::percent(100.0f));
        row_children.push_back(gap);
        row_children.push_back(props.trailing_widget);
    }

    auto content_row = row({
        .align_items = Align::Center,
        .width = StyleValue::percent(100.0f),
        .children = std::move(row_children),
    });

    auto bar = container(content_row);
    bar->color(props.background_color);
    bar->padding(EdgeInsets::symmetric(props.padding_vertical, props.padding_horizontal));

    return bar;
}

// ════════════════════════════════════════════════════════════════
// GridTile::build
// ════════════════════════════════════════════════════════════════

WidgetPtr GridTileWidget::build(BuildContext& ctx) {
    std::vector<WidgetPtr> stack_children;

    // Child fills the entire tile (Stack expand)
    if (props.child) {
        stack_children.push_back(Positioned::fill(props.child));
    }

    // Header — anchored to top
    if (props.header) {
        stack_children.push_back(Positioned {
            .child = props.header,
            .top = StyleValue::point(0.0f),
            .right = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
        });
    }

    // Footer — anchored to bottom
    if (props.footer) {
        stack_children.push_back(Positioned {
            .child = props.footer,
            .right = StyleValue::point(0.0f),
            .bottom = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
        });
    }

    return Stack {
        .fit = StackFit::Expand,
        .clip_behavior = Clip::HardEdge,
        .children = std::move(stack_children),
    };
}

} // namespace enki
