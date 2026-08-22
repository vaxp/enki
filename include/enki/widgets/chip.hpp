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

// ════════════════════════════════════════════════════════════════
// Chip Enums & Options
// ════════════════════════════════════════════════════════════════

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
    WidgetPtr leading = nullptr;         ///< Custom leading widget
    WidgetPtr trailing = nullptr;        ///< Custom trailing widget

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

// ════════════════════════════════════════════════════════════════
// Chip Implementation Widget
// ════════════════════════════════════════════════════════════════

class ChipWidget : public StatefulWidget {
public:
    ChipProps options;

    ChipWidget() = default;
    explicit ChipWidget(ChipProps opts) : StatefulWidget(opts.key), options(std::move(opts)) {}
    ChipWidget(Key k, ChipProps opt) : StatefulWidget(std::move(k)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Chip"; }
};

// ════════════════════════════════════════════════════════════════
// ChipGroup Implementation Widget
// ════════════════════════════════════════════════════════════════

struct ChipGroupProps {
    Key key = Key::none();
    bool single_choice = false;         ///< If true, acts as ChoiceGroup; otherwise FilterGroup
    float gap = 8.0f;
    std::function<void(int index)> on_choice_changed;
};

class ChipGroupWidget : public StatefulWidget {
public:
    std::vector<WidgetPtr> chips;
    ChipGroupProps options;

    ChipGroupWidget(std::vector<WidgetPtr> chips_, ChipGroupProps opts = {})
        : StatefulWidget(opts.key), chips(std::move(chips_)), options(std::move(opts)) {}
    
    ChipGroupWidget(Key k, std::vector<WidgetPtr> chips_, ChipGroupProps opt)
        : StatefulWidget(std::move(k)), chips(std::move(chips_)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ChipGroup"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Chip {
    Key key = Key::none();
    ChipType type = ChipType::Action;
    ChipVariant variant = ChipVariant::Filled;
    ChipSize size = ChipSize::Medium;

    std::string label = "";
    std::string avatar_icon = "";
    WidgetPtr leading = nullptr;
    WidgetPtr trailing = nullptr;

    bool selected = false;
    bool enabled  = true;
    bool deletable = false;
    bool pulsing_dot = false;

    // Styling Colors
    Color background_color = 0xFF1E293B;
    Color selected_color   = 0xFF0284C7;
    Color border_color     = 0xFF334155;
    Color text_color       = 0xFFFFFFFF;
    Color status_color     = 0xFF10B981;

    // Callbacks
    std::function<void()> on_tap = nullptr;
    std::function<void(bool selected)> on_selected = nullptr;
    std::function<void()> on_deleted = nullptr;

    operator WidgetPtr() const {
        ChipProps p;
        p.key = key;
        p.type = type;
        p.variant = variant;
        p.size = size;
        p.label = label;
        p.avatar_icon = avatar_icon;
        p.leading = leading;
        p.trailing = trailing;
        p.selected = selected;
        p.enabled = enabled;
        p.deletable = deletable;
        p.pulsing_dot = pulsing_dot;
        p.background_color = background_color;
        p.selected_color = selected_color;
        p.border_color = border_color;
        p.text_color = text_color;
        p.status_color = status_color;
        p.on_tap = on_tap;
        p.on_selected = on_selected;
        p.on_deleted = on_deleted;
        return std::make_shared<ChipWidget>(key, std::move(p));
    }
};

struct ChipGroup {
    Key key = Key::none();
    std::vector<WidgetPtr> chips;
    bool single_choice = false;
    float gap = 8.0f;
    std::function<void(int index)> on_choice_changed = nullptr;

    operator WidgetPtr() const {
        ChipGroupProps p;
        p.key = key;
        p.single_choice = single_choice;
        p.gap = gap;
        p.on_choice_changed = on_choice_changed;
        return std::make_shared<ChipGroupWidget>(key, chips, std::move(p));
    }
};

} // namespace enki
