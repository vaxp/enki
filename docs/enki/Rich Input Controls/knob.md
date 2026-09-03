# Knob (Rotary Dial)

> Studio-grade circular rotary dial input for audio gear, synthesizers, and instruments.

- **Header File**: `#include "enki/widgets/knob.hpp"`
- **C++ Class**: `enki::KnobWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Helper**: `enki::knob(KnobProps props)` (returns `enki::WidgetPtr`)
- **Render Object**: `enki::RenderKnob`
- **Underlying Mechanism**: Skia circular track with 270-degree sweep arc, center pointer needle, and vertical mouse drag gesture mapping.

---

## Overview

`Knob` is a rotary potentiometer control designed for professional audio software, synthesizers, and parameter manipulation. It supports unipolar ranges (e.g. 0 to 100%) and bipolar ranges centered at zero (e.g. -50 to +50 pan). Users adjust values by dragging vertically with sub-pixel precision.

---

## C++ API Definition

### Struct Definition (`enki/widgets/knob.hpp`)
```cpp
namespace enki {

struct KnobProps {
    float                               value = 0.0f;
    float                               min_value = 0.0f;
    float                               max_value = 100.0f;
    float                               step = 1.0f;
    float                               size = 72.0f;
    bool                                is_bipolar = false; // center 0, e.g. -50 to +50
    bool                                show_value = true;

    std::string                         label = "";
    std::string                         unit = "%";

    Color                               active_color = 0xFF00E5FF;
    Color                               track_color = 0x3300E5FF;
    Color                               dial_color = 0xFF0F172A;
    Color                               pointer_color = 0xFFFFFFFF;
    Color                               text_color = 0xFF94A3B8;

    std::function<void(float)>          on_value_changed;

    operator WidgetPtr() const;
};

class KnobWidget : public SingleChildRenderObjectWidget {
public:
    KnobProps props;

    explicit KnobWidget(KnobProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Knob"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr knob(KnobProps props) {
    return std::make_shared<KnobWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `value` | `float` | `0.0f` | Current numeric value of the dial. |
| `min_value` | `float` | `0.0f` | Minimum range bound. |
| `max_value` | `float` | `100.0f` | Maximum range bound. |
| `step` | `float` | `1.0f` | Quantization step for increments. |
| `size` | `float` | `72.0f` | Diameter of the dial control in pixels. |
| `is_bipolar` | `bool` | `false` | When `true`, progress arc originates from center top (e.g. Pan control). |
| `show_value` | `bool` | `true` | Renders value text and unit in the center of the dial. |
| `label` | `std::string` | `""` | Descriptive label displayed below the dial. |
| `unit` | `std::string` | `"%" ` | Unit suffix appended to numeric text display. |
| `active_color` | `Color` | `0xFF00E5FF` | Color of the progress sweep arc. |
| `track_color` | `Color` | `0x3300E5FF` | Color of the background circle track. |
| `dial_color` | `Color` | `0xFF0F172A` | Background color of the inner rotating disc. |
| `pointer_color`| `Color` | `0xFFFFFFFF` | Color of the pointer line indicator. |
| `text_color` | `Color` | `0xFF94A3B8` | Color of the central value and label typography. |
| `on_value_changed` | `std::function<void(float)>` | `nullptr` | Callback fired on value change during dragging. |

---

## Code Examples (From `widgets_demo/knob_demo/main.cpp`)

### 1. Volume Dial with Value Readout
```cpp
auto k1 = knob({
    .value = master_vol_,
    .min_value = 0.0f,
    .max_value = 100.0f,
    .step = 1.0f,
    .size = 80.0f,
    .label = "VOLUME",
    .unit = "%",
    .active_color = 0xFF00E5FF,
    .on_value_changed = [this](float v) {
        master_vol_ = v;
        setState([]{});
    },
});
```

### 2. Bipolar Stereo Pan Control
```cpp
auto k2 = knob({
    .value = pan_val_,
    .min_value = -50.0f,
    .max_value = 50.0f,
    .step = 1.0f,
    .size = 80.0f,
    .is_bipolar = true,
    .label = "PAN",
    .unit = "",
    .active_color = 0xFFF59E0B,
    .on_value_changed = [this](float v) {
        pan_val_ = v;
        setState([]{});
    },
});
```

### 3. Frequency Cutoff Control
```cpp
auto k3 = knob({
    .value = cutoff_val_,
    .min_value = 20.0f,
    .max_value = 20000.0f,
    .step = 50.0f,
    .size = 80.0f,
    .label = "CUTOFF",
    .unit = "Hz",
    .active_color = 0xFF10B981,
    .on_value_changed = [this](float v) {
        cutoff_val_ = v;
        setState([]{});
    },
});
```
