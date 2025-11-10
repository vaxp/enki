# SliverAppBar

> A collapsible, expandable, and pinnable header sliver designed for `CustomScrollView`, featuring a flexible hero space, action buttons, and pinned top navigation.

- **Header File**: `#include "enki/widgets/sliver.hpp"`
- **C++ Class**: `enki::SliverAppBarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::SliverAppBar` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::SliverAppBarProps`
- **Factory Helper**: `enki::sliverAppBar(props)`

---

## Overview

`SliverAppBar` integrates directly into a `CustomScrollView`'s sliver pipeline. When the user scrolls down, the app bar smoothly collapses from its `expanded_height` down to its `collapsed_height`. If `pinned = true`, the bar stays anchored at the top of the screen; if `floating = true`, it immediately snaps back into view as soon as the user scrolls upwards, regardless of the current scroll offset.

---

## C++ API Definition

### Declarative Struct & Factory Function
```cpp
namespace enki {

struct SliverAppBar {
    Key                    key              = Key::none();
    WidgetPtr              title            = nullptr;          ///< Primary header title
    WidgetPtr              leading          = nullptr;          ///< Back button or menu icon
    std::vector<WidgetPtr> actions          = {};              ///< Trailing toolbar actions
    WidgetPtr              flexible_space   = nullptr;          ///< Hero image or expanded content
    WidgetPtr              bottom           = nullptr;          ///< Fixed sub-toolbar (e.g. TabBar)
    float                  expanded_height  = 200.0f;           ///< Height when fully uncollapsed
    float                  collapsed_height = 56.0f;            ///< Resting toolbar height when pinned
    bool                   pinned           = true;             ///< Keep toolbar visible at top
    bool                   floating         = false;            ///< Reveal immediately on scroll up
    bool                   snap             = false;            ///< Snap fully open when floated
    bool                   center_title     = false;
    Color                  background_color = 0xFF1E293B;
    Color                  foreground_color = 0xFFFFFFFF;
    float                  elevation        = 4.0f;

    operator WidgetPtr() const;
};

inline std::shared_ptr<SliverAppBarWidget> sliverAppBar(SliverAppBarProps props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `title` | `WidgetPtr` | `nullptr` | Header title widget (fades into primary position when collapsed). |
| `leading` | `WidgetPtr` | `nullptr` | Left-side leading icon button (e.g. back navigation chevron). |
| `actions` | `vector<WidgetPtr>`| `{}` | Right-aligned action buttons or controls. |
| `flexible_space` | `WidgetPtr` | `nullptr` | Background hero banner that stretches and compresses with scroll. |
| `expanded_height`| `float` | `200.0f` | Height of the app bar when scrolled to the very top. |
| `collapsed_height`| `float`| `56.0f` | Minimum height when collapsed and pinned to the screen top. |
| `pinned` | `bool` | `true` | Retains the toolbar visible at `collapsed_height` during scroll. |
| `floating` | `bool` | `false` | Animates the bar back into view as soon as user scrolls backwards. |

---

## Code Examples (From `widgets_demo/sliver_demo/main.cpp`)

### 1. Collapsible Hero App Bar with Flexible Space
```cpp
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildHeroSliverAppBar() {
    auto heroFlexibleSpace = container({
        .color = 0xFF1E1B4B, // Deep indigo
        .align = Alignment::BottomLeft,
        .padding = EdgeInsets::all(20.0f),
        .child = column({
            .gap = 6_px,
            .children = {
                text("Project Analytics", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                text("Real-time GPU engine metrics and server telemetry", { .color = 0xFFA5B4FC, .font_size = 13.0f })
            }
        })
    });

    return SliverAppBar {
        .title = text("Analytics Dashboard", { .color = 0xFFFFFFFF, .font_weight = FontWeight::Bold }),
        .leading = button(text("◄"), []{}),
        .flexible_space = heroFlexibleSpace,
        .expanded_height = 180.0f,
        .collapsed_height = 56.0f,
        .pinned = true, // Remains pinned as a 56px toolbar
        .elevation = 6.0f
    };
}
```

---

## See Also
- [**CustomScrollView**](./custom_scroll_view.md) — Viewport hosting sliver app bars.
- [**SliverList**](./sliver_list.md) — 1D linear list sliver.
- [**NavigationBar**](../Navigation/navigation_bar.md) — Fixed standard navigation bar.
