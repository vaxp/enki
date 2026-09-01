# SizedBox

> A box with a specified width and height, commonly used to constrain a child or create fixed whitespace gaps in `Row` and `Column` layouts.

- **Header File**: `#include "enki/widgets/container.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::ContainerWidget>`
- **Factory Helper**: `enki::sizedBox(float width, float height, WidgetPtr child = nullptr)`
- **Underlying Mechanism**: Lightweight `ContainerWidget` configured with explicit point width and height dimensions.

---

## Overview

`SizedBox` is Enki's standard solution for:
1. **Fixed Spacing Gaps**: Creating fixed-pixel distances between items in a `Row` (e.g. `sizedBox(12.0f, 0.0f)`) or `Column` (e.g. `sizedBox(0.0f, 16.0f)`).
2. **Fixed Sizing Constraints**: Enforcing exact pixel width and height on a child widget without extra decoration.
3. **Empty Zero-Size Placeholders**: Serving as a no-op placeholder (e.g. `sizedBox(0.0f, 0.0f)`).

---

## C++ API Definition

```cpp
namespace enki {

/// @brief Creates a box with explicit pixel width and height.
/// @param width Width in points/pixels.
/// @param height Height in points/pixels.
/// @param child Optional child widget to constrain.
inline std::shared_ptr<ContainerWidget> sizedBox(float width, float height, WidgetPtr child = nullptr) {
    return container({
        .width = StyleValue::point(width),
        .height = StyleValue::point(height),
        .child = std::move(child)
    });
}

} // namespace enki
```

---

## Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `width` | `float` | Required | Fixed width in pixels. |
| `height` | `float` | Required | Fixed height in pixels. |
| `child` | `WidgetPtr` | `nullptr` | Optional child widget to be constrained to these dimensions. |

---

## Code Examples (From `widgets_demo/` & `examples/`)

### 1. Vertical Gaps in a Column (Pattern from `widgets_demo/gesture_demo/main.cpp`)
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildCardContent() {
    return column({
        .children = {
            text("Step 1: Configuration", { .font_weight = FontWeight::Bold }),
            sizedBox(0.0f, 6.0f), // 6px vertical gap
            text("Select your preferred graphics API.", { .color = 0xFF94A3B8 }),
            sizedBox(0.0f, 16.0f), // 16px vertical gap
            confirmButton,
        }
    });
}
```

### 2. Horizontal Spacing in a Row
```cpp
auto userRow = row({
    .align_items = Align::Center,
    .children = {
        avatarIcon,
        sizedBox(12.0f, 0.0f), // 12px horizontal gap
        userNameLabel,
    }
});
```

### 3. Sizing a Fixed-Dimension Button or Thumbnail
```cpp
auto fixedIconBox = sizedBox(48.0f, 48.0f, icon(Icons::Settings));
```

### 4. Zero-Size Conditional Placeholder
```cpp
// Returns badge if available, or zero-sized box when hidden
badgeVisible ? badgeWidget : sizedBox(0.0f, 0.0f);
```

---

## See Also
- [**Container**](./container.md) — When visual decorations (backgrounds, borders, shadows) are also needed.
- [**Spacer**](./spacer.md) — Flexible, expanding whitespace instead of fixed pixels.
- [**ConstrainedBox**](./constrained_box.md) — Min/max range constraints.
