# Enki Basic UI Widgets

> Foundational visual, interactive, and structural UI elements rendered via **Skia 2D** and integrated with the **Anu Layout Engine**.

The Basic UI category provides essential graphical user interface primitives for application presentation, visual branding, user interaction, media display, and content separation. Every widget supports C++20 designated initializers, direct factory helpers, and hardware-accelerated rendering.

---

## Widget Catalog (Basic UI)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Text**](./text.md) | `class Text`, `text(...)` | `<enki/widgets/text.hpp>` | Typography display with Skia SkParagraph, ellipsis overflow, BiDi, and text selection. |
| 2 | [**RichText**](./rich_text.md) | `class RichText`, `richText(...)`, `span(...)` | `<enki/widgets/text.hpp>` | Multi-style formatted inline text runs with individual colors, weights, and click/hover events. |
| 3 | [**Icon**](./icon.md) | `struct Icon`, `class IconWidget`, `icon(...)` | `<enki/widgets/icon.hpp>` | Vector icon display supporting Material Design font glyphs and custom SVG paths. |
| 4 | [**Image**](./image.md) | `struct ImageProps`, `class ImageWidget`, `image(...)` | `<enki/widgets/image.hpp>` | Hardware-accelerated image display with in-memory caching, BoxFit scaling, and clipping. |
| 5 | [**Avatar**](./avatar.md) | `struct Avatar`, `AvatarGroup`, `AvatarWidget` | `<enki/widgets/avatar.hpp>` | Circular/rounded user profile avatar with initials fallback, online badge, and overlapping groups. |
| 6 | [**Badge**](./badge.md) | `struct Badge`, `class BadgeWidget` | `<enki/widgets/badge.hpp>` | Notification dot, counter pill, or status indicator floating over any child widget. |
| 7 | [**Divider**](./divider.md) | `struct Divider`, `divider(...)`, `DividerWidget` | `<enki/widgets/divider.hpp>` | Horizontal separating line supporting solid, dashed, dotted, and gradient styles with center label. |
| 8 | [**VerticalDivider**](./vertical_divider.md) | `struct VerticalDivider`, `verticalDivider(...)` | `<enki/widgets/divider.hpp>` | Vertical separating line for dividing adjacent columns, toolbars, and panels. |
| 9 | [**Card**](./card.md) | `struct Card`, `class CardWidget` | `<enki/widgets/card.hpp>` | Elevated content surface with background color, rounded corners, borders, and soft drop shadows. |
| 10 | [**Chip**](./chip.md) | `struct Chip`, `ChipGroup`, `ChipWidget` | `<enki/widgets/chip.hpp>` | Compact interactive token for action triggers, multi-filters, single-choice options, and deletable tags. |
| 11 | [**Button**](./button.md) | `struct Button`, `button(...)`, `ButtonWidget` | `<enki/widgets/button.hpp>` | Interactive button featuring Hover, Pressed, Disabled states, ripple animations, and SkSL shaders. |
| 12 | [**IconButton**](./icon_button.md) | `struct IconButton`, `class IconButtonWidget` | `<enki/widgets/icon_button.hpp>` | Compact circular or square clickable surface specifically tailored for icon actions. |
| 13 | [**FloatingActionButton**](./floating_action_button.md) | `struct FloatingActionButton`, `FloatingActionButtonWidget` | `<enki/widgets/floating_action_button.hpp>` | High-elevation prominent action button with circular shape and deep shadow. |
| 14 | [**Placeholder**](./placeholder.md) | `struct Placeholder`, `class PlaceholderWidget` | `<enki/widgets/placeholder.hpp>` | Scaffolding box supporting interactive blueprints, animated skeleton shimmers, and media slots. |

---

## Quick Example (Combining Basic UI Widgets)

Below is a real-world user profile card combining `Card`, `Avatar`, `Badge`, `Text`, `Button`, and `Divider`:

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/card.hpp"
#include "enki/widgets/avatar.hpp"
#include "enki/widgets/badge.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildUserProfileCard() {
    return Card {
        .elevation = 12.0f,
        .padding = StyleInsets::all(20_px),
        .child = column({
            .align_items = Align::Center,
            .gap = 12_px,
            .children = {
                // User Avatar with Online status
                Avatar {
                    .radius = 36.0f,
                    .initials = "EJ",
                    .background_color = 0xFF6366F1,
                    .show_badge = true,
                    .badge_color = 0xFF10B981, // Emerald online
                },
                
                // Name & Bio
                text("Enki Jafar", { .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                text("Lead GUI Engine Architect", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                
                // Divider with label
                Divider {
                    .style = DividerStyle::Gradient,
                    .thickness = 1.0f,
                    .color = 0xFF334155,
                },
                
                // Action Button
                Button {
                    .child = text("View Profile", { .font_weight = FontWeight::SemiBold }),
                    .on_pressed = []() { /* Navigate */ }
                }
            }
        })
    };
}
```
