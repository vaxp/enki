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
/// ExpansionPanel Options
/// ════════════════════════════════════════════════════════════════

struct ExpansionPanelOptions {
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
/// ExpansionPanelList Widget
/// ════════════════════════════════════════════════════════════════

class ExpansionPanelList : public StatefulWidget {
public:
    std::vector<ExpansionPanelItem> panels;
    ExpansionPanelOptions options;
    std::shared_ptr<ExpansionPanelController> controller;

    ExpansionPanelList() = default;
    ExpansionPanelList(std::vector<ExpansionPanelItem> panels_, ExpansionPanelOptions opts = {},
                       std::shared_ptr<ExpansionPanelController> ctrl = nullptr)
        : panels(std::move(panels_)), options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ExpansionPanelList"; }
};

inline std::shared_ptr<ExpansionPanelList> expansionPanelList(
    std::vector<ExpansionPanelItem> panels,
    ExpansionPanelOptions options = {},
    std::shared_ptr<ExpansionPanelController> controller = nullptr) {
    return std::make_shared<ExpansionPanelList>(std::move(panels), std::move(options), std::move(controller));
}

} // namespace enki
