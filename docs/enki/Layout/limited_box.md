# LimitedBox

> A layout box that limits its dimensions only when its incoming constraints are unbounded.

- **Header File**: `#include "enki/widgets/limited_box.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class LimitedBoxWidget`, `struct LimitedBoxProps`, `using LimitedBox = LimitedBoxProps`
- **Helper Function**: `limitedBox(...)`

---

## Overview

`LimitedBox` applies maximum width or height limits to its child **only when the parent provides unconstrained (infinite) dimensions**. If the parent provides bounded constraints (such as in a normal dialog, container, or rigid column), `LimitedBox` steps aside and allows the parent's constraints to govern the layout.

This widget is critical when embedding flexible widgets inside scrolling viewports (`ScrollView`, `ListView`, or horizontal feeds) where one axis is unconstrained (`max_height = infinity` or `max_width = infinity`), preventing child widgets from expanding infinitely or collapsing.

### Key Architectural Behaviors:
- **Conditional Boundary Clamping**:
  - In an unconstrained vertical axis: if `max_height` is provided, `LimitedBox` clamps the incoming constraint from `infinity` down to `max_height`.
  - In an unconstrained horizontal axis: if `max_width` is provided, `LimitedBox` clamps `infinity` down to `max_width`.
- **Pass-Through in Bounded Contexts**: When incoming constraints are already bounded, `LimitedBox` has zero effect on sizing, ensuring natural responsive design.
- **Strict C++20 Declarative Syntax**: Supports `limitedBox({ .max_height = 200.0f, .child = ... })`.

---

## C++ API Definition

### Header: `<enki/widgets/limited_box.hpp>`

```cpp
namespace enki {

struct LimitedBoxProps {
    Key                  key        = Key::none();
    std::optional<float> max_width  = std::nullopt;
    std::optional<float> max_height = std::nullopt;
    WidgetPtr            child      = nullptr;

    operator WidgetPtr() const;
};

using LimitedBox = LimitedBoxProps;

// Declarative factory helpers:
std::shared_ptr<LimitedBoxWidget> limitedBox(LimitedBoxProps props = {});
std::shared_ptr<LimitedBoxWidget> limitedBox(WidgetPtr child,
                                            std::optional<float> max_w = std::nullopt,
                                            std::optional<float> max_h = std::nullopt,
                                            Key key = Key::none());

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and performance diffing. |
| `max_width` | `std::optional<float>` | `std::nullopt` | The maximum width applied only when width is unconstrained (`max_width == infinity`). |
| `max_height` | `std::optional<float>` | `std::nullopt` | The maximum height applied only when height is unconstrained (`max_height == infinity`). |
| `child` | `WidgetPtr` | `nullptr` | The child widget to be conditionally limited. |

---

## Real Code Examples

### 1. Clamping Height in an Unbounded Scroll View (From `widgets_demo/limited_box_demo/main.cpp`)
Inside a vertical `scrollView`, height is unconstrained. `LimitedBox` prevents the card from taking 320px, clamping it neatly to 160px:

```cpp
#include "enki/widgets/limited_box.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildLimitedCardItem(int index, Color accentColor, float maxHeightLimit) {
    std::string id_str = std::to_string(index);

    return limitedBox({
        .key        = Key::string("lbox_item_" + id_str),
        .max_height = maxHeightLimit, // E.g. 160.0f
        .child      = container({
            .color         = 0xFF172033,
            .border_radius = BorderRadius::circular(10.0f),
            .border        = Border(accentColor, 1.5f),
            .width         = StyleValue::percent(100.0f),
            .height        = StyleValue::point(320.0f), // Requests 320px, but LimitedBox limits it to 160px!
            .padding       = StyleInsets::all(12.0f),
            .child         = column({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    text({
                        .text        = "FEED ITEM #" + id_str,
                        .color       = accentColor,
                        .font_size   = 12.0f,
                        .font_weight = FontWeight::Bold,
                        .key         = Key::string("txt_title_" + id_str),
                    }),
                    text({
                        .text      = "LimitedBox prevents unbounded overflow in scrolling feeds.",
                        .color     = 0xFF94A3B8,
                        .font_size = 11.0f,
                        .key       = Key::string("txt_desc_" + id_str),
                    }),
                },
                .key = Key::string("col_card_" + id_str),
            }),
            .key = Key::string("box_card_" + id_str),
        }),
    });
}
```

### 2. Dual-Axis Limit in Reusable Component
```cpp
auto adaptiveThumbnail = limitedBox({
    .max_width  = 200.0f, // Active if placed in an unconstrained horizontal Row/ScrollView
    .max_height = 150.0f, // Active if placed in an unconstrained vertical Column/ScrollView
    .child      = myThumbnailWidget,
});
```

---

## See Also
- [**ConstrainedBox**](./constrained_box.md) — Unconditionally imposes min/max boundaries.
- [**OverflowBox**](./overflow_box.md) — Renders beyond parent bounds.
- [**ScrollView**](../Scrolling-Lists/scroll_view.md) — Scrolling viewport that provides unbounded constraints.
