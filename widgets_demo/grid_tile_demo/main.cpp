/// @file main.cpp — GridTile standalone demo
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/grid_tile.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

using namespace enki;

static const Color kColors[] = {
    0xFF2563EB, 0xFF10B981, 0xFFF59E0B, 0xFFEF4444, 0xFF8B5CF6, 0xFF0EA5E9
};

static WidgetPtr swatch(int i) {
    auto c = container();
    c->color(kColors[i % 6]);
    c->width(StyleValue::percent(100.0f));
    c->height(StyleValue::percent(100.0f));
    return c;
}

class GridTileDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto header = std::make_shared<Text>("GridTile Demo");
        header->fontSize(24.0f).bold().color(0xFFFFFFFF);
        auto hdr_wrap = container(header);
        hdr_wrap->padding(EdgeInsets::symmetric(16.0f, 20.0f));
        hdr_wrap->color(0xFF0D1117);
        hdr_wrap->width(StyleValue::percent(100.0f));

        // 2×3 manual grid using Wrap
        std::vector<WidgetPtr> tiles;
        for (int i = 0; i < 6; ++i) {
            auto footer = std::make_shared<GridTileBar>();
            footer->title(std::make_shared<Text>("Tile " + std::to_string(i+1),
                          TextStyle{.color=0xFFFFFFFF,.font_size=13.0f}));
            footer->subtitle(std::make_shared<Text>("Header + Footer overlay",
                             TextStyle{.color=0xFFCCCCCC,.font_size=11.0f}));

            auto header_bar = std::make_shared<GridTileBar>();
            header_bar->backgroundColor(0x99000000);
            header_bar->trailing(std::make_shared<Text>("★", TextStyle{.color=0xFFFCD34D,.font_size=14.0f}));

            auto tile = gridTile(swatch(i), header_bar, footer);

            auto cell = std::make_shared<FlexItem>(tile);
            cell->flexBasis(StyleValue::percent(50.0f));
            cell->flexGrow(0.0f);
            cell->flexShrink(0.0f);
            cell->aspectRatio(1.0f);

            tiles.push_back(cell);
        }

        auto grid = std::make_shared<Wrap>(std::move(tiles));
        grid->rowGap(StyleValue::point(4.0f));
        grid->columnGap(StyleValue::point(4.0f));
        grid->padding(StyleInsets::all(8.0f));
        grid->width(StyleValue::percent(100.0f));
        grid->flexShrink(0.0f);

        auto scroll = scrollView(
            ScrollOptions{.direction=Axis::Vertical,.show_scrollbar=true},
            grid
        );
        auto scroll_flex = std::make_shared<FlexItem>(scroll);
        scroll_flex->flexGrow(1.0f).flexShrink(1.0f);

        auto root_col = column({hdr_wrap, scroll_flex});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));

        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));
        return root;
    }
};

class GridTileDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<GridTileDemoState>(); }
    std::string_view typeName() const override { return "GridTileDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title = "ENKI — GridTile Demo";
    cfg.width = 640; cfg.height = 700;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<GridTileDemoApp>(), cfg);
}
