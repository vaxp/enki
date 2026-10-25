# Enki Typography & Code Presentation Suite

> Hardware-accelerated text layout via Skia SkParagraph, interactive mouse selection, clipboard copying, auto-scrolling ticker marquees, and syntax-highlighted code blocks.

The **Typography — Extended** category provides dedicated components for advanced desktop text interaction and developer tool displays. Built on Enki's high-performance paragraph layout engine (`RenderParagraph`), these widgets support mouse drag selection, system clipboard integration, continuous smooth ticker animation, and syntax-colored monospace code display.

---

## Architectural Highlights

- **SkParagraph Engine**: Subpixel anti-aliased font rendering, ligature support, bidirectional text (LTR/RTL), and accurate glyph bounding box queries.
- **Interactive Mouse Selection**: `SelectableText` enables mouse drag selection ranges, double-click word selection, triple-click paragraph selection, and clipboard copy operations via `Platform::instance()->setClipboardText()`.
- **Fading Edge Ticker Tape**: `Marquee` executes continuous single-line horizontal translation at configurable speeds (`velocity`), pausing when the mouse hovers, and applies soft alpha gradient masks to both edges (`fading_edge_length`).
- **Syntax Lexing & Themes**: `CodeBlock` features built-in tokenizers for C++, JSON, and Python, displaying line numbers, line highlighting, and popular color palettes (`One Dark`, `Dracula`, `VS Code Dark`, `GitHub Dark`).

---

## Widget Catalog (Typography — Extended)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**SelectableText**](./selectable_text.md) | `struct SelectableText`, `selectableText()` | `<enki/widgets/selectable_text.hpp>` | Interactive text supporting mouse drag highlight, word selection, and clipboard copy. |
| 2 | [**Marquee**](./marquee.md) | `struct Marquee`, `marquee()` | `<enki/widgets/marquee.hpp>` | Smooth auto-scrolling ticker tape with configurable velocity, direction, and edge fading. |
| 3 | [**CodeBlock**](./code_block.md) | `struct CodeBlock`, `codeBlock()` | `<enki/widgets/code_block.hpp>` | Syntax-highlighted monospace code viewer with line numbers, copy button, and themes. |

---

## Quick Example (Developer Documentation Card)

```cpp
#include "enki/widgets/typography.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildCodeSnippetCard() {
    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(12.0f),
        .padding = EdgeInsets::all(20.0f),
        .child = column({
            .gap = 12_px,
            .children = {
                // 1. Marquee ticker status
                Marquee {
                    .text = "🚀 NEW: ENKI v0.2.0 is now live!  •  Skia 2D GPU Acceleration  •  Pure C++20 Declarative Syntax",
                    .velocity = 55.0f,
                    .pause_on_hover = true,
                    .color = 0xFF38BDF8
                },

                // 2. Selectable explanation
                SelectableText {
                    .text = "You can highlight any portion of this text and press Ctrl+C to copy it.",
                    .color = 0xFFCBD5E1,
                    .selection_color = 0x6038BDF8
                },

                // 3. Syntax highlighted code block
                CodeBlock {
                    .code = "auto app = runApp(std::make_shared<MyApp>());",
                    .language = "cpp",
                    .show_line_numbers = true,
                    .show_copy_button = true,
                    .theme = CodeTheme::oneDark()
                }
            }
        })
    });
}
```
