# TabView

> A multi-view container widget that displays the child matching the currently selected tab index while avoiding unnecessary instantiation of inactive views.

- **Header File**: `#include "enki/widgets/tab_bar.hpp"`
- **C++ Class**: `enki::TabViewWidget` (inherits from `enki::StatelessWidget`)
- **Declarative Struct**: `enki::TabView` (converts implicitly to `WidgetPtr`)

---

## Overview

`TabView` pairs directly with a `TabBar`. Given a collection of `children` widgets and a `selected_index`, `TabViewWidget::build()` returns only the active child widget (`children[selected_index]`). Inactive views are not constructed, ensuring instantaneous switching and minimal memory footprint.

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct TabView {
    Key                    key            = Key::none();
    int                    selected_index = 0;
    std::vector<WidgetPtr> children;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `selected_index` | `int` | `0` | The index corresponding to the active view to display. |
| `children` | `std::vector<WidgetPtr>` | `{}` | The list of view widgets mapped 1-to-1 with the `TabBar` tabs. |

---

## Code Examples (From `widgets_demo/tab_bar_demo/main.cpp`)

### 1. Complete TabBar + TabView Pairing
```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/tab_bar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

class TabbedDashboardState : public State {
    int current_tab_ = 0;

public:
    WidgetPtr build(BuildContext& ctx) override {
        return column({
            .width = 100_pct,
            .height = 100_pct,
            .children = {
                // 1. Navigation Tab Strip
                TabBar {
                    .tabs = {
                        {"Overview",  Icons::Material::dashboard(), ""},
                        {"Analytics", Icons::Material::analytics(), ""},
                        {"Messages",  Icons::Material::chat(),      "5"},
                    },
                    .selected_index = current_tab_,
                    .on_tab_changed = [this](int idx) {
                        setState([this, idx] { current_tab_ = idx; });
                    }
                },
                // 2. Active Tab Content Area
                container({
                    .flex_grow = 1.0f,
                    .child = TabView {
                        .selected_index = current_tab_,
                        .children = {
                            buildOverviewPane(),
                            buildAnalyticsPane(),
                            buildMessagesPane(),
                        }
                    }
                })
            }
        });
    }
};
```

---

## See Also
- [**TabBar**](./tab_bar.md) — The horizontal tab selector strip.
- [**Page**](./page.md) — Independent full-screen page architecture.
