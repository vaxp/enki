#pragma once
/// @file navigation_bar.hpp
/// @brief Advanced NavigationBar Suite for ENKI Framework.
///
/// Features:
///   - Multiple styles: Material 3 Bottom Bar, macOS / iOS 18 Floating Capsule Dock,
///     Desktop Top Header Bar, and Segmented Capsule Tabs.
///   - 600+ FPS Direct Skia hardware-accelerated animated sliding indicator (Spring/Lerp physics).
///   - Multiple indicator styles: Pill, Underline, Dot, Glow, None.
///   - Multiple item layout orientations: Vertical (Mobile/Bottom), Horizontal (Desktop/Header),
///     IconOnly (Dock), LabelOnly.
///   - Smart Badging: Numeric counts, string tags, and glowing dot badges.
///   - Hover alpha feedback, press micro-bounce animations, tooltips, and disabled items.
///   - Full Desktop Header support with leading brand/logo and trailing actions/search.
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
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Enums & Layout Modes
// ════════════════════════════════════════════════════════════════

/// High-level navigation bar visual & layout style.
enum class NavigationBarStyle {
    BottomStandard,   ///< Material 3 style standard bottom navigation bar
    FloatingPill,     ///< macOS Dock / iOS 18 floating capsule island with glassmorphism
    TopHeader,        ///< Desktop & Web top navigation bar with brand, links, search & actions
    SegmentedCapsule  ///< Compact pill-shaped segmented navigation bar
};

/// Visual shape of the active item indicator.
enum class NavIndicatorStyle {
    Pill,             ///< Rounded capsule pill enclosing or highlighting the active item
    Underline,        ///< Sleek glowing underline below the active item
    Dot,              ///< Subtle radiant glowing dot beneath the active item
    Glow,             ///< Ambient gradient aura behind the active item
    None              ///< Color change only without dedicated indicator shape
};

/// Orientation and arrangement of icon + text inside each item.
enum class NavItemLayout {
    Vertical,         ///< Icon on top, label on bottom (Mobile standard)
    Horizontal,       ///< Icon leading on left, label on right (Desktop standard)
    IconOnly,         ///< Icon only without labels (Compact dock)
    LabelOnly         ///< Text label only without icons (Text tab strip)
};

// ════════════════════════════════════════════════════════════════
// NavigationBarItem — Single item descriptor
// ════════════════════════════════════════════════════════════════

struct NavigationBarItem {
    std::string label;
    IconData    icon;
    IconData    selected_icon;      ///< Optional active state icon (if empty, uses `icon`)
    std::string badge;              ///< Numeric count ("5", "99+") or tag text ("NEW")
    bool        dot_badge = false;  ///< If true, shows a small glowing notification dot
    bool        enabled = true;     ///< If false, item is dimmed and non-interactive
    std::string sublabel;           ///< Secondary descriptive subtitle
    std::string tooltip;            ///< Tooltip text shown on hover
    std::string id;                 ///< Semantic identifier / route tag

    NavigationBarItem() = default;
    NavigationBarItem(std::string lbl, IconData ic, std::string bdg = "", bool dot = false)
        : label(std::move(lbl)), icon(std::move(ic)), badge(std::move(bdg)), dot_badge(dot) {}
    NavigationBarItem(std::string lbl, IconData ic, IconData sel_ic, std::string bdg = "", bool dot = false)
        : label(std::move(lbl)), icon(std::move(ic)), selected_icon(std::move(sel_ic)), badge(std::move(bdg)), dot_badge(dot) {}
};

// ════════════════════════════════════════════════════════════════
// NavigationBarOptions — Comprehensive configuration
// ════════════════════════════════════════════════════════════════

struct NavigationBarOptions {
    NavigationBarStyle  style           = NavigationBarStyle::BottomStandard;
    NavIndicatorStyle   indicator_style = NavIndicatorStyle::Pill;
    NavItemLayout       item_layout     = NavItemLayout::Vertical;

