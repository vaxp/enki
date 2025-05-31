/// @file main.cpp
/// @brief ENKI Advanced Tooltip Widget Interactive Showcase.
/// Demonstrates native compositor tooltips, rich content, smart positioning, and custom styling using standard App & runApp.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/badge.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class TooltipDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // Title & Description Header
        auto title = text("Advanced Native Tooltips (NativePopup)");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Hover or interact with elements to spawn native floating desktop tooltips");
        sub->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. Basic Text Tooltip on Button
        auto btn_text = text("Save Changes");
        btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto btn1 = button(btn_text, [](){
            std::cout << "Save clicked\n";
        });

        TooltipOptions opt1;
        opt1.position = TooltipPosition::Top;
        auto tt1 = tooltip(btn1, "Click to save all your pending configuration changes", opt1);

        // 2. Rich Content Tooltip (Text + Badge/Icon)
        auto btn_danger_text = text("Delete Repository");
        btn_danger_text->fontSize(14.0f).color(0xFFFFFFFF).bold();

        ButtonOptions btn_danger_opt;
        btn_danger_opt.normal_color = 0xFFEF4444;
        btn_danger_opt.hover_color = 0xFFDC2626;
        auto btn_danger = button(btn_danger_text, [](){
            std::cout << "Delete clicked\n";
        }, btn_danger_opt);

        // Build rich tooltip content widget
        auto rich_title = text("Warning: Permanent Action");
        rich_title->fontSize(13.0f).bold().color(0xFFFCA5A5);

        auto rich_desc = text("This action cannot be undone.");
        rich_desc->fontSize(11.0f).color(0xFFE2E8F0);

        std::vector<WidgetPtr> rich_items = {rich_title, rich_desc};
        auto rich_col = column(rich_items);
        rich_col->alignItems(Align::Start);

        TooltipOptions opt2;
        opt2.position = TooltipPosition::Bottom;
        opt2.background_color = 0xEE7F1D1D; // Dark red tint
        opt2.border_color = 0xFFEF4444;
        auto tt2 = tooltip(btn_danger, rich_col, opt2);

        // 3. Custom Position Tooltips (Left & Right)
        auto btn_left_text = text("Tooltip Left");
        btn_left_text->fontSize(13.0f).color(0xFFFFFFFF);
        auto btn_left = button(btn_left_text, nullptr);
        TooltipOptions opt_left;
        opt_left.position = TooltipPosition::Left;
        auto tt_left = tooltip(btn_left, "Positioned to the left of the button", opt_left);

        auto btn_right_text = text("Tooltip Right");
        btn_right_text->fontSize(13.0f).color(0xFFFFFFFF);
        auto btn_right = button(btn_right_text, nullptr);
        TooltipOptions opt_right;
        opt_right.position = TooltipPosition::Right;
        auto tt_right = tooltip(btn_right, "Positioned to the right of the button", opt_right);

        // Row layout
        std::vector<WidgetPtr> r1_children = {tt1, tt2};
        auto row1 = row(r1_children);
        row1->justifyContent(Justify::Center).alignItems(Align::Center).gap(40_px);

        std::vector<WidgetPtr> r2_children = {tt_left, tt_right};
        auto row2 = row(r2_children);
        row2->justifyContent(Justify::Center).alignItems(Align::Center).gap(40_px);

        std::vector<WidgetPtr> b_children = {row1, row2};
        auto buttonsCol = column(b_children);
        buttonsCol->alignItems(Align::Center).gap(40_px);

        std::vector<WidgetPtr> m_children = {titleCol, buttonsCol};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class TooltipDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TooltipDemoState>();
    }
    std::string_view typeName() const override { return "TooltipDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Tooltip Widget Demo (Native Popup in App)\n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Tooltip Demo";
    config.width       = 800;
    config.height      = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<TooltipDemoApp>(), config);
}
