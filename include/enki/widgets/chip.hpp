#pragma once
/// @file chip.hpp
/// @brief Advanced Chip & ChipGroup widgets for ENKI Framework (Category 2. Basic UI).
/// Supports Action, Filter, Choice, Input (Deletable), and Status chips with leading icons/avatars,
/// trailing delete/counter widgets, multiple sizes, and Choice/Filter ChipGroup orchestrators.
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

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// Chip Enums & Options
/// ════════════════════════════════════════════════════════════════

enum class ChipType {
    Action,     ///< Action trigger chip
    Filter,     ///< Multi-selection filter chip with checkmark
    Choice,     ///< Mutually exclusive single-choice chip
    Input,      ///< Deletable tag with 'X' button
    Status      ///< Status badge with live indicator dot
};

enum class ChipVariant {
    Filled,     ///< Solid background
    Outlined,   ///< Transparent background with border
    Elevated    ///< Shadowed / glowing background
};

enum class ChipSize {
    Small,      ///< 24px height, compact font
    Medium,     ///< 32px height, standard font
    Large       ///< 40px height, prominent font
};

struct ChipProps {
    Key key = Key::none();
    ChipType type = ChipType::Action;
    ChipVariant variant = ChipVariant::Filled;
    ChipSize size = ChipSize::Medium;

    std::string label;
    std::string avatar_icon;            ///< Leading emoji/icon (e.g. "⚡", "👤")
    WidgetPtr leading;                  ///< Custom leading widget
    WidgetPtr trailing;                 ///< Custom trailing widget

    bool selected = false;              ///< For Filter and Choice chips
    bool enabled  = true;               ///< Interactivity flag
    bool deletable = false;             ///< Shows trailing 'X' delete button
    bool pulsing_dot = false;           ///< Shows animated/pulsing status dot

    // Styling Colors
    Color background_color = 0xFF1E293B;  // Slate 800
    Color selected_color   = 0xFF0284C7;  // Blue 600
    Color border_color     = 0xFF334155;  // Slate 700
    Color text_color       = 0xFFFFFFFF;
    Color status_color     = 0xFF10B981;  // Emerald 500

    // Callbacks
    std::function<void()> on_tap;
    std::function<void(bool selected)> on_selected;
    std::function<void()> on_deleted;
};

/// ════════════════════════════════════════════════════════════════
/// Chip Widget
/// ════════════════════════════════════════════════════════════════

class Chip : public StatefulWidget {
public:
    ChipProps options;

    Chip() = default;
    explicit Chip(ChipProps opts) : options(std::move(opts)) {}
    
    Chip(Key k, ChipProps opt) : StatefulWidget(std::move(k)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Chip"; }
};

/// ════════════════════════════════════════════════════════════════
/// ChipGroup Widget
/// ════════════════════════════════════════════════════════════════

struct ChipGroupProps {
    Key key = Key::none();
    bool single_choice = false;         ///< If true, acts as ChoiceGroup; otherwise FilterGroup
    float gap = 8.0f;
    std::function<void(int index)> on_choice_changed;
};

class ChipGroup : public StatefulWidget {
public:
    std::vector<std::shared_ptr<Chip>> chips;
    ChipGroupProps options;

    ChipGroup(std::vector<std::shared_ptr<Chip>> chips_, ChipGroupProps opts = {})
        : chips(std::move(chips_)), options(std::move(opts)) {}
    
    ChipGroup(Key k, std::vector<std::shared_ptr<Chip>> chips_, ChipGroupProps opt) : StatefulWidget(std::move(k)), chips(std::move(chips_)), options(std::move(opt)) {}
    
    ChipGroup(ChipGroupProps props) : StatefulWidget(std::move(props.key)), options(std::move(props)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ChipGroup"; }
};

/// ════════════════════════════════════════════════════════════════
/// Convenience Factory Helpers
/// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Chip> chip(ChipProps options = {}) {
    return std::make_shared<Chip>(std::move(options.key), std::move(options));
}

inline std::shared_ptr<Chip> actionChip(std::string label, std::function<void()> on_tap,
                                        std::string icon = "") {
    ChipProps opts;
    opts.type = ChipType::Action;
    opts.label = std::move(label);
    opts.avatar_icon = std::move(icon);
    opts.on_tap = std::move(on_tap);
    return std::make_shared<Chip>(opts);
}

inline std::shared_ptr<Chip> filterChip(std::string label, bool selected,
                                        std::function<void(bool)> on_selected,
                                        std::string icon = "") {
    ChipProps opts;
    opts.type = ChipType::Filter;
    opts.label = std::move(label);
    opts.selected = selected;
    opts.avatar_icon = std::move(icon);
    opts.on_selected = std::move(on_selected);
    return std::make_shared<Chip>(opts);
}

inline std::shared_ptr<Chip> choiceChip(std::string label, bool selected,
                                        std::function<void(bool)> on_selected) {
    ChipProps opts;
    opts.type = ChipType::Choice;
    opts.label = std::move(label);
    opts.selected = selected;
    opts.on_selected = std::move(on_selected);
    return std::make_shared<Chip>(opts);
}

inline std::shared_ptr<Chip> inputChip(std::string label, std::function<void()> on_deleted,
                                       std::string avatar = "") {
    ChipProps opts;
    opts.type = ChipType::Input;
    opts.label = std::move(label);
    opts.avatar_icon = std::move(avatar);
    opts.deletable = true;
    opts.on_deleted = std::move(on_deleted);
    return std::make_shared<Chip>(opts);
}

inline std::shared_ptr<Chip> statusChip(std::string label, Color status_color = 0xFF10B981,
                                        bool pulsing = true) {
    ChipProps opts;
    opts.type = ChipType::Status;
    opts.label = std::move(label);
    opts.status_color = status_color;
    opts.pulsing_dot = pulsing;
    return std::make_shared<Chip>(opts);
}

inline std::shared_ptr<ChipGroup> chipGroup(std::vector<std::shared_ptr<Chip>> chips,
                                            ChipGroupProps options = {}) {
    return std::make_shared<ChipGroup>(std::move(chips), std::move(options));
}

inline std::shared_ptr<ChipGroup> chipGroup(ChipGroupProps props) {
    return std::make_shared<ChipGroup>(std::move(props));
}

} // namespace enki
