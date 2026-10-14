# Flyout

> The specialized native compositor drawer and flyout surface in Enki, designed for Desktop Shell panels, Control Centers, Notification Drawers, System Trays, and Context Drawers with Wayland Layer Surface integration, procedural shaders, and smart quadrant anchoring.

- **Header File**: `#include "enki/widgets/flyout.hpp"`
- **C++ Class**: `enki::FlyoutWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Flyout` (converts implicitly to `WidgetPtr`)
- **Helper Function**: `enki::flyout(Flyout)`
- **Options Struct**: `enki::FlyoutOptions`
- **Controller**: `enki::FlyoutController`
- **Handle**: `enki::FlyoutHandle`
- **Enums**: `enki::FlyoutPlacement`, `enki::FlyoutTrigger`
- **Static Launcher**: `Flyout::show(ctx, builder, options)`

---

## Overview

While standard `Popup` widgets rely on strict compositor popup protocols (`xdg_popup` on Wayland with rigid child grabs), `Flyout` is engineered specifically for **Desktop Shell panels, Control Centers, and Notification Centers** (modeled after modern desktop environments like AetherShell).

`Flyout` creates a dedicated, unreserved top-level layer surface (`zwlr_layer_surface_v1` on Wayland with `exclusive_zone = -1` and `ShellLayer::Top`, `override_redirect` on X11, and `WS_EX_TOPMOST | WS_EX_TOOLWINDOW` on Win32). This grants several vital architectural benefits:
1. **Unreserved Floating Surface**: It floats freely above desktop panels without reserving screen geometry or causing compositor grab deadlocks.
2. **Nested Subsurfaces**: Unlike `xdg_popup` which restricts nested interaction, a `Flyout` drawer can host interactive child popups, dropdowns, and sliders seamlessly.
3. **Smart Quadrant Positioning (`FlyoutPlacement::Auto`)**: Automatically detects whether the trigger panel is docked at the top or bottom, and in the left or right half of the monitor, preventing any overflow past screen boundaries.
4. **Auto-Dismiss Toggle Debounce**: When clicking the trigger button while the flyout is already open, the system cleanly toggles the flyout OFF without re-trigger bounce.
5. **Full Skia BoxDecoration & GPU Shaders**: Direct support for procedural SkSL runtime border shaders (`border_shader`), background shaders (`background_shader`), SVG vector frames (`border_svg`), and multi-layer drop shadows (`box_shadow`).

---

## C++ API Definition

### Placement & Trigger Enums
```cpp
namespace enki {

enum class FlyoutPlacement {
    Auto,         ///< Automatically determine anchor & margins based on screen quadrant
    BelowAnchor,  ///< Force flyout below the trigger button
    AboveAnchor,  ///< Force flyout above the trigger button
    LeftAnchor,   ///< Force flyout to the left of the trigger button
    RightAnchor,  ///< Force flyout to the right of the trigger button
    AtCursor,     ///< Spawn exactly at mouse click coordinates (like a context popup menu)
    Manual,       ///< Spawn at custom_position coordinates
};

enum class FlyoutTrigger {
    Click,          ///< Toggle on primary (left) mouse click
    SecondaryClick, ///< Toggle on secondary (right) mouse click
    Manual          ///< Programmatic trigger only via FlyoutController
};

} // namespace enki
```

### Controller (`FlyoutController`)
```cpp
namespace enki {

class FlyoutController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;
    void setToggleCallback(std::function<void(bool)> cb);
};

} // namespace enki
```

### Handle (`FlyoutHandle`)
```cpp
namespace enki {

class FlyoutHandle {
public:
    void close();
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] SurfaceHost* surfaceHost() const;
};

} // namespace enki
```

