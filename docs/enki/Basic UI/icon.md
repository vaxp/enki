# Icon

> A vector icon widget that renders resolution-independent glyphs from Icon Fonts (Material Icons) or arbitrary SVG paths.

- **Header Files**: `#include "enki/widgets/icon.hpp"` and `#include "enki/widgets/icons_material.hpp"`
- **C++ Class**: `enki::IconWidget`
- **Declarative Struct**: `enki::Icon` (converts implicitly to `WidgetPtr`)
- **Data Source Struct**: `enki::IconData`
- **Render Object**: `enki::RenderIcon`
- **Underlying Engine**: Skia vector path and font glyph drawing with anti-aliasing

---

## Overview

`Icon` is Enki's standard icon rendering primitive. It supports:
1. **Material Design Font Glyphs**: Pre-defined in `Icons::*` (from `icons_material.hpp`).
2. **SVG Path Strings**: Arbitrary vector paths created via `IconData::svg("...")`.
3. **Automatic Size & Color Caching**: Ensures high-speed, 60+ FPS rendering without reparsing vector paths.

---

## C++ API Definition

### Data Model (`IconData`)
```cpp
namespace enki {

struct IconData {
    uint32_t    codepoint = 0;
    std::string font_family;
    std::string svg_path;

    static IconData font(uint32_t cp, std::string family);
    static IconData svg(std::string path);

    [[nodiscard]] bool isSvg() const;
    [[nodiscard]] bool empty() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Icon {
    Key      key   = Key::none();
    IconData data  = {};
    IconData icon  = {}; // Ergonomic alias
    float    size  = 24.0f;
    Color    color = 0xFFFFFFFF; // White

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<IconWidget> icon(IconData data, IconProps props = {});
inline std::shared_ptr<IconWidget> icon(IconProps props);
inline std::shared_ptr<IconWidget> icon(IconData data, float size, Color color = 0xFFFFFFFF);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `data` / `icon` | `IconData` | `{}` | The icon glyph or SVG path definition. |
| `size` | `float` | `24.0f` | Width and height bounding square in logical pixels. |
| `color` | `Color` | `0xFFFFFFFF` | 32-bit ARGB fill color for the icon. |
| `key` | `Key` | `Key::none()` | Identifier used during reconciliation. |

---

## Code Examples (From `widgets_demo/icon_demo/main.cpp`)

### 1. Using Standard Material Icons
```cpp
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icons_material.hpp"

using namespace enki;

// Direct helper
auto settingsIcon = icon(Icons::Settings, 28.0f, 0xFF38BDF8);

// Designated initializer struct
auto homeIcon = Icon {
    .icon = Icons::Home,
    .size = 24.0f,
    .color = 0xFF10B981,
};
```

### 2. Custom SVG Vector Path
```cpp
auto playButtonIcon = Icon {
    .icon = IconData::svg("M8 5v14l11-7z"),
    .size = 32.0f,
    .color = 0xFFFFFFFF,
};
```

### 3. Icon in Row with Label
```cpp
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

auto labeledIcon = row({
    .align_items = Align::Center,
    .gap = 8_px,
    .children = {
        icon(Icons::Folder, 20.0f, 0xFFF59E0B),
        text("Documents"),
    }
});
```

---

## See Also
- [**IconButton**](./icon_button.md) — Clickable button with built-in hover and ripple states for icons.
- [**Image**](./image.md) — Bitmap and raster image rendering.
