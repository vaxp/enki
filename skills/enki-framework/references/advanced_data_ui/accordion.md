# Accordion

> A collapsible content accordion widget supporting single and multiple expansion modes, three visual styling variants (Bordered, Separated, Flush), rotating chevrons, header badges, and programmatic controllers.

- **Header File**: `#include "enki/widgets/accordion.hpp"`
- **C++ Class**: `enki::AccordionWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Accordion` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::AccordionProps`
- **Item Descriptor**: `enki::AccordionItem`
- **Controller**: `enki::AccordionController`
- **Enums**: `enki::AccordionMode`, `enki::AccordionVariant`

---

## Overview

`Accordion` organizes content-heavy pages (such as FAQs, settings categories, and document outlines) into stacked expandable panels. When in **`AccordionMode::Single`**, expanding one panel automatically collapses all others; in **`AccordionMode::Multiple`**, users can expand multiple sections concurrently.

---

## C++ API Definition

### Enums & Item Model
```cpp
namespace enki {

enum class AccordionMode {
    Single,     ///< Only one section open at a time (auto-collapses others)
    Multiple    ///< Multiple sections can remain open simultaneously
};

enum class AccordionVariant {
    Bordered,   ///< Unified rounded container with internal section dividers
    Separated,  ///< Independent floating cards with vertical gap between items
    Flush       ///< Minimal transparent background with simple line dividers
};

struct AccordionItem {
    std::string id                    = "";
    std::string title                 = "";
    std::string subtitle              = "";
    std::string icon                  = "";          ///< Leading icon / emoji (e.g. 🔒, ⚙️)
    std::string badge_label           = "";          ///< Status tag (e.g. "PRO", "2 Active")
    Color       badge_bg              = 0x2E38BDF8;
    Color       badge_fg              = 0xFF38BDF8;

    WidgetPtr   content               = nullptr;     ///< Widget revealed when expanded
    bool        is_initially_expanded = false;
    bool        is_disabled           = false;
};

} // namespace enki
```

### Controller (`AccordionController`)
```cpp
namespace enki {

class AccordionController {
public:
    void expand(const std::string& id);
    void collapse(const std::string& id);
    void toggle(const std::string& id);
    void expandAll();
    void collapseAll();

    [[nodiscard]] bool isExpanded(const std::string& id) const;
    [[nodiscard]] std::set<std::string> getExpandedIds() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Accordion {
    Key                                  key              = Key::none();
    std::vector<AccordionItem>           items;
    std::shared_ptr<AccordionController> controller       = nullptr;

    AccordionMode                        mode             = AccordionMode::Single;
    AccordionVariant                     variant          = AccordionVariant::Bordered;

    bool                                 collapsible      = true;  ///< In Single mode, allows closing active section
    bool                                 show_chevron     = true;  ///< Rotating ⌃ / ⌄ arrow
    float                                gap              = 10.0f; ///< Vertical gap (for Separated variant)
    float                                border_radius    = 10.0f;

    Color                                background_color = 0xFF1E293B; // Slate 800
    Color                                border_color     = 0xFF334155; // Slate 700

    // Callbacks
    std::function<void(const std::string& id, bool is_expanded)> on_toggle = nullptr;
    std::function<void(const std::set<std::string>& active_ids)> on_change = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `vector<AccordionItem>`| `{}` | Section descriptors and content widgets. |
| `controller` | `shared_ptr<AccordionController>`| `nullptr`| Handle for expanding/collapsing sections programmatically. |
| `mode` | `AccordionMode` | `Single` | Single or multiple active section rule. |
| `variant` | `AccordionVariant` | `Bordered` | Visual theme (`Bordered`, `Separated`, `Flush`). |
| `collapsible` | `bool` | `true` | Allows collapsing the currently open panel in `Single` mode. |
| `gap` | `float` | `10.0f` | Spacing between floating cards in `Separated` mode. |

---

## Code Examples (From `widgets_demo/accordion_demo/main.cpp`)

### 1. Settings Accordion with Badges and Icons
```cpp
#include "enki/widgets/accordion.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildSettingsAccordion() {
    auto ctrl = std::make_shared<AccordionController>();

    std::vector<AccordionItem> items = {
        AccordionItem {
            .id = "sec",
            .title = "Security & Encryption",
            .subtitle = "Manage API keys and SSH certificates",
            .icon = "🔒",
            .badge_label = "2 Active",
            .badge_bg = 0x2E10B981,
            .badge_fg = 0xFF10B981,
            .content = text("SSH Keys: id_ed25519 authorized for deploy.", { .color = 0xFFCBD5E1 }),
            .is_initially_expanded = true
        },
        AccordionItem {
            .id = "net",
            .title = "Network Clusters",
            .subtitle = "Configure edge proxy endpoints",
            .icon = "🌐",
            .badge_label = "Healthy",
            .content = text("240 Edge Nodes synchronized worldwide.", { .color = 0xFFCBD5E1 })
        }
    };

    return Accordion {
        .items = std::move(items),
        .controller = ctrl,
        .mode = AccordionMode::Single,
        .variant = AccordionVariant::Separated,
        .gap = 12.0f
    };
}
```

---

## See Also
- [**ExpansionPanel**](./expansion_panel.md) — Step-by-step wizard panels with footer action bars.
- [**SplitView**](./split_view.md) — Resizable dual-pane container.
- [**Carousel**](./carousel.md) — Horizontal sliding cards.
