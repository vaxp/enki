/// @file main.cpp
/// @brief ENKI Advanced Native Menu & MenuBar Widget Demo.
/// Demonstrates MenuBar, cascading submenus, checkboxes, radio options, shortcuts, and NativePopup windows.

#include "enki/app/app.hpp"
#include "enki/widgets/menu.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/card.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class MenuDemoState : public State {
private:
    std::string last_action_ = "Ready. Click any menu item from the MenuBar above!";
    bool show_statusbar_ = true;
    bool show_minimap_ = false;
    int layout_mode_ = 1; // 0 = Compact, 1 = Standard, 2 = Expanded

    void logAction(const std::string& act) {
        last_action_ = act;
        std::cout << "[Menu Event] " << act << std::endl;
        setState([this] {});
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── 1. Construct MenuBar entries ──────────────────────────────

        // 1.1 File Menu & Submenu
        std::vector<MenuItem> export_subitems = {
            MenuItem::action("PDF Document (.pdf)", [this]() { logAction("Export -> PDF Document selected"); }, "📄"),
            MenuItem::action("PNG Image (.png)", [this]() { logAction("Export -> PNG Image selected"); }, "🖼"),
            MenuItem::action("SVG Vector (.svg)", [this]() { logAction("Export -> SVG Vector selected"); }, "📐"),
        };

        std::vector<MenuItem> file_items = {
            MenuItem::action("New File", [this]() { logAction("File -> New File"); }, "➕", "Ctrl+N"),
            MenuItem::action("Open File...", [this]() { logAction("File -> Open File..."); }, "📂", "Ctrl+O"),
            MenuItem::action("Save", [this]() { logAction("File -> Save"); }, "💾", "Ctrl+S"),
            MenuItem::action("Save As...", [this]() { logAction("File -> Save As..."); }, "📝", "Ctrl+Shift+S"),
            MenuItem::divider(),
            MenuItem::submenu("Export As", export_subitems, "🚀"),
            MenuItem::divider(),
            MenuItem::action("Exit Application", [this]() { logAction("File -> Exit clicked"); }, "🚪", "Ctrl+Q"),
        };

        // 1.2 Edit Menu
        std::vector<MenuItem> edit_items = {
            MenuItem::action("Undo", [this]() { logAction("Edit -> Undo"); }, "↩", "Ctrl+Z"),
            MenuItem::action("Redo", [this]() { logAction("Edit -> Redo"); }, "↪", "Ctrl+Y"),
            MenuItem::divider(),
            MenuItem::action("Cut", [this]() { logAction("Edit -> Cut"); }, "✂", "Ctrl+X"),
            MenuItem::action("Copy", [this]() { logAction("Edit -> Copy"); }, "📋", "Ctrl+C"),
            MenuItem::action("Paste", [this]() { logAction("Edit -> Paste"); }, "📌", "Ctrl+V"),
            MenuItem::divider(),
            MenuItem::action("Select All", [this]() { logAction("Edit -> Select All"); }, "🔍", "Ctrl+A"),
        };

        // 1.3 View Menu (Checkboxes & Radios)
        std::vector<MenuItem> layout_subitems = {
            MenuItem::radio("Compact View", 1, 0, layout_mode_ == 0, [this]() {
                layout_mode_ = 0;
                logAction("Layout changed to: Compact View");
            }),
            MenuItem::radio("Standard View", 1, 1, layout_mode_ == 1, [this]() {
                layout_mode_ = 1;
                logAction("Layout changed to: Standard View");
            }),
            MenuItem::radio("Expanded View", 1, 2, layout_mode_ == 2, [this]() {
                layout_mode_ = 2;
                logAction("Layout changed to: Expanded View");
            }),
        };

        std::vector<MenuItem> view_items = {
            MenuItem::checkbox("Show Status Bar", show_statusbar_, [this](bool val) {
                show_statusbar_ = val;
                logAction(std::string("View -> Show Status Bar: ") + (val ? "Enabled" : "Disabled"));
            }),
            MenuItem::checkbox("Show Minimap", show_minimap_, [this](bool val) {
                show_minimap_ = val;
                logAction(std::string("View -> Show Minimap: ") + (val ? "Enabled" : "Disabled"));
            }),
            MenuItem::divider(),
            MenuItem::submenu("Workspace Layout", layout_subitems, "🔲"),
        };

        // 1.4 Help Menu
        std::vector<MenuItem> help_items = {
            MenuItem::action("Documentation", [this]() { logAction("Help -> Documentation opened"); }, "📖", "F1"),
            MenuItem::action("Keyboard Shortcuts", [this]() { logAction("Help -> Keyboard Shortcuts opened"); }, "⌨"),
            MenuItem::divider(),
            MenuItem::action("About ENKI Engine", [this]() { logAction("Help -> About ENKI Engine v2.0"); }, "ℹ"),
        };

        std::vector<MenuEntry> bar_entries = {
            MenuEntry("File", file_items),
            MenuEntry("Edit", edit_items),
            MenuEntry("View", view_items),
            MenuEntry("Help", help_items),
        };

        auto app_menubar = menuBar(bar_entries);

        // ── 2. Standalone Context/Dropdown Button ─────────────────────
        std::vector<MenuItem> quick_items = {
            MenuItem::action("Create Snapshot", [this]() { logAction("Quick Action -> Snapshot created"); }, "📸"),
            MenuItem::action("Re-index Workspace", [this]() { logAction("Quick Action -> Re-indexing workspace"); }, "⚡"),
            MenuItem::divider(),
            MenuItem::action("Clear Cache", [this]() { logAction("Quick Action -> Cache cleared"); }, "🧹"),
        };

        auto quick_btn_txt = text("⚡ Quick Actions Menu  ▼");
        quick_btn_txt->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto quick_btn_box = container(quick_btn_txt);
        quick_btn_box->color(0xFF2563EB)
                     .borderRadius(8.0f)
                     .paddingSymmetric(8.0f, 16.0f);

        auto standalone_menu = menu(quick_btn_box, quick_items);

        // ── 3. Central Showcase Content ──────────────────────────────
        auto heading = text("ENKI Advanced Native Menu Suite");
        heading->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto desc = text("True multi-surface NativePopup architecture with cascading submenus, checkable items, and shortcuts.");
        desc->fontSize(13.0f).color(0xFF94A3B8);

        auto log_title = text("Recent Action Log:");
        log_title->fontSize(12.0f).bold().color(0xFF38BDF8);

        auto log_content = text(last_action_);
        log_content->fontSize(13.0f).color(0xFFF1F5F9);

        std::vector<WidgetPtr> log_col_items = {log_title, log_content};
        auto log_card = card(column(log_col_items));
        log_card->color(0xFF1E293B)
                .borderRadius(8.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(14.0f);

        auto log_box = container(log_card);
        log_box->width(550.0f);

        std::vector<WidgetPtr> center_items = {heading, desc, standalone_menu, log_box};
        auto center_col = column(center_items);
        center_col->gap(StyleValue::point(16.0f))
                  .alignItems(Align::Center)
                  .justifyContent(Justify::Center);

        auto content_container = container(center_col);
        content_container->flexGrow(1.0f)
                         .align(Alignment::Center);

        // ── 4. Bottom Status Bar (Toggleable) ─────────────────────────
        WidgetPtr statusbar_widget = nullptr;
        if (show_statusbar_) {
            auto status_txt = text("🟢 System Status: Active  |  ENKI Native Popups: Ready  |  Layout: " +
                                  std::string(layout_mode_ == 0 ? "Compact" : (layout_mode_ == 1 ? "Standard" : "Expanded")));
            status_txt->fontSize(11.0f).color(0xFF94A3B8);

            auto sb = container(status_txt);
            sb->color(0xFF0F172A)
              .border(0xFF334155, 1.0f)
              .paddingSymmetric(6.0f, 16.0f);
            statusbar_widget = sb;
        }

        // Main Scaffold
        std::vector<WidgetPtr> main_views;
        main_views.push_back(app_menubar);
        main_views.push_back(content_container);
        if (statusbar_widget) {
            main_views.push_back(statusbar_widget);
        }

        auto root_col = column(main_views);
        root_col->flexGrow(1.0f);

        auto app_root = container(root_col);
        app_root->color(0xFF0B1120)
                .paddingAll(12.0f)
                .flexGrow(1.0f);

        return app_root;
    }
};

class MenuDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<MenuDemoState>();
    }
    std::string_view typeName() const override { return "MenuDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Native Menu & MenuBar Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Menu & MenuBar Demo";
    config.width       = 950;
    config.height      = 520;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<MenuDemoApp>(), config);
}
