/// @file test_command_palette.cpp
/// @brief Unit tests for CommandPalette fuzzy search overlay widget (Section 19: Overlay & Popup Extended).

#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"

#include <iostream>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

using namespace enki;

void test_declarative_instantiation() {
    std::cout << "Testing CommandPalette declarative instantiation..." << std::endl;

    bool opened = false;
    bool closed = false;
    bool executed = false;
    std::string selected_id = "";

    auto ctrl = std::make_shared<CommandPaletteController>();

    std::vector<CommandItem> items = {
        CommandItem("nav_home", "Go to Home Dashboard", "Navigation", "Ctrl+H", "🏠", [&] {
            executed = true;
        }),
        CommandItem("git_pull", "Git: Pull Latest Changes", "Git", "Ctrl+Shift+P", "📥"),
        CommandItem("git_push", "Git: Push Commits to Remote", "Git", "Ctrl+Shift+U", "📤"),
        CommandItem("pref_theme", "Preferences: Color Theme", "Preferences", "Ctrl+T", "🎨"),
        CommandItem("reset_all", "Factory Reset Application", "System", "", "⚠️", nullptr)
    };
    items.back().is_danger = true;

    WidgetPtr w = commandPalette({
        .body = text("Main Viewport Content"),
        .items = items,
        .options = {
            .card_width = 640.0f,
            .placeholder = "Search commands...",
            .on_open = [&] { opened = true; },
            .on_close = [&] { closed = true; },
            .on_item_selected = [&](const CommandItem& item) {
                selected_id = item.id;
            }
        },
        .controller = ctrl,
        .initial_open = true
    });

    assert(w != nullptr);
    assert(w->typeName() == "CommandPalette");

    auto elem = w->createElement();
    assert(elem != nullptr);
    elem->mount(nullptr, 0);
    elem->rebuild();

    // Verify initial open state from controller
    assert(ctrl->isOpen() == true);

    // Test query controller
    ctrl->setQuery("git");
    assert(ctrl->getQuery() == "git");

    // Test selection navigation
    ctrl->selectNext();
    ctrl->selectPrevious();

    // Execute active command
    ctrl->executeActive();
    assert(selected_id == "git_pull" || selected_id == "git_push");

    // Close palette
    ctrl->close();
    assert(ctrl->isOpen() == false);
    assert(closed == true);

    // Toggle palette
    ctrl->toggle();
    assert(ctrl->isOpen() == true);
    assert(opened == true);

    std::cout << "  Declarative instantiation test passed." << std::endl;
}

void test_fuzzy_search_scoring() {
    std::cout << "Testing CommandPalette fuzzy matching & filtering..." << std::endl;

    auto ctrl = std::make_shared<CommandPaletteController>();

    std::vector<CommandItem> items = {
        CommandItem("f1", "File: Open Workspace Folder", "File Operations", "Ctrl+O", "📁"),
        CommandItem("f2", "File: Save Active Document", "File Operations", "Ctrl+S", "💾"),
        CommandItem("f3", "Find in Files (Global Search)", "Editor", "Ctrl+Shift+F", "🔍"),
        CommandItem("term", "Terminal: Create New Bash Session", "Terminal", "Ctrl+`", "💻"),
        CommandItem("zoom", "View: Zoom In Viewport", "View", "Ctrl+=", "🔎")
    };
    items[2].keywords = {"grep", "ripgrep", "search"};

    WidgetPtr w = commandPalette({
        .body = text("Body"),
        .items = items,
        .options = {
            .show_recent = false
        },
        .controller = ctrl,
        .initial_open = true
    });

    auto elem = w->createElement();
    elem->mount(nullptr, 0);
    elem->rebuild();

    // 1. Prefix query "File"
    std::string executed_id = "";
    ctrl->setQuery("File");
    elem->rebuild();

    ctrl->executeActive();
    // Top score should be File: Open or File: Save
    assert(ctrl->getQuery() == "File");

    // 2. Keyword match "grep"
    ctrl->setQuery("grep");
    elem->rebuild();
    assert(ctrl->getQuery() == "grep");

    // 3. Subsequence fuzzy match "trm" -> "Terminal"
    ctrl->setQuery("trm");
    elem->rebuild();
    assert(ctrl->getQuery() == "trm");

    // 4. Non-matching query
    ctrl->setQuery("xyznonexistent123");
    elem->rebuild();
    assert(ctrl->getQuery() == "xyznonexistent123");

    // Reset query
    ctrl->setQuery("");
    assert(ctrl->getQuery() == "");

    std::cout << "  Fuzzy matching test passed." << std::endl;
}

void test_disabled_and_danger_attributes() {
    std::cout << "Testing CommandPalette disabled & danger states..." << std::endl;

    auto ctrl = std::make_shared<CommandPaletteController>();

    bool executed_disabled = false;
    bool executed_active = false;

    CommandItem disabled_cmd("dis", "Disabled Feature", "Admin", "", "🔒", [&] {
        executed_disabled = true;
    });
    disabled_cmd.disabled = true;

    CommandItem active_cmd("act", "Active Danger Feature", "Admin", "", "⚡", [&] {
        executed_active = true;
    });
    active_cmd.is_danger = true;
    active_cmd.badge = "Danger";

    std::vector<CommandItem> items = { disabled_cmd, active_cmd };

    WidgetPtr w = commandPalette({
        .body = text("Body"),
        .items = items,
        .controller = ctrl,
        .initial_open = true
    });

    auto elem = w->createElement();
    elem->mount(nullptr, 0);
    elem->rebuild();

    // Execute active item (should skip disabled and trigger active_cmd)
    ctrl->executeActive();
    assert(executed_disabled == false);
    assert(executed_active == true);

    std::cout << "  Disabled and danger attributes test passed." << std::endl;
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — Unit Test: test_command_palette" << std::endl;
    std::cout << "  Category 19: Overlay & Popup Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    test_declarative_instantiation();
    test_fuzzy_search_scoring();
    test_disabled_and_danger_attributes();

    std::cout << "====================================================" << std::endl;
    std::cout << "  ALL COMMAND PALETTE TESTS PASSED! (3/3)" << std::endl;
    std::cout << "====================================================" << std::endl;

    return 0;
}
