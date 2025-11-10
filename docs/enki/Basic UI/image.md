# Image

> A hardware-accelerated image widget supporting asset/file loading, thread-safe memory caching, BoxFit scaling strategies, geometric clipping, and tint blending.

- **Header File**: `#include "enki/widgets/image.hpp"`
- **C++ Class**: `enki::ImageWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Props Struct**: `enki::ImageProps`
- **Style Struct**: `enki::ImageStyle`
- **Render Object**: `enki::RenderImage`
- **Underlying Engine**: Skia image decoding + Anu Flexbox aspect-ratio measurement

---

## Overview

`Image` provides high-performance bitmap rendering in Enki. It automatically leverages an in-memory `ImageCache` to prevent duplicate disk reads and decoding overhead. It supports all standard scaling modes (`BoxFit`), geometric clipping (`BorderRadius` and circular masks), alpha opacity, and Skia `BlendMode` color tinting.

---

## C++ API Definition

### Class Declaration & Static Factories
```cpp
namespace enki {

class ImageWidget : public SingleChildRenderObjectWidget {
public:
    ImageStyle style;

    explicit ImageWidget(std::shared_ptr<Image> image);
    explicit ImageWidget(std::string_view path);

    [[nodiscard]] std::string_view typeName() const override { return "ImageWidget"; }

    static std::shared_ptr<ImageWidget> asset(std::string_view path);
    static std::shared_ptr<ImageWidget> file(std::string_view path);
    static std::shared_ptr<ImageWidget> memory(const std::vector<uint8_t>& data);
    static std::shared_ptr<ImageWidget> fromImage(std::shared_ptr<Image> img);
};

} // namespace enki
```

### Declarative Props Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ImageProps {
    Key                       key         = Key::none();
    std::shared_ptr<Image>    image       = nullptr;
    std::string               source_path = "";

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    BoxFit                    fit         = BoxFit::Cover;
    Alignment                 alignment   = Alignment::Center;
    BorderRadius              border_radius = BorderRadius::zero();
    BoxShape                  shape       = BoxShape::Rectangle;

    std::optional<Color>      tint_color;
    BlendMode                 blend_mode  = BlendMode::SrcIn;
    float                     opacity     = 1.0f;
    bool                      clip_content = true;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<ImageWidget> image(ImageProps props);
inline std::shared_ptr<ImageWidget> image(std::string_view path, ImageProps props = {});
inline std::shared_ptr<ImageWidget> image(std::shared_ptr<Image> img, ImageProps props = {});

inline std::shared_ptr<ImageWidget> imageAsset(std::string_view path, ImageProps props = {});
inline std::shared_ptr<ImageWidget> imageFile(std::string_view path, ImageProps props = {});
inline std::shared_ptr<ImageWidget> imageMemory(const std::vector<uint8_t>& data, ImageProps props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `source_path` | `std::string` | `""` | Filepath or asset URI to load through `ImageCache`. |
| `image` | `std::shared_ptr<Image>` | `nullptr` | Pre-loaded decoded Skia image pointer. |
| `fit` | `BoxFit` | `BoxFit::Cover` | How the image is scaled within its box (`Cover`, `Contain`, `Fill`, etc.). |
| `alignment` | `Alignment` | `Alignment::Center` | Alignment of image within destination box if aspect ratios differ. |
| `width` / `height` | `std::optional<StyleValue>` | `auto_val` | Dimension constraints (`_px`, `_pct`). |
| `border_radius` | `BorderRadius` | `zero()` | Corner radius for clipping the image. |
| `shape` | `BoxShape` | `Rectangle` | Shape mask (`Rectangle` or `Circle`). |
| `tint_color` | `std::optional<Color>` | `std::nullopt` | Optional color tint applied with `blend_mode`. |
| `opacity` | `float` | `1.0f` | Alpha opacity (0.0 to 1.0). |
| `clip_content` | `bool` | `true` | Clips image content strictly to `border_radius`. |

---

## `BoxFit` Modes

- `BoxFit::Cover` — Scales proportionally to cover the entire target box; crops excess.
- `BoxFit::Contain` — Scales proportionally so the entire image fits inside target box (letterbox).
- `BoxFit::Fill` — Stretches image to fill target box without preserving aspect ratio.
- `BoxFit::FitWidth` — Scales source to match target width (height follows aspect ratio).
- `BoxFit::FitHeight` — Scales source to match target height (width follows aspect ratio).
- `BoxFit::None` — Centers image at 1:1 original scale.
- `BoxFit::ScaleDown` — Like `Contain` if source is larger than target box; otherwise like `None`.

---

## Code Examples (From `widgets_demo/image_demo/main.cpp`)

### 1. Rounded Card Hero Banner
```cpp
#include "enki/widgets/image.hpp"

using namespace enki;

WidgetPtr buildBanner() {
    return image({
        .source_path = "assets/hero_wallpaper.jpg",
        .width = 100_pct,
        .height = 200_px,
        .fit = BoxFit::Cover,
        .border_radius = BorderRadius::circular(12.0f),
    });
}
```

### 2. Circular Clipped Profile Picture
```cpp
auto userPhoto = image({
    .source_path = "assets/user_profile.png",
    .width = 64_px,
    .height = 64_px,
    .shape = BoxShape::Circle,
    .fit = BoxFit::Cover,
});
```

---

## See Also
- [**Avatar**](./avatar.md) — Specialized user profile avatar with fallback initials and status badges.
- [**Container**](./card.md) — Visual box styling.
