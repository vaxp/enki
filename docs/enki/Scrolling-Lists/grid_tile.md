# GridTile & GridTileBar

> A composite cell widget designed for `GridView`, providing a background media slot with optional top header and bottom footer translucent scrim overlay bars.

- **Header File**: `#include "enki/widgets/grid_tile.hpp"`
- **C++ Classes**: `enki::GridTileWidget`, `enki::GridTileBarWidget` (inherit from `enki::StatelessWidget`)
- **Declarative Structs**: `enki::GridTile`, `enki::GridTileBar` (convert implicitly to `WidgetPtr`)
- **Props Structs**: `enki::GridTileProps`, `enki::GridTileBarProps`

---

## Overview

`GridTile` is modeled after Material gallery cells where an underlying image, photo, or colored swatch is overlaid with metadata. It utilizes a `Stack` layout internally, placing:
- **`child`**: Base background widget filling 100% of the cell.
- **`header`**: Optional top overlay bar (e.g. favorite star, status pill).
- **`footer`**: Optional bottom overlay bar (e.g. image title, author subtitle, download icon).

`GridTileBar` is the ready-made companion widget that provides a semi-transparent black scrim background (`background_color = 0xCC000000`) and standard text layout slots for legibility over bright background images.

---

## C++ API Definition

### `GridTileBar` Declarative Struct
```cpp
namespace enki {

struct GridTileBar {
    Key       key                = Key::none();

    WidgetPtr leading            = nullptr;
    WidgetPtr leading_widget     = nullptr;
    WidgetPtr title              = nullptr;
    WidgetPtr title_widget       = nullptr;
    WidgetPtr subtitle           = nullptr;
    WidgetPtr subtitle_widget    = nullptr;
    WidgetPtr trailing           = nullptr;
    WidgetPtr trailing_widget    = nullptr;

    Color     background_color   = 0xCC000000; ///< 80% opaque dark scrim
    float     padding_vertical   = 8.0f;
    float     padding_horizontal = 8.0f;
    float     leading_gap        = 8.0f;
    float     trailing_gap       = 8.0f;

    operator WidgetPtr() const;
};

} // namespace enki
```

### `GridTile` Declarative Struct
```cpp
namespace enki {

struct GridTile {
    Key       key    = Key::none();
    WidgetPtr child  = nullptr; ///< Base content filling the tile
    WidgetPtr header = nullptr; ///< Optional top overlay
    WidgetPtr footer = nullptr; ///< Optional bottom overlay

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

### GridTile
| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Main background content (e.g. an `ImageWidget` or thumbnail). |
| `header` | `WidgetPtr` | `nullptr` | Top banner overlay (pinned to top of tile). |
| `footer` | `WidgetPtr` | `nullptr` | Bottom banner overlay (pinned to bottom of tile). |

### GridTileBar
| Property | Type | Default | Description |
|---|---|---|---|
| `title` | `WidgetPtr` | `nullptr` | Primary heading text. |
| `subtitle` | `WidgetPtr` | `nullptr` | Secondary caption text beneath `title`. |
| `leading` | `WidgetPtr` | `nullptr` | Leading icon or avatar slot on the left. |
| `trailing` | `WidgetPtr` | `nullptr` | Trailing action or icon button on the right. |
| `background_color` | `Color` | `0xCC000000` | Scrim background color ensuring text readability. |

---

## Code Examples (From `widgets_demo/grid_tile_demo/main.cpp`)

### 1. Photo Card with Star Header and Caption Footer
```cpp
#include "enki/widgets/grid_tile.hpp"
#include "enki/widgets/image.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildPhotoCard(const std::string& titleStr, const std::string& authorStr) {
    return GridTile {
        .child = container({
            .color = 0xFF1E293B,
            .width = 100_pct,
            .height = 100_pct,
        }),
        .header = GridTileBar {
            .trailing = text("★", { .color = 0xFFFCD34D, .font_size = 15.0f }),
            .background_color = 0x88000000, // Light scrim
        },
        .footer = GridTileBar {
            .title = text(titleStr, {
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold
            }),
            .subtitle = text("by " + authorStr, {
                .color = 0xFF94A3B8,
                .font_size = 11.0f
            }),
            .background_color = 0xCC000000, // Dark scrim
        }
    };
}
```

---

## See Also
- [**GridView**](./grid_view.md) — The parent multi-column grid layout.
- [**Card**](../Basic%20UI/card.md) — Elevated surface container.
- [**Stack**](../Layout/stack.md) — Underlying positioning primitive used by `GridTile`.
