#pragma once
/// @file timeline.hpp
/// @brief Advanced Timeline widget for ENKI Framework.
/// Supports vertical & horizontal orientations, alternate zig-zag alignments,
/// multi-state status nodes (completed, active, pending, failed), dashed/gradient connectors,
/// expandable cards, stepper mode, and Skia rendering.

#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include "enki/core/types.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>
#include <algorithm>

namespace enki {

/// Layout orientation
enum class TimelineOrientation {
    Vertical,
    Horizontal
};

/// Alignment / placement mode
enum class TimelineAlignment {
    Start,       ///< Line on left/top, content on right/bottom
    End,         ///< Line on right/bottom, content on left/top
    Alternate,   ///< Zig-zag alternating sides (even on left, odd on right)
    Center       ///< Centered track with timestamps opposite to cards
};

/// Item status / progress state
enum class TimelineItemStatus {
    Completed,   ///< Emerald checkmark ✓
    Active,      ///< Sky blue pulsing/glowing circle
    Pending,     ///< Slate hollow circle
    Warning,     ///< Amber warning ⚠️
    Failed       ///< Red cross ✕
};

/// Node visual shape
enum class TimelineNodeShape {
    Circle,
    Square,
    Icon,
    Number,
    Custom
};

/// Connector line style
enum class TimelineLineStyle {
    Solid,
    Dashed,
    Dotted,
    Gradient
};

/// ════════════════════════════════════════════════════════════════
/// Timeline Item Descriptor
/// ════════════════════════════════════════════════════════════════

struct TimelineItem {
    std::string id = "";
    std::string title = "";
    std::string timestamp = "";
    std::string description = "";
    std::string details = "";           ///< Expandable changelog or metadata
    std::string icon = "";              ///< Optional emoji or glyph
    std::string badge_text = "";        ///< Optional badge (e.g. "v1.0", "PROD")
    Color badge_bg = 0;
    Color badge_fg = 0;

    TimelineItemStatus status = TimelineItemStatus::Pending;
    TimelineNodeShape node_shape = TimelineNodeShape::Circle;
    Color custom_node_color = 0;

    bool is_expanded = false;
    bool is_clickable = true;

    TimelineItem() = default;
    TimelineItem(std::string item_id, std::string t, std::string time, std::string desc = "",
                 TimelineItemStatus st = TimelineItemStatus::Pending)
        : id(std::move(item_id)), title(std::move(t)), timestamp(std::move(time)),
          description(std::move(desc)), status(st) {}

    TimelineItem& setBadge(std::string text, Color bg = 0x2E38BDF8, Color fg = 0xFFFFFFFF) {
        badge_text = std::move(text);
        badge_bg = bg;
        badge_fg = fg;
        return *this;
    }

    TimelineItem& setDetails(std::string d) {
        details = std::move(d);
        return *this;
    }

    TimelineItem& setIcon(std::string ic) {
        icon = std::move(ic);
        node_shape = TimelineNodeShape::Icon;
        return *this;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Timeline Controller
/// ════════════════════════════════════════════════════════════════

class TimelineController {
private:
    std::vector<TimelineItem> items_;
    int active_step_index_ = 0;

public:
    TimelineController() = default;
    TimelineController(std::vector<TimelineItem> items, int active_step = 0)
        : items_(std::move(items)), active_step_index_(active_step) {}

    [[nodiscard]] const std::vector<TimelineItem>& getItems() const { return items_; }
    std::vector<TimelineItem>& getItemsMut() { return items_; }
    void setItems(std::vector<TimelineItem> items) { items_ = std::move(items); }

    void addItem(TimelineItem item) { items_.push_back(std::move(item)); }
    void removeItem(const std::string& item_id) {
        items_.erase(std::remove_if(items_.begin(), items_.end(),
            [&](const TimelineItem& it) { return it.id == item_id; }), items_.end());
    }

    [[nodiscard]] int getActiveStep() const { return active_step_index_; }
    void setActiveStep(int index) {
        if (index >= 0 && index < static_cast<int>(items_.size())) {
            active_step_index_ = index;
            for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
                if (i < active_step_index_) items_[i].status = TimelineItemStatus::Completed;
                else if (i == active_step_index_) items_[i].status = TimelineItemStatus::Active;
                else items_[i].status = TimelineItemStatus::Pending;
            }
        }
    }
    void nextStep() { setActiveStep(active_step_index_ + 1); }
    void prevStep() { setActiveStep(active_step_index_ - 1); }

    void toggleExpand(const std::string& item_id) {
        for (auto& item : items_) {
            if (item.id == item_id) {
                item.is_expanded = !item.is_expanded;
                break;
            }
        }
    }
};

/// ════════════════════════════════════════════════════════════════
/// Configuration Options for Timeline
/// ════════════════════════════════════════════════════════════════

struct TimelineProps {
    Key key = Key::none();
    std::shared_ptr<TimelineController> controller;

