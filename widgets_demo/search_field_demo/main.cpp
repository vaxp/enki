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
        auto title = text("Advanced SearchField & Command Palette Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Live debounced search, autocomplete suggestions, categorized command palette, history, and shortcuts");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── 1. Spotlight / Command Palette Field ──────────────────────
        SearchFieldOptions cmd_opts;
        cmd_opts.placeholder = "Type a command or search actions (Try 'build', 'file', 'set')...";
        cmd_opts.variant = SearchFieldVariant::Filled;
        cmd_opts.size = SearchFieldSize::Medium;
        cmd_opts.show_shortcut_badge = true;
        cmd_opts.shortcut_hint = "Ctrl+K";
        cmd_opts.suggestions_provider = [](std::string_view query) -> std::vector<SearchSuggestion> {
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
        };
        cmd_opts.on_search = [this](std::string_view) {
            debounce_search_count_++;
            setState([] {});
        };
        cmd_opts.on_submitted = [this](std::string_view q) {
            last_submitted_query_ = std::string(q);
            setState([] {});
        };
        cmd_opts.on_suggestion_selected = [this](const SearchSuggestion& item) {
            last_selected_item_ = item.title + " (" + item.category + ")";
            setState([] {});
        };

        auto cmd_field = searchField(cmd_ctrl_, cmd_opts);

        auto c1_title = text("1. Spotlight / Command Palette (with Categories & Hotkeys)");
        c1_title->fontSize(14.0f).bold().color(0xFF38BDF8);

        auto c1_desc = text("Press Ctrl+K or click to open. Use Up/Down arrows to navigate, Enter to execute, Tab to autocomplete.");
        c1_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c1_items = {c1_title, c1_desc, cmd_field};
        auto c1_col = column(c1_items);
        c1_col->gap(StyleValue::point(8.0f));

        auto card1 = container(c1_col);
        card1->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(560.0f);

        // ── 2. Project File Search ────────────────────────────────────
        SearchFieldOptions file_opts;
        file_opts.placeholder = "Search files by name (e.g. 'search', 'wayland', 'meson')...";
        file_opts.variant = SearchFieldVariant::Outlined;
        file_opts.size = SearchFieldSize::Medium;
        file_opts.focus_border_color = 0xFF10B981;
        file_opts.suggestions_provider = [](std::string_view query) -> std::vector<SearchSuggestion> {
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
        };
        file_opts.on_suggestion_selected = [this](const SearchSuggestion& item) {
            last_selected_item_ = item.title + " [" + item.badge + "]";
            setState([] {});
        };

        auto file_field = searchField(file_ctrl_, file_opts);

        auto c2_title = text("2. File Explorer Search (Outlined Style)");
        c2_title->fontSize(14.0f).bold().color(0xFF10B981);

        auto c2_desc = text("Instant filter with file extension badges, directory paths, and clear button.");
        c2_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c2_items = {c2_title, c2_desc, file_field};
        auto c2_col = column(c2_items);
        c2_col->gap(StyleValue::point(8.0f));

        auto card2 = container(c2_col);
        card2->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(560.0f);

        // ── 3. Pill Capsule Navbar Search ─────────────────────────────
        SearchFieldOptions pill_opts;
        pill_opts.placeholder = "Quick search docs...";
        pill_opts.variant = SearchFieldVariant::Pill;
        pill_opts.size = SearchFieldSize::Small;
        pill_opts.focus_border_color = 0xFFF59E0B;
        pill_opts.on_submitted = [this](std::string_view q) {
            last_submitted_query_ = std::string(q);
            setState([] {});
        };

        auto pill_field = searchField(pill_ctrl_, pill_opts);

        auto c3_title = text("3. Compact Navbar Pill Search Bar");
        c3_title->fontSize(14.0f).bold().color(0xFFF59E0B);

        auto c3_desc = text("Compact ~34px pill shape ideal for application titlebars, toolbars, and navigation rails.");
        c3_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c3_items = {c3_title, c3_desc, pill_field};
        auto c3_col = column(c3_items);
        c3_col->gap(StyleValue::point(8.0f));

        auto card3 = container(c3_col);
        card3->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(560.0f);

        // ── Live Status Panel ─────────────────────────────────────────
        auto status_hdr = text("📊 Live Interactive Search State");
        status_hdr->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto st_sel = text("Last Executed Action: " + last_selected_item_);
        st_sel->fontSize(12.0f).color(0xFF38BDF8);

        auto st_sub = text("Last Submitted Query: " + last_submitted_query_);
        st_sub->fontSize(12.0f).color(0xFF10B981);

        auto st_deb = text("Debounced Queries Dispatched: " + std::to_string(debounce_search_count_));
        st_deb->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> status_items = {status_hdr, st_sel, st_sub, st_deb};
        auto status_col = column(status_items);
        status_col->gap(StyleValue::point(4.0f));

        auto status_box = container(status_col);
        status_box->color(0xFF0F172A)
                  .borderRadius(8.0f)
                  .border(0xFF334155, 1.0f)
                  .paddingAll(12.0f)
                  .width(560.0f);

        // Top Row: Card1 & Card2
        std::vector<WidgetPtr> row1_items = {card1, card2};
        auto row1 = row(row1_items);
        row1->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Bottom Row: Card3 & Status Box
        std::vector<WidgetPtr> row2_items = {card3, status_box};
        auto row2 = row(row2_items);
        row2->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Main Column
        std::vector<WidgetPtr> main_items = {title_col, row1, row2};
        auto main_col = column(main_items);
        main_col->gap(StyleValue::point(16.0f))
                .alignItems(Align::Center);

        auto app_root = container(main_col);
        app_root->color(0xFF0B1120)
                .paddingAll(20.0f)
                .flexGrow(1.0f);

        return app_root;
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
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SearchFieldDemoApp>(), config);
}
