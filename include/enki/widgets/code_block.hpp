#pragma once
/// @file code_block.hpp
/// @brief CodeBlock Widget for ENKI Framework.
/// Monospace syntax-highlighted code display with line numbers, copy button,
/// language badge, line highlighting, and color themes.
///
/// 100% C++20 Declarative Syntax.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"
#include <string>
#include <vector>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// CodeTheme Model
// ════════════════════════════════════════════════════════════════

struct CodeTheme {
    Color background       = 0xFF1E222A;
    Color line_number_bg   = 0xFF181A1F;
    Color line_number_fg   = 0xFF5C6370;
    Color default_text     = 0xFFABB2BF;
    Color keyword          = 0xFFC678DD;
    Color type             = 0xFFE5C07B;
    Color string           = 0xFF98C379;
    Color number           = 0xFFD19A66;
    Color comment          = 0xFF5C6370;
    Color function         = 0xFF61AFEF;
    Color preprocessor     = 0xFFE06C75;
    Color operator_color   = 0xFF56B6C2;
    Color highlight_line_bg = 0x2538BDF8;
    BorderRadius border_radius = BorderRadius::circular(8.0f);

    static CodeTheme oneDark() {
        return CodeTheme{};
    }

    static CodeTheme dracula() {
        CodeTheme t;
        t.background       = 0xFF282A36;
        t.line_number_bg   = 0xFF21222C;
        t.line_number_fg   = 0xFF6272A4;
        t.default_text     = 0xFFF8F8F2;
        t.keyword          = 0xFFFF79C6;
        t.type             = 0xFF8BE9FD;
        t.string           = 0xFFF1FA8C;
        t.number           = 0xFFBD93F9;
        t.comment          = 0xFF6272A4;
        t.function         = 0xFF50FA7B;
        t.preprocessor     = 0xFFFF79C6;
        t.operator_color   = 0xFFFF79C6;
        t.highlight_line_bg = 0x30BD93F9;
        return t;
    }

    static CodeTheme vsCodeDark() {
        CodeTheme t;
        t.background       = 0xFF1E1E1E;
        t.line_number_bg   = 0xFF181818;
        t.line_number_fg   = 0xFF858585;
        t.default_text     = 0xFFD4D4D4;
        t.keyword          = 0xFF569CD6;
        t.type             = 0xFF4EC9B0;
        t.string           = 0xFFCE9178;
        t.number           = 0xFFB5CEA8;
        t.comment          = 0xFF6A9955;
        t.function         = 0xFFDCDCAA;
        t.preprocessor     = 0xFFC586C0;
        t.operator_color   = 0xFFD4D4D4;
        t.highlight_line_bg = 0x25569CD6;
        return t;
    }

    static CodeTheme githubDark() {
        CodeTheme t;
        t.background       = 0xFF0D1117;
        t.line_number_bg   = 0xFF161B22;
        t.line_number_fg   = 0xFF484F58;
        t.default_text     = 0xFFC9D1D9;
        t.keyword          = 0xFFFF7B72;
        t.type             = 0xFFFFA657;
        t.string           = 0xFFA5D6FF;
        t.number           = 0xFF79C0FF;
        t.comment          = 0xFF8B949E;
        t.function         = 0xFFD2A8FF;
        t.preprocessor     = 0xFFFF7B72;
        t.operator_color   = 0xFF79C0FF;
        t.highlight_line_bg = 0x2558A6FF;
        return t;
    }

    constexpr bool operator==(const CodeTheme&) const = default;
};

// ════════════════════════════════════════════════════════════════
// CodeBlock Widget
// ════════════════════════════════════════════════════════════════

struct CodeBlockProps {
    Key                 key = Key::none();
    std::string         code;
    std::string         language = "cpp";
    bool                show_line_numbers = true;
    bool                show_copy_button = true;
    bool                show_header = true;
    std::vector<int>    highlighted_lines = {};
    CodeTheme           theme = CodeTheme::oneDark();
    float               font_size = 13.0f;
    std::string         font_family = "monospace";
};

class CodeBlockWidget : public StatefulWidget {
public:
    CodeBlockProps props;

    CodeBlockWidget() = default;
    explicit CodeBlockWidget(CodeBlockProps p)
        : StatefulWidget(p.key), props(std::move(p)) {}
    CodeBlockWidget(Key k, CodeBlockProps p)
        : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CodeBlock"; }
};

struct CodeBlock {
    Key                 key = Key::none();
    std::string         code = "";
    std::string         language = "cpp";
    bool                show_line_numbers = true;
    bool                show_copy_button = true;
    bool                show_header = true;
    std::vector<int>    highlighted_lines = {};
    CodeTheme           theme = CodeTheme::oneDark();
    float               font_size = 13.0f;
    std::string         font_family = "monospace";

    operator WidgetPtr() const {
        CodeBlockProps p;
        p.key = key;
        p.code = code;
        p.language = language;
        p.show_line_numbers = show_line_numbers;
        p.show_copy_button = show_copy_button;
        p.show_header = show_header;
        p.highlighted_lines = highlighted_lines;
        p.theme = theme;
        p.font_size = font_size;
        p.font_family = font_family;
        return std::make_shared<CodeBlockWidget>(key, std::move(p));
    }
};

inline std::shared_ptr<CodeBlockWidget> codeBlock(std::string code, std::string language = "cpp") {
    CodeBlockProps p;
    p.code = std::move(code);
    p.language = std::move(language);
    return std::make_shared<CodeBlockWidget>(std::move(p));
}

inline std::shared_ptr<CodeBlockWidget> codeBlock(CodeBlockProps props) {
    return std::make_shared<CodeBlockWidget>(std::move(props));
}

} // namespace enki