    TimelineOrientation orientation = TimelineOrientation::Vertical;
    TimelineAlignment alignment = TimelineAlignment::Start;
    TimelineLineStyle line_style = TimelineLineStyle::Solid;

    float node_size = 24.0f;
    float line_thickness = 2.0f;
    float item_spacing = 20.0f;
    float card_width = 380.0f;

    // Styling Colors
    Color completed_color  = 0xFF10B981; // Emerald 500
    Color active_color     = 0xFF38BDF8; // Sky 400
    Color pending_color    = 0xFF475569; // Slate 600
    Color warning_color    = 0xFFF59E0B; // Amber 500
    Color failed_color     = 0xFFEF4444; // Red 500
    Color line_color       = 0xFF334155; // Slate 700
    Color card_bg_color    = 0xFF1E293B; // Slate 800
    Color card_border_color= 0xFF334155; // Slate 700
    Color title_color      = 0xFFF8FAFC; // Slate 50
    Color timestamp_color  = 0xFF94A3B8; // Slate 400
    Color desc_color       = 0xFFCBD5E1; // Slate 300
    Color details_bg_color = 0xFF0F172A; // Slate 900

    float card_border_radius = 8.0f;
    bool is_stepper = false;

    // Callbacks
    std::function<void(const TimelineItem& item)> on_item_tap;
    std::function<void(int step_index)> on_step_changed;
    std::function<void(const std::string& item_id, bool is_expanded)> on_item_expanded;
};

/// ════════════════════════════════════════════════════════════════
/// Timeline Widget Implementation
/// ════════════════════════════════════════════════════════════════

class TimelineWidget : public StatefulWidget {
public:
    TimelineProps props;

    TimelineWidget() = default;
    explicit TimelineWidget(TimelineProps p) : props(std::move(p)) {
        if (!props.controller) {
            props.controller = std::make_shared<TimelineController>();
        }
    }
    TimelineWidget(Key key, TimelineProps p) : StatefulWidget(std::move(key)), props(std::move(p)) {
        if (!props.controller) {
            props.controller = std::make_shared<TimelineController>();
        }
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Timeline"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Timeline {
    Key key = Key::none();
    std::shared_ptr<TimelineController> controller = nullptr;

    TimelineOrientation orientation = TimelineOrientation::Vertical;
    TimelineAlignment alignment = TimelineAlignment::Start;
    TimelineLineStyle line_style = TimelineLineStyle::Solid;

    float node_size = 24.0f;
    float line_thickness = 2.0f;
    float item_spacing = 20.0f;
    float card_width = 380.0f;

    Color completed_color  = 0xFF10B981;
    Color active_color     = 0xFF38BDF8;
    Color pending_color    = 0xFF475569;
    Color warning_color    = 0xFFF59E0B;
    Color failed_color     = 0xFFEF4444;
    Color line_color       = 0xFF334155;
    Color card_bg_color    = 0xFF1E293B;
    Color card_border_color= 0xFF334155;
    Color title_color      = 0xFFF8FAFC;
    Color timestamp_color  = 0xFF94A3B8;
    Color desc_color       = 0xFFCBD5E1;
    Color details_bg_color = 0xFF0F172A;

    float card_border_radius = 8.0f;
    bool is_stepper = false;

    std::function<void(const TimelineItem& item)> on_item_tap = nullptr;
    std::function<void(int step_index)> on_step_changed = nullptr;
    std::function<void(const std::string& item_id, bool is_expanded)> on_item_expanded = nullptr;

    operator WidgetPtr() const {
        TimelineProps p;
        p.key = key;
        p.controller = controller;
        p.orientation = orientation;
        p.alignment = alignment;
        p.line_style = line_style;
        p.node_size = node_size;
        p.line_thickness = line_thickness;
        p.item_spacing = item_spacing;
        p.card_width = card_width;
        p.completed_color = completed_color;
        p.active_color = active_color;
        p.pending_color = pending_color;
        p.warning_color = warning_color;
        p.failed_color = failed_color;
        p.line_color = line_color;
        p.card_bg_color = card_bg_color;
        p.card_border_color = card_border_color;
        p.title_color = title_color;
        p.timestamp_color = timestamp_color;
        p.desc_color = desc_color;
        p.details_bg_color = details_bg_color;
        p.card_border_radius = card_border_radius;
        p.is_stepper = is_stepper;
        p.on_item_tap = on_item_tap;
        p.on_step_changed = on_step_changed;
        p.on_item_expanded = on_item_expanded;

        return std::make_shared<TimelineWidget>(key, std::move(p));
    }
};

} // namespace enki
