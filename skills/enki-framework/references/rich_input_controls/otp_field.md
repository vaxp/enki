# OTPField (One-Time Password)

> Segmented One-Time Password input with auto-focus advance, paste distribution, and glowing boxes.

- **Header File**: `#include "enki/widgets/otp_field.hpp"`
- **C++ Class**: `enki::OTPFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Helper**: `enki::otpField(OTPFieldProps props)` (returns `enki::WidgetPtr`)
- **State Object**: `enki::OTPFieldState`
- **Underlying Mechanism**: Multi-cell flexbox layout with focused box glowing indicator, automatic keyboard navigation across digit slots, and global rich input focus routing.

---

## Overview

`OTPField` is designed for two-factor authentication (2FA), SMS verification, and security verification codes. It distributes characters into individual distinct boxed cells, advances focus automatically upon typing, handles backspaces across cells, and distributes multi-character pasted strings across the slots.

---

## C++ API Definition

### Struct Definition (`enki/widgets/otp_field.hpp`)
```cpp
namespace enki {

struct OTPFieldProps {
    int                                 length = 6;
    float                               box_size = 48.0f;
    float                               gap = 10.0f;
    float                               border_radius = 10.0f;
    bool                                is_obscured = false;
    bool                                has_error = false;
    bool                                auto_focus = false;
    std::string                         initial_value = "";

    Color                               active_border_color = 0xFF00E5FF;
    Color                               inactive_border_color = 0x33FFFFFF;
    Color                               box_background = 0x4D000000;
    Color                               text_color = 0xFFFFFFFF;
    Color                               error_color = 0xFFEF4444;

    std::function<void(const std::string&)> on_changed;
    std::function<void(const std::string&)> on_completed;

    operator WidgetPtr() const;
};

class OTPFieldWidget : public StatefulWidget {
public:
    OTPFieldProps props;

    explicit OTPFieldWidget(OTPFieldProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "OTPField"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr otpField(OTPFieldProps props) {
    return std::make_shared<OTPFieldWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `length` | `int` | `6` | Total count of digit boxes displayed. |
| `box_size` | `float` | `48.0f` | Width and height of each individual character box. |
| `gap` | `float` | `10.0f` | Spacing between adjacent boxes. |
| `border_radius` | `float` | `10.0f` | Corner rounding radius of the boxes. |
| `is_obscured` | `bool` | `false` | When `true`, masks characters as bullets instead of plain text. |
| `has_error` | `bool` | `false` | Highlights all boxes with `error_color` (e.g. for invalid codes). |
| `auto_focus` | `bool` | `false` | Automatically acquires global keyboard focus upon mounting. |
| `initial_value` | `std::string` | `""` | Initial prefilled OTP string. |
| `active_border_color` | `Color` | `0xFF00E5FF` | Border outline color for the currently focused digit box. |
| `inactive_border_color` | `Color` | `0x33FFFFFF` | Border color for unfocused digit boxes. |
| `box_background` | `Color` | `0x4D000000` | Fill color for the digit boxes. |
| `text_color` | `Color` | `0xFFFFFFFF` | Typography color for displayed digits. |
| `error_color` | `Color` | `0xFFEF4444` | Border color applied when `has_error` is true. |
| `on_changed` | `std::function<void(const std::string&)>` | `nullptr` | Callback fired on every character typed or deleted. |
| `on_completed` | `std::function<void(const std::string&)>` | `nullptr` | Callback triggered once all `length` digits have been entered. |

---

## Code Examples (From `widgets_demo/otp_field_demo/main.cpp`)

### 1. 6-Digit 2FA Verification Box
```cpp
auto otp = otpField({
    .length = 6,
    .box_size = 54.0f,
    .gap = 12.0f,
    .auto_focus = true,
    .on_changed = [this](const std::string& code) {
        current_otp_ = code;
        setState([]{});
    },
    .on_completed = [this](const std::string& code) {
        completed_otp_ = code;
        std::cout << ">>> Standalone OTP Completed: " << code << std::endl;
        setState([]{});
    },
});
```
