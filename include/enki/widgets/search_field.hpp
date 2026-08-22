#pragma once
/// @file search_field.hpp
/// @brief Advanced SearchField widget for ENKI Framework.
/// Supports suggestions, command palette mode, search history, debounce, match highlighting, and custom themes.

#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include "enki/core/types.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>

namespace enki {

/// Visual styling variants for SearchField
enum class SearchFieldVariant {
    Filled,      ///< Solid dark container with subtle border
    Outlined,    ///< Transparent container with crisp border
    Pill,        ///< Fully rounded capsule search bar
    CommandBar   ///< Spotlight/Raycast-style large command launcher
};

/// Predefined size presets
enum class SearchFieldSize {
    Small,   ///< Compact height ~34px
    Medium,  ///< Standard height ~42px
    Large    ///< Spacious command launcher height ~52px
};

/// ════════════════════════════════════════════════════════════════
/// Search Suggestion Item
/// ════════════════════════════════════════════════════════════════

struct SearchSuggestion {
    std::string id = "";
    std::string title = "";
    std::string subtitle = "";
    std::string category = ""; // e.g. "Commands", "Files", "Recent", "Settings"
    std::string badge = "";    // e.g. "Ctrl+N", "Folder", "Tag"
    std::string icon_char = "";// e.g. "📁", "⚡", "⚙️", "📄", "🔍"
    Color icon_color = 0xFF38BDF8;

    SearchSuggestion() = default;
    SearchSuggestion(std::string t, std::string sub = "", std::string cat = "", std::string b = "", std::string ic = "🔍")
        : title(std::move(t)), subtitle(std::move(sub)), category(std::move(cat)), badge(std::move(b)), icon_char(std::move(ic)) {}
};

/// ════════════════════════════════════════════════════════════════
/// SearchField Controller
/// ════════════════════════════════════════════════════════════════

class SearchFieldController {
private:
    std::string query_ = "";
    std::vector<std::string> recent_searches_;
    std::vector<SearchSuggestion> suggestions_;
    int active_suggestion_index_ = -1;
    bool is_loading_ = false;
    static constexpr size_t kMaxRecentHistory = 12;

public:
    SearchFieldController(std::string initial_query = "") : query_(std::move(initial_query)) {}

    [[nodiscard]] const std::string& getQuery() const { return query_; }
    void setQuery(std::string_view q) { query_ = std::string(q); }

    void addRecentSearch(const std::string& q) {
        if (q.empty()) return;
        for (auto it = recent_searches_.begin(); it != recent_searches_.end(); ++it) {
            if (*it == q) {
                recent_searches_.erase(it);
                break;
            }
        }
        recent_searches_.insert(recent_searches_.begin(), q);
        if (recent_searches_.size() > kMaxRecentHistory) {
            recent_searches_.pop_back();
        }
    }

    void removeRecentSearch(size_t index) {
        if (index < recent_searches_.size()) {
            recent_searches_.erase(recent_searches_.begin() + index);
        }
    }

    void clearRecentSearches() {
        recent_searches_.clear();
    }

    [[nodiscard]] const std::vector<std::string>& getRecentSearches() const {
        return recent_searches_;
    }

    void setSuggestions(std::vector<SearchSuggestion> list) {
        suggestions_ = std::move(list);
        active_suggestion_index_ = -1;
    }

    [[nodiscard]] const std::vector<SearchSuggestion>& getSuggestions() const {
        return suggestions_;
    }

    [[nodiscard]] int getActiveSuggestionIndex() const { return active_suggestion_index_; }
    void setActiveSuggestionIndex(int idx) { active_suggestion_index_ = idx; }

    [[nodiscard]] bool isLoading() const { return is_loading_; }
    void setLoading(bool loading) { is_loading_ = loading; }

