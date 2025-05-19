/// @file main.cpp
/// @brief ListTile Widget Demo — showcases all ListTile variants.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/list_tile.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class ListTileDemoState : public State {
    int selected_index_ = -1;

    static WidgetPtr makeIcon(Color color, char letter) {
        auto lbl = std::make_shared<Text>(std::string(1, letter));
        lbl->fontSize(16.0f)
            .bold()
            .color(Colors::White);

        auto circle = container(lbl);
        circle->width(36.0f);
        circle->height(36.0f);
        circle->borderRadius(18.0f);
        circle->color(color);
        circle->align(Alignment::Center);
        return circle;
    }

    static WidgetPtr chevron() {
        auto t = std::make_shared<Text>("›");
        t->fontSize(20.0f).color(0xFF606060);
        return t;
    }

    static WidgetPtr badge(const std::string& label, Color bg) {
        auto lbl = std::make_shared<Text>(label);
        lbl->fontSize(11.0f).bold().color(Colors::White);
        auto b = container(lbl);
        b->color(bg);
        b->borderRadius(10.0f);
        b->padding(EdgeInsets::symmetric(2.0f, 7.0f));
        return b;
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── Section label helper ───────────────────────────────
        auto section = [](const std::string& s) -> WidgetPtr {
            auto lbl = std::make_shared<Text>(s);
            lbl->fontSize(11.0f)
                .bold()
                .color(0xFF8B9BB4)
                .letterSpacing(1.2f);
            auto wrap = container(lbl);
            wrap->padding(EdgeInsets::symmetric(8.0f, 16.0f));
            wrap->width(StyleValue::percent(100.0f));
            return wrap;
        };

        // ── Divider helper ─────────────────────────────────────
        auto divline = []() -> WidgetPtr {
            auto d = container();
            d->height(StyleValue::point(1.0f));
            d->color(0x1AFFFFFF);
            d->margin(EdgeInsets::only(0, 0, 0, 72.0f));   // indent from leading
            return d;
        };

        std::vector<WidgetPtr> items;

        // ── Section 1: Basic tiles ─────────────────────────────
        items.push_back(section("BASIC"));

        auto tile1 = listTile();
        tile1->leading(makeIcon(0xFF2563EB, 'A'));
        tile1->title(std::make_shared<Text>("Simple Title", TextStyle{.color=0xFFE2E8F0, .font_size=15.0f}));
        tile1->onTap([this](){ setState([this]{ selected_index_ = 0; }); });
        items.push_back(tile1);
        items.push_back(divline());

        auto tile2 = listTile();
        tile2->leading(makeIcon(0xFF10B981, 'B'));
        tile2->title(std::make_shared<Text>("With Subtitle", TextStyle{.color=0xFFE2E8F0, .font_size=15.0f}));
        tile2->subtitle(std::make_shared<Text>("Secondary description text", TextStyle{.color=0xFF8B9BB4, .font_size=13.0f}));
        tile2->trailing(chevron());
        tile2->onTap([this](){ setState([this]{ selected_index_ = 1; }); });
        items.push_back(tile2);
        items.push_back(divline());

        auto tile3 = listTile();
        tile3->leading(makeIcon(0xFFF59E0B, 'C'));
        tile3->title(std::make_shared<Text>("With Badge Trailing", TextStyle{.color=0xFFE2E8F0, .font_size=15.0f}));
        tile3->subtitle(std::make_shared<Text>("Notifications: 5", TextStyle{.color=0xFF8B9BB4, .font_size=13.0f}));
        tile3->trailing(badge("5", 0xFFEF4444));
        tile3->onTap([this](){ setState([this]{ selected_index_ = 2; }); });
        items.push_back(tile3);

        // ── Section 2: Selected ────────────────────────────────
        items.push_back(section("SELECTION"));

        for (int i = 0; i < 3; ++i) {
            auto t = listTile();
            t->leading(makeIcon(0xFF8B5CF6, '0' + i + 1));
            t->title(std::make_shared<Text>("Selectable Item " + std::to_string(i+1),
                    TextStyle{.color=0xFFE2E8F0, .font_size=15.0f}));
            t->select(selected_index_ == i + 10);
            t->selectedColor(0x1A8B5CF6);
            t->onTap([this, i](){ setState([this, i]{ selected_index_ = i + 10; }); });
            items.push_back(t);
            items.push_back(divline());
        }

        // ── Section 3: Dense / Compact ─────────────────────────
        items.push_back(section("COMPACT (DENSE)"));

        for (int i = 1; i <= 4; ++i) {
            auto t = listTile();
            t->leading(makeIcon(0xFF0EA5E9, '0' + i));
            t->title(std::make_shared<Text>("Dense Item " + std::to_string(i),
                    TextStyle{.color=0xFFE2E8F0, .font_size=13.0f}));
            t->dense(true);
            t->onTap([i](){ std::cout << "Dense " << i << "\n"; });
            items.push_back(t);
        }

        // ── Section 4: Disabled ────────────────────────────────
        items.push_back(section("DISABLED"));

        auto disabled_tile = listTile();
        disabled_tile->leading(makeIcon(0xFF475569, 'D'));
        disabled_tile->title(std::make_shared<Text>("Disabled Tile", TextStyle{.color=0xFF475569, .font_size=15.0f}));
        disabled_tile->subtitle(std::make_shared<Text>("This tile cannot be interacted with",
                   TextStyle{.color=0xFF334155, .font_size=13.0f}));
        disabled_tile->enable(false);
        items.push_back(disabled_tile);

        // ── Layout ────────────────────────────────────────────
        auto content_col = std::make_shared<Column>(std::move(items));
        content_col->width(StyleValue::percent(100.0f));
        content_col->flexShrink(0.0f);

        auto scroll = scrollView(
            ScrollOptions{.direction=Axis::Vertical, .show_scrollbar=true},
            content_col
        );

        auto scroll_container = container(scroll);
        scroll_container->flex(1.0f);
        scroll_container->minHeight(0.0f);

        // Header
        auto header_title = std::make_shared<Text>("ListTile Demo");
        header_title->fontSize(26.0f).bold().color(0xFFFFFFFF);

        auto header_sub = std::make_shared<Text>(
            "leading · title · subtitle · trailing · hover · ripple · selection · dense · disabled");
        header_sub->fontSize(12.0f).color(0xFF8B9BB4);

        auto header_col = column({header_title, header_sub});
        header_col->gap(StyleValue::point(4.0f));

        auto header = container(header_col);
        header->padding(EdgeInsets::symmetric(16.0f, 20.0f));
        header->color(0xFF0D1117);
        header->width(StyleValue::percent(100.0f));

        // Status bar
        auto status_text = std::make_shared<Text>(
            selected_index_ >= 0
                ? "Selected index: " + std::to_string(selected_index_)
                : "Tap any tile to select");
        status_text->fontSize(13.0f).color(0xFF8B9BB4);

        auto status_bar = container(status_text);
        status_bar->padding(EdgeInsets::symmetric(8.0f, 20.0f));
        status_bar->color(0xFF161B22);
        status_bar->width(StyleValue::percent(100.0f));

        auto root_col = column({header, status_bar, scroll_container});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));

        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));

        return root;
    }
};

class ListTileDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ListTileDemoState>();
    }
    std::string_view typeName() const override { return "ListTileDemoApp"; }
};

int main() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  ENKI Engine — ListTile Demo          ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    AppConfig cfg;
    cfg.title      = "ENKI — ListTile Demo";
    cfg.width      = 480;
    cfg.height     = 700;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;

    return runApp(std::make_shared<ListTileDemoApp>(), cfg);
}
