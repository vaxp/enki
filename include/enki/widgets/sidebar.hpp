#pragma once
/// @file sidebar.hpp
/// @brief Sidebar — collapsible permanent side panel.
///
/// Unlike Drawer, Sidebar does NOT overlap content. It pushes the body
/// horizontally when expanded and shrinks to icon-only mode when collapsed.
/// Expansion/collapse is animated via AnimationController.
///
/// Features:
///   - Smooth expand/collapse animation.
///   - Toggle button (hamburger / arrow) in header.
///   - Left or Right placement.
///   - Configurable expanded/collapsed widths.
///   - Border between sidebar and body.
///
/// @copyright ENKI Framework — MIT License

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

struct SidebarItem {
    std::string label;
    IconData    icon;
    std::string badge;
};

// ════════════════════════════════════════════════════════════════
// SidebarSide
// ════════════════════════════════════════════════════════════════

enum class SidebarSide {
    Left,
    Right
};

// ════════════════════════════════════════════════════════════════
// SidebarOptions
// ════════════════════════════════════════════════════════════════

struct SidebarOptions {
    float    expanded_width    = 240.0f;
    float    collapsed_width   = 64.0f;
    Color    background_color  = 0xFF1E293B;
    Color    border_color      = 0xFF334155;
    Color    toggle_color      = 0xFF64748B;
    Color    toggle_hover_color = 0xFF818CF8;
    float    border_width      = 1.0f;
    float    toggle_size       = 36.0f;
    SidebarSide side           = SidebarSide::Left;
    bool     collapsible       = true;
    bool     initially_expanded = true;
    bool     show_toggle_button = true;

    constexpr bool operator==(const SidebarOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// Sidebar Widget Implementation
// ════════════════════════════════════════════════════════════════

class SidebarWidget : public StatefulWidget {
public:
    WidgetPtr                  sidebar_content; ///< Content inside the sidebar panel
    WidgetPtr                  body;            ///< Main content area
    SidebarOptions             options;
    std::function<void(bool)>  on_toggle;       ///< Called with new expanded state

    SidebarWidget() = default;
    SidebarWidget(WidgetPtr sidebar_content, WidgetPtr body, SidebarOptions opt = {},
                  std::function<void(bool)> on_toggle = nullptr)
        : sidebar_content(std::move(sidebar_content)), body(std::move(body)),
          options(std::move(opt)), on_toggle(std::move(on_toggle)) {}
    SidebarWidget(Key key, WidgetPtr sidebar_content, WidgetPtr body, SidebarOptions opt = {},
                  std::function<void(bool)> on_toggle = nullptr)
        : StatefulWidget(std::move(key)),
          sidebar_content(std::move(sidebar_content)), body(std::move(body)),
          options(std::move(opt)), on_toggle(std::move(on_toggle)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Sidebar"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Sidebar {
    Key                        key = Key::none();
    WidgetPtr                  sidebar_content = nullptr;
    WidgetPtr                  body = nullptr;
    SidebarOptions             options = {};
    std::function<void(bool)>  on_toggle = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<SidebarWidget>(key, sidebar_content, body, options, on_toggle);
    }
};

} // namespace enki
