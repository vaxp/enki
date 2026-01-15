# Start Application & Lifecycle

> A complete guide to bootstrapping an ENKI application, configuring runtime engine options in `int main()`, inspecting performance metrics, and setting up modern Client-Side Decorations (CSD).

- **Header Files**:
  - `#include "enki/app/app.hpp"` — Application runner, `AppConfig`, and lifecycle manager.
  - `#include "enki/widgets/window_frame.hpp"` — Client-Side Decoration (CSD) container and resize engine.
  - `#include "enki/widgets/titlebar.hpp"` — Desktop window header with window management buttons.
- **Primary Functions**:
  - `enki::runApp(WidgetPtr root, AppConfig config = {})`
  - `enki::windowFrame(WindowFrameProps props)`
  - `enki::titleBar(TitleBarProps props)`
- **Underlying Engine**: Direct EGL / OpenGL + Skia GPU Rasterization + Native Wayland (xdg-shell / layer-shell) & X11.

---

## Overview

Every ENKI application begins in the standard C++ `int main()` function. The framework manages the entire operating system interface:
1. **Platform Discovery**: Connects directly to Wayland or X11 display servers.
2. **GPU Surface Context**: Prepares native EGL/GL contexts and initializes Skia's `GrDirectContext`.
3. **Widget Tree Bootstrap**: Builds the root `Element` hierarchy and attaches the `BuildOwner`.
4. **High-Performance Event Loop**: Dispatches events, performs reactive Anu Flexbox layouts, schedules repaints, and paces frames with sub-millisecond precision.
5. **Clean Teardown**: Ensures all GPU buffers and native resources are released gracefully on window close.

---

## Minimal Quick Start

Here is the absolute minimal Enki application:

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

class HelloApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        return text("Hello, ENKI Framework!", {
            .color = 0xFFFFFFFF,
            .font_size = 20.0f,
        });
    }

    std::string_view typeName() const override { return "HelloApp"; }
};

int main() {
    return runApp(std::make_shared<HelloApp>());
}
```

---

## Configuring `int main()` via `AppConfig`

To configure window geometry, VSync, rendering quality, frame rate limits, or CSD decorations, pass an `AppConfig` struct to `runApp`:

```cpp
#include "enki/app/app.hpp"

