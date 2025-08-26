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

        auto card1_menu = ContextMenu {
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::point(320.0f),
                .padding = StyleInsets::all(20.0f),
                .child = column({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        text("📄 document_report_2026.pdf", { .color = 0xFFF1F5F9, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                        text("Right-click to open File Actions menu", { .color = 0xFF94A3B8, .font_size = 12.0f })
                    }
                })
            }),
            .items = card1_items
        };

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

        auto card2_menu = ContextMenu {
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::point(320.0f),
                .padding = StyleInsets::all(20.0f),
                .child = column({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        text("📦 enki-framework (main branch)", { .color = 0xFFF1F5F9, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                        text("Right-click for Git & Repository actions", { .color = 0xFF94A3B8, .font_size = 12.0f })
                    }
                })
            }),
            .items = card2_items,
            .options = {
                .background_color = 0xFA0F172A,
                .border_color = 0xFF38BDF8
            }
        };

        return container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::all(40.0f),
            .flex_grow = 1.0f,
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {
                    column({
                        .align_items = Align::Center,
                        .margin = StyleInsets::only(0, 0, 40.0f, 0),
                        .children = {
                            text("Advanced Native ContextMenu (NativePopup)", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                            text("Right-click or long-press on cards below to trigger native floating desktop context menus", { .color = 0xFF94A3B8, .font_size = 14.0f })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Center,
                        .gap = StyleValue::point(40.0f),
                        .children = { card1_menu, card2_menu }
                    })
                }
            })
        });
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
