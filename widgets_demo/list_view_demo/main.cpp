/// @file main.cpp
/// @brief ListView Widget Demo — static list, builder list, separated list, selection.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/list_view.hpp"
#include "enki/widgets/list_tile.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <string>

using namespace enki;

class ListViewDemoState : public State {
    int tab_       = 0;   // 0=static, 1=builder, 2=separated, 3=selection
    int selected_  = -1;
    static constexpr int kBuilderCount = 200;

    WidgetPtr tabButton(const std::string& label, int idx) {
        bool active = (tab_ == idx);
        auto lbl = std::make_shared<Text>(label);
        lbl->fontSize(13.0f)
            .color(active ? 0xFFFFFFFF : 0xFF8B9BB4);

        ButtonOptions opts;
        opts.normal_color  = active ? 0xFF2563EB : 0xFF161B22;
        opts.hover_color   = active ? 0xFF3B82F6 : 0xFF1E2937;
        opts.pressed_color = active ? 0xFF1D4ED8 : 0xFF0D1117;
        opts.border_radius = 6.0f;
        opts.padding       = EdgeInsets::symmetric(6.0f, 14.0f);
        opts.enable_ripple = true;
        opts.shadow_blur   = 0.0f;

        return std::make_shared<Button>(lbl, [this, idx](){
            setState([this, idx]{ tab_ = idx; selected_ = -1; });
        }, opts);
    }

