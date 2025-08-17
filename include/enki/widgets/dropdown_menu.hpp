#pragma once
/// @file dropdown_menu.hpp
/// @brief Advanced In-Window Overlay DropdownMenu widget for ENKI Framework.
///
/// DropdownMenu wraps the app's body content (like Drawer and BottomSheet) and renders
/// the floating menu panel as an absolutely-positioned Positioned child inside a Stack
/// that spans 100% of the window. This ensures the menu appears above all page content.
///
/// Usage:
///   auto page = dropdownMenu(trigger, items, body_widget, options);
///
/// Architecture (Category 7. Overlays — same as BottomSheet, Drawer):
///   Stack (100% x 100%)
///     ├── Positioned::fill(body_widget)       ← page content, invariant layout
///     ├── Scrim (transparent click-catcher)   ← when open
///     └── Positioned(menu_panel)              ← floating menu at anchor position
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

namespace enki {

/// Type of item in the dropdown menu
enum class DropdownMenuItemType {
    Standard,   ///< Regular clickable action
    Checkbox,   ///< Toggleable item with checkmark (✓)
    Radio,      ///< Radio selection bullet (◉/○)
    Header,     ///< Non-clickable group section title
    Divider     ///< Horizontal separator line
};

/// Preferred dropdown placement relative to anchor trigger
enum class DropdownPlacement {
    Auto,       ///< Opens below if space, flips above near bottom edge
    Bottom,     ///< Always opens below trigger
    Top         ///< Always opens above trigger
};

// ════════════════════════════════════════════════════════════════
// DropdownMenuItem
// ════════════════════════════════════════════════════════════════

struct DropdownMenuItem {
    std::string id;
    std::string label;
    std::string subtitle;
    std::string leading_icon;
    std::string trailing_shortcut;
    std::string badge_text;
    Color badge_bg = 0x2E38BDF8;
    Color badge_fg = 0xFF38BDF8;

    DropdownMenuItemType type = DropdownMenuItemType::Standard;
    bool is_checked  = false;
    bool is_disabled = false;
    bool is_danger   = false;

    DropdownMenuItem() = default;

    static DropdownMenuItem standard(std::string id_, std::string label_, std::string icon = "", std::string shortcut = "") {
        DropdownMenuItem it;
        it.id = std::move(id_); it.label = std::move(label_);
        it.leading_icon = std::move(icon); it.trailing_shortcut = std::move(shortcut);
        it.type = DropdownMenuItemType::Standard;
        return it;
    }

    static DropdownMenuItem checkbox(std::string id_, std::string label_, bool checked = false, std::string icon = "") {
        DropdownMenuItem it;
        it.id = std::move(id_); it.label = std::move(label_);
        it.leading_icon = std::move(icon);
        it.type = DropdownMenuItemType::Checkbox;
        it.is_checked = checked;
        return it;
    }

    static DropdownMenuItem radio(std::string id_, std::string label_, bool selected = false, std::string icon = "") {
        DropdownMenuItem it;
        it.id = std::move(id_); it.label = std::move(label_);
        it.leading_icon = std::move(icon);
        it.type = DropdownMenuItemType::Radio;
        it.is_checked = selected;
        return it;
    }

    static DropdownMenuItem header(std::string title) {
        DropdownMenuItem it;
        it.label = std::move(title);
        it.type = DropdownMenuItemType::Header;
        return it;
    }

    static DropdownMenuItem divider() {
        DropdownMenuItem it;
        it.type = DropdownMenuItemType::Divider;
        return it;
    }

    DropdownMenuItem& setBadge(std::string text, Color bg = 0x2E38BDF8, Color fg = 0xFF38BDF8) {
        badge_text = std::move(text);
        badge_bg = bg;
        badge_fg = fg;
        return *this;
    }

    DropdownMenuItem& setSubtitle(std::string sub) {
        subtitle = std::move(sub);
        return *this;
    }

    DropdownMenuItem& setDanger(bool d = true) { is_danger = d; return *this; }
    DropdownMenuItem& setDisabled(bool d = true) { is_disabled = d; return *this; }
};

// ════════════════════════════════════════════════════════════════
// DropdownMenuOptions
// ════════════════════════════════════════════════════════════════

class DropdownMenuController {
public:
    std::function<void()>               open_fn;
    std::function<void()>               close_fn;
    std::function<void()>               toggle_fn;
    std::function<void(const std::string&)> select_fn;
    std::function<bool()>               is_open_fn;
    std::function<std::string()>        get_selected_fn;

