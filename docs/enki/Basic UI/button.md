# Button

> An interactive button widget supporting Hover, Press, Disabled visual states, animated ripple effects, and custom SkSL shader injection.

- **Header File**: `#include "enki/widgets/button.hpp"`
- **C++ Class**: `enki::ButtonWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Button` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ButtonProps`
- **Helper Functions**: `enki::button(...)`
- **Underlying Engine**: Skia interactive render states + hardware-accelerated ripple animations

---

## Overview

`Button` is Enki's primary interactive push button. It manages pointer interaction states (`normal`, `hover`, `pressed`, `disabled`) with smooth transitions and click ripple effects. It also allows developers to inject custom SkSL GPU shaders via `custom_shader` for animated glowing borders, gradients, or pulsating effects.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

using ButtonCallback = std::function<void()>;

struct Button {
    Key            key              = Key::none();
    WidgetPtr      child            = nullptr;
    ButtonCallback on_pressed       = nullptr;
    bool           disabled         = false;

    // Interactive State Colors
    Color          normal_color     = 0xFF2563EB; // Primary blue
    Color          hover_color      = 0xFF3B82F6;
    Color          pressed_color    = 0xFF1D4ED8;
    Color          disabled_color   = 0xFF475569;
    
    // Geometry & Padding
    float          border_radius    = 8.0f;
    EdgeInsets     padding          = EdgeInsets::symmetric(10.0f, 20.0f);
    float          min_width        = 64.0f;
    float          min_height       = 36.0f;
    
    // Elevation Shadows
    Color          shadow_color     = 0x40000000;
    float          shadow_blur      = 4.0f;
    float          shadow_offset_dy = 2.0f;
    
    // Ripple Effect
    bool           enable_ripple    = true;
    Color          ripple_color     = 0x40FFFFFF;
    
    // SkSL Custom GPU Shader Injection
    std::string    custom_shader    = ""; 

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<ButtonWidget> button(WidgetPtr child,
                                            ButtonCallback on_pressed = nullptr,
                                            ButtonProps options = ButtonProps());

inline std::shared_ptr<ButtonWidget> button(ButtonProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Content displayed inside the button (e.g. `text("Submit")`). |
| `on_pressed` | `ButtonCallback` | `nullptr` | Click handler function. If `nullptr`, the button is automatically disabled. |
| `disabled` | `bool` | `false` | Explicitly disables button interactivity and applies `disabled_color`. |
| `normal_color` | `Color` | `0xFF2563EB` | Resting background color. |
| `hover_color` | `Color` | `0xFF3B82F6` | Background color when mouse pointer hovers over the button. |
| `pressed_color` | `Color` | `0xFF1D4ED8` | Background color while mouse button is held down. |
| `border_radius` | `float` | `8.0f` | Corner radius of the button box. |
| `padding` | `EdgeInsets` | `symmetric(10, 20)` | Padding around the child content. |
| `enable_ripple` | `bool` | `true` | Enables circular ripple expansion animation upon clicking. |
| `custom_shader` | `std::string` | `""` | Optional raw SkSL shader source code for custom GPU effects. |

---

## Code Examples (From `widgets_demo/button_demo/main.cpp`)

### 1. Primary Action Button
```cpp
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSaveButton() {
    return Button {
        .child = text("Save Changes", {
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        }),
        .on_pressed = []() {
            std::cout << "Saving changes...\n";
        },
    };
}
```

### 2. Danger / Delete Button
```cpp
auto deleteBtn = Button {
    .normal_color = 0xFFDC2626, // Red 600
    .hover_color  = 0xFFEF4444, // Red 500
    .pressed_color= 0xFFB91C1C, // Red 700
    .child = text("Delete Record"),
    .on_pressed = []() { /* Delete */ },
};
```

### 3. Icon + Text Button (via `Row`)
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/icon.hpp"

auto uploadBtn = Button {
    .child = row({
        .align_items = Align::Center,
        .gap = 8_px,
        .children = {
            icon(Icons::Upload, 18.0f),
            text("Upload File"),
        }
    }),
    .on_pressed = []() { /* Upload */ },
};
```

---

## See Also
- [**IconButton**](./icon_button.md) — Circular or transparent icon action trigger.
- [**FloatingActionButton**](./floating_action_button.md) — Elevated circular action button.
