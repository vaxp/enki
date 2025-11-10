# Enki Navigation Widgets & Routing Primitives

> Comprehensive navigation architecture, stack-based routing, animated transitions, multi-style navigation bars, tabs, collapsible sidebars, and overlay drawers for desktop applications.

The **Navigation** category provides structural and routing components for organizing user workflows across screens, views, and panes. Whether building single-window applications with multi-tier page navigation (`Navigator`, `RouteConfig`), responsive split-pane sidebars (`Sidebar`, `NavigationRail`), layered drawers (`Drawer`), or multi-view tabs (`TabBar`, `TabView`), Enki provides hardware-accelerated animations and clean declarative C++20 APIs.

---

## Widget Catalog (Navigation)

| # | Widget / Concept | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Navigator**](./navigator.md) | `struct Navigator`, `class NavigatorWidget` | `<enki/widgets/navigator.hpp>` | Stack-based route manager providing `push`, `pop`, and transition animations. |
| 2 | [**Route**](./route.md) | `struct RouteConfig`, `RouteTransition` | `<enki/widgets/navigator.hpp>` | Route descriptor defining name, lazy page builder, and transition style. |
| 3 | [**Page**](./page.md) | `StatelessWidget` / `StatefulWidget` | `<enki/widgets/navigator.hpp>` | Full-screen modular view pattern managed by the navigation stack. |
| 4 | [**TabBar**](./tab_bar.md) | `struct TabBar`, `class RenderTabBar` | `<enki/widgets/tab_bar.hpp>` | Horizontal tab strip with animated sliding indicator and badge counts. |
| 5 | [**TabView**](./tab_view.md) | `struct TabView`, `TabViewWidget` | `<enki/widgets/tab_bar.hpp>` | Lazy content switcher displaying the active tab's corresponding child. |
| 6 | [**NavigationRail**](./navigation_rail.md) | `struct NavigationRail` | `<enki/widgets/navigation_rail.hpp>` | Vertical compact side rail with collapsed (72px) and expanded (220px) states. |
| 7 | [**NavigationBar**](./navigation_bar.md) | `struct NavigationBar` | `<enki/widgets/navigation_bar.hpp>` | Multi-style bar: Bottom Standard, Floating Dock, Top Desktop Header, Segmented. |
| 8 | [**Breadcrumb**](./breadcrumb.md) | `struct Breadcrumb`, `BreadcrumbItem` | `<enki/widgets/breadcrumb.hpp>` | Hierarchical path indicator displaying clickable ancestor links and current page. |
| 9 | [**Drawer**](./drawer.md) | `struct Drawer`, `DrawerController` | `<enki/widgets/drawer.hpp>` | Slide-in overlay panel that floats above content with a dismissible scrim. |
| 10 | [**Sidebar**](./sidebar.md) | `struct Sidebar`, `SidebarWidget` | `<enki/widgets/sidebar.hpp>` | Permanent collapsible side panel that pushes and resizes adjacent content. |

---

## Navigation Architecture Comparison

Desktop applications typically combine different navigation patterns based on hierarchy:

```
┌─────────────────────────────────────────────────────────────┐
│                      Desktop Application                    │
├─────────────────┬───────────────────────────────────────────┤
│                 │ [Top Header NavigationBar / Breadcrumb]  │
│                 ├───────────────────────────────────────────┤
│   [Sidebar or   │ [TabBar Strip: Tab 1 | Tab 2 | Tab 3]     │
│ NavigationRail] ├───────────────────────────────────────────┤
│                 │                                           │
│  (Permanent or  │       [Navigator Stack / TabView]         │
│   Collapsible)  │         (Active Page Content)             │
│                 │                                           │
└─────────────────┴───────────────────────────────────────────┘
         ▲                                   ▲
         │                                   │
   Layout Pusher                   Slide-Over Overlay
   (`Sidebar`)                         (`Drawer`)
```

- **`Sidebar` vs `Drawer`**: `Sidebar` permanently allocates horizontal space in the layout tree and pushes content when expanding. `Drawer` is a modal slide-in overlay with a dark scrim that temporarily floats *on top* of the page.
- **`NavigationRail`**: A vertical icon rail (72px) optimized for high-density desktop navigation with instant expanding tooltips or flyouts.
- **`Navigator`**: A hierarchical page router with stack history (`push`, `pop`, `canPop`) and animated route transitions (`Slide`, `Fade`, `Scale`).

---

## Quick Example (Multi-View Application Skeleton)

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/sidebar.hpp"
#include "enki/widgets/navigator.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

class MainShell : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto sidebarContent = column({
            .gap = 8_px,
            .children = {
                text("Navigation", { .font_size = 14.0f, .font_weight = FontWeight::Bold }),
                button(text("Home"), [ctx]{ /* Navigate Home */ }),
                button(text("Settings"), [ctx]{ /* Navigate Settings */ }),
            }
        });

        auto bodyNavigator = Navigator {
            .initial_routes = {
                RouteConfig("home", []{
                    return text("Welcome to Home Page", { .font_size = 20.0f });
                })
            }
        };

        return Sidebar {
            .sidebar_content = sidebarContent,
            .body = bodyNavigator,
            .options = { .expanded_width = 240.0f, .collapsible = true }
        };
    }
};
```
