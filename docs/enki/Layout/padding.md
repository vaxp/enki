# Padding

> A widget that insets its child by specified distances on each edge.

- **Header Files**: `#include "enki/widgets/container.hpp"` and `#include "enki/core/types.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::ContainerWidget>`
- **Factory Helper**: `enki::paddingBox(EdgeInsets insets, WidgetPtr child)`
- **Declarative Property**: `.padding` on `Container`, `Row`, `Column`, and `FlexItem`

---

## Overview

In Enki, padding can be applied in two primary ways:
1. **Dedicated Helper Widget (`paddingBox`)**: Wraps any child widget in a standalone padding box.
2. **Container / Flex Inset Property (`.padding`)**: Directly set on a `Container`, `Row`, `Column`, or `Flexbox` without nesting extra elements in the widget tree.

---

## C++ API Definition

### Standalone Helper Function
```cpp
namespace enki {

/// @brief Wraps a child widget with padding insets.
inline std::shared_ptr<ContainerWidget> paddingBox(EdgeInsets insets, WidgetPtr child) {
    return container({
        .padding = StyleInsets::only(insets.top, insets.right, insets.bottom, insets.left),
        .child = std::move(child)
    });
}

} // namespace enki
```

### Inset Construction Types

#### `EdgeInsets` (Floating-point values)
```cpp
EdgeInsets::all(float v);
EdgeInsets::symmetric(float vertical, float horizontal);
EdgeInsets::only(float top = 0, float right = 0, float bottom = 0, float left = 0);
EdgeInsets::fromLTRB(float left, float top, float right, float bottom);
EdgeInsets::directional(float top, float bottom, float start, float end);
```

#### `StyleInsets` (Dimension values with `StyleValue` / literals)
```cpp
StyleInsets::all(StyleValue all);
StyleInsets::symmetric(StyleValue vertical, StyleValue horizontal);
StyleInsets::only(StyleValue t = {}, StyleValue r = {}, StyleValue b = {}, StyleValue l = {});
StyleInsets::directional(StyleValue t, StyleValue b, StyleValue s, StyleValue e);
```

---

## Code Examples

### 1. Using `paddingBox` Helper
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildPaddedText() {
    return paddingBox(
        EdgeInsets::symmetric(12.0f, 24.0f),
        text("Important Notice", { .font_weight = FontWeight::Bold })
    );
}
```

### 2. Inset on `Container` via `StyleInsets`
```cpp
auto card = container({
    .color = 0xFF1E293B,
    .border_radius = BorderRadius::circular(8.0f),
    .padding = StyleInsets::all(16_px),
    .child = myContent,
});
```

### 3. Asymmetric Padding (`EdgeInsets::only`)
```cpp
auto topPaddedSection = paddingBox(
    EdgeInsets::only(24.0f, 0.0f, 8.0f, 0.0f), // 24px top, 8px bottom
    contentWidget
);
```

---

## See Also
- [**Container**](./container.md) — Comprehensive visual container with embedded padding.
- [**SizedBox**](./sized_box.md) — Whitespace gaps between adjacent items.
