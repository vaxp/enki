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

    WidgetPtr content;                ///< Arbitrary child widget revealed on expansion
    bool is_initially_expanded = false;
    bool is_disabled = false;

    AccordionItem() = default;
    AccordionItem(std::string id_, std::string title_, WidgetPtr content_,
                  std::string icon_ = "", std::string subtitle_ = "",
                  bool initially_expanded = false)
        : id(std::move(id_)), title(std::move(title_)), subtitle(std::move(subtitle_)),
          icon(std::move(icon_)), content(std::move(content_)),
          is_initially_expanded(initially_expanded) {}

    AccordionItem& setBadge(std::string label, Color bg = 0x2E38BDF8, Color fg = 0xFF38BDF8) {
        badge_label = std::move(label);
        badge_bg = bg;
        badge_fg = fg;
        return *this;
    }

    AccordionItem& setDisabled(bool d) {
        is_disabled = d;
        return *this;
    }

    AccordionItem& setInitiallyExpanded(bool exp) {
        is_initially_expanded = exp;
        return *this;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Accordion Options
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
/// Accordion Widget
/// ════════════════════════════════════════════════════════════════

class Accordion : public StatefulWidget {
public:
    AccordionProps props;

    Accordion() = default;
    explicit Accordion(AccordionProps p) : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Accordion"; }
};

inline std::shared_ptr<Accordion> accordion(AccordionProps props = {}) {
    return std::make_shared<Accordion>(std::move(props));
}

inline std::shared_ptr<Accordion> accordion(
    std::vector<AccordionItem> items) {
    AccordionProps props;
    props.items = std::move(items);
    return std::make_shared<Accordion>(std::move(props));
}

} // namespace enki
