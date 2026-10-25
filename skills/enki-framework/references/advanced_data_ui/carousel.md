# Carousel

> An advanced carousel slider and swiper widget supporting autoplay with pause-on-hover, infinite looping, smooth slide/fade transitions, floating navigation arrows, and dot indicators.

- **Header File**: `#include "enki/widgets/carousel.hpp"`
- **C++ Class**: `enki::CarouselWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Carousel` (converts implicitly to `WidgetPtr`)
- **Controller**: `enki::CarouselController`
- **Effect Enum**: `enki::CarouselEffect` (`Slide`, `Fade`)

---

## Overview

`Carousel` presents a rotating slideshow of custom widgets (hero banners, product showcases, onboarding carousels, or tutorial cards). It includes automated timed sliding (`auto_play`), automatic timer suspension when the user hovers over the card (`pause_on_hover = true`), infinite circular looping, and manual navigation via interactive chevron arrows and bottom dot indicators.

---

## C++ API Definition

### `CarouselEffect` Enum
```cpp
namespace enki {

enum class CarouselEffect {
    Slide,      ///< Standard horizontal sliding transition
    Fade        ///< Smooth cross-fade transition
};

} // namespace enki
```

### Controller (`CarouselController`)
```cpp
namespace enki {

class CarouselController {
public:
    void nextPage();
    void previousPage();
    void jumpToPage(int index);
    void setAutoPlay(bool play);

    [[nodiscard]] int getCurrentPage() const;
    [[nodiscard]] int getPageCount() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Carousel {
    std::vector<WidgetPtr>              slides;
    std::shared_ptr<CarouselController> controller            = nullptr;

    CarouselEffect                      effect                = CarouselEffect::Slide;

    int                                 initial_index         = 0;
    bool                                auto_play             = true;
    int                                 auto_play_interval_ms = 3500;
    bool                                pause_on_hover        = true;
    bool                                infinite_loop         = true;

    bool                                show_arrows           = true;
    bool                                show_indicators       = true;

    float                               height                = 320.0f;
    float                               border_radius         = 12.0f;

    Color                               background_color      = 0xFF1E293B; // Slate 800
    Color                               border_color          = 0xFF334155; // Slate 700
    Color                               arrow_bg_color        = 0xCC0F172A; // Slate 900 translucent
    Color                               indicator_active      = 0xFF38BDF8; // Sky 400
    Color                               indicator_inactive    = 0xFF475569; // Slate 600

    std::function<void(int current)>    on_page_changed       = nullptr;
    Key                                 key                   = Key::none();

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `slides` | `vector<WidgetPtr>` | `{}` | List of slide widgets rendered in sequence. |
| `controller` | `shared_ptr<CarouselController>`| `nullptr`| Handle for programmatic page advancement. |
| `effect` | `CarouselEffect` | `Slide` | Transition style (`Slide` or `Fade`). |
| `auto_play` | `bool` | `true` | Automatically cycles through slides at regular intervals. |
| `auto_play_interval_ms`| `int` | `3500` | Duration in milliseconds each slide stays visible before advancing. |
| `pause_on_hover` | `bool` | `true` | Suspends autoplay timer while mouse cursor is over the carousel. |
| `infinite_loop` | `bool` | `true` | Loops back to first slide after passing the last slide. |
| `show_arrows` | `bool` | `true` | Displays floating left/right navigation arrow buttons. |
| `show_indicators` | `bool` | `true` | Displays bottom pagination dot indicators. |

---

## Code Examples (From `widgets_demo/carousel_demo/main.cpp`)

### 1. Hero Banner Carousel with Autoplay
```cpp
#include "enki/widgets/carousel.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildHeroCarousel() {
    auto slide1 = container({
        .color = 0xFF0F172A,
        .padding = EdgeInsets::all(28.0f),
        .child = text("🚀 ENKI 2.0 Engine Released", { .font_size = 20.0f, .color = 0xFFFFFFFF })
    });

    auto slide2 = container({
        .color = 0xFF1E293B,
        .padding = EdgeInsets::all(28.0f),
        .child = text("⚡ Skia Native GPU Compositor", { .font_size = 20.0f, .color = 0xFF38BDF8 })
    });

    return Carousel {
        .slides = { slide1, slide2 },
        .effect = CarouselEffect::Slide,
        .auto_play = true,
        .auto_play_interval_ms = 4000,
        .pause_on_hover = true,
        .infinite_loop = true,
        .height = 280.0f,
        .border_radius = 12.0f
    };
}
```

---

## See Also
- [**PageView**](../Scrolling-Lists/scroll_view.md) — Low-level swipeable page container.
- [**Timeline**](./timeline.md) — Chronological progress and milestones.
- [**Accordion**](./accordion.md) — Vertical collapsible content panels.
