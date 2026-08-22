#pragma once
/// @file resizable_panel.hpp
/// @brief Advanced ResizablePanel & Floating Tool Window widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports multi-edge/corner drag resizing, drag-to-move title bar, min/max constraints,
/// minimize/maximize toggles, bottom-right grip handles, and ResizablePanelController.
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

/// Window mode for ResizablePanel
enum class ResizablePanelMode {
    Floating,   ///< Freeform floating window with title bar and drag-to-move
    Docked      ///< Docked panel resizing along its boundary edge
};

/// ════════════════════════════════════════════════════════════════
/// ResizablePanel Options
/// ════════════════════════════════════════════════════════════════

struct ResizablePanelOptions {
    ResizablePanelMode mode = ResizablePanelMode::Floating;
    std::string title = "Tool Window";
    std::string icon = "🛠️";

    float initial_x = 240.0f;
    float initial_y = 120.0f;
    float initial_width = 460.0f;
    float initial_height = 320.0f;

    float min_width = 240.0f;
    float min_height = 160.0f;
    float max_width = 1000.0f;
    float max_height = 800.0f;

    bool show_header = true;
    bool show_corner_grip = true;
    bool allow_drag_move = true;
    bool allow_minimize = true;
    bool allow_maximize = true;
    bool allow_close = true;

    float border_radius = 10.0f;
    float handle_thickness = 6.0f;

    // Styling Colors
    Color background_color   = 0xFF1E293B; // Slate 800
    Color border_color       = 0xFF334155; // Slate 700
    Color active_border_col  = 0xFF0284C7; // Blue 600
    Color header_bg_color    = 0xFF0F172A; // Slate 900
    Color title_color        = 0xFFFFFFFF; // White
    Color grip_color         = 0xFF64748B; // Slate 500
    Color grip_hover_color   = 0xFF38BDF8; // Sky 400

    // Callbacks
    std::function<void(float width, float height)> on_resized;
    std::function<void(float x, float y)> on_moved;
    std::function<void(bool is_minimized)> on_minimize_toggled;
    std::function<void(bool is_maximized)> on_maximize_toggled;
    std::function<void()> on_closed;
};

/// ════════════════════════════════════════════════════════════════
/// ResizablePanel Controller
/// ════════════════════════════════════════════════════════════════

class ResizablePanelController {
public:
    std::function<void(float, float)> set_size_fn;
    std::function<void(float, float)> set_position_fn;
    std::function<void(bool)> set_minimized_fn;
    std::function<void(bool)> set_maximized_fn;
    std::function<void()> close_fn;
    std::function<void()> reset_fn;
    std::function<Point()> get_position_fn;
    std::function<Size()> get_size_fn;

    void setSize(float w, float h) { if (set_size_fn) set_size_fn(w, h); }
    void setPosition(float x, float y) { if (set_position_fn) set_position_fn(x, y); }
    void setMinimized(bool min) { if (set_minimized_fn) set_minimized_fn(min); }
    void setMaximized(bool max) { if (set_maximized_fn) set_maximized_fn(max); }
    void close() { if (close_fn) close_fn(); }
    void reset() { if (reset_fn) reset_fn(); }
    [[nodiscard]] Point getPosition() const { return get_position_fn ? get_position_fn() : Point{0, 0}; }
    [[nodiscard]] Size getSize() const { return get_size_fn ? get_size_fn() : Size{400, 300}; }
};

/// ════════════════════════════════════════════════════════════════
/// ResizablePanel Widget Implementation
/// ════════════════════════════════════════════════════════════════

class ResizablePanelWidget : public StatefulWidget {
public:
    WidgetPtr child;                 ///< Content inside the resizable panel
    WidgetPtr body;                  ///< Workspace page body wrapped in stack overlay
    ResizablePanelOptions options;
    std::shared_ptr<ResizablePanelController> controller;

    ResizablePanelWidget() = default;
    ResizablePanelWidget(WidgetPtr child_, WidgetPtr body_, ResizablePanelOptions opts = {},
                         std::shared_ptr<ResizablePanelController> ctrl = nullptr)
        : child(std::move(child_)), body(std::move(body_)),
          options(std::move(opts)), controller(std::move(ctrl)) {}
    ResizablePanelWidget(Key key, WidgetPtr child_, WidgetPtr body_, ResizablePanelOptions opts = {},
                         std::shared_ptr<ResizablePanelController> ctrl = nullptr)
        : StatefulWidget(std::move(key)),
          child(std::move(child_)), body(std::move(body_)),
          options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ResizablePanel"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct ResizablePanel {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    WidgetPtr body = nullptr;
    ResizablePanelOptions options = {};
    std::shared_ptr<ResizablePanelController> controller = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<ResizablePanelWidget>(key, child, body, options, controller);
    }
};

} // namespace enki