int main() {
    AppConfig config;
    config.title       = "My Enki Application";
    config.width       = 1280;
    config.height      = 800;
    config.resizable   = true;
    config.enable_csd  = true;
    config.app_id      = "org.enki.myapp";

    return runApp(std::make_shared<MyApp>(), config);
}
```

---

## `AppConfig` Reference Guide

The following table documents every available property in `AppConfig` (`include/enki/app/app.hpp`):

| Option | Type | Default | Description |
|---|---|---|---|
| `title` | `std::string` | `"ENKI App"` | Window title string reported to the operating system, taskbar, and native titlebar. |
| `width` | `int` | `800` | Initial window width in logical pixels. |
| `height` | `int` | `600` | Initial window height in logical pixels. |
| `resizable` | `bool` | `true` | Allows or restricts window resizing by the user. Disables native and CSD resize handles when `false`. |
| `vsync` | `bool` | `true` | Enables vertical synchronization with the monitor refresh rate (typically 60Hz, 120Hz, 144Hz, or 240Hz). Prevents screen tearing. |
| `msaa` | `bool` | `true` | Enables Multi-Sample Anti-Aliasing on Skia GPU rendering for smooth vector paths, rounded corners, and crisp fonts. |
| `clear_color` | `Color` | `0xFF0F172A` | Background color of the underlying native framebuffer before widgets are painted (32-bit ARGB hex). Can include alpha for translucent windows (e.g. `0x0000004D`). |
| `target_fps` | `int` | `60` | Target frame rate limit when `vsync = false`. Set to `0` to unlock frame rate completely for raw GPU/CPU benchmarking. |
| `window_mode` | `WindowMode` | `WindowMode::Normal` | Window role and display mode (`WindowMode::Normal`, `WindowMode::LayerShell`, `WindowMode::Popup`). |
| `show_performance_overlay` | `bool` | `false` | Renders a real-time HUD showing instantaneous FPS, CPU build/layout/paint times, Skia GPU submission, EGL swap latency, dirty element counts, and active tickers. |
| `enable_csd` | `bool` | `false` | Enables Client-Side Decorations. Instructs the window manager/compositor that the application will render its own custom frame, titlebar, and resize handles. |
| `app_id` | `std::string` | `"enki.app"` | Desktop environment identifier (Wayland `xdg_toplevel.set_app_id` / X11 `WM_CLASS`) used for window grouping, dock icons, and desktop rules. |

---

## Deep Dive into Key `AppConfig` Parameters

### 1. `enable_csd` (Client-Side Decorations)
By default (`enable_csd = false`), Enki requests Server-Side Decorations (SSD) from your desktop compositor (e.g., KWin, Mutter, Hyprland, Sway), which renders the standard operating system window frame and titlebar.

When setting `enable_csd = true`:
- On **Wayland**: Enki negotiates `zxdg_toplevel_decoration_v1::mode = client_side`, removing compositor-drawn borders.
- On **X11**: Enki sets `_MOTIF_WM_HINTS` to frameless mode.
- Your application can then wrap its content in `windowFrame(...)` to render custom titlebars, glowing borders, SkSL shaders, and interactive resize handles.

### 2. `vsync` and `target_fps` (Frame Rate Control & Benchmarking)
- **Standard UI Mode (`vsync = true`)**:
  Enki aligns presentation with your display refresh cycle. When the UI is completely static, Enki's smart dirty-checking mechanism drops idle CPU usage to **0.0%**.
- **Fixed Frame Rate Limit (`vsync = false, target_fps = 120`)**:
  Disables compositor blocking while strictly pacing frame submission to the specified target FPS using Enki's internal microsecond sleep scheduler.
- **Unlocked Benchmarking Mode (`vsync = false, target_fps = 0`)**:
  Removes all frame rate limiters. Skia rasterizes and presents frames as fast as your GPU and CPU can push them (often exceeding 350+ to 700+ FPS), perfect for benchmarking layout and paint performance.

### 3. `clear_color` and Translucent Windows
The `clear_color` property defines the color used by Skia to clear the framebuffer before painting the widget tree:
- **Solid Dark Background**:
  ```cpp
  config.clear_color = 0xFF0F172A; // Solid dark slate (Alpha = 255)
  ```
- **Translucent / Glass Window Background**:
  ```cpp
  config.clear_color = 0x0000004D; 
  ```

### 4. `show_performance_overlay` (Real-Time Developer HUD)
Enables the built-in diagnostic HUD drawn in the top-right corner of your window:
- **Instantaneous FPS**: Smoothed rolling frames per second.
- **CPU Phase Breakdown**:
  - `Build`: Time spent evaluating `widget->build()`.
  - `Layout`: Time spent in Anu Flexbox calculations.
  - `Paint`: Time spent issuing Skia drawing commands.
- **GPU Phase Breakdown**:
  - `GPU Render`: Time spent rasterizing Skia command buffers.
  - `EGL Swap`: Time spent presenting buffers to Wayland/X11 compositors.
- **Element Tree Stats**: Active elements vs dirty elements rebuilt this frame.
- **Active Tickers**: Real-time count of active animation controllers and ticker instances.

### 5. `window_mode` (Desktop Surfaces & Layer Shell)
- `WindowMode::Normal`: Standard desktop window.
- `WindowMode::LayerShell`: On Wayland compositors that support `zwlr_layer_shell_v1` (such as Hyprland, Sway, Wayfire), this mode allows building native desktop status bars, docks, wallpapers, launchers, and lock screens anchored to screen edges.
- `WindowMode::Popup`: Child popups and tooltips.

---

## Client-Side Decorations (CSD) Architecture

To create modern, frameless applications with custom titlebars, floating rounded corners, and native-feeling window resizing, combine `config.enable_csd = true` with `windowFrame(...)`.

### How `WindowFrame` Works
`WindowFrame` is a high-level container that sits at the root of your widget hierarchy:
1. **Interactive TitleBar**: Placed at the top with draggable window areas, minimize, maximize/restore, and close buttons.
2. **Application Content**: Placed below the titlebar, filling the remaining window area.
3. **8-Direction Native Resize Handles**: Surrounds the window edges (`Top`, `Bottom`, `Left`, `Right`) and 4 corners (`TopLeft`, `TopRight`, `BottomLeft`, `BottomRight`). Hovering automatically updates the cursor shape, and clicking triggers hardware compositor resize protocols via `window->beginResize()`.
4. **State-Aware Geometry**: Automatically transitions between rounded corners with subtle drop shadows when floating, and zero corner radius when maximized.

### `WindowFrameProps` Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `content` | `WidgetPtr` | `nullptr` | Root content widget of your application. |
| `title` | `std::string` | `"ENKI Application"` | Title text displayed in the header. |
| `titlebar` | `WidgetPtr` | `nullptr` | Optional custom titlebar widget (automatically created if `nullptr`). |
| `titlebar_style` | `TitleBarStyle` | `TitleBarStyle::VAXPOS` | Layout and button style: `TitleBarStyle::VAXPOS` or `TitleBarStyle::Default`. |
| `border_radius` | `float` | `10.0f` | Corner radius when floating (automatically becomes `0.0f` when maximized). |
| `border_color` | `Color` | `0x26FFFFFF` | Subtle outer border outline color. |
| `border_width` | `float` | `1.0f` | Border stroke thickness in logical pixels. |
| `background_color` | `Color` | `0xFF0F1117` | Window content background color. |
| `border_shader` | `std::string` | `""` | SkSL fragment shader code injected into the outer window border (for glowing or animated edges). |
| `background_shader` | `std::string` | `""` | SkSL fragment shader code evaluated across the entire window frame background. |
| `border_svg` | `std::string` | `""` | Vector SVG border outline for stylized or fantasy game frames. |
| `border_svg_slice` | `std::optional<SvgSlice>` | `std::nullopt` | 9-slice insets for scaling the SVG border without corner distortion. |
| `titlebar_background_color` | `std::optional<Color>` | `std::nullopt` | Active titlebar background color override. |
| `titlebar_inactive_background_color` | `std::optional<Color>` | `std::nullopt` | Background color when the window loses focus. |
| `resize_thickness` | `float` | `6.0f` | Grab thickness of the 4 outer edge resize hit-boxes. |
| `corner_size` | `float` | `14.0f` | Dimension of the 4 corner resize hit-boxes. |
| `enable_resize` | `bool` | `true` | Whether edge and corner resizing is enabled. |

### TitleBar Styles (`TitleBarStyle`)
- **`TitleBarStyle::VAXPOS`**:
  macOS / VAXPOS aesthetic: Circular "traffic light" control buttons placed on the far left with subtle ambient glow and hover glyphs (Close, Minimize, Maximize), accompanied by a centered window title.
- **`TitleBarStyle::Default`**:
  Traditional desktop layout: App icon and title aligned on the left, with standard rectangular minimize, maximize, and close buttons on the far right.

---

## Production Code Examples

### Example 1: Full Modern CSD Application (VAXPOS Style)
*(As used in `widgets_demo/container_demo/` and `widgets_demo/rich_inputs_demo/`)*

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class AppContentState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        // Application body content
        auto body = container({
            .color = 0xFF0F172A,
            .align = Alignment::Center,
            .width = 100_pct,
            .height = 100_pct,
            .padding = StyleInsets::all(24_px),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("Welcome to ENKI", {
                        .color = 0xFFFFFFFF,
                        .font_size = 28.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Client-Side Decorated Window with VAXPOS TitleBar", {
                        .color = 0xFF94A3B8,
                        .font_size = 14.0f,
                    })
                }
            })
        });

        // Wrap the entire app in WindowFrame
        return windowFrame(WindowFrameProps{
            .content = body,
            .title = "ENKI — CSD Production Showcase",
            .border_radius = 12.0f,
            .border_color = 0x3338BDF8,
            .border_width = 1.5f,
            .background_color = 0xFF0F172A,
            .titlebar_style = TitleBarStyle::VAXPOS,
            .titlebar_background_color = 0x33000000,
        });
    }
};

class MainApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<AppContentState>();
    }
    std::string_view typeName() const override { return "MainApp"; }
};

int main() {
    AppConfig config;
    config.title       = "ENKI — CSD Production Showcase";
    config.width       = 1024;
    config.height      = 640;
    config.resizable   = true;
    config.enable_csd  = true; // Enable Client-Side Decorations
    config.app_id      = "org.enki.csd_showcase";
    config.clear_color = 0x0000004D; // Translucent native surface

    return runApp(std::make_shared<MainApp>(), config);
}
```