### Configuration Options (`FlyoutOptions`)
```cpp
namespace enki {

struct FlyoutOptions {
    int32_t         width            = 360;          ///< Desired flyout width in pixels
    int32_t         height           = 480;          ///< Desired flyout height in pixels
    FlyoutPlacement placement        = FlyoutPlacement::Auto;
    FlyoutTrigger   trigger          = FlyoutTrigger::Click;
    Point           offset           = {0.0f, 6.0f}; ///< Gap from the trigger widget or cursor
    std::optional<Point> custom_position = std::nullopt; ///< Explicit screen coordinates when using Manual placement

    bool            auto_dismiss     = true;         ///< Automatically close when clicking outside
    KeyboardMode    keyboard_mode    = KeyboardMode::OnDemand; ///< Focus mode for keyboard navigation/search
    ShellLayer      layer            = ShellLayer::Top;        ///< System surface stacking layer

    // ── Full BoxDecoration / Custom Window Frame ───────────────
    std::optional<BoxDecoration> decoration = std::nullopt; ///< Full container decoration override

    // Visual styling shortcuts (applied when decoration is not explicitly set)
    Color           background_color = 0xF5181C24;   ///< Modern translucent dark background
    Color           border_color     = 0xFF333A46;   ///< Subtle border stroke
    float           border_width     = 1.0f;
    float           border_radius    = 16.0f;        ///< Rounded corners
    float           elevation        = 24.0f;        ///< Drop shadow radius
    Color           shadow_color     = 0x70000000;

    // Advanced procedural shaders & SVG vector frame injection
    std::string     border_shader    = "";           ///< SkSL procedural border shader (e.g. Cyber Neon, Pixel Art)
    std::string     background_shader= "";           ///< SkSL procedural background shader (e.g. Live Plasma)
    std::string     border_svg       = "";           ///< Direct SVG vector border (e.g. Sci-Fi HUD chamfers)
    std::string     background_svg   = "";           ///< Direct SVG vector background
    std::optional<GradientConfig> gradient = std::nullopt;
    std::vector<BoxShadow> box_shadow = {};

    EdgeInsets      padding          = EdgeInsets::all(16.0f);
    std::string     custom_shader    = "";           ///< Legacy alias for background_shader
    std::function<void()> on_close   = nullptr;      ///< Invoked when flyout is closed
};

} // namespace enki
```

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct Flyout {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder = nullptr;
    std::function<WidgetPtr(BuildContext&)> simple_builder = nullptr;
    FlyoutOptions options = {};
    std::shared_ptr<FlyoutController> controller = nullptr;

    operator WidgetPtr() const;

    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<FlyoutHandle>)> builder,
        FlyoutOptions options = FlyoutOptions()
    );

    static std::shared_ptr<FlyoutHandle> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&)> builder,
        FlyoutOptions options = FlyoutOptions()
    );
};

inline std::shared_ptr<FlyoutWidget> flyout(Flyout f);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Anchor widget on the shell panel that triggers the flyout. |
| `builder` | `Function(BuildContext&, shared_ptr<FlyoutHandle>)` | `nullptr` | Builder function creating the flyout's internal widget tree. Receives handle for programmatic dismissal. |
| `simple_builder` | `Function(BuildContext&)` | `nullptr` | Simplified builder overload when `FlyoutHandle` is not required. |
| `controller` | `shared_ptr<FlyoutController>` | `nullptr` | Programmatic controller for triggering `show()`, `hide()`, and `toggle()`. |
| `options.width` | `int32_t` | `360` | Desired pixel width of the native flyout surface. |
| `options.height` | `int32_t` | `480` | Desired pixel height of the native flyout surface. |
| `options.placement` | `FlyoutPlacement` | `Auto` | Positioning strategy (`Auto`, `BelowAnchor`, `AboveAnchor`, `LeftAnchor`, `RightAnchor`, `AtCursor`, `Manual`). |
| `options.trigger` | `FlyoutTrigger` | `Click` | User interaction activating the flyout (`Click`, `SecondaryClick`, `Manual`). |
| `options.offset` | `Point` | `{0.0f, 6.0f}` | Extra clearance gap `{dx, dy}` from the anchor widget or mouse cursor. |
| `options.custom_position` | `optional<Point>` | `nullopt` | Explicit screen coordinates when using `Manual` placement. |
| `options.auto_dismiss` | `bool` | `true` | Automatically closes the surface when clicking outside (includes toggle debounce). |
| `options.layer` | `ShellLayer` | `Top` | Windowing compositor layer (`Top`, `Overlay`, `Bottom`). |
| `options.keyboard_mode` | `KeyboardMode` | `OnDemand` | Keyboard focus policy (`OnDemand`, `Exclusive`, `None`). |
| `options.decoration` | `optional<BoxDecoration>` | `nullopt` | Complete `BoxDecoration` specification replacing default styling. |
| `options.border_shader` | `std::string` | `""` | SkSL shader source evaluated live at 60 FPS for procedural animated borders. |
| `options.background_shader`| `std::string` | `""` | SkSL shader source for procedural backgrounds (e.g. animated plasma, blur). |
| `options.border_svg` | `std::string` | `""` | Direct SVG vector string injected as a responsive window frame. |
| `options.box_shadow` | `vector<BoxShadow>` | `{}` | Multi-layered drop shadows or glowing aura effects. |
| `options.on_close` | `Function()` | `nullptr` | Callback executed when the flyout surface is dismissed or closed. |

---

## Code Examples (From `widgets_demo/flyout_demo/main.cpp`)

