# Text

> A foundational widget that renders a string of text with customizable typography, wrapping, overflow, and selection support.

- **Header File**: `#include "enki/widgets/text.hpp"`
- **C++ Class**: `enki::Text` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Props Struct**: `enki::TextProps`
- **Style Struct**: `enki::TextStyle`
- **Render Object**: `enki::RenderParagraph`
- **Underlying Engine**: Skia `SkParagraph` + Anu Layout Engine intrinsic measurement

---

## Overview

`Text` is the primary component for rendering text runs in Enki. It delegates layout and font rendering directly to Skia's high-performance `SkParagraph` engine while seamlessly integrating into Anu Flexbox for intrinsic text measurement, line-wrapping, and ellipsis truncation (`...`).

---

## C++ API Definition

### Class Declaration
```cpp
namespace enki {

class Text : public SingleChildRenderObjectWidget {
public:
    std::string                         data;
    TextStyle                           style;
    TextAlign                           text_align = TextAlign::Start;
    TextDirection                       text_direction = TextDirection::LTR;
    TextOverflow                        overflow = TextOverflow::Clip;
    std::optional<size_t>               max_lines;
    bool                                soft_wrap = true;
    bool                                selectable = false;
    Color                               selection_color = 0x6038BDF8;
    std::function<void(TextSelection)>  on_selection_changed = nullptr;

    explicit Text(std::string text, Key key = Key::none());
    Text(std::string text, TextStyle s, Key key = Key::none());

    [[nodiscard]] std::string_view typeName() const override { return "Text"; }
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<Text> text(std::string text);
inline std::shared_ptr<Text> text(std::string text, TextStyle style);
inline std::shared_ptr<Text> text(TextProps props);

} // namespace enki
```

---

## Properties Reference (`TextProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `text` | `std::string` | `""` | The plain text string to display. |
| `color` | `std::optional<Color>` | `0xFFFFFFFF` (White) | Text glyph color (32-bit ARGB). |
| `font_size` | `std::optional<float>` | `14.0f` | Font size in logical pixels. |
| `font_weight` | `std::optional<FontWeight>` | `FontWeight::Normal` | Weight of font glyphs (`Thin`, `Medium`, `Bold`, etc.). |
| `font_style` | `std::optional<FontStyle>` | `FontStyle::Normal` | Font slope (`Normal` or `Italic`). |
| `font_family` | `std::optional<std::string>` | `""` (System default) | Primary typeface name (e.g. `"Inter"`, `"Roboto"`). |
| `letter_spacing` | `std::optional<float>` | `0.0f` | Spacing between characters in pixels. |
| `word_spacing` | `std::optional<float>` | `0.0f` | Spacing between words in pixels. |
| `height` | `std::optional<float>` | `std::nullopt` | Line height multiplier (e.g. `1.3f`). |
| `shadows` | `std::vector<BoxShadow>` | `{}` | List of drop shadows applied under glyphs. |
| `text_align` | `std::optional<TextAlign>` | `TextAlign::Start` | Horizontal alignment (`Start`, `Center`, `End`, `Justify`). |
| `text_direction` | `std::optional<TextDirection>` | `TextDirection::LTR` | BiDi text direction (`LTR` or `RTL`). |
| `overflow` | `std::optional<TextOverflow>` | `TextOverflow::Clip` | Visual overflow handling (`Clip`, `Ellipsis`, `Fade`). |
| `max_lines` | `std::optional<size_t>` | `std::nullopt` | Maximum visible lines before truncating. |
| `soft_wrap` | `std::optional<bool>` | `true` | Whether text wraps onto new lines when hitting boundary. |
| `selectable` | `std::optional<bool>` | `false` | Enables mouse click-and-drag text selection and copy. |
| `selection_color` | `std::optional<Color>` | `0x6038BDF8` | Highlight color behind selected text. |
| `key` | `Key` | `Key::none()` | Identifier used during tree reconciliation. |

---

## Typography Enums

### `FontWeight`
- `FontWeight::Thin` (100)
- `FontWeight::ExtraLight` (200)
- `FontWeight::Light` (300)
- `FontWeight::Normal` / `Regular` (400)
- `FontWeight::Medium` (500)
- `FontWeight::SemiBold` (600)
- `FontWeight::Bold` (700)
- `FontWeight::ExtraBold` (800)
- `FontWeight::Black` (900)

### `TextOverflow`
- `TextOverflow::Clip` — Content overflowing the bounds is clipped.
- `TextOverflow::Ellipsis` — Truncates text with an ellipsis (`...`) at the last line.
- `TextOverflow::Fade` — Fades the edge of the text.

---

## Code Examples

### 1. Simple Heading & Subtitle (Pattern from `widgets_demo/`)
```cpp
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildCardTitles() {
    return column({
        .gap = 4_px,
        .children = {
            text("Performance Analytics", {
                .font_size = 20.0f,
                .font_weight = FontWeight::Bold,
                .color = 0xFFF8FAFC,
            }),
            text("Real-time GPU frame budget and memory allocation.", {
                .font_size = 13.0f,
                .color = 0xFF94A3B8,
            }),
        }
    });
}
```

### 2. Multi-line Truncated Text with Ellipsis
```cpp
auto descriptionLabel = text({
    .text = "This is a very long paragraph that might exceed the available preview width...",
    .font_size = 14.0f,
    .max_lines = 2,
    .overflow = TextOverflow::Ellipsis,
    .soft_wrap = true,
});
```

### 3. Selectable Text with Custom Highlight
```cpp
auto codeSnippet = text({
    .text = "git clone https://github.com/enki/enki.git",
    .font_family = "Fira Code",
    .selectable = true,
    .selection_color = 0x806366F1, // Purple highlight
});
```

---

## See Also
- [**RichText**](./rich_text.md) — For multi-styled inline spans with links and spans.
- [**Icon**](./icon.md) — Vector icons accompanying text labels.
