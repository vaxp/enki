# Navigator

> A stack-based page router and view coordinator managing hierarchical screen navigation with animated transitions, history stacks, and back navigation.

- **Header File**: `#include "enki/widgets/navigator.hpp"`
- **C++ Class**: `enki::NavigatorWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Navigator` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::NavigatorProps`
- **State Controller**: `enki::NavigatorState`
- **Navigation Primitives**: `Navigator::push(ctx, route)`, `Navigator::pop(ctx)`, `Navigator::canPop(ctx)`

---

## Overview

`Navigator` manages a stack of `RouteConfig` instances. The top-most route on the stack is the currently visible screen. When a new route is pushed via `Navigator::push()`, it animates into view over the existing screen. When popped via `Navigator::pop()`, it reverses its animation, returning the user to the previous screen.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Navigator {
    Key                      key                    = Key::none();
    std::vector<RouteConfig> initial_routes;
    Color                    background_color       = 0xFF0F172A;
    int                      transition_duration_ms = 300;

    // Static Navigation Methods
    static void push(BuildContext& ctx, RouteConfig route);
    static void pop(BuildContext& ctx);
    static bool canPop(BuildContext& ctx);

    operator WidgetPtr() const;
};

} // namespace enki
```

### Route Configuration (`RouteConfig`)
```cpp
namespace enki {

enum class RouteTransition {
    None,   ///< Instantaneous swap
    Slide,  ///< Slide from right on push, slide to left on pop
    Fade,   ///< Crossfade opacity transition
    Scale,  ///< Zoom scale + opacity fade
};

struct RouteConfig {
    std::string                name;
    std::function<WidgetPtr()> builder;
    RouteTransition            transition = RouteTransition::Slide;

    RouteConfig() = default;
    RouteConfig(std::string name, std::function<WidgetPtr()> builder,
                RouteTransition trans = RouteTransition::Slide);
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `initial_routes` | `std::vector<RouteConfig>` | `{}` | Initial stack of routes loaded on startup (first element is the root page). |
| `background_color` | `Color` | `0xFF0F172A` | Background color of the navigator container surface. |
| `transition_duration_ms` | `int` | `300` | Duration in milliseconds for entrance/exit animations. |

---

## Static Methods Reference

| Method | Parameters | Description |
|---|---|---|
| `Navigator::push` | `(BuildContext& ctx, RouteConfig route)` | Pushes a new route onto the stack with its transition animation. |
| `Navigator::pop` | `(BuildContext& ctx)` | Pops the current route from the stack, animating back to the previous screen. |
| `Navigator::canPop`| `(BuildContext& ctx)` | Returns `true` if there are at least two routes on the stack (safe to pop). |

---

## Code Examples (From `widgets_demo/navigator_demo/main.cpp`)

### 1. Defining the Root Navigator
```cpp
#include "enki/widgets/navigator.hpp"

using namespace enki;

WidgetPtr buildAppRoot() {
    return Navigator {
        .initial_routes = {
            RouteConfig("home", []() -> WidgetPtr {
                return std::make_shared<HomePage>();
            })
        },
        .transition_duration_ms = 250,
    };
}
```

### 2. Pushing a Detail Screen from Inside a Widget
```cpp
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

class HomePage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .child = button(text("Open Settings"), [ctx]() {
                auto c = ctx;
                Navigator::push(c, RouteConfig(
                    "settings",
                    []() -> WidgetPtr { return std::make_shared<SettingsPage>(); },
                    RouteTransition::Slide
                ));
            })
        });
    }
};
```

### 3. Popping Back to Previous Screen
```cpp
class SettingsPage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .child = button(text("Back to Home"), [ctx]() {
                auto c = ctx;
                if (Navigator::canPop(c)) {
                    Navigator::pop(c);
                }
            })
        });
    }
};
```

---

## See Also
- [**Route**](./route.md) — Route configuration and transition animations.
- [**Page**](./page.md) — Page design and view lifecycle patterns.
- [**Breadcrumb**](./breadcrumb.md) — Path breadcrumb component reflecting current route depth.
