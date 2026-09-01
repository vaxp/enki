# Page Architecture & View Lifecycle

> The architecture, design conventions, and lifecycle management for full-screen pages and views navigated via Enki's `Navigator`.

- **Associated Header**: `#include "enki/widgets/navigator.hpp"`
- **Base Classes**: `enki::StatelessWidget` or `enki::StatefulWidget`
- **Instantiation Method**: `RouteConfig("name", []() -> WidgetPtr { return std::make_shared<MyPage>(); })`
- **Context Injection**: `BuildContext& ctx` (used to invoke `Navigator::pop(ctx)` and `Navigator::push(ctx)`)

---

## Overview

In Enki, a **Page** is not an esoteric low-level primitive, but a full-screen, self-contained `Widget` (typically a `StatelessWidget` or `StatefulWidget`) mounted as the visual payload of a `RouteConfig`. 

Pages encapsulate:
1. **Full-Viewport Dimensions**: Typically root containers configured with `.width = 100_pct` and `.height = 100_pct`.
2. **Context-Aware Navigation**: Buttons and gestures inside the page use the injected `BuildContext& ctx` to manipulate the navigator stack (`Navigator::push`, `Navigator::pop`).
3. **Dedicated View Lifecycle**: Stateful pages manage their own state machines, background network requests, and timer subscriptions independently.

---

## Page Anatomy & Standard Layout

A production-grade Enki page commonly adheres to the following structural hierarchy:

```cpp
┌─────────────────────────────────────────────────────────────┐
│ Header / App Bar: Back Button [←] | Page Title | Actions    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│                                                             │
│ Scrollable Body: Content, Forms, Lists, or Detail Cards    │
│                                                             │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ Footer Actions / Status Bar: Primary Submit | Cancel Button │
└─────────────────────────────────────────────────────────────┘
```

---

## Lifecycle Stages (for `StatefulWidget` Pages)

| Phase | State Method | Description |
|---|---|---|
| **Mount** | `initState()` | Triggered when the route is pushed onto the `Navigator` stack. Used to allocate controllers, fetch initial data, and bind listeners. |
| **Render** | `build(BuildContext& ctx)` | Evaluated on initial transition and whenever `setState()` is invoked inside the page. |
| **Unmount** | `dispose()` | Triggered when the route is popped (`Navigator::pop()`) and permanently removed from memory. Used to free resources, cancel timers, and close streams. |

---

## Code Example (Full Page Pattern)

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/navigator.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class OrderDetailPage : public StatefulWidget {
public:
    int order_id;
    explicit OrderDetailPage(int id) : order_id(id) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "OrderDetailPage"; }
};

class OrderDetailPageState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* page = static_cast<const OrderDetailPage*>(widget());

        // Header with back navigation button
        auto headerRow = row({
            .align_items = Align::Center,
            .gap = 14_px,
            .children = {
                button(text("← Back"), [ctx]() {
                    auto c = ctx;
                    if (Navigator::canPop(c)) {
                        Navigator::pop(c);
                    }
                }),
                text("Order #" + std::to_string(page->order_id), {
                    .font_size = 20.0f,
                    .font_weight = FontWeight::Bold,
                    .color = 0xFFFFFFFF
                }),
            }
        });

        auto contentCard = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = EdgeInsets::all(20.0f),
            .child = text("Order Details and tracking information...", { .color = 0xFFCBD5E1 })
        });

        return container({
            .color = 0xFF0F172A,
            .width = 100_pct,
            .height = 100_pct,
            .padding = EdgeInsets::all(24.0f),
            .child = column({
                .gap = 20_px,
                .children = { headerRow, contentCard }
            })
        });
    }
};

std::unique_ptr<State> OrderDetailPage::createState() {
    return std::make_unique<OrderDetailPageState>();
}
```

---

## See Also
- [**Navigator**](./navigator.md) — The parent routing stack.
- [**Route**](./route.md) — Route descriptors (`RouteConfig`) that instantiate pages.
- [**Breadcrumb**](./breadcrumb.md) — Hierarchical breadcrumb navigation trails.
