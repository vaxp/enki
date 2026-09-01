# RichText

> A paragraph widget that renders formatted text composed of multiple `TextSpan` elements with individual styling, font weights, colors, and interactive click/hover callbacks.

- **Header File**: `#include "enki/widgets/text.hpp"`
- **C++ Class**: `enki::RichText` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Props Struct**: `enki::RichTextProps`
- **Span Types**: `enki::TextSpan`, `enki::InlineSpan`
- **Helpers**: `richText(...)`, `span(...)`
- **Underlying Engine**: Skia `SkParagraph` with rich span tree construction

---

## Overview

Unlike `Text` (which applies a uniform style across the entire string), `RichText` accepts a hierarchical tree of `InlineSpan` / `TextSpan` objects. Different sections of a sentence can have distinct font sizes, colors, decorations, and click/hover handlers, making it ideal for terms of service links, user mentions (`@user`), code highlights, and inline tags.

---

## C++ API Definition

### Class Declaration
```cpp
namespace enki {

class RichText : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<InlineSpan>         text_span;
    TextStyle                           default_style;
    TextAlign                           text_align = TextAlign::Start;
    TextDirection                       text_direction = TextDirection::LTR;
    TextOverflow                        overflow = TextOverflow::Clip;
    std::optional<size_t>               max_lines;
    bool                                soft_wrap = true;
    bool                                selectable = false;
    Color                               selection_color = 0x6038BDF8;
    std::function<void(TextSelection)>  on_selection_changed = nullptr;

    explicit RichText(std::shared_ptr<InlineSpan> span, Key key = Key::none());

    [[nodiscard]] std::string_view typeName() const override { return "RichText"; }
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<RichText> richText(std::shared_ptr<InlineSpan> span);
inline std::shared_ptr<RichText> richText(RichTextProps props);

inline std::shared_ptr<TextSpan> span(TextSpanProps props);
inline std::shared_ptr<TextSpan> span(std::string text,
                                      std::optional<TextStyle> style = std::nullopt,
                                      std::vector<std::shared_ptr<InlineSpan>> children = {});

} // namespace enki
```

---

## `TextSpanProps` Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `text` | `std::string` | `""` | Text snippet rendered for this span. |
| `style` | `std::optional<TextStyle>` | `std::nullopt` | Individual style override for this span. |
| `children` | `std::vector<std::shared_ptr<InlineSpan>>` | `{}` | Nested child spans that inherit and extend this span's style. |
| `on_click` | `std::function<void()>` | `nullptr` | Callback triggered when the user clicks this specific text span. |
| `on_hover` | `std::function<void(bool is_hovered)>` | `nullptr` | Callback triggered when mouse enters or leaves this text span. |

---

## Code Examples (From `widgets_demo/richtext_demo/main.cpp`)

### 1. Mixed Styles & Clickable Mentions
```cpp
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildTermsNotice() {
    auto linkSpan = span({
        .text = "Terms of Service",
        .style = TextStyle{
            .color = 0xFF38BDF8,
            .decoration = TextDecoration::Underline,
            .decoration_color = 0xFF38BDF8,
        },
        .on_click = []() {
            // Open terms in browser or modal dialog
        },
    });

    auto fullParagraph = span({
        .text = "",
        .children = {
            span("By proceeding, you agree to our "),
            linkSpan,
            span(" and Privacy Policy."),
        }
    });

    return richText({
        .text_span = fullParagraph,
        .default_style = TextStyle{
            .color = 0xFF94A3B8,
            .font_size = 13.5f,
        },
    });
}
```

### 2. Nested Spans with State-Driven Hover
```cpp
auto interactiveUser = span({
    .text = "@enki_dev",
    .style = TextStyle{
        .color = is_hovered ? 0xFFFBCFE8 : 0xFFF472B6,
        .font_weight = FontWeight::Bold,
    },
    .on_hover = [this](bool hovered) {
        setState([this, hovered]() { is_hovered = hovered; });
    }
});
```

---

## See Also
- [**Text**](./text.md) — Standard single-style text component.
