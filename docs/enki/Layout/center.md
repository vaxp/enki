# Center

> A convenience layout helper widget that centers its child both horizontally and vertically within itself.

- **Header File**: `#include "enki/widgets/container.hpp"`
- **C++ Return Type**: `std::shared_ptr<enki::ContainerWidget>`
- **Factory Helper**: `enki::centerBox(WidgetPtr child)`
- **Underlying Mechanism**: `ContainerWidget` with `.align = Alignment::Center` (`justify_content = Center`, `align_items = Center`)

---

## Overview

`Center` provides a quick and clean shorthand to center any widget inside available space. It can be invoked via the `centerBox(child)` helper function or by setting `.align = Alignment::Center` on any `Container`.

---

## C++ API Definition

```cpp
namespace enki {

/// @brief Centers the given child widget within its parent space.
inline std::shared_ptr<ContainerWidget> centerBox(WidgetPtr child) {
    return container({
        .align = Alignment::Center,
        .child = std::move(child)
    });
}

} // namespace enki
```

---

## Code Examples

### 1. Using `centerBox` Helper
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/spinner.hpp"

using namespace enki;

WidgetPtr buildLoadingScreen() {
    return centerBox(spinner());
}
```

### 2. Centering with Explicit Container Dimensions
```cpp
auto centeredCard = container({
    .width = 400_px,
    .height = 300_px,
    .color = 0xFF0F172A,
    .border_radius = BorderRadius::circular(16.0f),
    .align = Alignment::Center, // Automatically centers the child
    .child = text("Welcome to Enki", { .font_size = 18.0f }),
});
```

---

## See Also
- [**Align**](./align.md) — 9-point alignment anchor system.
- [**Container**](./container.md) — Visual box model.
