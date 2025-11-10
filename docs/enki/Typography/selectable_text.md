# SelectableText

> An interactive typography widget that enables mouse drag highlight selection, word/all text selection, and direct copying to the system clipboard.

- **Header File**: `#include "enki/widgets/selectable_text.hpp"`
- **Declarative Struct**: `enki::SelectableText` (converts implicitly to `WidgetPtr`)
- **Factory Helpers**: `enki::selectableText()`
- **Selection Event**: `enki::TextSelection` (`start()`, `end()`, `isValid()`, `isCollapsed()`)

---

## Overview

Unlike standard `Text` widgets (which are lightweight and read-only), `SelectableText` enables full mouse pointer interaction:
- **Click & Drag**: Highlights arbitrary character ranges.
- **Double-Click**: Selects an entire word.
- **Triple-Click**: Selects the entire paragraph.
- **Clipboard Copy**: Integrates with `Platform::instance()->setClipboardText()` or user keyboard shortcut `Ctrl+C`.

---

## C++ API Definition

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct SelectableText {
    std::string                        text                 = "";
    std::optional<TextStyle>           style                = std::nullopt;
    std::optional<Color>               color                = std::nullopt;
    std::optional<float>               font_size            = std::nullopt;
    std::optional<FontWeight>          font_weight          = std::nullopt;
    std::optional<FontStyle>           font_style           = std::nullopt;
    std::optional<std::string>         font_family          = std::nullopt;
    Color                              selection_color      = 0x6038BDF8; ///< Highlight scrim color
    std::optional<TextAlign>           text_align           = std::nullopt;
    std::optional<TextDirection>       text_direction       = std::nullopt;
    std::optional<TextOverflow>        overflow             = std::nullopt;
    std::optional<size_t>              max_lines            = std::nullopt;
    std::optional<bool>                soft_wrap            = std::nullopt;
    std::function<void(TextSelection)> on_selection_changed = nullptr;
    Key                                key                  = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<Text> selectableText(std::string text);
inline std::shared_ptr<Text> selectableText(std::string text, TextStyle style);
inline std::shared_ptr<Text> selectableText(TextProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `text` | `string` | `""` | The string content to render. |
| `selection_color` | `Color` | `0x6038BDF8` | Semi-transparent highlight tint drawn beneath selected glyphs. |
| `font_size` | `optional<float>` | `nullopt` | Font size in points. |
| `font_weight` | `optional<FontWeight>`| `nullopt`| Font thickness (`Normal`, `Bold`, `SemiBold`). |
| `on_selection_changed`| `function<void(TextSelection)>`| `nullptr`| Dispatched whenever the mouse selection range is updated. |

---

## Code Examples (From `widgets_demo/typography_demo/main.cpp`)

### 1. Selectable Paragraph with Dynamic Metric Updates & Copy
```cpp
#include "enki/widgets/selectable_text.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/button.hpp"
#include "enki/platform/platform.hpp"

using namespace enki;

WidgetPtr buildArticleParagraph() {
    std::string content = "SelectableText allows native text highlight and clipboard export.";

    return SelectableText {
        .text = content,
        .style = TextStyle{
            .color = 0xFFCBD5E1,
            .font_size = 14.0f,
            .height = 1.6f
        },
        .selection_color = 0x6038BDF8, // Translucent sky blue
        .on_selection_changed = [](TextSelection sel) {
            if (sel.isValid() && !sel.isCollapsed()) {
                std::cout << "Selected range: " << sel.start() << " to " << sel.end() << "\n";
            }
        }
    };
}
```

---

## See Also
- [**Text**](../Basic%20UI/text.md) — Standard static text widget.
- [**RichText**](../Basic%20UI/rich_text.md) — Multi-style inline text spans.
- [**CodeBlock**](./code_block.md) — Syntax-highlighted code with line numbers.
