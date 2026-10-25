# NumberField

> A high-precision numeric input widget with steppers, horizontal mouse drag scrubbing, math expression evaluation, unit prefixes/suffixes, and currency formatting.

- **Header File**: `#include "enki/widgets/number_field.hpp"`
- **C++ Class**: `enki::NumberFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::NumberField` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::NumberFieldProps`
- **Controller**: `enki::NumberFieldController`
- **Enums**: `enki::NumberFieldStepperPosition`, `enki::NumberFieldSize`, `enki::NumberFieldWrapMode`

---

## Overview

`NumberField` provides specialized numeric handling beyond generic text boxes. It supports:
1. **Interactive Stepper Buttons**: Stacked (▲/▼), split sides (−/+), or horizontal buttons.
2. **Mouse Drag Scrubbing**: Dragging horizontally adjusts values rapidly (Blender/Figma style).
3. **In-place Math Expressions**: Evaluates expressions typed by the user (e.g. `1920 / 2`, `100 + 25`).
4. **Units & Currencies**: Pre-configured prefix (`"$ "`, `"€ "`) and suffix (`" px"`, `" %"`, `" MB"`) decorations.
5. **Precision & Clamping**: Strict min/max bounds and decimal precision controls.

---

## C++ API Definition

### Controller (`NumberFieldController`)
```cpp
namespace enki {

class NumberFieldController {
public:
    NumberFieldController(double initial_val = 0.0);

    [[nodiscard]] double getValue() const;
    void setValue(double val, bool save_undo = true);

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    bool undo();
    bool redo();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class NumberFieldStepperPosition {
    RightVertical,   ///< Stacked ▲ / ▼ arrows on the right
    Sides,           ///< − on the left, + on the right
    RightHorizontal, ///< − + side-by-side on the right
    None             ///< Clean input without stepper buttons
};

struct NumberField {
    Key                                    key                      = Key::none();
    std::shared_ptr<NumberFieldController> controller               = nullptr;
    double                                 initial_value            = 0.0;

    // Bounds & Step Sizes
    double                                 min_value                = -std::numeric_limits<double>::infinity();
    double                                 max_value                = std::numeric_limits<double>::infinity();
    double                                 step                     = 1.0;
    double                                 large_step               = 10.0; // With Shift key
    double                                 fine_step                = 0.1;  // With Alt key
    int                                    precision                = 0;    // 0 for int, > 0 for decimals

    // Feature Flags
    bool                                   allow_decimals           = true;
    bool                                   allow_negative           = true;
    bool                                   allow_expressions        = true; // Evaluates "100+50"
    bool                                   enable_scrubbing         = true; // Drag to adjust
    bool                                   enable_auto_repeat       = true; // Auto-repeat stepping on hold
    bool                                   show_thousands_separator = false;

    NumberFieldStepperPosition             stepper_position         = NumberFieldStepperPosition::RightVertical;
    NumberFieldSize                        size                     = NumberFieldSize::Medium;
    NumberFieldWrapMode                    wrap_mode                = NumberFieldWrapMode::Clamp;

    // Prefixes & Suffixes
    std::string                            prefix_text              = ""; // e.g. "$ "
    std::string                            suffix_text              = ""; // e.g. " px"

    // Callbacks
    std::function<void(double)>            on_changed               = nullptr;
    std::function<void(double)>            on_submitted             = nullptr;
    std::function<std::string(double)>     custom_formatter         = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `controller` | `std::shared_ptr<NumberFieldController>` | `nullptr` | Shared controller managing the double numeric value. |
| `initial_value` | `double` | `0.0` | Initial value if no controller is provided. |
| `min_value` / `max_value` | `double` | `±inf` | Allowed value range. Values exceeding bounds are clamped. |
| `step` | `double` | `1.0` | Delta increment when clicking steppers or pressing Arrow keys. |
| `precision` | `int` | `0` | Number of decimal places displayed (0 enforces integers). |
| `allow_expressions`| `bool` | `true` | Allows typing math arithmetic (`1920/2` evaluates to `960`). |
| `enable_scrubbing` | `bool` | `true` | Enables horizontal mouse click-and-drag value adjustment. |
| `stepper_position` | `NumberFieldStepperPosition` | `RightVertical` | Layout position of `+` / `-` buttons. |
| `prefix_text` | `std::string` | `""` | Leading label inside the field (e.g. `"$ "`). |
| `suffix_text` | `std::string` | `""` | Trailing unit label inside the field (e.g. `" px"`). |

---

## Code Examples (From `widgets_demo/number_field_demo/main.cpp`)

### 1. Currency Input with Thousands Separator
```cpp
#include "enki/widgets/number_field.hpp"

using namespace enki;

auto priceField = NumberField {
    .initial_value = 1250.0,
    .min_value = 0.0,
    .step = 50.0,
    .precision = 2,
    .show_thousands_separator = true,
    .prefix_text = "$ ",
    .on_changed = [](double val) {
        std::cout << "Price: " << val << "\n";
    }
};
```

### 2. Dimension Field with Math Expression Support
```cpp
auto widthField = NumberField {
    .initial_value = 1920.0,
    .min_value = 100.0,
    .max_value = 7680.0,
    .step = 10.0,
    .allow_expressions = true, // User can type "1920 / 2"
    .suffix_text = " px",
};
```

### 3. Percentage Scrubbing Slider
```cpp
auto opacityField = NumberField {
    .initial_value = 100.0,
    .min_value = 0.0,
    .max_value = 100.0,
    .step = 1.0,
    .enable_scrubbing = true, // Drag horizontally to adjust
    .suffix_text = " %",
};
```

---

## See Also
- [**Slider**](./slider.md) — Horizontal bar slider for visual numeric selection.
- [**TextField**](./text_field.md) — Standard alphanumeric input.
