# SegmentedControl

> Horizontally grouped mutually-exclusive option buttons with smooth animated sliding indicator.

- **Header File**: `#include "enki/widgets/segmented_control.hpp"`
- **C++ Class**: `enki::SegmentedControlWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Helper**: `enki::segmentedControl(SegmentedControlProps props)` (returns `enki::WidgetPtr`)
- **Render Object**: `enki::RenderSegmentedControl`
- **Underlying Mechanism**: Skia rounded-rect pill slider animated via internal frame Ticker with hit-testing per segment slot.

---

## Overview

`SegmentedControl` presents a horizontal strip of mutually exclusive choices (like tabs or view switchers). When the active index changes, the background highlight thumb smoothly slides to the newly selected segment using a physics-driven animation.

---

## C++ API Definition

### Struct Definition (`enki/widgets/segmented_control.hpp`)
```cpp
namespace enki {

struct SegmentItem {
    std::string label;
    std::string icon = "";
    bool        enabled = true;

    SegmentItem(std::string l = "", std::string i = "")
        : label(std::move(l)), icon(std::move(i)) {}
};

struct SegmentedControlProps {
    std::vector<SegmentItem>            items;
    int                                 selected_index = 0;
    std::function<void(int)>            on_change;

    Color                               track_color = 0x59000000;
    Color                               thumb_color = 0xFF0C3559;
    Color                               thumb_border_color = 0x6600E5FF;
    Color                               active_text_color = 0xFF38BDF8;
    Color                               inactive_text_color = 0xFF94A3B8;
    Color                               border_color = 0x3300E5FF;
    float                               border_width = 1.0f;
    float                               border_radius = 10.0f;
    float                               thumb_radius = 8.0f;
    float                               height = 38.0f;
    float                               width = 0.0f; // 0 = fit content or fill
    float                               padding = 4.0f;

    operator WidgetPtr() const;
};

class SegmentedControlWidget : public SingleChildRenderObjectWidget {
public:
    SegmentedControlProps props;

    explicit SegmentedControlWidget(SegmentedControlProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "SegmentedControl"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr segmentedControl(SegmentedControlProps props) {
    return std::make_shared<SegmentedControlWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `std::vector<SegmentItem>` | `{}` | List of items to display in the control. |
| `selected_index` | `int` | `0` | Zero-based index of the currently active segment. |
| `on_change` | `std::function<void(int)>` | `nullptr` | Callback triggered when a new segment is clicked. |
| `track_color` | `Color` | `0x59000000` | Background color of the container track. |
| `thumb_color` | `Color` | `0xFF0C3559` | Color of the sliding active thumb pill. |
| `thumb_border_color` | `Color` | `0x6600E5FF` | Border stroke color of the sliding thumb. |
| `active_text_color` | `Color` | `0xFF38BDF8` | Text color of the selected item. |
| `inactive_text_color` | `Color` | `0xFF94A3B8` | Text color of unselected items. |
| `border_color` | `Color` | `0x3300E5FF` | Outer boundary stroke color. |
| `border_width` | `float` | `1.0f` | Outer boundary border stroke width. |
| `border_radius` | `float` | `10.0f` | Corner radius of the container track. |
| `thumb_radius` | `float` | `8.0f` | Corner radius of the sliding thumb. |
| `height` | `float` | `38.0f` | Height of the segmented control. |
| `width` | `float` | `0.0f` | Fixed width in pixels (`0.0f` for auto-sizing). |
| `padding` | `float` | `4.0f` | Inner inset between the track and the thumb. |

---

## Code Examples (From `widgets_demo/segmented_control_demo/main.cpp`)

### 1. Date Range Filter
```cpp
auto seg_time = segmentedControl({
    .items = {
        SegmentItem("Day"),
        SegmentItem("Week"),
        SegmentItem("Month"),
        SegmentItem("Year"),
    },
    .selected_index = selected_time_,
    .on_change = [this](int idx) {
        selected_time_ = idx;
        setState([]{});
    },
    .height = 38.0f,
    .width = 440.0f,
});
```

### 2. Custom Styled View Mode Selector
```cpp
auto seg_view = segmentedControl({
    .items = {
        SegmentItem("Grid"),
        SegmentItem("List"),
        SegmentItem("Table"),
    },
    .selected_index = selected_view_,
    .on_change = [this](int idx) {
        selected_view_ = idx;
        setState([]{});
    },
    .thumb_color = 0xFF78350F,
    .thumb_border_color = 0xFFF59E0B,
    .active_text_color = 0xFFF59E0B,
    .height = 38.0f,
    .width = 380.0f,
});
```
