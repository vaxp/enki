# SearchField

> An advanced search input and command palette launcher featuring debounced live queries, categorized suggestions, match highlighting, and hotkey badges.

- **Header File**: `#include "enki/widgets/search_field.hpp"`
- **C++ Class**: `enki::SearchFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::SearchField` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::SearchFieldProps`
- **Controller**: `enki::SearchFieldController`
- **Suggestion Model**: `enki::SearchSuggestion`
- **Enums**: `enki::SearchFieldVariant`, `enki::SearchFieldSize`

---

## Overview

`SearchField` handles application search flows ranging from compact navbar search bars to full Spotlight / Raycast style command palettes. It supports:
1. **Debounced Query Execution**: Configurable `debounce_ms` delay preventing excessive backend search invocations while typing.
2. **Search Suggestion Popup**: Dropdown list categorizing results into sections (e.g. "Commands", "Files", "Recent Searches").
3. **Recent Search Memory**: Built-in history buffer remembering past user queries.
4. **Visual Variants**: `Filled`, `Outlined`, `Pill` (capsule), and `CommandBar` (high-elevation modal launcher).
5. **Hotkey Shortcut Badges**: Displays indicators like `"Ctrl+K"` or `"⌘K"`.

---

## C++ API Definition

### Suggestion Model (`SearchSuggestion`)
```cpp
namespace enki {

struct SearchSuggestion {
    std::string id         = "";
    std::string title      = "";
    std::string subtitle   = "";
    std::string category   = ""; // e.g. "Commands", "Files", "Recent"
    std::string badge      = ""; // e.g. "Ctrl+N", ".cpp"
    std::string icon_char  = "🔍";
    Color       icon_color = 0xFF38BDF8;

    SearchSuggestion() = default;
    SearchSuggestion(std::string t, std::string sub = "", std::string cat = "",
                     std::string b = "", std::string ic = "🔍");
};

} // namespace enki
```

### Controller (`SearchFieldController`)
```cpp
namespace enki {

class SearchFieldController {
public:
    [[nodiscard]] const std::string& getQuery() const;
    void setQuery(std::string_view q);

    void addRecentSearch(const std::string& q);
    void clearRecentSearches();
    [[nodiscard]] const std::vector<std::string>& getRecentSearches() const;

    void setSuggestions(std::vector<SearchSuggestion> list);
    [[nodiscard]] const std::vector<SearchSuggestion>& getSuggestions() const;

    [[nodiscard]] bool isLoading() const;
    void setLoading(bool loading);
    void clear();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class SearchFieldVariant {
    Filled,      ///< Solid dark container with subtle border
    Outlined,    ///< Transparent container with crisp border
    Pill,        ///< Fully rounded capsule search bar
    CommandBar   ///< Spotlight/Raycast-style large command launcher
};

enum class SearchFieldSize {
    Small,   ///< Compact height ~34px
    Medium,  ///< Standard height ~42px
    Large    ///< Spacious command launcher height ~52px
};

struct SearchField {
    Key                                    key                     = Key::none();
    std::shared_ptr<SearchFieldController> controller              = nullptr;

    std::string                            placeholder             = "Search or type a command...";
    SearchFieldVariant                     variant                 = SearchFieldVariant::Filled;
    SearchFieldSize                        size                    = SearchFieldSize::Medium;

    bool                                   show_search_icon        = true;
    bool                                   show_clear_button       = true;
    bool                                   show_shortcut_badge     = true;
    std::string                            shortcut_hint           = "Ctrl+K";
    bool                                   auto_focus              = false;

    // Suggestions & Debounce
    bool                                   show_suggestions        = true;
    int                                    max_visible_suggestions = 6;
    double                                 debounce_ms             = 250.0;

    // Callbacks
    std::function<std::vector<SearchSuggestion>(std::string_view)> suggestions_provider = nullptr;
    std::function<void(std::string_view)>                         on_changed           = nullptr;
    std::function<void(std::string_view)>                         on_search            = nullptr;
    std::function<void(std::string_view)>                         on_submitted         = nullptr;
    std::function<void(const SearchSuggestion&)>                  on_suggestion_selected = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `variant` | `SearchFieldVariant` | `Filled` | Visual design (`Filled`, `Outlined`, `Pill`, `CommandBar`). |
| `size` | `SearchFieldSize` | `Medium` | Vertical size preset (`Small`, `Medium`, `Large`). |
| `placeholder` | `std::string` | `"Search or type..."` | Watermark text displayed inside the field. |
| `shortcut_hint` | `std::string` | `"Ctrl+K"` | Text shown in the trailing shortcut badge. |
| `debounce_ms` | `double` | `250.0` | Debounce delay in milliseconds before invoking `on_search`. |
| `suggestions_provider`| `Function` | `nullptr` | Returns suggestions dynamically based on query string. |
| `on_suggestion_selected`| `Function` | `nullptr` | Callback triggered when user clicks or presses Enter on an item. |

---

## Code Examples (From `widgets_demo/search_field_demo/main.cpp`)

### 1. Spotlight Command Palette
```cpp
#include "enki/widgets/search_field.hpp"

using namespace enki;

auto commandPalette = SearchField {
    .placeholder = "Type a command or search project...",
    .variant = SearchFieldVariant::CommandBar,
    .size = SearchFieldSize::Large,
    .shortcut_hint = "⌘K",
    .auto_focus = true,
    .suggestions_provider = [](std::string_view query) -> std::vector<SearchSuggestion> {
        return {
            SearchSuggestion("Build Project", "Compile active target", "BUILD", "Ctrl+B", "⚡"),
            SearchSuggestion("Find in Files", "Grep across workspace", "NAVIGATE", "Ctrl+Shift+F", "🔍"),
            SearchSuggestion("Settings", "Open user preferences", "APP", "Ctrl+,", "⚙️"),
        };
    },
    .on_suggestion_selected = [](const SearchSuggestion& item) {
        std::cout << "Executing: " << item.title << "\n";
    }
};
```

### 2. Rounded Pill Search Bar for Navbars
```cpp
auto navbarSearch = SearchField {
    .placeholder = "Quick search...",
    .variant = SearchFieldVariant::Pill,
    .size = SearchFieldSize::Small,
    .show_shortcut_badge = false,
    .on_search = [](std::string_view q) {
        // Query database after 250ms debounce
    }
};
```

---

## See Also
- [**TextField**](./text_field.md) — Raw text input widget.
- [**ComboBox**](./combo_box.md) — Form select dropdown.
