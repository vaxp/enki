/// @file main.cpp
/// @brief ENKI Advanced SearchField & Command Palette Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/search_field.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using namespace enki;

// Mock database of command palette suggestions
static std::vector<SearchSuggestion> getMockCommands() {
    return {
        SearchSuggestion("Create New File", "Create an empty source or document file", "COMMANDS", "Ctrl+N", "📄"),
        SearchSuggestion("Open Workspace...", "Select and open project folder", "COMMANDS", "Ctrl+O", "📂"),
        SearchSuggestion("Build Project", "Run meson compile in build directory", "COMMANDS", "Ctrl+B", "⚡"),
        SearchSuggestion("Run Application", "Execute the current active target", "COMMANDS", "F5", "▶️"),
        SearchSuggestion("Command Palette", "View all available IDE actions", "COMMANDS", "Ctrl+Shift+P", "⌘"),
        SearchSuggestion("Settings / Preferences", "Configure editor, themes, and keybindings", "SETTINGS", "Ctrl+,", "⚙️"),
        SearchSuggestion("Keyboard Shortcuts", "Browse and edit hotkeys", "SETTINGS", "Ctrl+K Ctrl+S", "⌨️"),
        SearchSuggestion("Terminal Window", "Spawn integrated bash shell", "APPS", "Ctrl+`", "💻"),
        SearchSuggestion("System Monitor", "Inspect CPU, memory and render frame times", "APPS", "Ctrl+M", "📊")
    };
}

// Mock database of project files
static std::vector<SearchSuggestion> getMockFiles() {
    return {
        SearchSuggestion("search_field.hpp", "include/enki/widgets/", "FILES", ".hpp", "📄"),
        SearchSuggestion("search_field.cpp", "src/widgets/", "FILES", ".cpp", "⚡"),
        SearchSuggestion("number_field.hpp", "include/enki/widgets/", "FILES", ".hpp", "📄"),
        SearchSuggestion("number_field.cpp", "src/widgets/", "FILES", ".cpp", "⚡"),
        SearchSuggestion("text_area.cpp", "src/widgets/", "FILES", ".cpp", "⚡"),
        SearchSuggestion("wayland_platform.cpp", "src/platform/wayland/", "FILES", ".cpp", "🌐"),
        SearchSuggestion("meson.build", "Root build script configuration", "CONFIG", "Build", "🔧"),
        SearchSuggestion("WIDGETS_ROADMAP.md", "Documentation and progress roadmap", "DOCS", "MD", "📝")
    };
}

class SearchFieldDemoState : public State {
private:
    std::shared_ptr<SearchFieldController> cmd_ctrl_;
    std::shared_ptr<SearchFieldController> file_ctrl_;
    std::shared_ptr<SearchFieldController> pill_ctrl_;