### 1. Control Center Flyout with `Auto` Alignment & Close Button
```cpp
#include "enki/widgets/flyout.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr createControlCenterButton() {
    auto trigger_btn = container({
        .color = 0xFF272C35,
        .border_radius = BorderRadius::circular(8.0f),
        .padding = StyleInsets::symmetric(8.0f, 14.0f),
        .child = text("⚙ Quick Settings", {
            .color = 0xFFFFFFFF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        }),
    });

    return flyout(Flyout{
        .child = trigger_btn,
        .builder = [](BuildContext& ctx, std::shared_ptr<FlyoutHandle> handle) {
            return column({
                .align_items = Align::Stretch,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("Control Center", { .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("Wi-Fi: Connected (5GHz)", { .color = 0xFF38BDF8 }),
                    text("Bluetooth: Active", { .color = 0xFF38BDF8 }),
                    button({
                        .child = text("Close Panel", { .color = 0xFFE2E8F0 }),
                        .on_pressed = [handle]() {
                            if (handle) handle->close();
                        },
                        .normal_color = 0xFF334155,
                    }),
                },
            });
        },
        .options = FlyoutOptions{
            .width = 360,
            .height = 420,
            .placement = FlyoutPlacement::Auto,
            .auto_dismiss = true,
        },
    });
}
```

### 2. Custom Pixel Art Window Frame (SkSL Border Shader)
```cpp
const std::string pixel_art_border_shader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        float p = 3.0; // Retro pixel size
        vec2 g = floor(fragCoord / p);
        vec2 sz = floor(resolution / p);
        float dL = g.x;
        float dR = sz.x - 1.0 - g.x;
        float dT = g.y;
        float dB = sz.y - 1.0 - g.y;
        float dX = min(dL, dR);
        float dY = min(dT, dB);

        vec4 cOutline = vec4(0.18, 0.16, 0.19, 1.0);
        vec4 cDark    = vec4(0.32, 0.29, 0.34, 1.0);
        vec4 cMid     = vec4(0.48, 0.45, 0.50, 1.0);
        vec4 cLight   = vec4(0.68, 0.65, 0.71, 1.0);

        if (dX < 5.0 && dY < 5.0) {
            if (dX == 0.0 && dY == 0.0) return vec4(0.0);
            if (dX == 0.0 || dY == 0.0 || dX == 4.0 || dY == 4.0) return cOutline;
            if (dX == 2.0 && dY == 2.0) return cDark;
            if (dY <= 2.0 && dX <= 2.0) return cLight;
            return cMid;
        }
        float depth = min(dX, dY);
        if (depth > 4.0) return vec4(0.0);
        float along = (dX < dY) ? g.y : g.x;
        float modStep = mod(along, 4.0);
        if (depth == 0.0 && (modStep == 1.0 || modStep == 2.0)) return cOutline;
        if (depth == 1.0) return (modStep == 1.0 || modStep == 2.0) ? cLight : cOutline;
        if (depth == 2.0) return (modStep == 0.0 || modStep == 3.0) ? cDark : cMid;
        if (depth == 3.0) return (modStep == 0.0 || modStep == 3.0) ? cOutline : cDark;
        if (depth == 4.0 && (modStep == 0.0 || modStep == 3.0)) return cOutline;
        return cOutline;
    }
)";

auto retroFlyout = flyout(Flyout{
    .child = trigger_btn,
    .builder = buildContent,
    .options = FlyoutOptions{
        .width = 380,
        .height = 500,
        .background_color = 0xF5131117,
        .border_width = 15.0f,
        .border_shader = pixel_art_border_shader,
        .box_shadow = {
            BoxShadow::glow(0x407A7380, 20.0f),
            BoxShadow::standard(0x80000000, 15.0f, 8.0f)
        },
        .padding = EdgeInsets::all(20.0f),
    },
});
```

### 3. Context Flyout Spawning at Mouse Click (`AtCursor`)
```cpp
auto contextFlyout = flyout(Flyout{
    .child = my_canvas_widget,
    .builder = [](BuildContext& ctx, std::shared_ptr<FlyoutHandle> handle) {
        return buildContextMenu(handle);
    },
    .options = FlyoutOptions{
        .width = 220,
        .height = 280,
        .placement = FlyoutPlacement::AtCursor,        // Opens directly at pointer coordinates
        .trigger = FlyoutTrigger::SecondaryClick,     // Opens on right-click
        .offset = Point{4.0f, 4.0f},
        .auto_dismiss = true,
    },
});
```

### 4. Imperative Programmatic Launcher (`Flyout::show`)
```cpp
void openSystemFlyout(BuildContext& ctx) {
    Flyout::show(ctx, [](BuildContext& innerCtx, std::shared_ptr<FlyoutHandle> handle) {
        return container({
            .child = text("Imperatively launched top-level flyout!"),
        });
    }, FlyoutOptions{
        .width = 320,
        .height = 240,
        .placement = FlyoutPlacement::Auto,
    });
}
```

---

## See Also
- [**Popup**](./popup.md) — Standard child-grab floating surfaces for transient menus and tooltips.
- [**Menu**](./menu.md) — Desktop application menu bar and cascading submenus.
- [**ContextMenu**](./context_menu.md) — Secondary-click popup menus.
- [**Container**](../Basic%20UI/container.md) — Advanced SkSL decoration and layout container.
