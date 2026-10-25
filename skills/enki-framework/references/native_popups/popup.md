# Popup

> The universal native compositor floating surface widget in Enki, providing 15 placement modes, cursor tracking, dismissal controls, programmatic controllers, and cross-window boundary floating.

- **Header File**: `#include "enki/widgets/popup.hpp"`
- **C++ Class**: `enki::PopupWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Popup` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::PopupWidgetOptions` (alias `PopupConfig`)
- **Controller**: `enki::PopupController`
- **Enums**: `enki::PopupPlacement`, `enki::PopupTrigger`
- **Static Launcher**: `Popup::show(ctx, builder, options)`

---

## Overview

`Popup` is the foundational building block for all native floating surfaces in Enki. It binds an anchor widget (`child`) with arbitrary popup content (`builder`). When activated via clicks, hover, or programmatic controller calls, Enki requests a new native subsurface from the operating system's window compositor. This surface renders independently with its own Skia canvas, elevation shadows, and rounded corners, without being clipped by the parent window.

---

## C++ API Definition

### Placement & Trigger Enums
```cpp
namespace enki {

enum class PopupPlacement {
    TopLeft, TopCenter, TopRight,
    BottomLeft, BottomCenter, BottomRight,
    LeftTop, LeftCenter, LeftBottom,
    RightTop, RightCenter, RightBottom,
    FollowCursor,   ///< Tracks mouse pointer coordinates dynamically
    CenterScreen,   ///< Centers on display monitor
    Manual          ///< Absolute coordinate positioning via manual_position
};

enum class PopupTrigger {
    Click,          ///< Toggle on left mouse click
    Hover,          ///< Open on hover, close on mouse exit
    LongPress,      ///< Open on touch/mouse hold
    SecondaryClick, ///< Open on right mouse click
    Manual          ///< Programmatic trigger only via PopupController
};

} // namespace enki
```

### Controller (`PopupController`)
```cpp
namespace enki {

class PopupController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;
};

} // namespace enki
```

### Configuration Options (`PopupWidgetOptions`)
```cpp
namespace enki {

struct PopupWidgetOptions {
    PopupPlacement placement        = PopupPlacement::BottomCenter;
    PopupTrigger   trigger          = PopupTrigger::Click;

    Point          manual_position  = {0.0f, 0.0f}; ///< When placement == Manual
    Point          offset           = {0.0f, 0.0f}; ///< Additional pixel offset (x, y)

    Color          background_color = 0xFA1F242C;   ///< ARGB translucent background
    Color          border_color     = 0xFF363B42;
    float          border_width     = 1.0f;
    float          border_radius    = 10.0f;

    float          elevation        = 12.0f;        ///< Skia drop shadow blur
    Color          shadow_color     = 0x60000000;

    EdgeInsets     padding          = EdgeInsets::all(12.0f);
    Size           content_size     = Size{240.0f, 160.0f};
    bool           auto_dismiss     = true;         ///< Auto-close on click outside

    std::string    custom_shader    = "";           ///< Optional SkSL runtime shader
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Popup {
    Key                                                                   key        = Key::none();
    WidgetPtr                                                             child      = nullptr;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> builder    = nullptr;
    std::function<WidgetPtr(BuildContext&)>                               simple_builder = nullptr;
    PopupWidgetOptions                                                    options    = {};
    std::shared_ptr<PopupController>                                      controller = nullptr;

    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
        PopupWidgetOptions options = PopupWidgetOptions()
    );

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The anchor widget whose bounds position the popup. |
| `builder` | `Function(BuildContext&, NativePopup)`| `nullptr` | Factory constructing the popup's internal widget tree. |
| `simple_builder`| `Function(BuildContext&)` | `nullptr` | Simplified builder overload when native handle is unneeded. |
| `controller` | `shared_ptr<PopupController>`| `nullptr` | Controller for programmatic opening and closing. |
| `options.placement`| `PopupPlacement` | `BottomCenter` | Directional alignment relative to target anchor. |
| `options.trigger` | `PopupTrigger` | `Click` | User interaction activating the popup. |
| `options.auto_dismiss`| `bool` | `true` | Automatically closes when clicking outside the surface. |

---

## Code Examples (From `widgets_demo/popup_demo/main.cpp`)

### 1. Anchored Card Popup with `TopCenter` Placement
```cpp
#include "enki/widgets/popup.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildUserMenu() {
    return Popup {
        .child = button(text("User Profile"), []{}),
        .simple_builder = [](BuildContext& ctx) {
            return column({
                .gap = 8_px,
                .children = {
                    text("Logged in as Admin", { .font_weight = FontWeight::Bold }),
                    text("admin@company.com", { .color = 0xFF94A3B8, .font_size = 11.0f }),
                }
            });
        },
        .options = {
            .placement = PopupPlacement::BottomRight,
            .content_size = Size{220.0f, 90.0f},
            .background_color = 0xFA1E293B,
        }
    };
}
```

### 2. Follow Cursor Hover Popup
```cpp
auto inspectorCard = Popup {
    .child = text("Hover over data point"),
    .simple_builder = [](BuildContext& ctx) {
        return text("Value: 98.4% (Healthy)", { .color = 0xFF10B981 });
    },
    .options = {
        .placement = PopupPlacement::FollowCursor,
        .trigger = PopupTrigger::Hover,
        .offset = Point{15.0f, 15.0f},
    }
};
```

### 3. Imperative Launcher (`Popup::show`)
```cpp
void onActionButtonClicked(BuildContext& ctx) {
    Popup::show(ctx, [](BuildContext& innerCtx) {
        return text("Directly spawned native modal popup!");
    }, {
        .placement = PopupPlacement::CenterScreen,
        .content_size = Size{320.0f, 180.0f},
    });
}
```

---

## See Also
- [**Popover**](./popover.md) — Specialized popup featuring a pointer arrow tail.
- [**Tooltip**](./tooltip.md) — Lightweight popup for brief informational messages.
- [**ContextMenu**](./context_menu.md) — Right-click popup menus.
