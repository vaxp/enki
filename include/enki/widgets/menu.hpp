#pragma once
/// @file menu.hpp
/// @brief Advanced Native Menu & MenuBar Widget built on NativePopup surfaces.
///
/// Supports horizontal MenuBar, dropdown menus, cascading submenus, checkbox items,
/// radio group items, keyboard shortcuts hints, separators, and Skia styling.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/shell/shell_types.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace enki {

/// Type of item displayed within a Menu
enum class MenuItemType {
    Action,
    Checkbox,
    Radio,
    Submenu,
    Divider
};

/// Configuration styling for Menu and MenuBar
struct MenuOptions {
    Color background_color  = 0xFA1E293B;  ///< Dark slate background (ARGB)
    Color border_color      = 0xFF334155;  ///< Menu popup border color
    Color item_hover_color  = 0xFF334155;  ///< Hover highlight color
    Color text_color        = 0xFFF1F5F9;  ///< Primary text color
    Color text_sec_color    = 0xFF94A3B8;  ///< Secondary text / shortcut color
    Color accent_color      = 0xFF38BDF8;  ///< Active / checked accent color
    Color disabled_color    = 0xFF64748B;  ///< Muted color for disabled items

    float border_width      = 1.0f;
    float border_radius     = 8.0f;
    float elevation         = 12.0f;       ///< Skia drop shadow elevation
    float min_width         = 190.0f;      ///< Minimum width of menu popup
    EdgeInsets padding      = EdgeInsets::all(6.0f);
    bool auto_dismiss       = true;
};

/// Represents an individual item within a Menu
class MenuItem {
public:
    MenuItemType type = MenuItemType::Action;
    std::string icon = "";
    std::string label = "";
    std::string shortcut = "";
    bool enabled = true;
    bool checked = false;
    int radio_group = 0;
    int radio_value = 0;

    std::function<void()> on_selected = nullptr;
    std::function<void(bool)> on_toggle = nullptr;
    std::vector<MenuItem> submenu_items;

    MenuItem() = default;

    /// Creates a standard clickable action item.
    static MenuItem action(std::string label, std::function<void()> on_selected,
                           std::string icon = "", std::string shortcut = "", bool enabled = true) {
        MenuItem item;
        item.type = MenuItemType::Action;
        item.label = std::move(label);
        item.on_selected = std::move(on_selected);
        item.icon = std::move(icon);
        item.shortcut = std::move(shortcut);
        item.enabled = enabled;
        return item;
    }

    /// Creates a checkable toggle item.
    static MenuItem checkbox(std::string label, bool checked, std::function<void(bool)> on_toggle,
                             std::string shortcut = "", bool enabled = true) {
        MenuItem item;
        item.type = MenuItemType::Checkbox;
        item.label = std::move(label);
        item.checked = checked;
        item.on_toggle = std::move(on_toggle);
        item.shortcut = std::move(shortcut);
        item.enabled = enabled;
        return item;
    }

    /// Creates a radio group selectable item.
    static MenuItem radio(std::string label, int group, int value, bool checked,
                          std::function<void()> on_selected, bool enabled = true) {
        MenuItem item;
        item.type = MenuItemType::Radio;
        item.label = std::move(label);
        item.radio_group = group;
        item.radio_value = value;
        item.checked = checked;
        item.on_selected = std::move(on_selected);
        item.enabled = enabled;
        return item;
    }

    /// Creates a cascading submenu item.
    static MenuItem submenu(std::string label, std::vector<MenuItem> items,
                            std::string icon = "", bool enabled = true) {
        MenuItem item;
        item.type = MenuItemType::Submenu;
        item.label = std::move(label);
        item.submenu_items = std::move(items);
        item.icon = std::move(icon);
        item.enabled = enabled;
        return item;
    }

    /// Creates a horizontal separator divider.
    static MenuItem divider() {
        MenuItem item;
        item.type = MenuItemType::Divider;
        return item;
    }
};

/// Represents a top-level menu category inside a MenuBar (e.g. "File", "Edit")
struct MenuEntry {
    std::string label;
    std::vector<MenuItem> items;
    bool enabled = true;

    MenuEntry(std::string label, std::vector<MenuItem> items, bool enabled = true)
        : label(std::move(label)), items(std::move(items)), enabled(enabled) {}
};

// ════════════════════════════════════════════════════════════════
// MenuBar Widget
// ════════════════════════════════════════════════════════════════

class MenuBar : public StatefulWidget {
public:
    std::vector<MenuEntry> entries;
    MenuOptions options;

    MenuBar(std::vector<MenuEntry> entries, MenuOptions options = MenuOptions())
        : entries(std::move(entries)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "MenuBar"; }
};

inline WidgetPtr menuBar(std::vector<MenuEntry> entries, MenuOptions options = MenuOptions()) {
    return std::make_shared<MenuBar>(std::move(entries), std::move(options));
}

// ════════════════════════════════════════════════════════════════
// Standalone / Anchor Menu Widget
// ════════════════════════════════════════════════════════════════

class Menu : public StatefulWidget {
public:
    WidgetPtr child;
    std::vector<MenuItem> items;
    MenuOptions options;

    Menu(WidgetPtr child, std::vector<MenuItem> items, MenuOptions options = MenuOptions())
        : child(std::move(child)), items(std::move(items)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Menu"; }
};

inline WidgetPtr menu(WidgetPtr child, std::vector<MenuItem> items, MenuOptions options = MenuOptions()) {
    return std::make_shared<Menu>(std::move(child), std::move(items), std::move(options));
}

} // namespace enki
