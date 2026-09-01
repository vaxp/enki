# ExpansionPanel

> A workflow and stepper expansion panel list widget supporting radio and multi-expansion modes, dynamic elevation gap transitions, header step numbers, badges, and footer action bars.

- **Header File**: `#include "enki/widgets/expansion_panel.hpp"`
- **C++ Class**: `enki::ExpansionPanelListWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ExpansionPanelList` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ExpansionPanelListProps`
- **Item Descriptor**: `enki::ExpansionPanelItem`
- **Controller**: `enki::ExpansionPanelController`

---

## Overview

`ExpansionPanelList` organizes multi-step workflows (such as multi-stage checkout flows, server configuration wizards, or registration forms) into distinct panels. Expanded panels dynamically introduce an elevated vertical margin (`expanded_elevation_gap = 16.0f`) to stand out from adjacent panels and support integrated **footer action bars** (e.g. `[Cancel]`, `[Next Step]`).

---

## C++ API Definition

### `ExpansionPanelItem` Struct
```cpp
namespace enki {

struct ExpansionPanelItem {
    std::string            id                    = "";
    std::string            title                 = "";
    std::string            subtitle              = "";
    std::string            icon_or_step          = "";          ///< Leading step pill (e.g. "1", "2") or emoji
    std::string            badge_label           = "";          ///< Status tag (e.g. "Completed", "Required")
    Color                  badge_bg              = 0x2E10B981;
    Color                  badge_fg              = 0xFF10B981;

    WidgetPtr              body                  = nullptr;     ///< Main content revealed when expanded
    std::vector<WidgetPtr> footer_actions;                      ///< Buttons in footer bar (e.g. [Back], [Next])

    bool                   is_initially_expanded = false;
    bool                   is_disabled           = false;
    bool                   can_tap_on_header     = true;

    ExpansionPanelItem() = default;
    ExpansionPanelItem(std::string id, std::string title, WidgetPtr body,
                       std::string icon_step = "", std::string subtitle = "",
                       bool initially_expanded = false);

    ExpansionPanelItem& setBadge(std::string label, Color bg = 0x2E10B981, Color fg = 0xFF10B981);
    ExpansionPanelItem& setFooterActions(std::vector<WidgetPtr> actions);
    ExpansionPanelItem& setDisabled(bool d = true);
};

} // namespace enki
```

### Controller (`ExpansionPanelController`)
```cpp
namespace enki {

class ExpansionPanelController {
public:
    void expand(int index);
    void collapse(int index);
    void toggle(int index);
    void expandAll();
    void collapseAll();

    [[nodiscard]] bool isExpanded(int index) const;
    [[nodiscard]] std::set<int> getExpandedIndices() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ExpansionPanelList {
    Key                                       key                    = Key::none();
    std::vector<ExpansionPanelItem>           panels;
    std::shared_ptr<ExpansionPanelController> controller             = nullptr;

    bool                                      is_radio_mode          = false; ///< Only 1 panel open at a time
    float                                     gap                    = 12.0f; ///< Regular spacing between panels
    float                                     expanded_elevation_gap = 16.0f; ///< Extra vertical spacing for expanded item
    float                                     border_radius          = 10.0f;
    bool                                      show_chevron           = true;

    Color                                     background_color       = 0xFF1E293B; // Slate 800
    Color                                     border_color           = 0xFF334155; // Slate 700
    Color                                     expanded_border_col    = 0xFF0284C7; // Blue 600

    // Callbacks
    std::function<void(int index, bool is_expanded)>   on_panel_toggled   = nullptr;
    std::function<void(const std::set<int>& expanded)> on_expansion_changed = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `panels` | `vector<ExpansionPanelItem>`| `{}` | Step panel descriptors and bodies. |
| `controller` | `shared_ptr<ExpansionPanelController>`| `nullptr`| Controller to expand/collapse panels by index. |
| `is_radio_mode`| `bool` | `false` | When true, expanding one panel collapses all others automatically. |
| `gap` | `float` | `12.0f` | Vertical gap between adjacent panels in their resting state. |
| `expanded_elevation_gap`| `float` | `16.0f` | Added vertical breathing room isolating the active expanded panel. |
| `show_chevron` | `bool` | `true` | Renders animated expanding chevron icon in header. |

---

## Code Examples (From `widgets_demo/expansion_panel_demo/main.cpp`)

### 1. Multi-Step Wizard with Footer Actions
```cpp
#include "enki/widgets/expansion_panel.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSetupWizard() {
    auto ctrl = std::make_shared<ExpansionPanelController>();

    auto step1Body = text("Select 16 vCPUs and 64GB ECC Memory.", { .color = 0xFFE2E8F0 });
    auto step2Body = text("Configure Zero-Trust VPC network rules.", { .color = 0xFFE2E8F0 });

    ExpansionPanelItem p1("step_1", "1. Node Hardware Configuration", step1Body, "1", "Hardware sizing", true);
    p1.setBadge("Completed", 0x2E10B981, 0xFF10B981)
      .setFooterActions({
          button(text("Next Step →"), [ctrl] { ctrl->expand(1); })
      });

    ExpansionPanelItem p2("step_2", "2. Network & Security VPC", step2Body, "2", "Firewall rules", false);
    p2.setBadge("Pending", 0x2E475569, 0xFF94A3B8)
      .setFooterActions({
          button(text("← Back"),      [ctrl] { ctrl->expand(0); }),
          button(text("Deploy Pod"),  [] { std::cout << "Deploying!\n"; })
      });

    return ExpansionPanelList {
        .panels = { p1, p2 },
        .controller = ctrl,
        .is_radio_mode = true,
        .gap = 12.0f,
        .expanded_elevation_gap = 16.0f
    };
}
```

---

## See Also
- [**Accordion**](./accordion.md) — Category-based expandable sections.
- [**Timeline**](./timeline.md) — Read-only or wizard event tracks.
- [**SplitView**](./split_view.md) — Resizable dual-pane container.
