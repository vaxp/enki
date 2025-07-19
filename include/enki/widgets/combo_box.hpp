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

/// ════════════════════════════════════════════════════════════════
/// ComboBox Item Model
/// ════════════════════════════════════════════════════════════════

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

/// ════════════════════════════════════════════════════════════════
/// ComboBox Options
/// ════════════════════════════════════════════════════════════════

struct ComboBoxOptions {
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

/// ════════════════════════════════════════════════════════════════
/// ComboBox Controller
/// ════════════════════════════════════════════════════════════════

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

/// ════════════════════════════════════════════════════════════════
/// ComboBox Widget
/// ════════════════════════════════════════════════════════════════

class ComboBox : public StatefulWidget {
public:
    std::vector<ComboBoxItem> items;
    WidgetPtr body;                              ///< Main page body content to wrap in stack overlay
    ComboBoxOptions options;
    std::shared_ptr<ComboBoxController> controller;

    ComboBox() = default;
    ComboBox(std::vector<ComboBoxItem> items_, WidgetPtr body_, ComboBoxOptions opts = {},
             std::shared_ptr<ComboBoxController> ctrl = nullptr)
        : items(std::move(items_)), body(std::move(body_)),
          options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ComboBox"; }
};

inline std::shared_ptr<ComboBox> comboBox(
    std::vector<ComboBoxItem> items,
    WidgetPtr body,
    ComboBoxOptions options = {},
    std::shared_ptr<ComboBoxController> controller = nullptr) {
    return std::make_shared<ComboBox>(std::move(items), std::move(body), std::move(options), std::move(controller));
}

} // namespace enki
