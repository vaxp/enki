#pragma once
/// @file drawer.hpp
/// @brief Drawer — slide-in overlay navigation panel.
///
/// Drawer slides in from the left (or right) OVER the body content with a
/// semi-transparent overlay behind it. Tapping the overlay dismisses it.
/// The animation is driven by AnimationController (slide + fade overlay).
///
/// Features:
///   - Smooth slide-in/out animation with AnimationController.
///   - Scrim overlay (semi-transparent) that dismisses the drawer on tap.
///   - Left or Right placement.
///   - Configurable width, radius, and colors.
///   - Programmatic open/close via DrawerController.
///   - Fluent builder API.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// DrawerSide
// ════════════════════════════════════════════════════════════════

enum class DrawerSide {
    Left,
    Right
};

// ════════════════════════════════════════════════════════════════
// DrawerOptions
// ════════════════════════════════════════════════════════════════

struct DrawerOptions {
    float    width             = 280.0f;
    Color    background_color  = 0xFF1E293B;
    Color    overlay_color     = 0x80000000;  ///< Scrim color
    Color    border_color      = 0xFF334155;
    float    border_radius     = 0.0f;        ///< Far-side corners radius
    float    shadow_blur       = 24.0f;
    Color    shadow_color      = 0x80000000;
    DrawerSide side            = DrawerSide::Left;
    bool     close_on_overlay  = true;

    constexpr bool operator==(const DrawerOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// DrawerController — allows external open/close
// ════════════════════════════════════════════════════════════════

class DrawerController {
public:
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<bool()> is_open_fn;

    void open()  { if (open_fn)  open_fn();  }
    void close() { if (close_fn) close_fn(); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }
};

// ════════════════════════════════════════════════════════════════
// Drawer Widget
// ════════════════════════════════════════════════════════════════

class Drawer : public StatefulWidget {
public:
    WidgetPtr                  drawer_content;  ///< Content inside the drawer panel
    WidgetPtr                  body;            ///< Background body content
    bool                       initial_open = false;
    DrawerOptions              options;
    std::function<void()>      on_close;
    std::function<void()>      on_open;
    std::shared_ptr<DrawerController> controller; ///< Optional external controller

    Drawer() = default;
    Drawer(WidgetPtr drawer_content, WidgetPtr body, DrawerOptions opt = {})
        : drawer_content(std::move(drawer_content)), body(std::move(body)),
          options(std::move(opt)) {}

    // Fluent API
    Drawer& width(float w)           { options.width = w;              return *this; }
    Drawer& backgroundColor(Color c) { options.background_color = c;   return *this; }
    Drawer& overlayColor(Color c)    { options.overlay_color = c;      return *this; }
    Drawer& side(DrawerSide s)       { options.side = s;               return *this; }
    Drawer& borderRadius(float r)    { options.border_radius = r;      return *this; }
    Drawer& onClose(std::function<void()> fn)  { on_close = std::move(fn); return *this; }
    Drawer& onOpen(std::function<void()> fn)   { on_open  = std::move(fn); return *this; }
    Drawer& setController(std::shared_ptr<DrawerController> c) {
        controller = std::move(c);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Drawer"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Drawer> drawer(WidgetPtr drawer_content, WidgetPtr body,
                                      DrawerOptions options = {}) {
    return std::make_shared<Drawer>(std::move(drawer_content), std::move(body),
                                   std::move(options));
}

} // namespace enki
