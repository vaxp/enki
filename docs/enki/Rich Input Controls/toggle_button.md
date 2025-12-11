# ToggleButton

> An atomic pressable button switching between active (ON) and inactive (OFF) visual states.

- **Header File**: `#include "enki/widgets/toggle_button.hpp"`
- **C++ Class**: `enki::ToggleButtonWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Helper**: `enki::toggleButton(ToggleButtonProps props)` (returns `enki::WidgetPtr`)
- **Underlying Mechanism**: Composes Enki's primitive `button()` and `icon()` with 4 distinct theme styles: `Filled`, `Outlined`, `Ghost`, and `Glow`.

---

## Overview

`ToggleButton` serves as an atomic binary state switch in button form (similar to mute buttons, pin toggles, or playback loop toggles). It provides visual feedback for both active and inactive states with hover and press transitions.

---

## C++ API Definition

### Struct Definition (`enki/widgets/toggle_button.hpp`)
```cpp
namespace enki {

enum class ToggleButtonStyle {
    Filled,    ///< Solid background with high contrast when active
    Outlined,  ///< Glowing border and subtle background tint
    Ghost,     ///< Transparent background, accent color on text/icon only
    Glow,      ///< Futuristic neon outer shadow/glow when active
};

struct ToggleButtonProps {
    bool                                is_toggled = false;
    std::function<void(bool)>           on_toggle;
    std::string                         label = "";
    std::string                         icon = "";
    WidgetPtr                           child = nullptr;

    ToggleButtonStyle                   style = ToggleButtonStyle::Filled;
    Color                               active_color = 0xFF00E5FF;
    Color                               inactive_color = 0xFF94A3B8;
    Color                               active_background = 0xFF0C3559;
    Color                               inactive_background = 0x33000000;
    Color                               border_color = 0x4D00E5FF;
    float                               border_width = 1.0f;
    float                               border_radius = 8.0f;
    EdgeInsets                          padding = EdgeInsets::symmetric(8.0f, 16.0f);
    bool                                enabled = true;

    operator WidgetPtr() const;
};

class ToggleButtonWidget : public StatelessWidget {
public:
    ToggleButtonProps props;

    explicit ToggleButtonWidget(ToggleButtonProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "ToggleButton"; }
    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
};

inline WidgetPtr toggleButton(ToggleButtonProps props) {
    return std::make_shared<ToggleButtonWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `is_toggled` | `bool` | `false` | Current state of the button (`true` = ON, `false` = OFF). |
| `on_toggle` | `std::function<void(bool)>` | `nullptr` | Callback invoked with the new inverted state when clicked. |
| `label` | `std::string` | `""` | Optional text displayed on the button. |
| `icon` | `std::string` | `""` | Optional icon or glyph symbol. |
| `child` | `WidgetPtr` | `nullptr` | Custom child widget override for the button content. |
| `style` | `ToggleButtonStyle` | `Filled` | Visual presentation mode (`Filled`, `Outlined`, `Ghost`, `Glow`). |
| `active_color` | `Color` | `0xFF00E5FF` | Foreground accent color when `is_toggled` is true. |
| `inactive_color`| `Color` | `0xFF94A3B8` | Foreground text/icon color when untoggled. |
| `active_background`| `Color` | `0xFF0C3559` | Background color when toggled. |
| `inactive_background`| `Color` | `0x33000000` | Background color when untoggled. |
| `border_color` | `Color` | `0x4D00E5FF` | Outline border color. |
| `border_width` | `float` | `1.0f` | Border stroke width. |
| `border_radius`| `float` | `8.0f` | Corner rounding radius. |
| `padding` | `EdgeInsets` | `symmetric(8, 16)` | Inset padding around the button content. |
| `enabled` | `bool` | `true` | When `false`, disables clicks and dims the control. |

---

## Code Examples (From `widgets_demo/toggle_button_demo/main.cpp`)

### 1. The 4 Visual Styles Comparison
```cpp
auto tb1 = toggleButton({
    .is_toggled = t1_,
    .on_toggle = [this](bool val) {
        t1_ = val;
        setState([]{});
    },
    .label = "Filled Style",
    .icon = "●",
    .style = ToggleButtonStyle::Filled,
    .active_color = 0xFF00E5FF,
});

auto tb2 = toggleButton({
    .is_toggled = t2_,
    .on_toggle = [this](bool val) {
        t2_ = val;
        setState([]{});
    },
    .label = "Outlined Style",
    .icon = "◆",
    .style = ToggleButtonStyle::Outlined,
    .active_color = 0xFFF59E0B,
});

auto tb3 = toggleButton({
    .is_toggled = t3_,
    .on_toggle = [this](bool val) {
        t3_ = val;
        setState([]{});
    },
    .label = "Glow Style",
    .icon = "⚡",
    .style = ToggleButtonStyle::Glow,
    .active_color = 0xFF10B981,
});

auto tb4 = toggleButton({
    .is_toggled = t4_,
    .on_toggle = [this](bool val) {
        t4_ = val;
        setState([]{});
    },
    .label = "Ghost Style",
    .icon = "★",
    .style = ToggleButtonStyle::Ghost,
    .active_color = 0xFFA855F7,
});
```
