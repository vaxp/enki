#pragma once
/// @file spotlight.hpp
/// @brief Advanced Spotlight & Interactive Feature Tour overlay widget for ENKI Framework (Category 19. Overlay & Popup Extended).
///
/// Features:
///   - Full-screen dimmed overlay with inverse-cutout spotlight focused on target widgets
///   - Multiple cutout shapes: RoundedRectangle, Circle/Oval, Rectangle with configurable padding & radius
///   - Animated beacon / pulse ring around the spotlight cutout to guide user focus
///   - Floating tour popover card auto-positioned (Top, Bottom, Left, Right, Auto) with viewport boundary clamping
///   - Multi-step tour engine (steps sequence, step indicators, Next, Back, Skip, Finish)
///   - Configurable target interaction (pass-through clicks to spotlighted widget)
///   - C++20 Declarative API with designated initializers & imperative controller
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
#include <optional>

namespace enki {

/// Cutout hole shape for the spotlighted element
enum class SpotlightShape {
    RoundedRectangle, ///< Rounded rect with corner radius (Default)
    Circle,           ///< Perfect circle or oval centered on target
    Rectangle         ///< Sharp corner rectangle
};

/// Placement of the tour popover card relative to the spotlight target
enum class SpotlightPlacement {
    Auto,             ///< Auto-detect based on available viewport space (Default)
    Top,              ///< Position above target
    Bottom,           ///< Position below target
    Left,             ///< Position to the left of target
    Right             ///< Position to the right of target
};

// ════════════════════════════════════════════════════════════════
// Spotlight Step Definition
// ════════════════════════════════════════════════════════════════

struct SpotlightStep {
    std::string id = "";
    std::string title = "";
    std::string description = "";

    Rect target_bounds = Rect{0.0f, 0.0f, 0.0f, 0.0f}; ///< Global viewport coordinates of target
    Key target_key = Key::none();                       ///< Optional widget key to track target dynamically

    SpotlightShape shape = SpotlightShape::RoundedRectangle;
    float corner_radius = 10.0f;
    EdgeInsets padding = EdgeInsets::all(8.0f);        ///< Breathing room around target bounds

    SpotlightPlacement placement = SpotlightPlacement::Auto;
    bool allow_target_click = true;                    ///< Pass pointer clicks through hole to target widget
    bool show_pulse_ring = true;                       ///< Render animated halo / beacon around cutout

    std::string next_button_label = "";                ///< Defaults to "Next" or "Finish"
    std::string back_button_label = "Back";
    std::string skip_button_label = "Skip Tour";

    SpotlightStep() = default;

    SpotlightStep(std::string t, std::string desc, Rect bounds,
                  SpotlightShape s = SpotlightShape::RoundedRectangle,
                  SpotlightPlacement p = SpotlightPlacement::Auto)
        : title(std::move(t)), description(std::move(desc)), target_bounds(bounds),
          shape(s), placement(p) {}
};

// ════════════════════════════════════════════════════════════════
// Spotlight Options
// ════════════════════════════════════════════════════════════════

struct SpotlightOptions {
    Color overlay_color       = 0xCC080C14; ///< 80% deep obsidian dark scrim
    Color pulse_ring_color    = 0xFF38BDF8; ///< Sky 400 beacon halo
    Color card_bg_color       = 0xF80F172A; ///< Slate 900 popover card
    Color card_border_color   = 0xFF334155; ///< Slate 700 card border
    Color title_color         = 0xFFF8FAFC; ///< Slate 50
    Color description_color   = 0xFF94A3B8; ///< Slate 400
    Color step_badge_color    = 0xFF38BDF8; ///< Sky 400 accent badge

    float card_width          = 340.0f;     ///< Popover card width
    float card_border_radius   = 12.0f;
    float popover_distance    = 14.0f;      ///< Gap between cutout hole and popover card

    bool dismiss_on_scrim_tap = false;      ///< Advance or close when clicking dark backdrop
    bool show_step_indicator  = true;       ///< Show "Step X of Y" dots or counter
    bool show_skip_button     = true;

    // Callbacks
    std::function<void(size_t index, const SpotlightStep&)> on_step_change;
    std::function<void()> on_finish;
    std::function<void()> on_skip;
};

// ════════════════════════════════════════════════════════════════
// Spotlight Tour Controller
// ════════════════════════════════════════════════════════════════

class SpotlightTourController {
public:
    std::function<void()> start_fn;
    std::function<void()> next_fn;
    std::function<void()> previous_fn;
    std::function<void()> skip_fn;
    std::function<void()> finish_fn;
    std::function<void(size_t)> go_to_step_fn;
    std::function<size_t()> get_step_index_fn;
    std::function<size_t()> get_total_steps_fn;
    std::function<bool()> is_active_fn;
    std::function<void(Rect)> update_target_rect_fn;
    std::function<void(std::vector<SpotlightStep>)> set_steps_fn;

    void start() { if (start_fn) start_fn(); }
    void next() { if (next_fn) next_fn(); }
    void previous() { if (previous_fn) previous_fn(); }
    void skip() { if (skip_fn) skip_fn(); }
    void finish() { if (finish_fn) finish_fn(); }
    void goToStep(size_t idx) { if (go_to_step_fn) go_to_step_fn(idx); }

    [[nodiscard]] size_t getCurrentStepIndex() const { return get_step_index_fn ? get_step_index_fn() : 0; }
    [[nodiscard]] size_t getTotalSteps() const { return get_total_steps_fn ? get_total_steps_fn() : 0; }
    [[nodiscard]] bool isActive() const { return is_active_fn ? is_active_fn() : false; }

    void updateTargetRect(Rect rect) { if (update_target_rect_fn) update_target_rect_fn(rect); }
    void setSteps(std::vector<SpotlightStep> steps) { if (set_steps_fn) set_steps_fn(std::move(steps)); }
};

// ════════════════════════════════════════════════════════════════
// Spotlight Widget
// ════════════════════════════════════════════════════════════════

class SpotlightWidget : public StatefulWidget {
public:
    WidgetPtr body;                              ///< Underlying page wrapped in stack overlay
    std::vector<SpotlightStep> steps;            ///< Tour sequence steps
    SpotlightOptions options;
    std::shared_ptr<SpotlightTourController> controller;
    bool initial_active = false;

    explicit SpotlightWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Spotlight"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct SpotlightProps {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;                   ///< Alias for body
    std::vector<SpotlightStep> steps = {};
    SpotlightOptions options = {};
    std::shared_ptr<SpotlightTourController> controller = nullptr;
    bool initial_active = false;
};

struct Spotlight {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;
    std::vector<SpotlightStep> steps = {};
    SpotlightOptions options = {};
    std::shared_ptr<SpotlightTourController> controller = nullptr;
    bool initial_active = false;

    operator WidgetPtr() const {
        auto w = std::make_shared<SpotlightWidget>(key);
        w->body = body ? body : child;
        w->steps = steps;
        w->options = options;
        w->controller = controller;
        w->initial_active = initial_active;
        return w;
    }
};

inline WidgetPtr spotlight(const SpotlightProps& props) {
    auto w = std::make_shared<SpotlightWidget>(props.key);
    w->body = props.body ? props.body : props.child;
    w->steps = props.steps;
    w->options = props.options;
    w->controller = props.controller;
    w->initial_active = props.initial_active;
    return w;
}

} // namespace enki
