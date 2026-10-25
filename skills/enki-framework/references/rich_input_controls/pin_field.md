# PinField

> Secure PIN entry with delayed masking, masked bullets, and glowing state indicators.

- **Header File**: `#include "enki/widgets/pin_field.hpp"`
- **C++ Class**: `enki::PinFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Helper**: `enki::pinField(PinFieldProps props)` (returns `enki::WidgetPtr`)
- **State Object**: `enki::PinFieldState`
- **Underlying Mechanism**: Timed obscure reveal via internal ticker, glowing security bullets, and numeric keystroke filtering.

---

## Overview

`PinField` is tailored for entering numeric PIN codes (such as lock screen codes, wallet passcodes, and master passwords). Unlike standard password fields, it briefly reveals each newly typed digit for a user-specified delay (`obscure_delay_ms`) before converting it into a security bullet `●`, improving typing accuracy on touchscreen or desktop keyboards.

---

## C++ API Definition

### Struct Definition (`enki/widgets/pin_field.hpp`)
```cpp
namespace enki {

struct PinFieldProps {
    int                                 length = 4;
    float                               box_size = 46.0f;
    float                               gap = 12.0f;
    float                               border_radius = 12.0f;
    int                                 obscure_delay_ms = 400; // 0 = immediate
    std::string                         mask_char = "●";
    bool                                numeric_only = true;
    bool                                has_error = false;
    bool                                auto_focus = false;
    std::string                         initial_value = "";

    Color                               active_border_color = 0xFF00E5FF;
    Color                               inactive_border_color = 0x33FFFFFF;
    Color                               filled_dot_color = 0xFF38BDF8;
    Color                               box_background = 0x59000000;
    Color                               error_color = 0xFFEF4444;

    std::function<void(const std::string&)> on_changed;
    std::function<void(const std::string&)> on_completed;

    operator WidgetPtr() const;
};

class PinFieldWidget : public StatefulWidget {
public:
    PinFieldProps props;

    explicit PinFieldWidget(PinFieldProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "PinField"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr pinField(PinFieldProps props) {
    return std::make_shared<PinFieldWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `length` | `int` | `4` | Required PIN digit count. |
| `box_size` | `float` | `46.0f` | Dimension (width & height) of each PIN cell. |
| `gap` | `float` | `12.0f` | Spacing between adjacent PIN cells. |
| `border_radius` | `float` | `12.0f` | Corner rounding radius of the PIN cells. |
| `obscure_delay_ms` | `int` | `400` | Milliseconds to keep the last typed character visible before masking. |
| `mask_char` | `std::string` | `"●"` | Symbol used for masked digits. |
| `numeric_only` | `bool` | `true` | Restricts accepted input strictly to numbers `0` through `9`. |
| `has_error` | `bool` | `false` | When true, renders an error highlight around all PIN boxes. |
| `auto_focus` | `bool` | `false` | Automatically claims keyboard focus on mount. |
| `initial_value` | `std::string` | `""` | Initial prefilled PIN string. |
| `active_border_color` | `Color` | `0xFF00E5FF` | Border highlight color for active cell. |
| `inactive_border_color`| `Color` | `0x33FFFFFF` | Border color for idle cells. |
| `filled_dot_color` | `Color` | `0xFF38BDF8` | Color of the masked bullet symbol. |
| `box_background` | `Color` | `0x59000000` | Fill color of the PIN cells. |
| `error_color` | `Color` | `0xFFEF4444` | Highlight color for error states. |
| `on_changed` | `std::function<void(const std::string&)>` | `nullptr` | Callback fired on keystroke modification. |
| `on_completed` | `std::function<void(const std::string&)>` | `nullptr` | Callback fired when user finishes typing `length` digits. |

---

## Code Examples (From `widgets_demo/pin_field_demo/main.cpp`)

### 1. 4-Digit Security Passcode Entry
```cpp
auto pin = pinField({
    .length = 4,
    .box_size = 56.0f,
    .gap = 14.0f,
    .obscure_delay_ms = 400,
    .auto_focus = true,
    .on_changed = [this](const std::string& p) {
        current_pin_ = p;
        setState([]{});
    },
    .on_completed = [this](const std::string& p) {
        confirmed_pin_ = p;
        std::cout << ">>> Standalone PIN Entered: " << p << std::endl;
        setState([]{});
    },
});
```
