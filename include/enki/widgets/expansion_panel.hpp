#pragma once
/// @file expansion_panel.hpp
/// @brief Advanced ExpansionPanel & ExpansionPanelList widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports radio/multi-expansion modes, dynamic elevation gap transitions, header summaries & badges,
/// footer action bars, and ExpansionPanelController.
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
#include <set>
#include <optional>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// ExpansionPanel Item Model
/// ════════════════════════════════════════════════════════════════

struct ExpansionPanelItem {
    std::string id = "";
    std::string title = "";
    std::string subtitle = "";
    std::string icon_or_step = "";     ///< Leading step pill or icon (e.g. "1", "2", 🚀, 📦, 💳)
    std::string badge_label = "";      ///< Trailing badge (e.g. "Completed", "Required")
    Color badge_bg = 0x2E10B981;       ///< Badge background color
    Color badge_fg = 0xFF10B981;       ///< Badge text color

    WidgetPtr body;                    ///< Content revealed when panel is expanded
    std::vector<WidgetPtr> footer_actions; ///< Action buttons in footer bar (e.g. [Cancel], [Next Step])

    bool is_initially_expanded = false;
    bool is_disabled = false;
    bool can_tap_on_header = true;

    ExpansionPanelItem() = default;
    ExpansionPanelItem(std::string id_, std::string title_, WidgetPtr body_,
                       std::string icon_step = "", std::string subtitle_ = "",
                       bool initially_expanded = false)
        : id(std::move(id_)), title(std::move(title_)), subtitle(std::move(subtitle_)),
          icon_or_step(std::move(icon_step)), body(std::move(body_)),
          is_initially_expanded(initially_expanded) {}

    ExpansionPanelItem& setBadge(std::string label, Color bg = 0x2E10B981, Color fg = 0xFF10B981) {
        badge_label = std::move(label);
        badge_bg = bg;
        badge_fg = fg;
        return *this;
    }

    ExpansionPanelItem& setFooterActions(std::vector<WidgetPtr> actions) {
        footer_actions = std::move(actions);
        return *this;
    }

    ExpansionPanelItem& setDisabled(bool d) {
        is_disabled = d;
        return *this;
    }
};

/// ════════════════════════════════════════════════════════════════
/// ExpansionPanel Controller
/// ════════════════════════════════════════════════════════════════

class ExpansionPanelController {
public:
    std::function<void(int)> expand_fn;
    std::function<void(int)> collapse_fn;
    std::function<void(int)> toggle_fn;
    std::function<void()> expand_all_fn;
    std::function<void()> collapse_all_fn;
    std::function<bool(int)> is_expanded_fn;
    std::function<std::set<int>()> get_expanded_indices_fn;

    void expand(int idx) { if (expand_fn) expand_fn(idx); }
    void collapse(int idx) { if (collapse_fn) collapse_fn(idx); }
    void toggle(int idx) { if (toggle_fn) toggle_fn(idx); }
    void expandAll() { if (expand_all_fn) expand_all_fn(); }
    void collapseAll() { if (collapse_all_fn) collapse_all_fn(); }
    [[nodiscard]] bool isExpanded(int idx) const { return is_expanded_fn ? is_expanded_fn(idx) : false; }
    [[nodiscard]] std::set<int> getExpandedIndices() const { return get_expanded_indices_fn ? get_expanded_indices_fn() : std::set<int>{}; }
};

/// ════════════════════════════════════════════════════════════════
/// ExpansionPanelList Props & Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct ExpansionPanelListProps {
    Key key = Key::none();
    std::vector<ExpansionPanelItem> panels;
    std::shared_ptr<ExpansionPanelController> controller;

    bool is_radio_mode = false;        ///< If true, only 1 panel expanded at a time
    float gap = 12.0f;                 ///< Spacing between panels
    float expanded_elevation_gap = 16.0f; ///< Extra vertical margin for expanded panel
    float border_radius = 10.0f;

    // Styling Colors
    Color background_color    = 0xFF1E293B; // Slate 800
    Color border_color        = 0xFF334155; // Slate 700
    Color expanded_border_col = 0xFF0284C7; // Blue 600
    Color title_color         = 0xFFFFFFFF; // White
    Color subtitle_color      = 0xFF94A3B8; // Slate 400
    Color chevron_color       = 0xFF94A3B8; // Slate 400
    Color divider_color       = 0xFF334155; // Slate 700
    Color step_pill_bg        = 0xFF0F172A; // Slate 900
    Color step_pill_fg        = 0xFF38BDF8; // Sky 400

    bool show_chevron = true;

    // Callbacks
    std::function<void(int panel_index, bool is_expanded)> on_panel_toggled;
    std::function<void(const std::set<int>& expanded_indices)> on_expansion_changed;
};

class ExpansionPanelListWidget : public StatefulWidget {
public:
    ExpansionPanelListProps props;

    ExpansionPanelListWidget() = default;
    explicit ExpansionPanelListWidget(ExpansionPanelListProps p) : StatefulWidget(p.key), props(std::move(p)) {}
    ExpansionPanelListWidget(Key k, ExpansionPanelListProps p) : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ExpansionPanelList"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct ExpansionPanelList {
    Key key = Key::none();
    std::vector<ExpansionPanelItem> panels;
    std::shared_ptr<ExpansionPanelController> controller = nullptr;

    bool is_radio_mode = false;
    float gap = 12.0f;
    float expanded_elevation_gap = 16.0f;
    float border_radius = 10.0f;

    Color background_color    = 0xFF1E293B;
    Color border_color        = 0xFF334155;
    Color expanded_border_col = 0xFF0284C7;
    Color title_color         = 0xFFFFFFFF;
    Color subtitle_color      = 0xFF94A3B8;
    Color chevron_color       = 0xFF94A3B8;
    Color divider_color       = 0xFF334155;
    Color step_pill_bg        = 0xFF0F172A;
    Color step_pill_fg        = 0xFF38BDF8;

    bool show_chevron = true;

    std::function<void(int panel_index, bool is_expanded)> on_panel_toggled = nullptr;
    std::function<void(const std::set<int>& expanded_indices)> on_expansion_changed = nullptr;

    operator WidgetPtr() const {
        ExpansionPanelListProps p;
        p.key = key;
        p.panels = panels;
        p.controller = controller;
        p.is_radio_mode = is_radio_mode;
        p.gap = gap;
        p.expanded_elevation_gap = expanded_elevation_gap;
        p.border_radius = border_radius;
        p.background_color = background_color;
        p.border_color = border_color;
        p.expanded_border_col = expanded_border_col;
        p.title_color = title_color;
        p.subtitle_color = subtitle_color;
        p.chevron_color = chevron_color;
        p.divider_color = divider_color;
        p.step_pill_bg = step_pill_bg;
        p.step_pill_fg = step_pill_fg;
        p.show_chevron = show_chevron;
        p.on_panel_toggled = on_panel_toggled;
        p.on_expansion_changed = on_expansion_changed;
        return std::make_shared<ExpansionPanelListWidget>(key, std::move(p));
    }
};

} // namespace enki
