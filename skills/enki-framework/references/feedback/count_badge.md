# CountBadge

> An animated numeric notification badge widget that anchors to a target child, executes smooth spring pop scale transitions whenever the number updates, and formats large numbers with overflow caps (e.g. "99+").

- **Header File**: `#include "enki/widgets/feedback_status.hpp"`
- **C++ Class**: `enki::CountBadgeWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::CountBadge` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::CountBadgeProps`
- **Factory Helpers**: `enki::countBadge()`

---

## Overview

`CountBadge` is designed for unread messages, shopping cart items, notification bells, and inbox badges. Key features include:
- **Spring Scale Transition**: When `count` increments or decrements, the badge automatically plays a lively pop-and-rebound scale animation (`animation_duration = 300ms`).
- **Overflow Thresholding**: When `count` exceeds `max_count` (default 99), it automatically formats the label to `"99+"` (or equivalent threshold).
- **Zero Suppression**: If `show_zero = false` (default), the badge automatically collapses to zero size when `count <= 0`.
- **Anchor Alignment**: Can be anchored to any corner of the child (e.g., `Alignment::TopRight` or `TopLeft`) with fine-tuning pixel `offset`.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct CountBadge {
    Key                       key                = Key::none();
    WidgetPtr                 child              = nullptr;                           ///< Anchored icon or target widget
    int                       count              = 0;                                 ///< Current numeric value
    std::optional<int>        max_count          = 99;                                ///< Capped threshold (e.g. "99+")
    bool                      show_zero          = false;                             ///< Hide badge when count is 0
    Color                     bg_color           = 0xFFEF4444;                        ///< Badge pill fill color (Red default)
    Color                     text_color         = 0xFFFFFFFF;                        ///< Numeral font color
    float                     font_size          = 11.0f;                             ///< Numeral text size
    Alignment                 alignment          = Alignment::TopRight;               ///< Anchor position on child
    Point                     offset             = {0.0f, 0.0f};                      ///< Subpixel adjustment
    std::chrono::milliseconds animation_duration = std::chrono::milliseconds(300);   ///< Spring pop transition time

    operator WidgetPtr() const;
};

inline WidgetPtr countBadge(const CountBadgeProps& props);
inline WidgetPtr countBadge(
    WidgetPtr child,
    int count,
    std::optional<int> max_count = 99,
    Color bg_color = 0xFFEF4444
);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | The base widget (icon, avatar, button) that the badge attaches to. |
| `count` | `int` | `0` | The integer count to display. |
| `max_count` | `optional<int>` | `99` | Displays `"{max}+"` when `count > max_count`. |
| `show_zero` | `bool` | `false` | If false, hides the badge completely when `count <= 0`. |
| `bg_color` | `Color` | `0xFFEF4444` | Background pill color (e.g. Danger Red). |
| `alignment` | `Alignment` | `TopRight` | Corner attachment position on the child bounding box. |
| `animation_duration`| `milliseconds` | `300ms` | Duration of the spring scale bounce animation. |

---

## Code Examples (From `widgets_demo/feedback_status_demo/main.cpp`)

### 1. Notification Bell & Shopping Cart Badges
```cpp
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildInboxNotificationIcon(int unreadCount) {
    auto inboxButton = container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .padding = EdgeInsets::all(12.0f),
        .child = text("📬 Messages", { .color = 0xFFFFFFFF, .font_weight = FontWeight::Bold })
    });

    return countBadge({
        .child = inboxButton,
        .count = unreadCount,
        .max_count = 99,          // Displays "99+" if unreadCount > 99
        .show_zero = false,       // Disappears completely if 0 messages
        .bg_color = 0xFF2563EB,   // Royal blue
        .alignment = Alignment::TopRight
    });
}
```

---

## See Also
- [**Notification**](./notification.md) — Toast banner notifications.
- [**Pulse**](./pulse.md) — Live beacon indicator animation.
- [**Ripple**](./ripple.md) — Touch ink-ripple feedback.
