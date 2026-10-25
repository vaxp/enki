# Placeholder

> A scaffolding and loading widget that supports wireframe blueprints with real-time dimension tags, animated skeleton shimmers, media drop slots, and pre-composed skeleton layouts.

- **Header File**: `#include "enki/widgets/placeholder.hpp"`
- **C++ Class**: `enki::PlaceholderWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::Placeholder` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::PlaceholderProps`
- **Pre-composed Helpers**: `placeholderCardSkeleton(...)`, `placeholderListSkeleton(...)`
- **Underlying Engine**: Skia vector drawing with real-time animated gradient shimmer shaders

---

## Overview

`Placeholder` serves two key roles in modern GUI development:
1. **Design & Prototyping Scaffolding**: Draws wireframe blueprint boxes with diagonal crosshairs and real-time pixel dimension badges (`320 × 120 px`).
2. **Skeleton Loading Screens**: Renders smooth 60fps linear shimmer animations across empty placeholders while data loads over the network.
3. **Empty Media Slots**: Dashed-border containers with iconography for drag-and-drop file targets.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class PlaceholderStyle {
    Blueprint,      ///< Wireframe crosshair with diagonal X and dimension badge
    Skeleton,       ///< Smooth animated linear shimmer loader
    MediaSlot,      ///< Dashed border empty state / drop zone slot
    Solid           ///< Clean tinted scaffolding block
};

struct Placeholder {
    Key              key              = Key::none();
    PlaceholderStyle style            = PlaceholderStyle::Blueprint;

    float            width            = 200.0f;
    float            height           = 120.0f;
    float            corner_radius    = 8.0f;
    float            stroke_width     = 1.5f;

    std::string      label;                      // Custom title / slot tag
    std::string      sublabel;                   // Optional subtext
    std::string      icon             = "📷";     // Icon for MediaSlot style

    bool             show_dimensions  = true;     // Shows "W × H px" badge
    bool             animated_shimmer = true;     // Shimmer wave for skeleton mode

    // Color Customization
    Color            background_color = 0x22334155;
    Color            stroke_color     = 0xFF475569;
    Color            crosshair_color  = 0x4464748B;
    Color            shimmer_color    = 0x4438BDF8; // Highlight shimmer color
    Color            text_color       = 0xFFCBD5E1;
    Color            badge_bg_color   = 0xCC0F172A;

    std::function<void()> on_tap      = nullptr;

    operator WidgetPtr() const;
};

// Pre-composed skeleton builders
WidgetPtr placeholderCardSkeleton(float w = 280.0f);
WidgetPtr placeholderListSkeleton(int rows = 3, float w = 320.0f);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `style` | `PlaceholderStyle` | `Blueprint` | Visual mode (`Blueprint`, `Skeleton`, `MediaSlot`, `Solid`). |
| `width` | `float` | `200.0f` | Width of the placeholder box in pixels. |
| `height` | `float` | `120.0f` | Height of the placeholder box in pixels. |
| `corner_radius` | `float` | `8.0f` | Curvature radius of corners. |
| `stroke_width` | `float` | `1.5f` | Stroke width for blueprint crosshairs and dashed borders. |
| `label` | `std::string` | `""` | Title centered inside the placeholder box. |
| `sublabel` | `std::string` | `""` | Secondary subtitle or drop prompt. |
| `icon` | `std::string` | `"📷"` | Glyph or emoji rendered in `MediaSlot` style. |
| `show_dimensions` | `bool` | `true` | Renders a small badge showing exact pixel dimensions. |
| `animated_shimmer` | `bool` | `true` | Enables continuous gradient shimmer wave animation. |
| `on_tap` | `std::function<void()>` | `nullptr` | Tap callback for interactive drop slots. |

---

## Code Examples (From `widgets_demo/placeholder_demo/main.cpp`)

### 1. Blueprint Wireframe with Dimension Tag
```cpp
#include "enki/widgets/placeholder.hpp"

using namespace enki;

WidgetPtr buildChartScaffold() {
    return Placeholder {
        .style = PlaceholderStyle::Blueprint,
        .width = 400.0f,
        .height = 220.0f,
        .label = "Financial Performance Chart",
        .show_dimensions = true, // Shows badge "400 × 220 px"
    };
}
```

### 2. Media Upload Drop Slot
```cpp
auto uploadSlot = Placeholder {
    .style = PlaceholderStyle::MediaSlot,
    .width = 300.0f,
    .height = 120.0f,
    .label = "Drop Avatar or Photo Here",
    .icon = "🖼️",
    .on_tap = []() {
        // Open file picker
    }
};
```

### 3. Pre-composed Skeleton Card Loader
```cpp
// Renders an animated shimmer avatar + title + subtitle skeleton
WidgetPtr loadingCard = placeholderCardSkeleton(320.0f);

// Renders an animated shimmer 3-row list
WidgetPtr loadingList = placeholderListSkeleton(/*rows=*/4, /*width=*/360.0f);
```

---

## See Also
- [**Card**](./card.md) — The production container that placeholders typically mock.
- [**Image**](./image.md) — Media widget replaced by `MediaSlot` during loading states.
