#pragma once
/// @file command_palette.hpp
/// @brief Advanced Keyboard-Driven Command Palette & Fuzzy Search Overlay Widget for ENKI Framework (Category 19. Overlay & Popup Extended).
///
/// Features:
///   - Keyboard-driven command launcher overlay (Ctrl+K / Cmd+K style)
///   - High-performance fuzzy search scoring algorithm with character match highlighting
///   - Categorized command grouping (Recent, Navigation, Actions, Git, Settings, etc.)
///   - Full keyboard navigation: Up/Down arrow navigation, Enter execution, Escape dismissal
///   - Recent commands tracking (MRU history)
///   - Action shortcuts badge display (e.g., "Ctrl+P", "Alt+Return")
///   - Danger / destructive action styling
///   - Animated scale & fade in/out transitions with barrier click-to-dismiss
///   - Imperative controller (CommandPaletteController) and C++20 declarative API
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// Command Item Definition
/// ════════════════════════════════════════════════════════════════

struct CommandItem {
    std::string id = "";
    std::string title = "";
    std::string subtitle = "";
    std::string category = "Commands";
    std::string icon = "⚡";
    std::string shortcut = "";
    std::string badge = "";
    std::vector<std::string> keywords = {};

    bool disabled = false;
    bool is_danger = false;

    std::function<void()> on_execute = nullptr;

    CommandItem() = default;
    CommandItem(std::string id_, std::string title_, std::string category_ = "Commands",
                std::string shortcut_ = "", std::string icon_ = "⚡",
                std::function<void()> on_exec = nullptr)
        : id(std::move(id_)), title(std::move(title_)), category(std::move(category_)),
          icon(std::move(icon_)), shortcut(std::move(shortcut_)), on_execute(std::move(on_exec)) {}
};

/// ════════════════════════════════════════════════════════════════
/// CommandPalette Options
/// ════════════════════════════════════════════════════════════════

struct CommandPaletteOptions {
    Color overlay_color         = 0xCC080C14; ///< Deep dark backdrop scrim (80% obsidian)
    Color card_bg_color         = 0xF80F172A; ///< Slate 900 palette card
    Color border_color          = 0xFF334155; ///< Slate 700 card border
    Color input_bg_color        = 0xFF0B0F19; ///< Slate 950 search field background
    Color input_text_color      = 0xFFF8FAFC; ///< Slate 50 query text
    Color placeholder_color     = 0xFF64748B; ///< Slate 500 placeholder text
    Color item_hover_bg         = 0x3338BDF8; ///< Sky 500/20% hover highlight
    Color item_selected_bg      = 0xFF0284C7; ///< Sky 600 active item selection
    Color item_title_color      = 0xFFF8FAFC; ///< Slate 50 item text
    Color item_subtitle_color   = 0xFF94A3B8; ///< Slate 400 description text
    Color highlight_match_color = 0xFF38BDF8; ///< Sky 400 fuzzy matched characters highlight
    Color shortcut_badge_bg    = 0xFF1E293B; ///< Slate 800 shortcut tag container
    Color shortcut_text_color   = 0xFF94A3B8; ///< Slate 400 shortcut label
    Color section_header_color  = 0xFF64748B; ///< Slate 500 category header text

    float card_width            = 620.0f;     ///< Palette card width
    float max_list_height       = 400.0f;     ///< Max height of results viewport
    float card_border_radius    = 14.0f;      ///< Outer card rounded corners
    float top_margin            = 90.0f;      ///< Top offset distance from viewport top

    std::string placeholder     = "Type a command or search...";
    std::string empty_text      = "No matching commands found";

    bool enable_global_shortcut = true;      ///< Listen for Ctrl+K / Cmd+K to toggle
    bool auto_close_on_select   = true;      ///< Auto-dismiss overlay after executing command
    bool barrier_dismissible    = true;      ///< Close when clicking outside on backdrop scrim
    bool show_recent            = true;      ///< Show recent commands when query is empty
    size_t max_results          = 20;        ///< Max items to display in list

    // Callbacks
    std::function<void()> on_open;
    std::function<void()> on_close;
    std::function<void(const std::string&)> on_query_change;
    std::function<void(const CommandItem&)> on_item_selected;
};

/// ════════════════════════════════════════════════════════════════
/// CommandPalette Controller
/// ════════════════════════════════════════════════════════════════

class CommandPaletteController {
public:
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<void()> toggle_fn;
    std::function<bool()> is_open_fn;
    std::function<void(std::string)> set_query_fn;
    std::function<std::string()> get_query_fn;
    std::function<void(std::vector<CommandItem>)> set_items_fn;
    std::function<void()> select_next_fn;
    std::function<void()> select_prev_fn;
    std::function<void()> execute_active_fn;
    std::function<void()> clear_recent_fn;

    void open() { if (open_fn) open_fn(); }
    void close() { if (close_fn) close_fn(); }
    void toggle() { if (toggle_fn) toggle_fn(); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }

    void setQuery(std::string q) { if (set_query_fn) set_query_fn(std::move(q)); }
    [[nodiscard]] std::string getQuery() const { return get_query_fn ? get_query_fn() : ""; }

    void setItems(std::vector<CommandItem> items) { if (set_items_fn) set_items_fn(std::move(items)); }
    void selectNext() { if (select_next_fn) select_next_fn(); }
    void selectPrevious() { if (select_prev_fn) select_prev_fn(); }
    void executeActive() { if (execute_active_fn) execute_active_fn(); }
    void clearRecent() { if (clear_recent_fn) clear_recent_fn(); }
};

/// ════════════════════════════════════════════════════════════════
/// CommandPalette Widget
/// ════════════════════════════════════════════════════════════════

class CommandPaletteWidget : public StatefulWidget {
public:
    WidgetPtr body;                               ///< Underlying page wrapped in stack overlay
    std::vector<CommandItem> items;               ///< Initial command library
    CommandPaletteOptions options;
    std::shared_ptr<CommandPaletteController> controller;
    bool initial_open = false;

    explicit CommandPaletteWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CommandPalette"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct CommandPaletteProps {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;                    ///< Alias for body
    std::vector<CommandItem> items = {};
    CommandPaletteOptions options = {};
    std::shared_ptr<CommandPaletteController> controller = nullptr;
    bool initial_open = false;
};

struct CommandPalette {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;
    std::vector<CommandItem> items = {};
    CommandPaletteOptions options = {};
    std::shared_ptr<CommandPaletteController> controller = nullptr;
    bool initial_open = false;

    operator WidgetPtr() const {
        auto w = std::make_shared<CommandPaletteWidget>(key);
        w->body = body ? body : child;
        w->items = items;
        w->options = options;
        w->controller = controller;
        w->initial_open = initial_open;
        return w;
    }
};

inline WidgetPtr commandPalette(const CommandPaletteProps& props) {
    auto w = std::make_shared<CommandPaletteWidget>(props.key);
    w->body = props.body ? props.body : props.child;
    w->items = props.items;
    w->options = props.options;
    w->controller = props.controller;
    w->initial_open = props.initial_open;
    return w;
}

} // namespace enki