---

### Example 2: High-Performance Benchmarking Setup
To measure the raw rendering ceiling and display engine metrics:

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

int main() {
    AppConfig config;
    config.title                    = "Enki Benchmark";
    config.width                    = 1280;
    config.height                   = 720;
    config.vsync                    = false; // Disable display sync lock
    config.target_fps               = 0;     // Unlocked FPS (Max throughput)
    config.show_performance_overlay = true;  // Render real-time performance HUD

    return runApp(std::make_shared<MyBenchmarkWidget>(), config);
}
```

---

### Example 3: CSD Window with Glowing SkSL Border Shader
You can inject a live SkSL shader into the outer border of the entire application window:

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/window_frame.hpp"

using namespace enki;

// Cyber neon border SkSL shader
const std::string window_neon_shader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float angle = atan(uv.y - 0.5, uv.x - 0.5);
        float glow = sin(angle * 4.0 + time * 3.0) * 0.5 + 0.5;
        vec3 cyan = vec3(0.06, 0.72, 0.95);
        vec3 purple = vec3(0.66, 0.33, 0.98);
        return vec4(mix(cyan, purple, glow), 1.0);
    }
)";

WidgetPtr buildWindow(WidgetPtr app_content) {
    return windowFrame(WindowFrameProps{
        .content = app_content,
        .title = "Cyber Window",
        .border_radius = 16.0f,
        .border_width = 2.0f,
        .border_shader = window_neon_shader, // Live GPU shader on window outline!
        .titlebar_style = TitleBarStyle::VAXPOS,
    });
}
```

