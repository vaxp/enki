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
        auto col = std::make_shared<Column>(std::move(text_children));
        col->gap(StyleValue::point(2.0f));
        text_col = col;
    }

    if (text_col) {
        auto expanded = std::make_shared<FlexItem>(text_col);
        expanded->flexGrow(1.0f).flexShrink(1.0f);
        row_children.push_back(expanded);
    }

    if (props.trailing_widget) {
        auto gap = container();
        gap->width(props.leading_gap);
        gap->height(StyleValue::percent(100.0f));
        row_children.push_back(gap);
        row_children.push_back(props.trailing_widget);
    }

    auto content_row = std::make_shared<Row>(std::move(row_children));
    content_row->alignItems(Align::Center);
    content_row->width(StyleValue::percent(100.0f));

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
        auto hdr = std::make_shared<Positioned>(props.header);
        hdr->top(0).left(0).right(0);
        stack_children.push_back(hdr);
    }

    // Footer — anchored to bottom
    if (props.footer) {
        auto ftr = std::make_shared<Positioned>(props.footer);
        ftr->bottom(0).left(0).right(0);
        stack_children.push_back(ftr);
    }

    auto s = std::make_shared<Stack>(std::move(stack_children));
    s->clip(Clip::HardEdge);
    s->fit(StackFit::Expand);
    return s;
}

} // namespace enki
