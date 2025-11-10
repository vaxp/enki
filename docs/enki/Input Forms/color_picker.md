# ColorPicker

> A graphic design and theming color picker widget featuring a 2D Saturation-Value canvas, Hue & Alpha gradient sliders, HEX/RGBA/HSV formats, and curated swatches.

- **Header File**: `#include "enki/widgets/color_picker.hpp"`
- **C++ Class**: `enki::ColorPickerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ColorPicker` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ColorPickerProps`
- **Controller**: `enki::ColorPickerController`
- **Enums**: `enki::ColorPickerMode`, `enki::ColorFormat`

---

## Overview

`ColorPicker` gives users full creative control over color selection. It supports:
1. **2D Saturation-Value Plane**: Interactive crosshair tracking saturation (X) and value/brightness (Y).
2. **Hue & Alpha Sliders**: Continuous rainbow hue slider and transparency alpha slider with checkered pattern.
3. **Format Support**: Seamless switching between `#RRGGBB` / `#AARRGGBB` hex, RGBA integers, and HSV models.
4. **Preset Palette**: Pre-configured palette swatches for standard application theme colors.

---

## C++ API Definition

### Controller (`ColorPickerController`)
```cpp
namespace enki {

class ColorPickerController {
public:
    void setColor(Color c);
    void open();
    void close();
    [[nodiscard]] Color getColor() const;
    [[nodiscard]] std::string getHex() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class ColorPickerMode {
    InputPopup,     ///< Clickable color well with dropdown popup
    Inline          ///< Self-contained embedded color inspector card
};

enum class ColorFormat {
    HEX,
    RGBA,
    HSV
};

struct ColorPicker {
    std::shared_ptr<ColorPickerController> controller       = nullptr;
    WidgetPtr                              body             = nullptr;

    ColorPickerMode                        mode             = ColorPickerMode::InputPopup;
    ColorFormat                            default_format   = ColorFormat::HEX;

    Color                                  initial_color    = 0xFF38BDF8; // Sky 400
    bool                                   enable_alpha     = true;
    bool                                   show_palette     = true;
    bool                                   show_comparison  = true;

    std::vector<Color>                     palette          = {
        0xFFEF4444, 0xFFF97316, 0xFFF59E0B, 0xFF10B981, 0xFF06B6D4,
        0xFF38BDF8, 0xFF3B82F6, 0xFF6366F1, 0xFFF43F5E, 0xFF000000
    };

    std::function<void(Color color)>       on_color_changed = nullptr;
    std::function<void(Color color)>       on_color_submitted = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `mode` | `ColorPickerMode` | `InputPopup` | Display mode (`InputPopup` color well or `Inline` card). |
| `initial_color` | `Color` | `0xFF38BDF8` | Default color selection. |
| `enable_alpha` | `bool` | `true` | Enables the alpha transparency channel slider. |
| `show_palette` | `bool` | `true` | Renders the swatch row of quick theme colors. |
| `show_comparison`| `bool` | `true` | Shows side-by-side Old vs New color preview box. |
| `palette` | `std::vector<Color>` | `10 colors` | List of preset color swatches shown in the picker. |
| `on_color_changed`| `std::function<void(Color)>` | `nullptr` | Live callback as the cursor or sliders move. |

---

## Code Examples (From `widgets_demo/color_picker_demo/main.cpp`)

### 1. Color Well Dropdown for Theme Setting
```cpp
#include "enki/widgets/color_picker.hpp"

using namespace enki;

auto themeColorPicker = ColorPicker {
    .mode = ColorPickerMode::InputPopup,
    .initial_color = 0xFF6366F1, // Indigo 500
    .enable_alpha = false,
    .on_color_changed = [](Color c) {
        std::cout << "Selected color: " << std::hex << c << "\n";
    }
};
```

### 2. Embedded Inline Color Inspector
```cpp
auto canvasColorInspector = ColorPicker {
    .mode = ColorPickerMode::Inline,
    .default_format = ColorFormat::RGBA,
    .initial_color = 0xFF10B981, // Emerald 500
    .enable_alpha = true,
    .on_color_changed = [](Color c) {
        // Update canvas brush color in real time
    }
};
```

---

## See Also
- [**Slider**](./slider.md) — Base component used for sliders.
- [**Card**](../Basic%20UI/card.md) — Container hosting color inspector panels.
