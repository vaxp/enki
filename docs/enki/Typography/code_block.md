# CodeBlock

> A monospace syntax-highlighted code display widget with line numbers, a one-click clipboard copy button, multi-language support, custom line highlighting, and popular color themes.

- **Header File**: `#include "enki/widgets/code_block.hpp"`
- **C++ Class**: `enki::CodeBlockWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::CodeBlock` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::CodeBlockProps`
- **Factory Helpers**: `enki::codeBlock()`
- **Themes Model**: `enki::CodeTheme` (`oneDark()`, `dracula()`, `vsCodeDark()`, `githubDark()`)

---

## Overview

`CodeBlock` provides developer-friendly code presentation for technical documentation, tutorials, settings viewers, and debug monitors. It includes built-in tokenizers for multiple languages (including C++, JSON, and Python) to highlight keywords, types, strings, numbers, and comments. It also features optional gutter line numbers, a header banner with a language badge, line highlighting (e.g. lines 5 and 6), and an integrated copy-to-clipboard button.

---

## C++ API Definition

### `CodeTheme` Struct
```cpp
namespace enki {

struct CodeTheme {
    Color        background        = 0xFF1E222A;
    Color        line_number_bg    = 0xFF181A1F;
    Color        line_number_fg    = 0xFF5C6370;
    Color        default_text      = 0xFFABB2BF;
    Color        keyword           = 0xFFC678DD;
    Color        type              = 0xFFE5C07B;
    Color        string            = 0xFF98C379;
    Color        number            = 0xFFD19A66;
    Color        comment           = 0xFF5C6370;
    Color        function          = 0xFF61AFEF;
    Color        preprocessor      = 0xFFE06C75;
    Color        operator_color    = 0xFF56B6C2;
    Color        highlight_line_bg = 0x2538BDF8;
    BorderRadius border_radius     = BorderRadius::circular(8.0f);

    static CodeTheme oneDark();
    static CodeTheme dracula();
    static CodeTheme vsCodeDark();
    static CodeTheme githubDark();
};

} // namespace enki
```

### Declarative Struct & Factory Functions
```cpp
namespace enki {

struct CodeBlock {
    Key              key               = Key::none();
    std::string      code              = "";
    std::string      language          = "cpp";              ///< "cpp", "json", "python", etc.
    bool             show_line_numbers = true;
    bool             show_copy_button  = true;
    bool             show_header       = true;               ///< Top toolbar with language badge & copy button
    std::vector<int> highlighted_lines = {};                 ///< 1-indexed lines with background highlight tint
    CodeTheme        theme             = CodeTheme::oneDark();
    float            font_size         = 13.0f;
    std::string      font_family       = "monospace";

    operator WidgetPtr() const;
};

inline std::shared_ptr<CodeBlockWidget> codeBlock(std::string code, std::string language = "cpp");
inline std::shared_ptr<CodeBlockWidget> codeBlock(CodeBlockProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `code` | `string` | `""` | Source code string to format and highlight. |
| `language` | `string` | `"cpp"` | Language syntax lexer (`"cpp"`, `"json"`, `"python"`). |
| `show_line_numbers` | `bool` | `true` | Renders a left gutter with line count numerals. |
| `show_copy_button` | `bool` | `true` | Displays a copy-to-clipboard button in the header. |
| `show_header` | `bool` | `true` | Displays the top header bar showing language and tools. |
| `highlighted_lines`| `vector<int>` | `{}` | List of line numbers (1-based) to visually emphasize. |
| `theme` | `CodeTheme` | `oneDark()`| Syntax palette (`oneDark()`, `dracula()`, `vsCodeDark()`, `githubDark()`). |

---

## Code Examples (From `widgets_demo/typography_demo/main.cpp`)

### 1. C++ Code Block with Dracula Theme & Line Highlighting
```cpp
#include "enki/widgets/code_block.hpp"

using namespace enki;

WidgetPtr buildCppCodeSnippet() {
    std::string sourceCode =
        "#include <enki/widgets/code_block.hpp>\n\n"
        "WidgetPtr buildSnippet() {\n"
        "    return CodeBlock {\n"
        "        .code = \"int main() { return 0; }\",\n" // Line 5 (highlighted)
        "        .language = \"cpp\",\n"                  // Line 6 (highlighted)
        "        .theme = CodeTheme::dracula()\n"
        "    };\n"
        "}\n";

    return CodeBlock {
        .code = sourceCode,
        .language = "cpp",
        .show_line_numbers = true,
        .show_copy_button = true,
        .show_header = true,
        .highlighted_lines = {5, 6}, // Focus user attention on lines 5 & 6
        .theme = CodeTheme::dracula(),
        .font_size = 13.5f
    };
}
```

---

## See Also
- [**SelectableText**](./selectable_text.md) — Standard paragraph selection and copy.
- [**Marquee**](./marquee.md) — Auto-scrolling ticker tape.
- [**RichText**](../Basic%20UI/rich_text.md) — Formatted inline text spans.
