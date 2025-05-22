#pragma once
/// @file navigation_bar.hpp
/// @brief NavigationBar — horizontal bottom navigation bar.
///
/// Features:
///   - Horizontal icon + label items with animated active indicator pill.
///   - Badge support per item.
///   - Hover feedback.
///   - Fully configurable via NavigationBarOptions.
///   - Fluent builder API.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/icon.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// NavigationBarItem
// ════════════════════════════════════════════════════════════════

struct NavigationBarItem {
    std::string label;
    IconData    icon;
    std::string badge;  ///< empty = no badge
};

// ════════════════════════════════════════════════════════════════
// NavigationBarOptions
// ════════════════════════════════════════════════════════════════

struct NavigationBarOptions {
    Color background_color  = 0xFF1E293B;
    Color border_color      = 0xFF334155;
    Color active_color      = 0xFF818CF8;
    Color inactive_color    = 0xFF64748B;
    Color indicator_color   = 0x1A818CF8;  ///< Active item background pill
    Color badge_color       = 0xFFEF4444;
    Color badge_text_color  = 0xFFFFFFFF;
    Color hover_color       = 0x0FFFFFFF;

    float height            = 64.0f;
    float icon_font_size    = 22.0f;
    float label_font_size   = 11.0f;
    float indicator_radius  = 16.0f;
    float indicator_w       = 56.0f;
    float indicator_h       = 32.0f;
    float border_top_width  = 1.0f;
    bool  show_labels       = true;

    constexpr bool operator==(const NavigationBarOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// NavigationBar Widget
// ════════════════════════════════════════════════════════════════

class NavigationBar : public StatefulWidget {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index = 0;
    std::function<void(int)>       on_item_selected;
    NavigationBarOptions           options;

    NavigationBar() = default;
    NavigationBar(std::vector<NavigationBarItem> items, int selected,
                  std::function<void(int)> on_selected,
                  NavigationBarOptions opt = {})
        : items(std::move(items)), selected_index(selected),
          on_item_selected(std::move(on_selected)), options(std::move(opt)) {}

    // Fluent API
    NavigationBar& backgroundColor(Color c) { options.background_color = c; return *this; }
    NavigationBar& activeColor(Color c)     { options.active_color = c;     return *this; }
    NavigationBar& inactiveColor(Color c)   { options.inactive_color = c;   return *this; }
    NavigationBar& height(float h)          { options.height = h;           return *this; }
    NavigationBar& showLabels(bool v)       { options.show_labels = v;      return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "NavigationBar"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<NavigationBar> navigationBar(
        std::vector<NavigationBarItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        NavigationBarOptions options = {}) {
    return std::make_shared<NavigationBar>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

} // namespace enki
