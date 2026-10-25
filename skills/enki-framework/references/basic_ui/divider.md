# Divider

> A thin horizontal line that visually groups and separates content in lists, cards, and forms.

- **Header File**: `#include "enki/widgets/divider.hpp"`
- **C++ Class**: `enki::DividerWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::Divider` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DividerProps`
- **Helper Function**: `enki::divider(DividerProps props = {})`
- **Underlying Engine**: Skia 2D canvas drawing with direct line path strokes

---

## Overview

`Divider` draws a horizontal separating rule with support for:
1. **Line Styles**: `Solid`, `Dashed`, `Dotted`, and `Gradient` fade.
2. **Center Labels**: Optional text centered along the divider (e.g. `"OR"`, `"Section Break"`).
3. **Caps & Insets**: Leading (`indent`) and trailing (`end_indent`) margin offsets and rounded end caps (`round_caps`).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Divider {
    Key          key             = Key::none();
    float        height          = 16.0f;          // Total bounding box height
    float        thickness       = 1.0f;           // Stroke thickness of the line
    float        indent          = 0.0f;           // Leading empty space (left)
    float        end_indent      = 0.0f;           // Trailing empty space (right)
    Color        color           = 0xFF334155;     // Slate line color

    DividerStyle style           = DividerStyle::Solid;

    // Dashed / Dotted settings
    float        dash_length     = 6.0f;
    float        dash_gap        = 4.0f;

    // Gradient fade at both ends
    bool         gradient_fade   = false;

    // Center Label
    std::string  label           = "";
    float        label_font_size = 11.5f;
    Color        label_color     = 0xFF64748B;
    float        label_padding   = 10.0f;
    Color        label_bg_color  = 0xFF0F172A;

    // Rounded caps
    bool         round_caps      = false;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Function
```cpp
namespace enki {

inline std::shared_ptr<DividerWidget> divider(DividerProps props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `height` | `float` | `16.0f` | Total vertical space allocated for the divider widget. |
| `thickness` | `float` | `1.0f` | Thickness of the drawn line in pixels. |
| `indent` | `float` | `0.0f` | Empty space to the left of the divider line. |
| `end_indent` | `float` | `0.0f` | Empty space to the right of the divider line. |
| `color` | `Color` | `0xFF334155` | Line color (32-bit ARGB). |
| `style` | `DividerStyle` | `DividerStyle::Solid` | Line pattern (`Solid`, `Dashed`, `Dotted`, `Gradient`). |
| `dash_length` | `float` | `6.0f` | Length of each dash segment. |
| `dash_gap` | `float` | `4.0f` | Space between dash/dot segments. |
| `label` | `std::string` | `""` | Optional text displayed at the center of the divider. |
| `label_bg_color` | `Color` | `0xFF0F172A` | Background behind label text (should match parent card/window bg). |
| `round_caps` | `bool` | `false` | When true, renders rounded endpoints on line segments. |

---

## `DividerStyle` Enum

- `DividerStyle::Solid` — Continuous solid line stroke.
- `DividerStyle::Dashed` — Segmented dashes spaced by `dash_gap`.
- `DividerStyle::Dotted` — Compact dotted sequence.
- `DividerStyle::Gradient` — Smoothly fades from transparent at edges to solid color at center.

---

## Code Examples (From `widgets_demo/divider_demo/main.cpp`)

### 1. Default Solid & Accent Dividers
```cpp
#include "enki/widgets/divider.hpp"

using namespace enki;

// Standard subtle divider
auto standardLine = Divider { .thickness = 1.0f, .color = 0xFF334155 };

// Highlighted accent line
auto accentLine = Divider { .thickness = 2.0f, .color = 0xFF38BDF8 };
```

### 2. Login Form Divider with Center Label ("OR")
```cpp
auto orDivider = Divider {
    .thickness = 1.0f,
    .color = 0xFF475569,
    .label = "OR",
    .label_font_size = 11.0f,
    .label_color = 0xFF94A3B8,
    .label_bg_color = 0xFF1E293B, // Matches card background
};
```

### 3. Gradient Fade with Indents
```cpp
auto elegantBreak = Divider {
    .thickness = 2.0f,
    .indent = 30.0f,
    .end_indent = 30.0f,
    .color = 0xFF8B5CF6, // Violet
    .style = DividerStyle::Gradient,
};
```

---

## See Also
- [**VerticalDivider**](./vertical_divider.md) — Vertical counterpart for side-by-side dividing.
- [**Card**](./card.md) — Surfaces where dividers are commonly placed.
