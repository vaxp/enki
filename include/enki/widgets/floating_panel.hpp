#pragma once
/// @file floating_panel.hpp
/// @brief Advanced Draggable & Resizable Floating Window overlay widget for ENKI Framework (Category 19. Overlay & Popup Extended).
///
/// Features:
///   - Freeform floating tool window / inspector HUD rendered above the main widget tree
///   - Smooth drag-to-move by title bar with viewport boundary clamping
///   - 8-direction multi-edge and corner resize handles (N, S, E, W, NW, NE, SW, SE) with cursor feedback
///   - Window management: Minimize (collapse to title bar), Maximize (fill viewport), and Close
///   - Active vs Inactive focus states with glow elevation & border highlights
///   - Magnetic edge snapping to window borders
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
#include <optional>

namespace enki {

/// Window state for FloatingPanel
enum class FloatingPanelDisplayState {
    Normal,     ///< Standard floating window
    Minimized,  ///< Collapsed to title bar only
    Maximized   ///< Maximized to fill viewport
};

// ════════════════════════════════════════════════════════════════
// FloatingPanel Options
// ════════════════════════════════════════════════════════════════

struct FloatingPanelOptions {
    std::string title = "Tool Inspector";
    std::string icon = "🎛️";

    float initial_x = 220.0f;
    float initial_y = 100.0f;
    float initial_width = 460.0f;
    float initial_height = 340.0f;

    float min_width = 240.0f;
    float min_height = 140.0f;
    float max_width = 1200.0f;
    float max_height = 900.0f;

    bool allow_drag = true;
    bool allow_resize = true;
    bool allow_minimize = true;
    bool allow_maximize = true;
    bool allow_close = true;

    bool snap_to_edges = true;
    float snap_threshold = 16.0f;

    float border_radius = 12.0f;
    float resize_handle_thickness = 8.0f;

    // Theme Colors
    Color background_color    = 0xF80F172A; ///< Slate 900
    Color border_color        = 0xFF334155; ///< Slate 700
    Color active_border_color = 0xFF38BDF8; ///< Sky 400 active highlight
    Color titlebar_bg_color   = 0xFF0B0F19; ///< Slate 950 header
    Color title_color         = 0xFFF8FAFC; ///< Slate 50
    Color subtitle_color      = 0xFF94A3B8; ///< Slate 400

    // Callbacks
    std::function<void(float x, float y)> on_moved;
    std::function<void(float w, float h)> on_resized;
    std::function<void(float x, float y)> on_drag_update;
    std::function<void(float w, float h)> on_resize_update;
    std::function<void(FloatingPanelDisplayState state)> on_state_changed;
    std::function<void()> on_closed;
};

// ════════════════════════════════════════════════════════════════
// FloatingPanel Controller
// ════════════════════════════════════════════════════════════════

class FloatingPanelController {
public:
    std::function<void()> show_fn;
    std::function<void()> hide_fn;
    std::function<void()> toggle_fn;
    std::function<bool()> is_open_fn;
    std::function<void(float, float)> set_position_fn;
    std::function<Point()> get_position_fn;
    std::function<void(float, float)> set_size_fn;
    std::function<Size()> get_size_fn;
    std::function<void(FloatingPanelDisplayState)> set_state_fn;
    std::function<FloatingPanelDisplayState()> get_state_fn;
    std::function<void()> bring_to_front_fn;

    void show()   { if (show_fn) show_fn(); }
    void hide()   { if (hide_fn) hide_fn(); }
    void toggle() { if (toggle_fn) toggle_fn(); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }

    void setPosition(float x, float y) { if (set_position_fn) set_position_fn(x, y); }
    [[nodiscard]] Point getPosition() const { return get_position_fn ? get_position_fn() : Point{0, 0}; }

    void setSize(float w, float h) { if (set_size_fn) set_size_fn(w, h); }
    [[nodiscard]] Size getSize() const { return get_size_fn ? get_size_fn() : Size{400, 300}; }

    void minimize() { if (set_state_fn) set_state_fn(FloatingPanelDisplayState::Minimized); }
    void maximize() { if (set_state_fn) set_state_fn(FloatingPanelDisplayState::Maximized); }
    void restore()  { if (set_state_fn) set_state_fn(FloatingPanelDisplayState::Normal); }
    [[nodiscard]] FloatingPanelDisplayState getState() const {
        return get_state_fn ? get_state_fn() : FloatingPanelDisplayState::Normal;
    }

    void bringToFront() { if (bring_to_front_fn) bring_to_front_fn(); }
};

// ════════════════════════════════════════════════════════════════
// FloatingPanel Widget
// ════════════════════════════════════════════════════════════════

class FloatingPanelWidget : public StatefulWidget {
public:
    WidgetPtr content;                           ///< Inside content of the floating window
    WidgetPtr body;                              ///< Main page body content wrapped in stack overlay
    FloatingPanelOptions options;
    std::shared_ptr<FloatingPanelController> controller;
    bool initial_open = true;

    explicit FloatingPanelWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FloatingPanel"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct FloatingPanelProps {
    Key key = Key::none();
    WidgetPtr content = nullptr;
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;                   ///< Alias for body
    FloatingPanelOptions options = {};
    std::shared_ptr<FloatingPanelController> controller = nullptr;
    bool initial_open = true;
};

struct FloatingPanel {
    Key key = Key::none();
    WidgetPtr content = nullptr;
    WidgetPtr body = nullptr;
    WidgetPtr child = nullptr;
    FloatingPanelOptions options = {};
    std::shared_ptr<FloatingPanelController> controller = nullptr;
    bool initial_open = true;

    operator WidgetPtr() const {
        auto w = std::make_shared<FloatingPanelWidget>(key);
        w->content = content;
        w->body = body ? body : child;
        w->options = options;
        w->controller = controller;
        w->initial_open = initial_open;
        return w;
    }
};

inline WidgetPtr floatingPanel(const FloatingPanelProps& props) {
    auto w = std::make_shared<FloatingPanelWidget>(props.key);
    w->content = props.content;
    w->body = props.body ? props.body : props.child;
    w->options = props.options;
    w->controller = props.controller;
    w->initial_open = props.initial_open;
    return w;
}

} // namespace enki