    // ── Color Palette ──────────────────────────────────────────
    Color background_color   = 0xFF1E293B;  ///< Primary surface background
    Color border_color       = 0xFF334155;  ///< Border or separator color
    Color active_color       = 0xFF38BDF8;  ///< Active icon & label tint (Cyan/Sky)
    Color inactive_color     = 0xFF94A3B8;  ///< Inactive icon & label tint (Slate)
    Color indicator_color    = 0x2638BDF8;  ///< Active indicator pill/fill color
    Color indicator_border   = 0x4D38BDF8;  ///< Active indicator outline border color
    Color hover_color        = 0x14FFFFFF;  ///< Hover highlight tint
    Color badge_color        = 0xFFEF4444;  ///< Notification badge background (Red)
    Color badge_text_color   = 0xFFFFFFFF;  ///< Notification badge text color
    Color glow_color         = 0x6638BDF8;  ///< Glow aura color for indicator & dots
    Color shadow_color       = 0x66000000;  ///< Shadow color for floating styles

    // ── Geometry & Dimensions ──────────────────────────────────
    float height             = 68.0f;       ///< Total bar height
    float width              = 0.0f;        ///< 0 = 100% full width, >0 = fixed width (for floating pill/dock)
    float icon_font_size     = 22.0f;       ///< Icon glyph size
    float label_font_size    = 11.5f;       ///< Main label font size
    float sublabel_font_size = 9.5f;        ///< Sublabel font size
    float indicator_radius   = 16.0f;       ///< Corner radius of indicator pill
    float indicator_w        = 56.0f;       ///< Width of indicator (or 0 for auto-fit)
    float indicator_h        = 34.0f;       ///< Height of indicator
    float indicator_thickness= 3.0f;        ///< Thickness for Underline indicator
    float border_width       = 1.0f;        ///< Border stroke width
    float corner_radius      = 0.0f;        ///< Bar corner radius (e.g. 24px for floating pill)
    float padding_horizontal = 16.0f;       ///< Left & right internal padding
    float padding_vertical   = 8.0f;        ///< Top & bottom internal padding
    float item_gap           = 6.0f;        ///< Spacing between items (for segmented / dock)

    // ── Floating & Glassmorphism ───────────────────────────────
    bool  enable_glassmorphism = false;     ///< Apply multi-layer glass backdrop effect
    float floating_margin_bottom = 16.0f;   ///< Floating margin from bottom edge
    float shadow_blur        = 20.0f;       ///< Blur radius for floating shadow
    float shadow_offset_y    = 8.0f;        ///< Y offset for floating shadow

    // ── Desktop / Top Header Extras ────────────────────────────
    std::string leading_title;              ///< Optional brand / application title on left
    std::string leading_subtitle;           ///< Optional brand subtitle
    IconData    leading_icon;               ///< Optional brand icon or logo
    bool        show_search_placeholder = false; ///< Draw inline search input mock on desktop
    std::string search_hint  = "Search...";
    std::vector<std::string> trailing_actions; ///< Action button labels (e.g. "Docs", "GitHub")

    // ── Animation & Physics ────────────────────────────────────
    bool  enable_animations  = true;        ///< Smooth 600+ FPS sliding indicator
    float animation_speed    = 16.0f;       ///< Lerp interpolation speed factor (higher = faster snap)
    bool  show_tooltips      = true;        ///< Render tooltips when hovering items
    bool  show_labels        = true;        ///< Global toggle for labels

