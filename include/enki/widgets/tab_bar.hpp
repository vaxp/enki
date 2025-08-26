#pragma once
/// @file tab_bar.hpp
/// @brief TabBar and TabView navigation widgets.
///
/// TabBar: Horizontal tab strip with animated indicator underline.
/// TabView: Displays the content corresponding to the selected tab index.
///
/// Features:
///   - Animated indicator that slides between tabs (AnimationController).
///   - Optional icons and/or labels per tab item.
///   - Hover feedback on each tab.
///   - Badge support per tab item.
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
// TabItem — a single tab descriptor
// ════════════════════════════════════════════════════════════════

struct TabItem {
    std::string label;
    IconData    icon;   ///< Use IconData for icons
    std::string badge;  ///< Badge text (empty = no badge)
};

// ════════════════════════════════════════════════════════════════
// TabBarOptions
// ════════════════════════════════════════════════════════════════

struct TabBarOptions {
    Color background_color  = 0xFF1E293B;
    Color active_color      = 0xFF818CF8;
    Color inactive_color    = 0xFF64748B;
    Color indicator_color   = 0xFF818CF8;
    Color hover_color       = 0x1A818CF8;
    Color badge_color       = 0xFFEF4444;
    Color badge_text_color  = 0xFFFFFFFF;

    float tab_height        = 48.0f;
    float indicator_height  = 3.0f;
    float indicator_radius  = 2.0f;
    float label_font_size   = 13.0f;
    float icon_font_size    = 18.0f;
    float badge_font_size   = 10.0f;
    float item_min_width    = 80.0f;
    float padding_h         = 16.0f;
    float gap               = 6.0f;   ///< Gap between icon and label

    bool show_icons         = true;
    bool show_labels        = true;

    constexpr bool operator==(const TabBarOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderTabBar — custom render object
// ════════════════════════════════════════════════════════════════

class RenderTabBar : public RenderBox {
public:
    std::vector<TabItem> tabs;
    int                  selected_index = 0;
    float                indicator_x    = 0.0f; ///< Animated indicator left edge
    float                indicator_w    = 0.0f; ///< Animated indicator width
    int                  hovered_index  = -1;
    TabBarOptions        options;
    
    std::vector<std::unique_ptr<RenderIcon>> icon_renderers;

    RenderTabBar(std::vector<TabItem> tabs, int sel, TabBarOptions opt);

    void paint(PaintContext& context) override;
    bool hitTestSelf(Point localPoint) const override;
    SystemCursor cursor() const override;

    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerExit(const PointerEvent& e) override;

    void syncLayout() override;

    std::function<void(int)> on_tap;
    std::function<void(int)> on_hover_change;

private:
    int getIndexFromPoint(Point p) const;
    /// Compute the x-center of tab at index, given total width.
    float tabCenterX(int index, float total_w) const;
    /// Compute the left edge and width of tab at index.
    void tabRect(int index, float total_w, float& out_x, float& out_w) const;
};

// ════════════════════════════════════════════════════════════════
// TabBar Widget Implementation
// ════════════════════════════════════════════════════════════════

class TabBarWidget : public StatefulWidget {
public:
    std::vector<TabItem>          tabs;
    int                           selected_index = 0;
    std::function<void(int)>      on_tab_changed;
    TabBarOptions                 options;

    TabBarWidget() = default;
    TabBarWidget(std::vector<TabItem> tabs, int selected, std::function<void(int)> on_changed,
                 TabBarOptions opt = {})
        : tabs(std::move(tabs)), selected_index(selected),
          on_tab_changed(std::move(on_changed)), options(std::move(opt)) {}
    TabBarWidget(Key key, std::vector<TabItem> tabs, int selected, std::function<void(int)> on_changed,
                 TabBarOptions opt = {})
        : StatefulWidget(std::move(key)),
          tabs(std::move(tabs)), selected_index(selected),
          on_tab_changed(std::move(on_changed)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TabBar"; }
};

// ════════════════════════════════════════════════════════════════
// TabView Widget Implementation
// ════════════════════════════════════════════════════════════════

/// @brief Displays the child at `selected_index`. Children outside are not built.
class TabViewWidget : public StatelessWidget {
public:
    int                    selected_index = 0;
    std::vector<WidgetPtr> children;

    TabViewWidget() = default;
    TabViewWidget(int selected, std::vector<WidgetPtr> kids)
        : selected_index(selected), children(std::move(kids)) {}
    TabViewWidget(Key key, int selected, std::vector<WidgetPtr> kids)
        : StatelessWidget(std::move(key)),
          selected_index(selected), children(std::move(kids)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "TabView"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct TabBar {
    Key                           key = Key::none();
    std::vector<TabItem>          tabs;
    int                           selected_index = 0;
    std::function<void(int)>      on_tab_changed = nullptr;
    TabBarOptions                 options = {};

    operator WidgetPtr() const {
        return std::make_shared<TabBarWidget>(key, tabs, selected_index, on_tab_changed, options);
    }
};

struct TabView {
    Key                           key = Key::none();
    int                           selected_index = 0;
    std::vector<WidgetPtr>        children;

    operator WidgetPtr() const {
        return std::make_shared<TabViewWidget>(key, selected_index, children);
    }
};

} // namespace enki
