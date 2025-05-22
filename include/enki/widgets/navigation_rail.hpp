#pragma once
/// @file navigation_rail.hpp
/// @brief NavigationRail — vertical side navigation rail.
///
/// Features:
///   - Vertical list of icon + label items with active indicator.
///   - Collapse/Expand animation (AnimationController).
///   - Optional header widget (e.g. logo or toggle button).
///   - Badge support per item.
///   - Hover feedback with cursor change.
///   - Fluent builder API.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/icon.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <string_view>
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// NavigationRailItem
// ════════════════════════════════════════════════════════════════

struct NavigationRailItem {
    std::string label;
    IconData    icon;
    std::string badge;  ///< empty = none
};

// ════════════════════════════════════════════════════════════════
// NavigationRailOptions
// ════════════════════════════════════════════════════════════════

struct NavigationRailOptions {
    Color background_color  = 0xFF1E293B;
    Color border_color      = 0xFF334155;
    Color active_color      = 0xFF818CF8;
    Color inactive_color    = 0xFF64748B;
    Color indicator_color   = 0x1A818CF8;
    Color hover_color       = 0x0FFFFFFF;
    Color badge_color       = 0xFFEF4444;
    Color badge_text_color  = 0xFFFFFFFF;

    float collapsed_width   = 72.0f;
    float expanded_width    = 220.0f;
    float item_height       = 52.0f;
    float icon_font_size    = 20.0f;
    float label_font_size   = 13.0f;
    float indicator_radius  = 12.0f;
    float border_right_width = 1.0f;
    float header_height     = 56.0f;    ///< Height of the header area (toggle button)
    float padding_v         = 8.0f;

    bool  initially_expanded = true;

    constexpr bool operator==(const NavigationRailOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// NavigationRail Widget
// ════════════════════════════════════════════════════════════════

class NavigationRail : public StatefulWidget {
public:
    std::vector<NavigationRailItem> items;
    int                             selected_index = 0;
    std::function<void(int)>        on_item_selected;
    NavigationRailOptions           options;
    WidgetPtr                       header;  ///< Optional header widget (nullptr = use default toggle)

    NavigationRail() = default;
    NavigationRail(std::vector<NavigationRailItem> items, int selected,
                   std::function<void(int)> on_selected,
                   NavigationRailOptions opt = {})
        : items(std::move(items)), selected_index(selected),
          on_item_selected(std::move(on_selected)), options(std::move(opt)) {}

    // Fluent API
    NavigationRail& backgroundColor(Color c)   { options.background_color = c;   return *this; }
    NavigationRail& activeColor(Color c)       { options.active_color = c;       return *this; }
    NavigationRail& collapsedWidth(float w)    { options.collapsed_width = w;    return *this; }
    NavigationRail& expandedWidth(float w)     { options.expanded_width = w;     return *this; }
    NavigationRail& initiallyExpanded(bool v)  { options.initially_expanded = v; return *this; }
    NavigationRail& setHeader(WidgetPtr h)     { header = std::move(h);          return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "NavigationRail"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<NavigationRail> navigationRail(
        std::vector<NavigationRailItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        NavigationRailOptions options = {}) {
    return std::make_shared<NavigationRail>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

} // namespace enki
