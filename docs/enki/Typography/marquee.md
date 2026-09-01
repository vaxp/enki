# Marquee

> An auto-scrolling single-line typography widget (ticker tape) with configurable velocity, scroll direction, loop spacing, pause-on-hover, and smooth edge fade masks.

- **Header File**: `#include "enki/widgets/marquee.hpp"`
- **C++ Class**: `enki::MarqueeWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::Marquee` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::MarqueeProps`
- **Factory Helpers**: `enki::marquee()`
- **Direction Enum**: `enki::MarqueeDirection` (`RightToLeft`, `LeftToRight`)

---

## Overview

`Marquee` continuously scrolls a single line of text across the screen. It is specifically designed for news feeds, stock tickers, status monitors, media player track titles, and alert bars. It includes automated timer management, seamless circular looping with a configurable `blank_space` gap, automatic pause when the user hovers over the ticker (`pause_on_hover = true`), and subtle gradient alpha masks along both ends (`fading_edge_length`) to eliminate harsh clipping boundaries.

---

## C++ API Definition

### `MarqueeDirection` Enum & Declarative Struct
```cpp
namespace enki {

enum class MarqueeDirection {
    RightToLeft, ///< Natural ticker direction (moves right to left)
    LeftToRight  ///< Inverted ticker direction
};

struct Marquee {
    std::string               text               = "";
    std::optional<TextStyle>  style              = std::nullopt;
    std::optional<Color>      color              = std::nullopt;
    std::optional<float>      font_size          = std::nullopt;
    std::optional<FontWeight> font_weight        = std::nullopt;
    float                     velocity           = 50.0f;                     ///< Speed in pixels per second
    float                     blank_space        = 60.0f;                     ///< Spacing before loop repeats
    MarqueeDirection          direction          = MarqueeDirection::RightToLeft;
    bool                      pause_on_hover     = true;                      ///< Freeze scroll on mouse hover
    float                     fading_edge_length = 24.0f;                     ///< Pixel width of side gradient fade
    Key                       key                = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<MarqueeWidget> marquee(std::string text);
inline std::shared_ptr<MarqueeWidget> marquee(const MarqueeProps& props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `text` | `string` | `""` | The ticker string to animate. |
| `velocity` | `float` | `50.0f` | Scroll speed in pixels per second. |
| `blank_space` | `float` | `60.0f` | Horizontal gap in pixels between the end of the text and its repetition. |
| `direction` | `MarqueeDirection` | `RightToLeft` | Animation direction (`RightToLeft` or `LeftToRight`). |
| `pause_on_hover` | `bool` | `true` | Temporarily halts scrolling while the mouse cursor rests inside. |
| `fading_edge_length`| `float`| `24.0f` | Width of the gradient mask fading out edges smoothly. |

---

## Code Examples (From `widgets_demo/typography_demo/main.cpp`)

### 1. Stock Market & Breaking News Ticker
```cpp
#include "enki/widgets/marquee.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr buildBreakingNewsTicker() {
    return container({
        .color = 0xFF0F172A,
        .border_radius = BorderRadius::circular(8.0f),
        .padding = EdgeInsets::symmetric(10.0f, 16.0f),
        .child = Marquee {
            .text = "⚡ BREAKING: ENKI GUI Framework releases next-generation Typography widgets!  •  NASDAQ: ENKI +14.2%  •  ",
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .velocity = 60.0f,          // 60 pixels/sec
            .blank_space = 70.0f,       // 70px separator between loops
            .pause_on_hover = true,     // User can read text comfortably by hovering
            .fading_edge_length = 32.0f // Smooth alpha fade
        }
    });
}
```

---

## See Also
- [**SelectableText**](./selectable_text.md) — Interactive text selection.
- [**Carousel**](../Advanced%20%20Data%20UI/carousel.md) — Slide-based media/widget rotator.
- [**Text**](../Basic%20UI/text.md) — Standard static text.
