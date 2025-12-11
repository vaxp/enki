# RatingBar

> An interactive star/icon rating input widget with fractional support and hover feedback.

- **Header File**: `#include "enki/widgets/rating_bar.hpp"`
- **C++ Class**: `enki::RatingBarWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Helper**: `enki::ratingBar(RatingBarProps props)` (returns `enki::WidgetPtr`)
- **Render Object**: `enki::RenderRatingBar`
- **Underlying Mechanism**: Skia 5-point vector star path rendering, fractional clipping for half-star precision, and outer bloom glows.

---

## Overview

`RatingBar` allows users to rate content, reviews, or media items using star ratings. It supports full-star selection, half-star increments (e.g. 3.5 / 5.0), and hover tracking. It also features a `is_read_only` mode for static rating presentations.

---

## C++ API Definition

### Struct Definition (`enki/widgets/rating_bar.hpp`)
```cpp
namespace enki {

struct RatingBarProps {
    float                               rating = 0.0f;
    int                                 max_rating = 5;
    float                               item_size = 24.0f;
    float                               item_spacing = 6.0f;
    bool                                allow_half = true;
    bool                                is_read_only = false;

    Color                               active_color = 0xFFF59E0B;   // Amber Gold
    Color                               inactive_color = 0x33FFFFFF; // Subtle translucent
    Color                               glow_color = 0x66F59E0B;

    std::function<void(float)>          on_rating_changed;
    std::function<void(float)>          on_hover;

    operator WidgetPtr() const;
};

class RatingBarWidget : public SingleChildRenderObjectWidget {
public:
    RatingBarProps props;

    explicit RatingBarWidget(RatingBarProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "RatingBar"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr ratingBar(RatingBarProps props) {
    return std::make_shared<RatingBarWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `rating` | `float` | `0.0f` | Current score / selected rating value. |
| `max_rating` | `int` | `5` | Maximum number of stars displayed. |
| `item_size` | `float` | `24.0f` | Width and height of each individual star item. |
| `item_spacing` | `float` | `6.0f` | Horizontal gap between adjacent stars. |
| `allow_half` | `bool` | `true` | When `true`, enables 0.5 fractional increments based on pointer offset. |
| `is_read_only` | `bool` | `false` | Disables user interaction and click/hover events. |
| `active_color` | `Color` | `0xFFF59E0B` | Filled star color (default: amber gold). |
| `inactive_color`| `Color` | `0x33FFFFFF` | Empty star background outline color. |
| `glow_color` | `Color` | `0x66F59E0B` | Color of the outer halo glow behind active stars. |
| `on_rating_changed` | `std::function<void(float)>` | `nullptr` | Callback triggered when user taps/clicks a star rating. |
| `on_hover` | `std::function<void(float)>` | `nullptr` | Callback fired on mouse hover with the preview rating value. |

---

## Code Examples (From `widgets_demo/rating_bar_demo/main.cpp`)

### 1. Fractional Half-Star Rating
```cpp
auto rb1 = ratingBar({
    .rating = rating1_,
    .max_rating = 5,
    .item_size = 36.0f,
    .item_spacing = 10.0f,
    .allow_half = true,
    .active_color = 0xFFF59E0B,
    .on_rating_changed = [this](float r) {
        rating1_ = r;
        setState([]{});
    },
});
```

### 2. Cyan Integer Full-Star Rating
```cpp
auto rb2 = ratingBar({
    .rating = rating2_,
    .max_rating = 5,
    .item_size = 36.0f,
    .item_spacing = 10.0f,
    .allow_half = false,
    .active_color = 0xFF00E5FF,
    .on_rating_changed = [this](float r) {
        rating2_ = r;
        setState([]{});
    },
});
```