    void open()   { if (open_fn)   open_fn();   }
    void close()  { if (close_fn)  close_fn();  }
    void toggle() { if (toggle_fn) toggle_fn(); }
    void select(const std::string& id) { if (select_fn) select_fn(id); }
    [[nodiscard]] bool isOpen()          const { return is_open_fn      ? is_open_fn()      : false; }
    [[nodiscard]] std::string getSelected() const { return get_selected_fn ? get_selected_fn() : ""; }
};

struct DropdownMenuProps {
    std::vector<DropdownMenuItem>       items;
    WidgetPtr                           body = nullptr;           ///< Page body to wrap
    WidgetPtr                           custom_trigger = nullptr; ///< Optional custom trigger widget
    std::shared_ptr<DropdownMenuController> controller = nullptr;

    float menu_width      = 240.0f;
    float max_menu_height = 340.0f;
    float border_radius   = 8.0f;
    bool  close_on_select = true;
    DropdownPlacement placement = DropdownPlacement::Bottom;

    // Trigger styling (for default select-button trigger)
    std::string placeholder = "Select an option...";
    std::string selected_id = "";
    float trigger_width     = 220.0f;
    float trigger_height    = 38.0f;

    // Anchor coordinates (absolute pixel position of trigger top-left in window)
    float anchor_x = 0.0f;  ///< Filled in at runtime by the trigger GestureDetector
    float anchor_y = 0.0f;  ///< Filled in at runtime by the trigger GestureDetector

    // Theme
    Color background_color = 0xFF1E293B;
    Color border_color     = 0xFF334155;
    Color trigger_bg       = 0xFF0F172A;
    Color trigger_border   = 0xFF334155;
    Color hover_color      = 0x2238BDF8;
    Color text_color       = 0xFFF1F5F9;
    Color subtitle_color   = 0xFF94A3B8;
    Color shortcut_color   = 0xFF64748B;
    Color header_color     = 0xFF38BDF8;
    Color divider_color    = 0xFF334155;
    Color danger_color     = 0xFFEF4444;

    // Callbacks
    std::function<void(const DropdownMenuItem&)>       on_selected;
    std::function<void(const std::string&, bool)>      on_toggle_checked;
    std::function<void()>                              on_opened;
    std::function<void()>                              on_closed;
};

// ════════════════════════════════════════════════════════════════
// DropdownMenuController
// ════════════════════════════════════════════════════════════════



// ════════════════════════════════════════════════════════════════
// DropdownMenu Widget
// ════════════════════════════════════════════════════════════════

/// @brief Advanced in-window overlay dropdown menu.
///
/// Wraps body content in a Stack; the floating menu panel is an absolutely-positioned
/// Positioned widget at the trigger anchor coordinates. This matches the Drawer/BottomSheet
/// overlay architecture exactly.
class DropdownMenu : public StatefulWidget {
public:
    DropdownMenuProps                 props;

    DropdownMenu() = default;
    explicit DropdownMenu(DropdownMenuProps p)
        : props(std::move(p)) {}

    // Fluent API
    DropdownMenu& selected(std::string id)         { props.selected_id = std::move(id); return *this; }
    DropdownMenu& placeholder(std::string p)       { props.placeholder = std::move(p);  return *this; }
    DropdownMenu& menuWidth(float w)               { props.menu_width = w;              return *this; }
    DropdownMenu& triggerWidth(float w)            { props.trigger_width = w;           return *this; }
    DropdownMenu& anchorAt(float x, float y)       { props.anchor_x = x; props.anchor_y = y; return *this; }
    DropdownMenu& setController(std::shared_ptr<DropdownMenuController> c) {
        props.controller = std::move(c); return *this;
    }
    DropdownMenu& onSelected(std::function<void(const DropdownMenuItem&)> cb) {
        props.on_selected = std::move(cb); return *this;
    }
    DropdownMenu& onToggleChecked(std::function<void(const std::string&, bool)> cb) {
        props.on_toggle_checked = std::move(cb); return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DropdownMenu"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Helpers
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<DropdownMenu> dropdownMenu(DropdownMenuProps props) {
    return std::make_shared<DropdownMenu>(std::move(props));
}

} // namespace enki
