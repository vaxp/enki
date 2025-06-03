/// @file main.cpp
/// @brief ENKI Advanced ContextMenu Widget Interactive Showcase.
/// Demonstrates native compositor context menus (right-click / long-press popups), submenus, shortcuts, and custom styling.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/card.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/context_menu.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ContextMenuDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // Title & Description Header
        auto title = text("Advanced Native ContextMenu (NativePopup)");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Right-click or long-press on cards below to trigger native floating desktop context menus");
        sub->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. Context Menu items for Card 1 (File Operations)
        std::vector<ContextMenuItemPtr> card1_items = {
            contextMenuItem("Copy File", []() { std::cout << "[ContextMenu] Action: Copy File\n"; }, "Ctrl+C"),
            contextMenuItem("Cut File", []() { std::cout << "[ContextMenu] Action: Cut File\n"; }, "Ctrl+X"),
            contextMenuItem("Paste", []() { std::cout << "[ContextMenu] Action: Paste\n"; }, "Ctrl+V"),
            contextMenuDivider(),
            contextMenuItem("Rename", []() { std::cout << "[ContextMenu] Action: Rename\n"; }, "F2"),
            contextMenuItem("Duplicate", []() { std::cout << "[ContextMenu] Action: Duplicate\n"; }, "Ctrl+D"),
            contextMenuDivider(),
            contextMenuItem("Delete File", []() { std::cout << "[ContextMenu] Action: Delete File (Danger)\n"; }, "Del", nullptr, false, true),
        };

        auto card1_text = text("📄 document_report_2026.pdf");
        card1_text->fontSize(15.0f).color(0xFFF1F5F9).bold();

        auto card1_desc = text("Right-click to open File Actions menu");
        card1_desc->fontSize(12.0f).color(0xFF94A3B8);

        auto card1_col = column({card1_text, card1_desc});
        card1_col->gap(StyleValue::point(6.0f));

        auto card1_box = container(card1_col);
        card1_box->color(0xFF1E293B)
                 .borderRadius(10.0f)
                 .border(0xFF334155, 1.0f)
                 .paddingAll(20.0f)
                 .width(320.0f);

        auto card1_menu = contextMenu(card1_box, card1_items);

        // 2. Context Menu items for Card 2 (Code Editor / Repository Actions)
        std::vector<ContextMenuItemPtr> card2_items = {
            contextMenuItem("Git Pull", []() { std::cout << "[ContextMenu] Action: Git Pull\n"; }),
            contextMenuItem("Git Commit & Push...", []() { std::cout << "[ContextMenu] Action: Commit & Push\n"; }),
            contextMenuDivider(),
            contextMenuItem("Open in Terminal", []() { std::cout << "[ContextMenu] Action: Open Terminal\n"; }, "Ctrl+Alt+T"),
            contextMenuItem("Copy Remote URL", []() { std::cout << "[ContextMenu] Action: Copy Remote URL\n"; }),
            contextMenuDivider(),
            contextMenuItem("Force Purge Cache", []() { std::cout << "[ContextMenu] Action: Purge Cache (Danger)\n"; }, "", nullptr, false, true),
        };

        auto card2_text = text("📦 enki-framework (main branch)");
        card2_text->fontSize(15.0f).color(0xFFF1F5F9).bold();

        auto card2_desc = text("Right-click for Git & Repository actions");
        card2_desc->fontSize(12.0f).color(0xFF94A3B8);

        auto card2_col = column({card2_text, card2_desc});
        card2_col->gap(StyleValue::point(6.0f));

        auto card2_box = container(card2_col);
        card2_box->color(0xFF1E293B)
                 .borderRadius(10.0f)
                 .border(0xFF334155, 1.0f)
                 .paddingAll(20.0f)
                 .width(320.0f);

        ContextMenuOptions opt2;
        opt2.background_color = 0xFA0F172A;
        opt2.border_color = 0xFF38BDF8;

        auto card2_menu = contextMenu(card2_box, card2_items, opt2);

        // Layout rows
        std::vector<WidgetPtr> r_children = {card1_menu, card2_menu};
        auto cardsRow = row(r_children);
        cardsRow->justifyContent(Justify::Center).alignItems(Align::Center).gap(40_px);

        std::vector<WidgetPtr> m_children = {titleCol, cardsRow};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class ContextMenuDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ContextMenuDemoState>();
    }
    std::string_view typeName() const override { return "ContextMenuDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — ContextMenu Widget Demo (NativePopup)\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — ContextMenu Demo";
    config.width       = 800;
    config.height      = 450;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ContextMenuDemoApp>(), config);
}
