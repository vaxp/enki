# Timeline

> A chronological event stream and stepper widget supporting vertical and horizontal orientations, zig-zag alternating alignments, multi-state status nodes, dashed/gradient lines, expandable card details, and stepper modes.

- **Header File**: `#include "enki/widgets/timeline.hpp"`
- **C++ Class**: `enki::TimelineWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Timeline` (converts implicitly to `WidgetPtr`)
- **Item Descriptor**: `enki::TimelineItem`
- **Controller**: `enki::TimelineController`
- **Enums**: `enki::TimelineOrientation`, `enki::TimelineAlignment`, `enki::TimelineItemStatus`, `enki::TimelineNodeShape`, `enki::TimelineLineStyle`

---

## Overview

`Timeline` visually displays sequential progress, system audit trails, deployment pipelines, and historical changelogs. It supports both standard vertical layout and horizontal workflow steppers, featuring configurable line styles (solid, dashed, dotted, gradient) and custom status nodes (e.g. checkmarks, active glowing circles, warning triangles, or error crosses).

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class TimelineOrientation {
    Vertical,
    Horizontal
};

enum class TimelineAlignment {
    Start,       ///< Line on left/top, content on right/bottom
    End,         ///< Line on right/bottom, content on left/top
    Alternate,   ///< Zig-zag alternating sides (even left, odd right)
    Center       ///< Centered track with timestamps opposite to cards
};

enum class TimelineItemStatus {
    Completed,   ///< Emerald checkmark ✓
    Active,      ///< Sky blue glowing/pulsing node
    Pending,     ///< Slate hollow circle
    Warning,     ///< Amber warning ⚠️
    Failed       ///< Red cross ✕
};

enum class TimelineNodeShape {
    Circle,
    Square,
    Icon,
    Number,
    Custom
};

enum class TimelineLineStyle {
    Solid,
    Dashed,
    Dotted,
    Gradient
};

} // namespace enki
```

### Item Descriptor (`TimelineItem`)
```cpp
namespace enki {

struct TimelineItem {
    std::string        id          = "";
    std::string        title       = "";
    std::string        timestamp   = "";
    std::string        description = "";
    std::string        details     = "";          ///< Expandable markdown/text details
    std::string        icon        = "";          ///< Emoji or glyph
    std::string        badge_text  = "";

    TimelineItemStatus status      = TimelineItemStatus::Pending;
    TimelineNodeShape  node_shape  = TimelineNodeShape::Circle;
    bool               is_expanded = false;

    TimelineItem() = default;
    TimelineItem(std::string id, std::string title, std::string time,
                 std::string desc = "", TimelineItemStatus st = TimelineItemStatus::Pending);

    TimelineItem& setBadge(std::string text, Color bg = 0x2E38BDF8, Color fg = 0xFFFFFFFF);
    TimelineItem& setDetails(std::string details);
    TimelineItem& setIcon(std::string icon);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Timeline {
    Key                                  key                = Key::none();
    std::shared_ptr<TimelineController>  controller         = nullptr;

    TimelineOrientation                  orientation        = TimelineOrientation::Vertical;
    TimelineAlignment                    alignment          = TimelineAlignment::Start;
    TimelineLineStyle                    line_style         = TimelineLineStyle::Solid;

    float                                node_size          = 24.0f;
    float                                line_thickness     = 2.0f;
    float                                item_spacing       = 24.0f;
    float                                card_width         = 320.0f;
    float                                card_border_radius = 8.0f;

    bool                                 is_stepper         = false;

    // Callbacks
    std::function<void(const TimelineItem&)>                     on_item_tap      = nullptr;
    std::function<void(int step_index)>                          on_step_changed  = nullptr;
    std::function<void(const std::string& id, bool is_expanded)> on_item_expanded = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `controller` | `shared_ptr<TimelineController>`| `nullptr`| Manages timeline items and active stepper step index. |
| `orientation` | `TimelineOrientation` | `Vertical` | Direction of the track (`Vertical` or `Horizontal`). |
| `alignment` | `TimelineAlignment` | `Start` | Placement of cards relative to line (`Start`, `Alternate`, `Center`). |
| `line_style` | `TimelineLineStyle` | `Solid` | Connector visual style (`Solid`, `Dashed`, `Gradient`). |
| `is_stepper` | `bool` | `false` | Enables interactive step-by-step wizard mode. |
| `item_spacing` | `float` | `24.0f` | Vertical/horizontal distance between successive event cards. |

---

## Code Examples (From `widgets_demo/timeline_demo/main.cpp`)

### 1. Zig-Zag Release Roadmap Timeline
```cpp
#include "enki/widgets/timeline.hpp"

using namespace enki;

WidgetPtr buildReleaseTimeline() {
    std::vector<TimelineItem> items;

    TimelineItem it1("m1", "v1.0 Architecture Finalized", "June 2026",
                     "Core element tree and reactive reconciler completed.",
                     TimelineItemStatus::Completed);
    it1.setIcon("🚀").setBadge("COMPLETED", 0x2E10B981, 0xFFFFFFFF);
    items.push_back(it1);

    TimelineItem it2("m2", "Skia Compositor Integration", "August 2026",
                     "Direct hardware GPU rasterization with multi-layer caching.",
                     TimelineItemStatus::Active);
    it2.setIcon("⚡").setBadge("IN PROGRESS", 0x2E38BDF8, 0xFFFFFFFF)
       .setDetails("• Anti-aliased text via SkParagraph\n• Zero-copy overlay rendering\n• 60+ FPS animations");
    items.push_back(it2);

    auto ctrl = std::make_shared<TimelineController>(std::move(items));

    return Timeline {
        .controller = ctrl,
        .orientation = TimelineOrientation::Vertical,
        .alignment = TimelineAlignment::Alternate, // Alternates left and right
        .line_style = TimelineLineStyle::Gradient,
        .item_spacing = 30.0f
    };
}
```

---

## See Also
- [**DataGrid**](./data_grid.md) — High-density tabular data presentation.
- [**ExpansionPanel**](./expansion_panel.md) — Collapsible step-by-step wizard panels.
- [**Calendar**](./calendar.md) — Date-based event scheduling.