---

## Accessing Runtime App & Window APIs

Within your widgets or states, you can interact with the running application instance via `App::instance()`:

```cpp
#include "enki/app/app.hpp"

// Query runtime window information
App* app = App::instance();
if (app) {
    // Change window title dynamically
    app->setTitle("Saving Document...");

    // Query active window size and DPI scale
    Size sz = app->windowSize();
    float dpi = app->dpiScale();

    // Query real-time performance statistics
    FrameStats stats = app->frameStats();
    double current_fps = stats.fps;
    double cpu_time = stats.cpu_time_ms;

    // Request application shutdown
    // app->quit();
}
```

---

## Best Practices

> [!TIP]
> **Zero CPU Idle Mode**: When designing custom animations or widgets, use Enki's built-in `Ticker` or `AnimationController`. When animations stop, Enki automatically halts frame generation, keeping idle CPU consumption at **0.0%**.

> [!IMPORTANT]
> **CSD Resize Insets**: When `enable_csd = true` is active, always wrap your root widget with `windowFrame(...)`. If you build a completely custom root layout without `windowFrame`, make sure you provide titlebar dragging (`window->beginMove()`) and resize edge hitboxes (`window->beginResize()`) so desktop users can manage your window naturally.

---

## See Also
- [**Container**](../Layout/container.md) — Comprehensive visual decoration, SkSL shaders, and SVG 9-slice injection.
- [**Flexbox (Row & Column)**](../Layout/row.md) — Declarative Flexbox layouts.
- [**Spring Physics**](../Animation%20&%20Motion/spring_physics.md) — Physics-based fluid motion.
- [**Timeline & Stagger**](../Animation%20&%20Motion/timeline_and_stagger.md) — Multi-track coordinated motion.