    std::string last_selected_item_ = "None";
    std::string last_submitted_query_ = "None";
    int debounce_search_count_ = 0;

public:
    void initState() override {
        State::initState();
        cmd_ctrl_  = std::make_shared<SearchFieldController>();
        file_ctrl_ = std::make_shared<SearchFieldController>();
        pill_ctrl_ = std::make_shared<SearchFieldController>();

        // Pre-populate some recent search history
        cmd_ctrl_->addRecentSearch("Build Project");
        cmd_ctrl_->addRecentSearch("Create New File");
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced SearchField & Command Palette Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Live debounced search, autocomplete suggestions, categorized command palette, history, and shortcuts", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .children = {title, sub}
        });

        // ── 1. Spotlight / Command Palette Field ──────────────────────
        auto cmd_field = SearchField {
            .controller = cmd_ctrl_,
            .placeholder = "Type a command or search actions (Try 'build', 'file', 'set')...",
            .variant = SearchFieldVariant::Filled,
            .size = SearchFieldSize::Medium,
            .show_shortcut_badge = true,
            .shortcut_hint = "Ctrl+K",
            .suggestions_provider = [](std::string_view query) -> std::vector<SearchSuggestion> {
                auto all = getMockCommands();
                if (query.empty()) return all;
                std::vector<SearchSuggestion> filtered;
                std::string lq(query);
                std::transform(lq.begin(), lq.end(), lq.begin(), ::tolower);
                for (const auto& item : all) {
                    std::string lt = item.title;
                    std::transform(lt.begin(), lt.end(), lt.begin(), ::tolower);
                    if (lt.find(lq) != std::string::npos) {
                        filtered.push_back(item);
                    }
                }
                return filtered;
            },
            .on_search = [this](std::string_view) {
                debounce_search_count_++;
                setState([] {});
            },
            .on_submitted = [this](std::string_view q) {
                last_submitted_query_ = std::string(q);
                setState([] {});
            },
            .on_suggestion_selected = [this](const SearchSuggestion& item) {
                last_selected_item_ = item.title + " (" + item.category + ")";
                setState([] {});
            }
        };

        auto c1_title = text("1. Spotlight / Command Palette (with Categories & Hotkeys)", {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c1_desc = text("Press Ctrl+K or click to open. Use Up/Down arrows to navigate, Enter to execute, Tab to autocomplete.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(560.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c1_title, c1_desc, cmd_field}
            })
        });

        // ── 2. Project File Search ────────────────────────────────────
        auto file_field = SearchField {
            .controller = file_ctrl_,
            .placeholder = "Search files by name (e.g. 'search', 'wayland', 'meson')...",
            .variant = SearchFieldVariant::Outlined,
            .size = SearchFieldSize::Medium,
            .focus_border_color = 0xFF10B981,
            .suggestions_provider = [](std::string_view query) -> std::vector<SearchSuggestion> {
                auto all = getMockFiles();
                if (query.empty()) return all;
                std::vector<SearchSuggestion> filtered;
                std::string lq(query);
                std::transform(lq.begin(), lq.end(), lq.begin(), ::tolower);
                for (const auto& item : all) {
                    std::string lt = item.title;
                    std::transform(lt.begin(), lt.end(), lt.begin(), ::tolower);
                    if (lt.find(lq) != std::string::npos) {
                        filtered.push_back(item);
                    }
                }
                return filtered;
            },
            .on_suggestion_selected = [this](const SearchSuggestion& item) {
                last_selected_item_ = item.title + " [" + item.badge + "]";
                setState([] {});
            }
        };

        auto c2_title = text("2. File Explorer Search (Outlined Style)", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c2_desc = text("Instant filter with file extension badges, directory paths, and clear button.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card2 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(560.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c2_title, c2_desc, file_field}
            })
        });

        // ── 3. Pill Capsule Navbar Search ─────────────────────────────
        auto pill_field = SearchField {
            .controller = pill_ctrl_,
            .placeholder = "Quick search docs...",
            .variant = SearchFieldVariant::Pill,
            .size = SearchFieldSize::Small,
            .focus_border_color = 0xFFF59E0B,
            .on_submitted = [this](std::string_view q) {
                last_submitted_query_ = std::string(q);
                setState([] {});
            }
        };

        auto c3_title = text("3. Compact Navbar Pill Search Bar", {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c3_desc = text("Compact ~34px pill shape ideal for application titlebars, toolbars, and navigation rails.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card3 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(560.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c3_title, c3_desc, pill_field}
            })
        });

        // ── Live Status Panel ─────────────────────────────────────────
        auto status_hdr = text("📊 Live Interactive Search State", {
            .color = 0xFFFFFFFF,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        auto st_sel = text("Last Executed Action: " + last_selected_item_, {
            .color = 0xFF38BDF8,
            .font_size = 12.0f,
        });

        auto st_sub = text("Last Submitted Query: " + last_submitted_query_, {
            .color = 0xFF10B981,
            .font_size = 12.0f,
        });

        auto st_deb = text("Debounced Queries Dispatched: " + std::to_string(debounce_search_count_), {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto status_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(560.0f),
            .padding = StyleInsets::all(12.0f),
            .child = column({
                .gap = StyleValue::point(4.0f),
                .children = {status_hdr, st_sel, st_sub, st_deb}
            })
        });

        // Top Row: Card1 & Card2
        auto row1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card1, card2}
        });

        // Bottom Row: Card3 & Status Box
        auto row2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card3, status_box}
        });

        // Main Column
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {title_col, row1, row2}
        });

        return container({
            .color = 0xFF0B1120,
            .padding = StyleInsets::all(20.0f),
            .flex_grow = 1.0f,
            .child = main_col
        });
    }
};

class SearchFieldDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SearchFieldDemoState>();
    }
    std::string_view typeName() const override { return "SearchFieldDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced SearchField Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced SearchField Demo";
    config.width       = 1200;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SearchFieldDemoApp>(), config);
}
