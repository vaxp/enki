# FloatingPanel

> An in-window freeform draggable and resizable floating window overlay widget (MDI / tool palette / HUD inspector) featuring 8-direction multi-edge resize handles, title bar dragging, window state toggling (minimize, maximize, restore), active focus elevation, and magnetic edge snapping.

- **Header File**: `#include "enki/widgets/floating_panel.hpp"`
- **C++ Class**: `enki::FloatingPanelWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::FloatingPanel` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::FloatingPanelOptions`
- **Controller**: `enki::FloatingPanelController`
- **Enums**: `enki::FloatingPanelDisplayState`
- **Helper Function**: `enki::floatingPanel(const FloatingPanelProps&)`

---

## Overview

`FloatingPanel` provides a full-featured in-window floating tool window, inspector palette, or audio HUD. Rendered within Enki's **Container-Wrapping Architecture**, it sits above the main viewport `body` in a top-level `Stack`, allowing users to drag it around freely, resize it from any corner or edge, and toggle between normal, minimized, and maximized display states.

### Key Capabilities
- **Title Bar & Window Chrome**:
  - Interactive header with custom icon, title, and standard window buttons (minimize, maximize/restore, close).
  - Drag-to-move by dragging anywhere across the title bar with viewport boundary clamping.
- **8-Direction Resizing**:
  - Invisible edge and corner resize handles: N, S, E, W, NW, NE, SW, SE.
  - Interactive cursor switching (`ResizeHorizontal`, `ResizeVertical`, `ResizeTopLeft`, etc.).
  - Size constraints enforcement (`min_width`, `min_height`, `max_width`, `max_height`).
- **Window State Management**:
  - `Normal`: standard floating window dimensions.
  - `Minimized`: collapses to title bar only (38px height) to conserve workspace screen real estate.
  - `Maximized`: fills the application viewport; clicking restore returns back to previous floating bounds.
- **Elevation & Focus**:
  - Clicking on the panel brings it to the front and activates its glow border highlight (`0xFF38BDF8`).
- **Magnetic Edge Snapping**:
  - Automatically snaps to window boundaries when dragging within a configurable threshold distance (e.g. 16px).

---

## C++ API Definition

### Data Structures & Enums

```cpp
namespace enki {

enum class FloatingPanelDisplayState {
    Normal,     ///< Standard floating window
    Minimized,  ///< Collapsed to title bar only
    Maximized   ///< Maximized to fill viewport
};

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
    Color background_color    = 0xF80F172A;
    Color border_color        = 0xFF334155;
    Color active_border_color = 0xFF38BDF8;
    Color titlebar_bg_color   = 0xFF0B0F19;
    Color title_color         = 0xFFF8FAFC;
    Color subtitle_color      = 0xFF94A3B8;

    std::function<void(float x, float y)> on_moved;
    std::function<void(float w, float h)> on_resized;
    std::function<void(FloatingPanelDisplayState state)> on_state_changed;
    std::function<void()> on_closed;
};

class FloatingPanelController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;

    void setPosition(float x, float y);
    [[nodiscard]] Point getPosition() const;

    void setSize(float w, float h);
    [[nodiscard]] Size getSize() const;

    void minimize();
    void maximize();
    void restore();
    [[nodiscard]] FloatingPanelDisplayState getState() const;

    void bringToFront();
};

} // namespace enki
```

---

## Declarative Usage Example

```cpp
#include "enki/widgets/floating_panel.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

auto panel_ctrl = std::make_shared<FloatingPanelController>();

WidgetPtr app_view = FloatingPanel {
    .content = container({
        .child = text("Inspector Tool Content")
    }),
    .body = container({
        .child = text("Main Application Workspace")
    }),
    .options = {
        .title = "DSP Audio Matrix",
        .icon = "🎛️",
        .initial_x = 240.0f,
        .initial_y = 120.0f,
        .initial_width = 460.0f,
        .initial_height = 320.0f
    },
    .controller = panel_ctrl
};
```

---

## Updating Roadmaps & Summary Table

In `WIDGETS_ROADMAP_v0.2.0.md`, section 19:
- [x] **FloatingPanel** — Draggable, resizable floating window rendered above the main widget tree
