/// @file main.cpp
/// @brief GridView + GridTile Demo

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/grid_view.hpp"
#include "enki/widgets/grid_tile.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

// Fake "photo" tile using gradient-colored containers
static const Color kPalette[] = {
    0xFF2563EB, 0xFF10B981, 0xFFF59E0B, 0xFFEF4444,
    0xFF8B5CF6, 0xFF0EA5E9, 0xFFEC4899, 0xFF14B8A6,
    0xFFF97316, 0xFF6366F1,
};

static WidgetPtr fakePhoto(int i) {
    Color c = kPalette[i % 10];
    auto label = std::make_shared<Text>(std::to_string(i + 1));
    label->fontSize(24.0f).bold().color(Colors::White);

    auto tile_content = container(label);
    tile_content->color(c);
    tile_content->align(Alignment::Center);
    tile_content->width(StyleValue::percent(100.0f));
    tile_content->height(StyleValue::percent(100.0f));
    return tile_content;
}

class GridDemoState : public State {
    int tab_ = 2;  // 0=3-col fixed, 1=2-col fixed, 2=max-extent responsive

    WidgetPtr tabButton(const std::string& label, int idx) {
        bool active = (tab_ == idx);
        auto lbl = std::make_shared<Text>(label);
        lbl->fontSize(13.0f).color(active ? 0xFFFFFFFF : 0xFF8B9BB4);
        ButtonOptions opts;
        opts.normal_color  = active ? 0xFF2563EB : 0xFF161B22;
        opts.hover_color   = active ? 0xFF3B82F6 : 0xFF1E2937;
        opts.pressed_color = active ? 0xFF1D4ED8 : 0xFF0D1117;
        opts.border_radius = 6.0f;
        opts.padding       = EdgeInsets::symmetric(6.0f, 14.0f);
        opts.shadow_blur   = 0.0f;
        return std::make_shared<Button>(lbl, [this, idx](){
            setState([this, idx]{ tab_ = idx; });
        }, opts);
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto header_title = std::make_shared<Text>("GridView + GridTile Demo");
        header_title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        std::string desc;
        if (tab_ == 0)      desc = "3 columns, aspect ratio 1:1";
        else if (tab_ == 1) desc = "2 columns, aspect ratio 4:3";
        else                desc = "Max-extent 160px — auto responsive columns";

        auto header_sub = std::make_shared<Text>(desc);
        header_sub->fontSize(12.0f).color(0xFF8B9BB4);

        auto header_col = column({header_title, header_sub});
        header_col->gap(StyleValue::point(4.0f));
        auto header = container(header_col);
        header->padding(EdgeInsets::symmetric(16.0f, 20.0f));
        header->color(0xFF0D1117);
        header->width(StyleValue::percent(100.0f));

        auto tab_row = row({
            tabButton("3 Cols", 0),
            tabButton("2 Cols", 1),
            tabButton("Responsive", 2),
        });
        tab_row->gap(StyleValue::point(6.0f));
        tab_row->padding(StyleInsets::symmetric(8.0f, 12.0f));
        auto tab_wrap = container(tab_row);
        tab_wrap->color(0xFF161B22);
        tab_wrap->width(StyleValue::percent(100.0f));

        // Grid content
        WidgetPtr grid;
        if (tab_ == 0) {
            grid = gridView(3, 30, [](int i) -> WidgetPtr {
                auto footer = gridTileBar();
                footer->title(std::make_shared<Text>("Item " + std::to_string(i+1),
                            TextStyle{.color=0xFFFFFFFF, .font_size=12.0f}));
                return gridTile(fakePhoto(i), nullptr, footer);
            });
            auto gv = std::static_pointer_cast<GridView>(grid);
            gv->crossAxisSpacing(4.0f);
            gv->mainAxisSpacing(4.0f);
            gv->paddingAll(4.0f);
            gv->childAspectRatio(1.0f);

        } else if (tab_ == 1) {
            grid = gridView(2, 20, [](int i) -> WidgetPtr {
                auto footer = gridTileBar();
                footer->title(std::make_shared<Text>("Photo " + std::to_string(i+1),
                            TextStyle{.color=0xFFFFFFFF, .font_size=13.0f}));
                footer->subtitle(std::make_shared<Text>("Subtitle " + std::to_string(i+1),
                               TextStyle{.color=0xFFB0C4D8, .font_size=11.0f}));
                return gridTile(fakePhoto(i), nullptr, footer);
            });
            auto gv = std::static_pointer_cast<GridView>(grid);
            gv->crossAxisSpacing(8.0f);
            gv->mainAxisSpacing(8.0f);
            gv->paddingAll(8.0f);
            gv->childAspectRatio(4.0f / 3.0f);

        } else {
            grid = gridViewExtent(160.0f, 25, [](int i) -> WidgetPtr {
                auto t = std::make_shared<Text>(std::to_string(i+1));
                t->fontSize(28.0f).bold().color(Colors::White);
                auto c = container(t);
                c->color(kPalette[i % 10]);
                c->borderRadius(8.0f);
                c->align(Alignment::Center);
                c->width(StyleValue::percent(100.0f));
                c->height(StyleValue::percent(100.0f));
                return c;
            });
            auto gv = std::static_pointer_cast<GridView>(grid);
            gv->crossAxisSpacing(8.0f);
            gv->mainAxisSpacing(8.0f);
            gv->paddingAll(8.0f);
            gv->childAspectRatio(1.0f);
        }

        auto grid_flex = std::make_shared<FlexItem>(grid);
        grid_flex->flexGrow(1.0f).flexShrink(1.0f);

        auto root_col = column({header, tab_wrap, grid_flex});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));

        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));
        return root;
    }
};

class GridDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<GridDemoState>(); }
    std::string_view typeName() const override { return "GridDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title      = "ENKI — GridView + GridTile Demo";
    cfg.width      = 640;
    cfg.height     = 700;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<GridDemoApp>(), cfg);
}
