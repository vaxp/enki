#pragma once
/// @file accordion.hpp
/// @brief Advanced Accordion widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports Single/Multiple expansion modes, 3 visual variants (Bordered, Separated, Flush),
/// rotating chevron indicators, header badges, disabled states, and AccordionController.
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

/// Accordion expansion mode
enum class AccordionMode {
    Single,     ///< Only one section open at a time (auto-collapses others)
    Multiple    ///< Multiple sections can remain open simultaneously
};

/// Accordion visual styling variant
enum class AccordionVariant {
    Bordered,   ///< Unified rounded container with internal section dividers
    Separated,  ///< Independent floating cards with spacing between items
    Flush       ///< Minimal transparent background with simple line dividers
};

/// ════════════════════════════════════════════════════════════════
/// Accordion Item Model
/// ════════════════════════════════════════════════════════════════

struct AccordionItem {
    std::string id = "";
    std::string title = "";
    std::string subtitle = "";
    std::string icon = "";            ///< Leading emoji/icon (e.g. ⚙️, 🔒, 📦, 🎨)
    std::string badge_label = "";     ///< Header badge tag (e.g. [PRO], [Active])
    Color badge_bg = 0x2E38BDF8;      ///< Badge background color
    Color badge_fg = 0xFF38BDF8;      ///< Badge text color

    WidgetPtr content = nullptr;      ///< Arbitrary child widget revealed on expansion
    bool is_initially_expanded = false;
    bool is_disabled = false;
};

/// ════════════════════════════════════════════════════════════════
/// Accordion Options & Props
/// ════════════════════════════════════════════════════════════════

class AccordionController;

struct AccordionProps {
    Key key = Key::none();
    std::vector<AccordionItem> items;
    std::shared_ptr<AccordionController> controller;

    AccordionMode mode = AccordionMode::Single;
    AccordionVariant variant = AccordionVariant::Bordered;

    bool collapsible = true;          ///< In Single mode, allows closing the active section
    bool show_chevron = true;         ///< Show rotating ⌃ / ⌄ indicator

    float gap = 10.0f;                ///< Gap between sections (used in Separated variant)
    float border_radius = 10.0f;

    // Styling Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color header_hover_bg  = 0x33334155; // Slate 700 with opacity
    Color title_color      = 0xFFFFFFFF; // White
    Color subtitle_color   = 0xFF94A3B8; // Slate 400
    Color chevron_color    = 0xFF94A3B8; // Slate 400
    Color divider_color    = 0xFF334155; // Slate 700

    // Callbacks
    std::function<void(const std::string& id, bool is_expanded)> on_toggle;
    std::function<void(const std::set<std::string>& active_ids)> on_change;
};

/// ════════════════════════════════════════════════════════════════
/// Accordion Controller
/// ════════════════════════════════════════════════════════════════

class AccordionController {
public:
    std::function<void(const std::string&)> expand_fn;
    std::function<void(const std::string&)> collapse_fn;
    std::function<void(const std::string&)> toggle_fn;
    std::function<void()> expand_all_fn;
    std::function<void()> collapse_all_fn;
    std::function<bool(const std::string&)> is_expanded_fn;
    std::function<std::set<std::string>()> get_expanded_ids_fn;

    void expand(const std::string& id) { if (expand_fn) expand_fn(id); }
    void collapse(const std::string& id) { if (collapse_fn) collapse_fn(id); }
    void toggle(const std::string& id) { if (toggle_fn) toggle_fn(id); }
    void expandAll() { if (expand_all_fn) expand_all_fn(); }
    void collapseAll() { if (collapse_all_fn) collapse_all_fn(); }
    [[nodiscard]] bool isExpanded(const std::string& id) const { return is_expanded_fn ? is_expanded_fn(id) : false; }
    [[nodiscard]] std::set<std::string> getExpandedIds() const { return get_expanded_ids_fn ? get_expanded_ids_fn() : std::set<std::string>{}; }
};

/// ════════════════════════════════════════════════════════════════
/// Accordion Implementation Widget
/// ════════════════════════════════════════════════════════════════

class AccordionWidget : public StatefulWidget {
public:
    AccordionProps props;

    AccordionWidget() = default;
    explicit AccordionWidget(AccordionProps p) : props(std::move(p)) {}
    AccordionWidget(Key key, AccordionProps p) : StatefulWidget(std::move(key)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Accordion"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Accordion Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Accordion {
    Key key = Key::none();
    std::vector<AccordionItem> items;
    std::shared_ptr<AccordionController> controller = nullptr;

    AccordionMode mode = AccordionMode::Single;
    AccordionVariant variant = AccordionVariant::Bordered;

    bool collapsible = true;
    bool show_chevron = true;

    float gap = 10.0f;
    float border_radius = 10.0f;

    // Styling Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color header_hover_bg  = 0x33334155; // Slate 700 with opacity
    Color title_color      = 0xFFFFFFFF; // White
    Color subtitle_color   = 0xFF94A3B8; // Slate 400
    Color chevron_color    = 0xFF94A3B8; // Slate 400
    Color divider_color    = 0xFF334155; // Slate 700

    // Callbacks
    std::function<void(const std::string& id, bool is_expanded)> on_toggle = nullptr;
    std::function<void(const std::set<std::string>& active_ids)> on_change = nullptr;

    operator WidgetPtr() const {
        AccordionProps p;
        p.key = key;
        p.items = items;
        p.controller = controller;
        p.mode = mode;
        p.variant = variant;
        p.collapsible = collapsible;
        p.show_chevron = show_chevron;
        p.gap = gap;
        p.border_radius = border_radius;
        p.background_color = background_color;
        p.border_color = border_color;
        p.header_hover_bg = header_hover_bg;
        p.title_color = title_color;
        p.subtitle_color = subtitle_color;
        p.chevron_color = chevron_color;
        p.divider_color = divider_color;
        p.on_toggle = on_toggle;
        p.on_change = on_change;
        return std::make_shared<AccordionWidget>(key, std::move(p));
    }
};

} // namespace enki