    void clear() {
        query_.clear();
        suggestions_.clear();
        active_suggestion_index_ = -1;
        is_loading_ = false;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Configuration Options for SearchField
/// ════════════════════════════════════════════════════════════════

struct SearchFieldProps {
    Key key = Key::none();
    std::shared_ptr<SearchFieldController> controller = nullptr;

    std::string placeholder = "Search or type a command...";
    SearchFieldVariant variant = SearchFieldVariant::Filled;
    SearchFieldSize size = SearchFieldSize::Medium;

    bool show_search_icon = true;
    bool show_clear_button = true;
    bool show_shortcut_badge = true;
    std::string shortcut_hint = "Ctrl+K";
    bool auto_focus = false;
    bool read_only = false;

    // Suggestions & Debounce
    bool show_suggestions = true;
    int max_visible_suggestions = 6;
    double debounce_ms = 250.0;

    // Styling
    TextStyle style;
    Color background_color   = 0xFF0F172A;
    Color border_color       = 0xFF334155;
    Color focus_border_color = 0xFF38BDF8;
    Color icon_color         = 0xFF94A3B8;
    Color placeholder_color  = 0xFF64748B;
    Color cursor_color       = 0xFF38BDF8;
    Color selection_color    = 0x4D38BDF8;
    Color badge_bg_color     = 0xFF1E293B;
    Color badge_text_color   = 0xFF94A3B8;
    Color popup_bg_color     = 0xFF0F172A;
    Color item_hover_color   = 0xFF1E293B;
    Color match_highlight_col= 0xFF38BDF8;

    float border_radius = 10.0f;
    EdgeInsets padding = EdgeInsets::symmetric(8.0f, 12.0f);

    // Callbacks
    std::function<std::vector<SearchSuggestion>(std::string_view query)> suggestions_provider = nullptr;
    std::function<void(std::string_view query)> on_changed = nullptr;
    std::function<void(std::string_view query)> on_search = nullptr;
    std::function<void(std::string_view query)> on_submitted = nullptr;
    std::function<void(const SearchSuggestion& item)> on_suggestion_selected = nullptr;
};

/// ════════════════════════════════════════════════════════════════
/// SearchField Widget Implementation
/// ════════════════════════════════════════════════════════════════

class SearchFieldWidget : public StatefulWidget {
public:
    SearchFieldProps props;

    SearchFieldWidget(SearchFieldProps p)
        : props(std::move(p)) {
        if (!props.controller) props.controller = std::make_shared<SearchFieldController>();
    }

    SearchFieldWidget(Key key, SearchFieldProps p)
        : StatefulWidget(std::move(key)), props(std::move(p)) {
        if (!props.controller) props.controller = std::make_shared<SearchFieldController>();
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "SearchField"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct SearchField {
    Key key = Key::none();
    std::shared_ptr<SearchFieldController> controller = nullptr;

    std::string placeholder = "Search or type a command...";
    SearchFieldVariant variant = SearchFieldVariant::Filled;
    SearchFieldSize size = SearchFieldSize::Medium;

    bool show_search_icon = true;
    bool show_clear_button = true;
    bool show_shortcut_badge = true;
    std::string shortcut_hint = "Ctrl+K";
    bool auto_focus = false;
    bool read_only = false;

    bool show_suggestions = true;
    int max_visible_suggestions = 6;
    double debounce_ms = 250.0;

    TextStyle style;
    Color background_color   = 0xFF0F172A;
    Color border_color       = 0xFF334155;
    Color focus_border_color = 0xFF38BDF8;
    Color icon_color         = 0xFF94A3B8;
    Color placeholder_color  = 0xFF64748B;
    Color cursor_color       = 0xFF38BDF8;
    Color selection_color    = 0x4D38BDF8;
    Color badge_bg_color     = 0xFF1E293B;
    Color badge_text_color   = 0xFF94A3B8;
    Color popup_bg_color     = 0xFF0F172A;
    Color item_hover_color   = 0xFF1E293B;
    Color match_highlight_col= 0xFF38BDF8;

    float border_radius = 10.0f;
    EdgeInsets padding = EdgeInsets::symmetric(8.0f, 12.0f);

    std::function<std::vector<SearchSuggestion>(std::string_view query)> suggestions_provider = nullptr;
    std::function<void(std::string_view query)> on_changed = nullptr;
    std::function<void(std::string_view query)> on_search = nullptr;
    std::function<void(std::string_view query)> on_submitted = nullptr;
    std::function<void(const SearchSuggestion& item)> on_suggestion_selected = nullptr;

    operator WidgetPtr() const {
        SearchFieldProps p;
        p.key = key;
        p.controller = controller;
        p.placeholder = placeholder;
        p.variant = variant;
        p.size = size;
        p.show_search_icon = show_search_icon;
        p.show_clear_button = show_clear_button;
        p.show_shortcut_badge = show_shortcut_badge;
        p.shortcut_hint = shortcut_hint;
        p.auto_focus = auto_focus;
        p.read_only = read_only;
        p.show_suggestions = show_suggestions;
        p.max_visible_suggestions = max_visible_suggestions;
        p.debounce_ms = debounce_ms;
        p.style = style;
        p.background_color = background_color;
        p.border_color = border_color;
        p.focus_border_color = focus_border_color;
        p.icon_color = icon_color;
        p.placeholder_color = placeholder_color;
        p.cursor_color = cursor_color;
        p.selection_color = selection_color;
        p.badge_bg_color = badge_bg_color;
        p.badge_text_color = badge_text_color;
        p.popup_bg_color = popup_bg_color;
        p.item_hover_color = item_hover_color;
        p.match_highlight_col = match_highlight_col;
        p.border_radius = border_radius;
        p.padding = padding;
        p.suggestions_provider = suggestions_provider;
        p.on_changed = on_changed;
        p.on_search = on_search;
        p.on_submitted = on_submitted;
        p.on_suggestion_selected = on_suggestion_selected;

        return std::make_shared<SearchFieldWidget>(key, std::move(p));
    }
};

} // namespace enki
