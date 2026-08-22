#pragma once
/// @file combo_box.hpp
/// @brief Advanced ComboBox / Searchable Select widget for ENKI Framework (Category 3. Input / Forms).
/// Supports single-select, multi-select tag chips, live search filtering, option grouping,
/// keyboard navigation (Arrow keys / Enter / Escape), clear buttons, and ComboBoxController.
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

/// Selection mode for ComboBox
enum class ComboBoxMode {
    Single,     ///< Single selection; choice fills the input text
    Multi,      ///< Multi-selection; choices render as removable tag chips
    Custom      ///< Allows typing arbitrary custom values
};

// ════════════════════════════════════════════════════════════════
// ComboBox Item Model
// ════════════════════════════════════════════════════════════════

struct ComboBoxItem {
    std::string id = "";
    std::string label = "";
    std::string subtitle = "";
    std::string icon = "";            ///< Leading emoji/icon (e.g. 🦀, ⚡, 🐍, 🌐, 🔒)
    std::string group = "";           ///< Group category (e.g. "Cloud Regions", "Languages")
    std::string badge = "";           ///< Trailing badge (e.g. "FAST", "PRO", "EU")
    Color badge_color = 0xFF38BDF8;

    bool is_disabled = false;

    ComboBoxItem() = default;
    ComboBoxItem(std::string id_, std::string label_, std::string icon_ = "",
                 std::string subtitle_ = "", std::string group_ = "")
        : id(std::move(id_)), label(std::move(label_)), subtitle(std::move(subtitle_)),
          icon(std::move(icon_)), group(std::move(group_)) {}

    ComboBoxItem& setBadge(std::string b, Color c = 0xFF38BDF8) {
        badge = std::move(b);
        badge_color = c;
        return *this;
    }

    ComboBoxItem& setDisabled(bool d) {
        is_disabled = d;
        return *this;
    }
};

// ════════════════════════════════════════════════════════════════
// ComboBox Controller
// ════════════════════════════════════════════════════════════════

class ComboBoxController {
public:
    std::function<void(const std::string&)> select_fn;
    std::function<void(const std::vector<std::string>&)> select_multi_fn;
    std::function<void()> clear_fn;
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<void()> toggle_fn;
    std::function<std::string()> get_value_fn;
    std::function<std::vector<std::string>()> get_multi_values_fn;
    std::function<bool()> is_open_fn;

    void select(const std::string& id) { if (select_fn) select_fn(id); }
    void selectMultiple(const std::vector<std::string>& ids) { if (select_multi_fn) select_multi_fn(ids); }
    void clear() { if (clear_fn) clear_fn(); }
    void open() { if (open_fn) open_fn(); }
    void close() { if (close_fn) close_fn(); }
    void toggle() { if (toggle_fn) toggle_fn(); }
    [[nodiscard]] std::string getValue() const { return get_value_fn ? get_value_fn() : ""; }
    [[nodiscard]] std::vector<std::string> getMultiValues() const { return get_multi_values_fn ? get_multi_values_fn() : std::vector<std::string>{}; }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }
};

// ════════════════════════════════════════════════════════════════
// ComboBox Options
// ════════════════════════════════════════════════════════════════

struct ComboBoxProps {
    Key key = Key::none();
    std::vector<ComboBoxItem> items;
    WidgetPtr body = nullptr;                              ///< Main page body content to wrap in stack overlay
    std::shared_ptr<ComboBoxController> controller = nullptr;

    ComboBoxMode mode = ComboBoxMode::Single;
    std::string placeholder = "Search or select option...";

    float width = 320.0f;
    float input_height = 42.0f;
    float max_menu_height = 260.0f;
    float border_radius = 8.0f;

    bool allow_clear = true;          ///< Show ✕ button to clear selection
    bool allow_custom_value = false;  ///< Allow adding custom values by typing
    bool show_search_icon = true;     ///< Show 🔍 search icon

