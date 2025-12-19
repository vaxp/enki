# IntrinsicWidth

> A layout widget that sizes its child to the child's maximum intrinsic (natural) width, with optional step quantization.

- **Header File**: `#include "enki/widgets/intrinsic_width.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class IntrinsicWidthWidget`, `struct IntrinsicWidthProps`, `struct IntrinsicWidth`
- **Helper Function**: `intrinsicWidth(...)`

---

## Overview

`IntrinsicWidth` measures the intrinsic (unconstrained natural) width of its child widget tree and forces the child to assume that width. When children with varying widths are placed in a vertical `Column`, wrapping them in `IntrinsicWidth` causes all children to stretch to match the width of the widest child, eliminating ragged button columns and ragged dialog action bars.

Additionally, `IntrinsicWidth` supports step quantization via `step_width` and `step_height`. When set, the measured intrinsic dimension snaps upwards to the next multiple of the specified step value.

### Key Architectural Behaviors:
- **Two-Pass Intrinsic Measurement**: During `syncLayout()`, `RenderIntrinsicWidth` calculates the natural width of its child without infinite stretching, constraints it to incoming bounds, and applies step snapping.
- **Column Synchronization**: Combined with `.align_items = Align::Stretch` on an inner `Column`, every item expands to match the widest element without hardcoding pixel widths.
- **Designated Initializer Support**: Fully compliant with C++20 designated initializers (`IntrinsicWidthProps`).

---

## C++ API Definition

### Header: `<enki/widgets/intrinsic_width.hpp>`

```cpp
namespace enki {

struct IntrinsicWidthProps {
    Key                  key         = Key::none();
    std::optional<float> step_width  = std::nullopt;
    std::optional<float> step_height = std::nullopt;
    WidgetPtr            child       = nullptr;
};

struct IntrinsicWidth {
    Key                  key         = Key::none();
    std::optional<float> step_width  = std::nullopt;
    std::optional<float> step_height = std::nullopt;
    WidgetPtr            child       = nullptr;

    operator WidgetPtr() const;
};

// Declarative factory helpers:
std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(const IntrinsicWidthProps& props);
std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(WidgetPtr child);
std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(float step_width, WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and efficient diffing. |
| `step_width` | `std::optional<float>` | `std::nullopt` | If non-null, snaps the computed child width to the nearest multiple of this value. |
| `step_height` | `std::optional<float>` | `std::nullopt` | If non-null, snaps the computed child height to the nearest multiple of this value. |
| `child` | `WidgetPtr` | `nullptr` | The child widget whose intrinsic width is measured and imposed. |

---

## Real Code Examples

### 1. Unified Action Column (From `widgets_demo/intrinsic_width_demo/main.cpp`)
All buttons in this vertical column automatically synchronize their widths to match the widest button, without hardcoding any widths:

```cpp
#include "enki/widgets/intrinsic_width.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildSynchronizedActionColumn() {
    std::vector<std::string> labels = {
        "Save Local Draft",
        "Execute Multi-Region Kubernetes Zero-Downtime Rollout",
        "Abort Transaction",
        "Generate Telemetry Report"
    };

    std::vector<WidgetPtr> buttons;
    for (const auto& lbl : labels) {
        buttons.push_back(button(
            text(lbl, {
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::Medium,
            }),
            [lbl]() { /* click action */ },
            {
                .normal_color = 0xFF4338CA,
                .hover_color  = 0xFF4F46E5,
                .border_radius = 6.0f,
                .padding = EdgeInsets::symmetric(10.0f, 16.0f),
            }
        ));
    }

    // Inner Column configured to stretch all items across the cross axis
    auto inner_column = column({
        .align_items = Align::Stretch,
        .gap = StyleValue::point(10.0f),
        .children = buttons,
        .key = Key::string("btn_col"),
    });

    // IntrinsicWidth wraps the column, constraining its width to the widest child button
    return intrinsicWidth({
        .child = inner_column,
        .key = Key::string("unified_intrinsic_width"),
    });
}
```

### 2. Stepped Grid Snapping (`step_width`)
Snaps the intrinsic width to 50px increments:

```cpp
auto quantizedMenu = intrinsicWidth({
    .step_width = 50.0f, // Snaps 123px -> 150px, 178px -> 200px
    .child = column({
        .align_items = Align::Stretch,
        .children = {
            text("Short item"),
            text("Medium length navigation item"),
        },
    }),
});
```

---

## See Also
- [**IntrinsicHeight**](./intrinsic_height.md) — Sizing children to their natural intrinsic height.
- [**ConstrainedBox**](./constrained_box.md) — Imposing min/max boundaries on child dimensions.
- [**LimitedBox**](./limited_box.md) — Clamping dimensions only when unconstrained.
