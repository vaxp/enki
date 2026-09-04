# CommandPalette

> An in-window keyboard-driven fuzzy-search command launcher overlay (Ctrl+K / Cmd+K style) featuring intelligent subsequence scoring with character match highlighting, category section grouping, recent commands MRU history, keyboard navigation (Arrow Up/Down, Enter, Esc), danger action badges, and animated entrance/exit transitions.

- **Header File**: `#include "enki/widgets/command_palette.hpp"`
- **C++ Class**: `enki::CommandPaletteWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::CommandPalette` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::CommandPaletteOptions`
- **Controller**: `enki::CommandPaletteController`
- **Data Struct**: `enki::CommandItem`
- **Helper Function**: `enki::commandPalette(const CommandPaletteProps&)`

---

## Overview

`CommandPalette` provides a modern, high-performance command launcher and quick-search overlay inspired by industry-standard developer tools (VS Code, Raycast, Sublime Text, Linear, and macOS Spotlight). Rendered within Enki's **Container-Wrapping Architecture**, it wraps the underlying viewport `body` inside an unclipped `100% × 100%` `Stack`, displaying an animated dark obsidian backdrop scrim and a centered floating command card when invoked.

### Key Capabilities

- **Intelligent Fuzzy Match & Highlight Engine**:
  - Exact match, prefix match, and subsequence matching.
  - Bonus weights for word boundaries (spaces, underscores, dashes, slashes) and camelCase capital letters.
  - Character match highlighting: visual emphasis (e.g. Sky 400 bold text runs) on matched characters in item titles.
  - Secondary match support in subtitles and keywords.
- **Full Keyboard Navigation**:
  - Global `Ctrl+K` / `Cmd+K` shortcut listener to open or toggle the palette from anywhere in the window.
  - `ArrowDown` / `ArrowUp` to cycle through items smoothly, automatically skipping disabled commands.
  - `Home` / `End` to jump directly to first or last available command.
  - `Enter` / `Return` to execute the active command and invoke its `on_execute` callback.
  - `Escape` or clicking the backdrop scrim to dismiss the palette.
- **Categorized Sections & Recent History**:
  - Automatically groups commands under distinct category headers (e.g., "Navigation", "Git", "Editor", "System").
  - Tracks executed commands in a Most-Recently-Used (MRU) history cache and surfaces them under a "Recent" section when the search box is empty.
- **Visual Styling & Danger States**:
  - Badges for keyboard shortcuts (`Ctrl+Shift+P`, `Alt+F`, etc.) and tags (`Pro`, `Danger`, etc.).
  - Highlighting for destructive actions (`is_danger = true`) with red danger accents.
- **Animated Physical Transitions**:
  - Hardware-accelerated scale-and-fade in/out driven by `AnimationController` and `Ticker`.

---

## C++ API Definition

### Data Structures

```cpp
namespace enki {

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
                std::function<void()> on_exec = nullptr);
};

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
    size_t max_results          = 40;        ///< Max items to display in list

    // Callbacks
    std::function<void()> on_open;
    std::function<void()> on_close;
    std::function<void(const std::string&)> on_query_change;
    std::function<void(const CommandItem&)> on_item_selected;
};

class CommandPaletteController {
public:
    void open();
    void close();
    void toggle();
    [[nodiscard]] bool isOpen() const;

    void setQuery(std::string q);
    [[nodiscard]] std::string getQuery() const;

    void setItems(std::vector<CommandItem> items);
    void selectNext();
    void selectPrevious();
    void executeActive();
    void clearRecent();
};

} // namespace enki
```

---

## Declarative Usage Example

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

auto palette_ctrl = std::make_shared<CommandPaletteController>();

std::vector<CommandItem> commands = {
    CommandItem("nav_dash", "Go to Dashboard", "Navigation", "Ctrl+1", "📊", [] {
        std::cout << "Dashboard opened\n";
    }),
    CommandItem("git_pull", "Git: Pull Latest Origin", "Git", "Ctrl+Shift+P", "📥", [] {
        std::cout << "Git pull executed\n";
    }),
    CommandItem("edit_format", "Format Code Buffer", "Editor", "Shift+Alt+F", "✨", [] {
        std::cout << "Buffer formatted\n";
    })
};

WidgetPtr app_view = CommandPalette {
    .body = container({
        .color = 0xFF0B0F19,
        .child = text("Press Ctrl+K anytime to launch commands", { .color = 0xFF94A3B8 })
    }),
    .items = commands,
    .options = {
        .placeholder = "Type a command or search...",
        .enable_global_shortcut = true
    },
    .controller = palette_ctrl
};
```

---

## Roadmap v0.2.0 Verification

- [x] **CommandPalette** — Keyboard-driven fuzzy-search command launcher overlay (Ctrl+K style)
- [x] **Spotlight** — Full-screen dimmed overlay with a highlighted "spotlight" region around a widget
- [x] **FloatingPanel** — Draggable, resizable floating window rendered above the main widget tree