    bool operator==(const NavigationBarOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// NavigationBar Widget
// ════════════════════════════════════════════════════════════════

class NavigationBar : public StatefulWidget {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index = 0;
    std::function<void(int)>       on_item_selected;
    std::function<void(int)>       on_item_reselect;   ///< Fired when clicking the already-selected item
    std::function<void(std::string_view)> on_action_clicked; ///< Fired when a header action is clicked
    NavigationBarOptions           options;

    NavigationBar() = default;
    NavigationBar(std::vector<NavigationBarItem> items, int selected,
                  std::function<void(int)> on_selected,
                  NavigationBarOptions opt = {})
        : items(std::move(items)), selected_index(selected),
          on_item_selected(std::move(on_selected)), options(std::move(opt)) {}

    // ── Fluent Builder API ─────────────────────────────────────
    NavigationBar& style(NavigationBarStyle s)          { options.style = s; return *this; }
    NavigationBar& indicatorStyle(NavIndicatorStyle s)  { options.indicator_style = s; return *this; }
    NavigationBar& itemLayout(NavItemLayout l)          { options.item_layout = l; return *this; }
    NavigationBar& backgroundColor(Color c)             { options.background_color = c; return *this; }
    NavigationBar& activeColor(Color c)                 { options.active_color = c; return *this; }
    NavigationBar& inactiveColor(Color c)               { options.inactive_color = c; return *this; }
    NavigationBar& indicatorColor(Color c)              { options.indicator_color = c; return *this; }
    NavigationBar& height(float h)                      { options.height = h; return *this; }
    NavigationBar& width(float w)                       { options.width = w; return *this; }
    NavigationBar& cornerRadius(float r)                { options.corner_radius = r; return *this; }
    NavigationBar& glassmorphism(bool v = true)         { options.enable_glassmorphism = v; return *this; }
    NavigationBar& showLabels(bool v)                   { options.show_labels = v; return *this; }
    NavigationBar& leadingBrand(IconData ic, std::string ttl, std::string sub = "") {
        options.leading_icon = std::move(ic);
        options.leading_title = std::move(ttl);
        options.leading_subtitle = std::move(sub);
        return *this;
    }
    NavigationBar& onReselect(std::function<void(int)> cb) {
        on_item_reselect = std::move(cb);
        return *this;
    }
    NavigationBar& onAction(std::function<void(std::string_view)> cb) {
        on_action_clicked = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "NavigationBar"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// Standard Material 3 Navigation Bar (typically bottom placed).
inline std::shared_ptr<NavigationBar> navigationBar(
        std::vector<NavigationBarItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        NavigationBarOptions options = {}) {
    return std::make_shared<NavigationBar>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

/// Floating Capsule Navigation Bar (macOS Dock / iOS 18 Floating Island).
inline std::shared_ptr<NavigationBar> floatingNavBar(
        std::vector<NavigationBarItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        float fixed_width = 460.0f,
        NavigationBarOptions options = {}) {
    options.style = NavigationBarStyle::FloatingPill;
    options.indicator_style = NavIndicatorStyle::Pill;
    options.width = fixed_width;
    options.height = 64.0f;
    options.corner_radius = 32.0f;
    options.enable_glassmorphism = true;
    options.background_color = 0xEE1E293B;
    options.border_color = 0x3338BDF8;
    options.indicator_color = 0x3338BDF8;
    options.indicator_radius = 20.0f;
    return std::make_shared<NavigationBar>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

/// Desktop / Web Top Header Navigation Bar.
inline std::shared_ptr<NavigationBar> topNavigationBar(
        std::vector<NavigationBarItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        std::string leading_title = "ENKI App",
        IconData leading_icon = {},
        NavigationBarOptions options = {}) {
    options.style = NavigationBarStyle::TopHeader;
    options.indicator_style = NavIndicatorStyle::Underline;
    options.item_layout = NavItemLayout::Horizontal;
    options.height = 60.0f;
    options.background_color = 0xFF0F172A;
    options.border_color = 0xFF1E293B;
    options.leading_title = std::move(leading_title);
    options.leading_icon = std::move(leading_icon);
    options.show_search_placeholder = true;
    return std::make_shared<NavigationBar>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

/// Segmented Capsule Navigation Tabs.
inline std::shared_ptr<NavigationBar> segmentedNavBar(
        std::vector<NavigationBarItem> items,
        int selected_index,
        std::function<void(int)> on_selected,
        float fixed_width = 380.0f,
        NavigationBarOptions options = {}) {
    options.style = NavigationBarStyle::SegmentedCapsule;
    options.indicator_style = NavIndicatorStyle::Pill;
    options.item_layout = NavItemLayout::Horizontal;
    options.width = fixed_width;
    options.height = 44.0f;
    options.corner_radius = 22.0f;
    options.background_color = 0xFF0F172A;
    options.indicator_color = 0xFF0284C7;
    options.active_color = 0xFFFFFFFF;
    options.inactive_color = 0xFF94A3B8;
    options.indicator_radius = 18.0f;
    options.indicator_w = 0.0f; // auto-fit
    options.indicator_h = 36.0f;
    return std::make_shared<NavigationBar>(
        std::move(items), selected_index, std::move(on_selected), std::move(options));
}

} // namespace enki
