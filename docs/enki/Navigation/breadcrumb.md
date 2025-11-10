# Breadcrumb

> A hierarchical path indicator widget displaying clickable ancestor links, custom separators, and an active leaf node label to communicate location within an application's hierarchy.

- **Header File**: `#include "enki/widgets/breadcrumb.hpp"`
- **C++ Class**: `enki::BreadcrumbWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::Breadcrumb` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::BreadcrumbProps`
- **Item Descriptor**: `enki::BreadcrumbItem`

---

## Overview

`Breadcrumb` represents the hierarchical navigation path of the user's current context (e.g. `Home / Workspaces / Enki / include`). Ancestor items are rendered with clickable hover highlights (`on_tap`), while the final leaf item automatically renders with `active_color` and bold styling to signify the current view.

---

## C++ API Definition

### `BreadcrumbItem` Descriptor
```cpp
namespace enki {

struct BreadcrumbItem {
    std::string           label;
    std::function<void()> on_tap = nullptr; ///< Click callback (or nullptr for non-clickable)
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Breadcrumb {
    Key                         key                 = Key::none();
    std::vector<BreadcrumbItem> items;

    Color                       active_color        = 0xFFF1F5F9; ///< Current item color
    Color                       inactive_color      = 0xFF64748B; ///< Ancestor items color
    Color                       hover_color         = 0xFF818CF8; ///< Hover tint on clickable links
    Color                       separator_color     = 0xFF475569;

    std::string                 separator           = "/";
    float                       font_size           = 13.0f;
    float                       separator_font_size = 12.0f;
    float                       item_spacing        = 8.0f;
    float                       separator_spacing   = 8.0f;
    bool                        bold_active         = true;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `std::vector<BreadcrumbItem>` | `{}` | Sequence of path items from root ancestor to active leaf. |
| `separator` | `std::string` | `"/"` | Delimiter string between items (e.g. `"/"`, `">"`, `"›"`). |
| `active_color` | `Color` | `0xFFF1F5F9` | Color applied to the current (last) item. |
| `inactive_color` | `Color` | `0xFF64748B` | Color applied to clickable ancestor links. |
| `hover_color` | `Color` | `0xFF818CF8` | Highlight tint when hovering clickable items. |
| `bold_active` | `bool` | `true` | When true, renders the current page label in bold font weight. |

---

## Code Examples (From `widgets_demo/breadcrumb_demo/main.cpp`)

### 1. Dynamic File Browser Breadcrumb
```cpp
#include "enki/widgets/breadcrumb.hpp"

using namespace enki;

WidgetPtr buildPathTrail(const std::vector<std::string>& pathSegments, auto onNavigateToSegment) {
    std::vector<BreadcrumbItem> items;
    for (size_t i = 0; i < pathSegments.size(); ++i) {
        bool isLast = (i == pathSegments.size() - 1);
        items.push_back(BreadcrumbItem {
            .label = pathSegments[i],
            .on_tap = isLast ? nullptr : [onNavigateToSegment, i]() {
                onNavigateToSegment(i);
            }
        });
    }

    return Breadcrumb {
        .items = std::move(items),
        .separator = "›",
        .separator_color = 0xFF475569,
        .active_color = 0xFF38BDF8, // Sky blue for current folder
    };
}
```

---

## See Also
- [**Navigator**](./navigator.md) — The stack-based router managing page history.
- [**NavigationBar**](./navigation_bar.md) — Top header bar hosting breadcrumbs.