    // Styling Colors
    Color background_color   = 0xFF0F172A; // Slate 900
    Color border_color       = 0xFF334155; // Slate 700
    Color border_focus_color = 0xFF0284C7; // Blue 600
    Color text_color         = 0xFFFFFFFF; // White
    Color placeholder_color  = 0xFF64748B; // Slate 500
    Color menu_bg_color      = 0xFF1E293B; // Slate 800
    Color item_hover_color   = 0x3338BDF8; // Sky 10%
    Color item_selected_col  = 0x330284C7; // Blue 20%
    Color chip_bg_color      = 0xFF1E293B; // Slate 800
    Color chip_border_color  = 0xFF38BDF8; // Sky 400

    // Coordinates for floating overlay dropdown
    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    // Callbacks
    std::function<void(const ComboBoxItem& item)> on_selected;
    std::function<void(const std::vector<ComboBoxItem>& items)> on_multi_changed;
    std::function<void(const std::string& custom_val)> on_custom_value;
};

// ════════════════════════════════════════════════════════════════
// ComboBox Implementation Widget
// ════════════════════════════════════════════════════════════════

class ComboBoxWidget : public StatefulWidget {
public:
    ComboBoxProps props;

    ComboBoxWidget() = default;
    explicit ComboBoxWidget(ComboBoxProps p)
        : StatefulWidget(p.key), props(std::move(p)) {}
    ComboBoxWidget(Key k, ComboBoxProps p)
        : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ComboBox"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative ComboBox Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct ComboBox {
    Key key = Key::none();
    std::vector<ComboBoxItem> items;
    WidgetPtr body = nullptr;
    std::shared_ptr<ComboBoxController> controller = nullptr;

    ComboBoxMode mode = ComboBoxMode::Single;
    std::string placeholder = "Search or select option...";

    float width = 320.0f;
    float input_height = 42.0f;
    float max_menu_height = 260.0f;
    float border_radius = 8.0f;

    bool allow_clear = true;
    bool allow_custom_value = false;
    bool show_search_icon = true;

    Color background_color   = 0xFF0F172A;
    Color border_color       = 0xFF334155;
    Color border_focus_color = 0xFF0284C7;
    Color text_color         = 0xFFFFFFFF;
    Color placeholder_color  = 0xFF64748B;
    Color menu_bg_color      = 0xFF1E293B;
    Color item_hover_color   = 0x3338BDF8;
    Color item_selected_col  = 0x330284C7;
    Color chip_bg_color      = 0xFF1E293B;
    Color chip_border_color  = 0xFF38BDF8;

    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    std::function<void(const ComboBoxItem& item)> on_selected = nullptr;
    std::function<void(const std::vector<ComboBoxItem>& items)> on_multi_changed = nullptr;
    std::function<void(const std::string& custom_val)> on_custom_value = nullptr;

    operator WidgetPtr() const {
        ComboBoxProps p;
        p.key = key;
        p.items = items;
        p.body = body;
        p.controller = controller;
        p.mode = mode;
        p.placeholder = placeholder;
        p.width = width;
        p.input_height = input_height;
        p.max_menu_height = max_menu_height;
        p.border_radius = border_radius;
        p.allow_clear = allow_clear;
        p.allow_custom_value = allow_custom_value;
        p.show_search_icon = show_search_icon;
        p.background_color = background_color;
        p.border_color = border_color;
        p.border_focus_color = border_focus_color;
        p.text_color = text_color;
        p.placeholder_color = placeholder_color;
        p.menu_bg_color = menu_bg_color;
        p.item_hover_color = item_hover_color;
        p.item_selected_col = item_selected_col;
        p.chip_bg_color = chip_bg_color;
        p.chip_border_color = chip_border_color;
        p.anchor_x = anchor_x;
        p.anchor_y = anchor_y;
        p.on_selected = on_selected;
        p.on_multi_changed = on_multi_changed;
        p.on_custom_value = on_custom_value;
        return std::make_shared<ComboBoxWidget>(key, std::move(p));
    }
};

} // namespace enki
