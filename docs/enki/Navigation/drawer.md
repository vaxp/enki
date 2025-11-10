# Drawer

> A slide-in modal navigation drawer widget that animates over the main body content with a dismissible semi-transparent scrim overlay and programmatic open/close control.

- **Header File**: `#include "enki/widgets/drawer.hpp"`
- **C++ Class**: `enki::DrawerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Drawer` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DrawerProps`
- **Controller**: `enki::DrawerController`
- **Options Struct**: `enki::DrawerOptions`
- **Side Enum**: `enki::DrawerSide` (`Left`, `Right`)

---

## Overview

`Drawer` wraps the application's main `body` content and provides a slide-in navigation panel (`child`). When opened (via `drawer_ctrl->open()` or `initial_open = true`), it slides smoothly into view from the left or right edge while casting a semi-transparent scrim overlay (`0x80000000`) over the body. Clicking anywhere on the scrim dismisses the drawer.

---

## C++ API Definition

### `DrawerController`
```cpp
namespace enki {

class DrawerController {
public:
    void open();
    void close();
    [[nodiscard]] bool isOpen() const;
};

} // namespace enki
```

### Configuration Options (`DrawerOptions`)
```cpp
namespace enki {

enum class DrawerSide {
    Left,
    Right
};

struct DrawerOptions {
    float      width            = 280.0f;
    Color      background_color = 0xFF1E293B;
    Color      overlay_color    = 0x80000000; ///< Semi-transparent scrim backdrop
    Color      border_color     = 0xFF334155;
    float      border_radius    = 0.0f;
    float      shadow_blur      = 24.0f;
    Color      shadow_color     = 0x80000000;
    DrawerSide side             = DrawerSide::Left;
    bool       close_on_overlay = true;       ///< Dismiss when tapping overlay

    constexpr bool operator==(const DrawerOptions&) const = default;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Drawer {
    Key                               key          = Key::none();
    WidgetPtr                         child        = nullptr; ///< Content inside the drawer
    WidgetPtr                         body         = nullptr; ///< Main page body content
    bool                              initial_open = false;

    DrawerOptions                     options;
    std::shared_ptr<DrawerController> controller   = nullptr;

    std::function<void()>             on_close     = nullptr;
    std::function<void()>             on_open      = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content placed inside the sliding drawer panel. |
| `body` | `WidgetPtr` | `nullptr` | Background application content overlaid by the drawer. |
| `controller` | `std::shared_ptr<DrawerController>`| `nullptr` | Programmatic handle for `open()`, `close()`, and `isOpen()`. |
| `options.side` | `DrawerSide` | `DrawerSide::Left`| Screen edge to slide in from (`Left` or `Right`). |
| `options.width` | `float` | `280.0f` | Width of the drawer panel in pixels. |
| `options.close_on_overlay`| `bool` | `true` | Closes drawer when user taps the dim backdrop overlay. |

---

## Code Examples (From `widgets_demo/drawer_demo/main.cpp`)

### 1. Slide-In Navigation Drawer with Hamburger Toggle
```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/drawer.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

class AppShellState : public State {
    std::shared_ptr<DrawerController> drawer_ = std::make_shared<DrawerController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto drawerMenu = column({
            .gap = 12_px,
            .children = {
                text("Navigation Menu", { .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                button(text("Profile"),  [this]() { drawer_->close(); }),
                button(text("Settings"), [this]() { drawer_->close(); }),
            }
        });

        auto mainBody = container({
            .color = 0xFF0F172A,
            .child = button(text("☰ Open Drawer"), [this]() {
                drawer_->open();
            })
        });

        return Drawer {
            .child = drawerMenu,
            .body = mainBody,
            .controller = drawer_,
            .options = {
                .width = 300.0f,
                .side = DrawerSide::Left,
                .background_color = 0xFF1E293B,
            }
        };
    }
};
```

---

## See Also
- [**Sidebar**](./sidebar.md) — Permanent layout sidebar that pushes and resizes content.
- [**NavigationRail**](./navigation_rail.md) — Vertical compact icon rail.
