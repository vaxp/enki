#pragma once
/// @file context_menu.hpp
/// @brief Advanced Native ContextMenu widget built on NativePopup.
///
/// ContextMenu spawns a floating native compositor surface (NativePopup)
/// on secondary tap (right click) or long press, providing submenus,
/// shortcuts, divider lines, and smart boundary fitting.
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

/// Type discriminator for context menu entries
enum class ContextMenuItemType {
    Item,
    Divider,
    SubMenu
};

/// Abstract base class for all context menu entries
class ContextMenuItemBase {
public:
    virtual ~ContextMenuItemBase() = default;
    [[nodiscard]] virtual ContextMenuItemType itemType() const = 0;
};

using ContextMenuItemPtr = std::shared_ptr<ContextMenuItemBase>;

/// Standard selectable context menu item
class ContextMenuItem : public ContextMenuItemBase {
public:
    std::string label;
    std::string shortcut_text;
    WidgetPtr icon_widget;
    std::function<void()> on_selected;
    bool disabled = false;
    bool danger   = false;

    ContextMenuItem(std::string label,
                    std::function<void()> on_selected = nullptr,
                    std::string shortcut = "",
                    WidgetPtr icon = nullptr,
                    bool disabled = false,
                    bool danger = false)
        : label(std::move(label)), shortcut_text(std::move(shortcut)),
          icon_widget(std::move(icon)), on_selected(std::move(on_selected)),
          disabled(disabled), danger(danger) {}

    [[nodiscard]] ContextMenuItemType itemType() const override { return ContextMenuItemType::Item; }
};

/// Horizontal divider line in context menu
class ContextMenuDivider : public ContextMenuItemBase {
public:
    [[nodiscard]] ContextMenuItemType itemType() const override { return ContextMenuItemType::Divider; }
};

/// Nested submenu entry in context menu
class ContextMenuSubMenu : public ContextMenuItemBase {
public:
    std::string label;
    WidgetPtr icon_widget;
    std::vector<ContextMenuItemPtr> children;
    bool disabled = false;

    ContextMenuSubMenu(std::string label,
                       std::vector<ContextMenuItemPtr> children,
                       WidgetPtr icon = nullptr,
                       bool disabled = false)
        : label(std::move(label)), icon_widget(std::move(icon)),
          children(std::move(children)), disabled(disabled) {}

    [[nodiscard]] ContextMenuItemType itemType() const override { return ContextMenuItemType::SubMenu; }
};

/// Configuration options for styling ContextMenu
struct ContextMenuOptions {
    Color background_color = 0xFA1F242C; ///< Dark slate menu surface (ARGB)
    Color text_color       = 0xFFF0F6FC; ///< Primary text color
    Color shortcut_color   = 0xFF8B949E; ///< Dimmed shortcut text color
    Color hover_color      = 0xFF30363D; ///< Item hover highlight background
    Color disabled_color   = 0xFF484F58; ///< Disabled text color
    Color danger_color     = 0xFFF85149; ///< Destructive action red color
    Color border_color     = 0xFF363B42; ///< Subtle outer border color

    float border_width     = 1.0f;
    float border_radius    = 8.0f;
    EdgeInsets padding     = EdgeInsets::all(6.0f);
    float elevation        = 10.0f;
    Color shadow_color     = 0x60000000;

    float min_width        = 180.0f;
    float max_width        = 280.0f;
    float item_height      = 32.0f;
    float font_size        = 13.0f;

    std::string custom_shader = "";      ///< Optional SkSL shader code
};

struct ContextMenuProps {
    Key key = Key::none();
    WidgetPtr child;
    std::vector<ContextMenuItemPtr> items;
    ContextMenuOptions options;
};

/// @brief ContextMenu widget wrapping a target child widget.
class ContextMenu : public StatefulWidget {
public:
    WidgetPtr child;
    std::vector<ContextMenuItemPtr> items;
    ContextMenuOptions options;

    ContextMenu(WidgetPtr child,
                std::vector<ContextMenuItemPtr> items,
                ContextMenuOptions options = ContextMenuOptions())
        : child(std::move(child)), items(std::move(items)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ContextMenu"; }
};

// ── Factory Helpers ────────────────────────────────────────────────

inline std::shared_ptr<ContextMenuItem> contextMenuItem(
    std::string label,
    std::function<void()> on_selected = nullptr,
    std::string shortcut = "",
    WidgetPtr icon = nullptr,
    bool disabled = false,
    bool danger = false) {
    return std::make_shared<ContextMenuItem>(
        std::move(label), std::move(on_selected), std::move(shortcut),
        std::move(icon), disabled, danger
    );
}

inline std::shared_ptr<ContextMenuDivider> contextMenuDivider() {
    return std::make_shared<ContextMenuDivider>();
}

inline std::shared_ptr<ContextMenuSubMenu> contextMenuSubMenu(
    std::string label,
    std::vector<ContextMenuItemPtr> children,
    WidgetPtr icon = nullptr,
    bool disabled = false) {
    return std::make_shared<ContextMenuSubMenu>(
        std::move(label), std::move(children), std::move(icon), disabled
    );
}

inline WidgetPtr contextMenu(WidgetPtr child,
                             std::vector<ContextMenuItemPtr> items,
                             ContextMenuOptions options = ContextMenuOptions()) {
    return std::make_shared<ContextMenu>(std::move(child), std::move(items), std::move(options));
}

inline WidgetPtr contextMenu(ContextMenuProps props) {
    auto cm = std::make_shared<ContextMenu>(std::move(props.child), std::move(props.items), std::move(props.options));
    cm->key = props.key;
    return cm;
}

} // namespace enki
