# Route (`RouteConfig` & `RouteTransition`)

> The route descriptor and transition animation specification in Enki's navigation system.

- **Header File**: `#include "enki/widgets/navigator.hpp"`
- **C++ Struct**: `enki::RouteConfig`
- **Transition Enum**: `enki::RouteTransition` (`None`, `Slide`, `Fade`, `Scale`)

---

## Overview

A **Route** represents an entry in the navigation history stack. It encapsulates:
1. **Semantic Route Name (`name`)**: A string tag identifying the view (e.g. `"home"`, `"profile"`, `"cart/checkout"`).
2. **Lazy Page Builder (`builder`)**: A factory callback `std::function<WidgetPtr()>` that lazily instantiates the target page widget only when pushed.
3. **Transition Animation (`transition`)**: The animation curve applied during entrance and exit.

---

## C++ API Definition

### `RouteTransition` Enum
```cpp
namespace enki {

enum class RouteTransition {
    None,   ///< Instantaneous swap without animation (0ms)
    Slide,  ///< Horizontal slide: enters from right on push, exits to right on pop
    Fade,   ///< Smooth alpha opacity crossfade
    Scale   ///< Subtle zoom expansion paired with opacity fade
};

} // namespace enki
```

### `RouteConfig` Struct
```cpp
namespace enki {

struct RouteConfig {
    std::string                name;
    std::function<WidgetPtr()> builder;
    RouteTransition            transition = RouteTransition::Slide;

    RouteConfig() = default;

    RouteConfig(std::string name,
                std::function<WidgetPtr()> builder,
                RouteTransition trans = RouteTransition::Slide)
        : name(std::move(name)), builder(std::move(builder)), transition(trans) {}
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `name` | `std::string` | `""` | Identifier for the route, useful for logging, analytics, and deep linking. |
| `builder` | `std::function<WidgetPtr()>` | `nullptr` | Lazy factory lambda returning the `WidgetPtr` to be mounted. |
| `transition` | `RouteTransition` | `RouteTransition::Slide`| Entrance and exit visual animation style. |

---

## Code Examples (From `widgets_demo/navigator_demo/main.cpp`)

### 1. Slide Route for Master-Detail Workflow
```cpp
#include "enki/widgets/navigator.hpp"

using namespace enki;

auto detailRoute = RouteConfig(
    "item_details",
    []() -> WidgetPtr {
        return std::make_shared<ItemDetailsPage>(42);
    },
    RouteTransition::Slide
);

Navigator::push(ctx, detailRoute);
```

### 2. Fade Route for Modal and Settings Overlays
```cpp
auto settingsRoute = RouteConfig(
    "settings_dialog",
    []() -> WidgetPtr {
        return std::make_shared<PreferencesPage>();
    },
    RouteTransition::Fade
);

Navigator::push(ctx, settingsRoute);
```

### 3. Instant Route Swap
```cpp
auto logoutRoute = RouteConfig(
    "login_screen",
    []() -> WidgetPtr {
        return std::make_shared<LoginPage>();
    },
    RouteTransition::None
);

Navigator::push(ctx, logoutRoute);
```

---

## See Also
- [**Navigator**](./navigator.md) — The stack-based router hosting the routes.
- [**Page**](./page.md) — The full-screen views produced by `builder`.