    static WidgetPtr colorDot(Color c) {
        auto d = container();
        d->width(10.0f);
        d->height(10.0f);
        d->borderRadius(5.0f);
        d->color(c);
        return d;
    }

public:
    WidgetPtr build(BuildContext& ctx) override {

        // ── Tab bar ───────────────────────────────────────────
        auto tab_row = row({
            tabButton("Static", 0),
            tabButton("Builder ×200", 1),
            tabButton("Separated", 2),
            tabButton("Selection", 3),
        });
        tab_row->gap(StyleValue::point(6.0f));
        tab_row->padding(StyleInsets::symmetric(8.0f, 12.0f));

        // ── Content ───────────────────────────────────────────
        WidgetPtr list_content;

        if (tab_ == 0) {
            // Static list of 20 items
            std::vector<WidgetPtr> static_items;
            static const Color kColors[] = {
                0xFF2563EB, 0xFF10B981, 0xFFF59E0B, 0xFFEF4444, 0xFF8B5CF6
            };
            for (int i = 0; i < 20; ++i) {
                Color c = kColors[i % 5];
                auto dot = container();
                dot->width(12.0f);
                dot->height(12.0f);
                dot->borderRadius(6.0f);
                dot->color(c);

                auto lbl = std::make_shared<Text>("Static Item " + std::to_string(i + 1));
                lbl->fontSize(15.0f).color(0xFFE2E8F0);

                auto r = row({dot, lbl});
                r->gap(StyleValue::point(12.0f));
                r->alignItems(Align::Center);

                auto wrap = container(r);
                wrap->padding(EdgeInsets::symmetric(14.0f, 16.0f));
                wrap->width(StyleValue::percent(100.0f));
                wrap->color(i % 2 == 0 ? 0xFF0D1117 : 0xFF161B22);
                static_items.push_back(wrap);
            }
            list_content = listView(std::move(static_items));

        } else if (tab_ == 1) {
            // Lazy builder — 200 items
            list_content = listView(kBuilderCount, [](int i) -> WidgetPtr {
                auto lbl = std::make_shared<Text>("Row " + std::to_string(i + 1));
                lbl->fontSize(14.0f).color(0xFFE2E8F0);

                auto idx_badge = std::make_shared<Text>("#" + std::to_string(i + 1));
                idx_badge->fontSize(11.0f).color(0xFF8B9BB4);

                auto flex_lbl = std::make_shared<FlexItem>(lbl);
                flex_lbl->flexGrow(1.0f);
                auto r = row({ flex_lbl, idx_badge });
                r->alignItems(Align::Center);
                r->width(StyleValue::percent(100.0f));

                auto wrap = container(r);
                wrap->padding(EdgeInsets::symmetric(12.0f, 16.0f));
                wrap->width(StyleValue::percent(100.0f));
                wrap->color(i % 2 == 0 ? 0xFF0D1117 : 0xFF161B22);
                return wrap;
            });

        } else if (tab_ == 2) {
            // Separated list
            auto lv = listView(15, [](int i) -> WidgetPtr {
                auto lbl = std::make_shared<Text>("Separated Item " + std::to_string(i + 1));
                lbl->fontSize(15.0f).color(0xFFE2E8F0);
                auto wrap = container(lbl);
                wrap->padding(EdgeInsets::symmetric(16.0f, 20.0f));
                wrap->width(StyleValue::percent(100.0f));
                return wrap;
            });

            lv->separated([](int idx) -> WidgetPtr {
                auto d = container();
                d->height(StyleValue::point(1.0f));
                d->color(0x1AFFFFFF);
                d->margin(EdgeInsets::only(0, 0, 0, 20.0f));
                return d;
            });
            list_content = lv;

        } else {
            // Selection demo
            auto lv = listView(12, [this](int i) -> WidgetPtr {
                bool sel = (selected_ == i);

                auto lbl = std::make_shared<Text>("Selectable Item " + std::to_string(i + 1));
                lbl->fontSize(15.0f).color(sel ? 0xFFFFFFFF : 0xFFE2E8F0);

                auto check = std::make_shared<Text>(sel ? "✓" : " ");
                check->fontSize(16.0f)
                      .bold()
                      .color(sel ? 0xFF2563EB : 0xFF334155);

                auto flex_lbl = std::make_shared<FlexItem>(lbl);
                flex_lbl->flexGrow(1.0f);
                auto r = row({ flex_lbl, check });
                r->alignItems(Align::Center);
                r->width(StyleValue::percent(100.0f));

                auto wrap = container(r);
                wrap->padding(EdgeInsets::symmetric(14.0f, 16.0f));
                wrap->width(StyleValue::percent(100.0f));
                wrap->color(sel ? 0x1A2563EB : Colors::Transparent);
                return wrap;
            });

            lv->onItemSelected([this](int idx){
                setState([this, idx]{ selected_ = (selected_ == idx) ? -1 : idx; });
            });
            list_content = lv;
        }

        // ── Assemble ──────────────────────────────────────────
        auto header_title = std::make_shared<Text>("ListView Demo");
        header_title->fontSize(26.0f).bold().color(0xFFFFFFFF);

        std::string desc;
        if (tab_ == 0)      desc = "20 static items — simple vector";
        else if (tab_ == 1) desc = "200 items via lazy builder — memory efficient";
        else if (tab_ == 2) desc = "15 items with custom separator widget";
        else                desc = "12 items with single-selection tracking";

        auto header_sub = std::make_shared<Text>(desc);
        header_sub->fontSize(12.0f).color(0xFF8B9BB4);

        auto header_col = column({header_title, header_sub});
        header_col->gap(StyleValue::point(4.0f));

        auto header = container(header_col);
        header->padding(EdgeInsets::symmetric(16.0f, 20.0f));
        header->color(0xFF0D1117);
        header->width(StyleValue::percent(100.0f));

        auto tab_container = container(tab_row);
        tab_container->color(0xFF161B22);
        tab_container->width(StyleValue::percent(100.0f));

        auto list_flex = std::make_shared<FlexItem>(list_content);
        list_flex->flexGrow(1.0f).flexShrink(1.0f);

        auto root_col = column({header, tab_container, list_flex});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));

        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));

        return root;
    }
};

class ListViewDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ListViewDemoState>();
    }
    std::string_view typeName() const override { return "ListViewDemoApp"; }
};

int main() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  ENKI Engine — ListView Demo          ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    AppConfig cfg;
    cfg.title      = "ENKI — ListView Demo";
    cfg.width      = 520;
    cfg.height     = 700;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;

    return runApp(std::make_shared<ListViewDemoApp>(), cfg);
}
