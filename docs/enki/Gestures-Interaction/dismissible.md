# Dismissible

> A swipe-to-dismiss row widget supporting horizontal and vertical swipe gestures, dual action backgrounds (e.g. swipe-right to archive, swipe-left to delete), dismissal confirmation, and threshold animations.

- **Header File**: `#include "enki/widgets/dismissible.hpp"`
- **C++ Class**: `enki::DismissibleWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Dismissible` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DismissibleProps`
- **Direction Enum**: `enki::DismissDirection`

---

## Overview

`Dismissible` wraps list items, cards, or notifications with swipe-away capabilities. As the user drags the item horizontally or vertically beyond a configurable threshold (`dismiss_threshold`, default `0.35f`), the underlying background widget is progressively revealed. Upon release, the row collapses smoothly, firing `on_dismissed` to let the parent state delete or archive the corresponding record.

---

## C++ API Definition

### `DismissDirection` Enum
```cpp
namespace enki {

enum class DismissDirection {
    Horizontal,     ///< Can be dismissed by swiping left or right
    EndToStart,     ///< Can only be dismissed by swiping left (RTL / Delete action)
    StartToEnd,     ///< Can only be dismissed by swiping right (LTR / Archive action)
    Vertical        ///< Can be dismissed by swiping up or down
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Dismissible {
    Key                                      key                  = Key::none();
    WidgetPtr                                child                = nullptr;
    std::string                              id                   = "";      ///< Unique identity string

    DismissDirection                         direction            = DismissDirection::Horizontal;
    float                                    dismiss_threshold    = 0.35f;   ///< Fraction of width needed (0.35 = 35%)

    WidgetPtr                                background           = nullptr; ///< Revealed when swiping right (StartToEnd)
    WidgetPtr                                secondary_background = nullptr; ///< Revealed when swiping left (EndToStart)

    // Callbacks
    std::function<void(DismissDirection dir)> on_dismissed        = nullptr;
    std::function<bool(DismissDirection dir)> confirm_dismiss     = nullptr; ///< Return false to abort dismissal
    std::function<void()>                     on_resize           = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `id` | `std::string` | `""` | Unique key identifying this dismissible instance in the list. |
| `child` | `WidgetPtr` | `nullptr` | The front foreground widget that moves with the user's swipe. |
| `direction` | `DismissDirection` | `Horizontal` | Permitted swipe axes (`Horizontal`, `StartToEnd`, `EndToStart`, `Vertical`). |
| `dismiss_threshold`| `float` | `0.35f` | Minimum swipe fraction required to commit dismissal rather than snapping back. |
| `background` | `WidgetPtr` | `nullptr` | Background displayed when swiping in the positive direction (left-to-right). |
| `secondary_background`| `WidgetPtr` | `nullptr` | Background displayed when swiping in the negative direction (right-to-left). |
| `confirm_dismiss`| `function<bool(dir)>`| `nullptr` | Optional predicate that can prompt a dialog or veto dismissal. |
| `on_dismissed` | `function<void(dir)>`| `nullptr` | Callback executed when the item has completed dismissal. |

---

## Code Examples (From `widgets_demo/gesture_suite_demo/main.cpp`)

### 1. Dual-Action Swipeable Notification Row
```cpp
#include "enki/widgets/dismissible.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildDismissibleNotification(int id, const std::string& message,
                                       std::function<void(int, bool)> onRemove) {
    // 1. Foreground Content Card
    auto contentCard = container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(0xFF334155, 1.0f),
        .padding = EdgeInsets::all(14.0f),
        .child = text(message, { .color = 0xFFF1F5F9 })
    });

    // 2. Swiping Right: Green "Archive" Background
    auto archiveBg = container({
        .color = 0x3310B981, // Emerald tint
        .border_radius = BorderRadius::circular(8.0f),
        .padding = EdgeInsets::symmetric(14.0f, 20.0f),
        .child = text("📥 Archive", { .color = 0xFF10B981, .font_weight = FontWeight::Bold })
    });

    // 3. Swiping Left: Red "Delete" Background
    auto deleteBg = container({
        .color = 0x33EF4444, // Crimson tint
        .border_radius = BorderRadius::circular(8.0f),
        .padding = EdgeInsets::symmetric(14.0f, 20.0f),
        .child = text("🗑️ Delete", { .color = 0xFFEF4444, .font_weight = FontWeight::Bold })
    });

    return Dismissible {
        .child = contentCard,
        .id = "notif_" + std::to_string(id),
        .direction = DismissDirection::Horizontal,
        .background = archiveBg,
        .secondary_background = deleteBg,
        .on_dismissed = [id, onRemove](DismissDirection dir) {
            bool isArchive = (dir == DismissDirection::StartToEnd);
            onRemove(id, isArchive);
        }
    };
}
```

---

## See Also
- [**Draggable**](./draggable.md) — Freeform drag-and-drop source widgets.
- [**GestureDetector**](./gesture_detector.md) — Low-level pan and swipe recognizers.
- [**ListView**](../Scrolling-Lists/list_view.md) — Scrollable lists of dismissible rows.
