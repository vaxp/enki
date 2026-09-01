# Stack

> A multi-child layout container that positions and layers its children along the Z-axis (2.5D multi-layered layout).

- **Header File**: `#include "enki/widgets/stack.hpp"`
- **C++ Class**: `enki::StackWidget`
- **Declarative Struct**: `enki::Stack` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::StackProps`
- **Render Object**: `enki::RenderStack`
- **Underlying Engine**: Anu Layout Engine (relative anchor container) + Skia (forward paint, reverse hit-test)

---

## Overview

`Stack` enables overlapping and layering of widgets. Children are painted in forward order (index `0` is the background, index `N-1` is the topmost foreground), while pointer hit-testing occurs in reverse order (topmost widgets receive events first).

`Stack` accommodates two kinds of children:
1. **Positioned Children** (wrapped in `Positioned`): Absolutely positioned relative to the stack boundaries via top, right, bottom, left coordinates.
2. **Non-Positioned Flow Children**: Placed and aligned according to the stack's `alignment` property (or stretched when `fit = StackFit::Expand`).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Stack {
    Key                       key           = Key::none();
    Alignment                 alignment     = Alignment::TopLeft;
    StackFit                  fit           = StackFit::Loose;
    Clip                      clip_behavior = Clip::HardEdge;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    std::vector<WidgetPtr>    children;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<StackWidget> stack(std::vector<WidgetPtr> children);
inline std::shared_ptr<StackWidget> stack(Alignment alignment, std::vector<WidgetPtr> children);
inline std::shared_ptr<StackWidget> stack(Alignment alignment, StackFit fit, std::vector<WidgetPtr> children);
inline std::shared_ptr<StackWidget> stack(StackProps props);
inline std::shared_ptr<StackWidget> stack(StackProps props, std::vector<WidgetPtr> children);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `children` | `std::vector<WidgetPtr>` | `{}` | List of layered child widgets (ordered from back to front). |
| `alignment` | `Alignment` | `Alignment::TopLeft` | Alignment applied to non-positioned children inside the stack. |
| `fit` | `StackFit` | `StackFit::Loose` | How non-positioned children are sized within incoming constraints. |
| `clip_behavior` | `Clip` | `Clip::HardEdge` | How content overflowing the stack boundary is clipped (`None`, `HardEdge`, `AntiAlias`). |
| `width` | `std::optional<StyleValue>` | `auto_val` | Explicit width constraint. |
| `height` | `std::optional<StyleValue>` | `auto_val` | Explicit height constraint. |
| `min_width` | `std::optional<StyleValue>` | `undefined_val` | Minimum width constraint. |
| `max_width` | `std::optional<StyleValue>` | `undefined_val` | Maximum width constraint. |
| `min_height` | `std::optional<StyleValue>` | `undefined_val` | Minimum height constraint. |
| `max_height` | `std::optional<StyleValue>` | `undefined_val` | Maximum height constraint. |
| `key` | `Key` | `Key::none()` | Identifier used during reconciliation. |

---

## Enums

### `Alignment`
Controls where non-positioned children are anchored:
- `Alignment::TopLeft`
- `Alignment::TopCenter`
- `Alignment::TopRight`
- `Alignment::CenterLeft`
- `Alignment::Center`
- `Alignment::CenterRight`
- `Alignment::BottomLeft`
- `Alignment::BottomCenter`
- `Alignment::BottomRight`

### `StackFit`
- `StackFit::Loose` — Children are sized naturally according to their own constraints.
- `StackFit::Expand` — Children are expanded and stretched to fill the full stack bounds.
- `StackFit::Passthrough` — Incoming constraints are passed directly through to children.

### `Clip`
- `Clip::None` — Overflowing content is visible beyond stack boundaries.
- `Clip::HardEdge` — Fast rectangle clipping without anti-aliasing.
- `Clip::AntiAlias` — Smooth anti-aliased edge clipping.
- `Clip::AntiAliasWithSaveLayer` — Anti-aliasing with an off-screen buffer layer.

---

## Code Examples

### 1. Avatar with Online Status Badge (From `widgets_demo/stack_demo/main.cpp`)
```cpp
#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildAvatarWithBadge() {
    auto avatar = container({
        .color = 0xFF312E81,
        .border_radius = BorderRadius::circular(36.0f),
        .border = Border(0xFF818CF8, 2.0f),
        .align = Alignment::Center,
        .width = 72_px,
        .height = 72_px,
        .child = text("👤", { .font_size = 32.0f }),
    });

    auto onlineDot = container({
        .color = 0xFF10B981,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(0xFF161D2F, 2.5f),
        .width = 16_px,
        .height = 16_px,
    });

    return Stack {
        .clip_behavior = Clip::None,
        .width = 76_px,
        .height = 76_px,
        .children = {
            avatar,
            Positioned {
                .right = 2_px,
                .bottom = 2_px,
                .child = onlineDot,
            }
        }
    };
}
```

### 2. Fullscreen Modal Overlay (From `widgets_demo/stack_demo/main.cpp`)
```cpp
WidgetPtr buildModalScreen(WidgetPtr mainContent, WidgetPtr dialogCard) {
    return Stack {
        .fit = StackFit::Expand,
        .children = {
            // Base layer: fills entire window
            Positioned::fill(mainContent),
            
            // Modal Backdrop overlay
            Positioned::fill(container({
                .color = 0x80000000, // 50% dimmed background
                .align = Alignment::Center,
                .child = dialogCard,
            }))
        }
    };
}
```

---

## See Also
- [**Positioned**](./positioned.md) — Absolute positioning of children inside a `Stack`.
- [**Align**](./align.md) — Alignment within single containers.
