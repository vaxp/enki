/// @file code_block.cpp
/// @brief CodeBlock Widget Implementation for ENKI Framework.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/code_block.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_set>

namespace enki {

namespace {

enum class TokenType {
    Keyword,
    Type,
    String,
    Number,
    Comment,
    Function,
    Preprocessor,
    Operator,
    Plain,
};

struct Token {
    std::string text;
    TokenType   type;
};

class Lexer {
public:
    static std::vector<std::vector<Token>> highlight(const std::string& code, const std::string& language) {
        static const std::unordered_set<std::string> cpp_keywords = {
            "auto", "break", "case", "catch", "class", "const", "constexpr", "continue",
            "default", "delete", "do", "else", "enum", "explicit", "export", "extern",
            "for", "friend", "goto", "if", "inline", "mutable", "namespace", "new",
            "noexcept", "nullptr", "operator", "private", "protected", "public",
            "register", "reinterpret_cast", "return", "sizeof", "static", "static_assert",
            "static_cast", "struct", "switch", "template", "this", "throw", "try",
            "typedef", "typeid", "typename", "union", "using", "virtual", "volatile", "while"
        };

        static const std::unordered_set<std::string> cpp_types = {
            "int", "float", "double", "char", "bool", "void", "short", "long", "unsigned",
            "signed", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
            "int32_t", "int64_t", "size_t", "std", "string", "string_view", "vector",
            "unique_ptr", "shared_ptr", "weak_ptr", "optional", "function", "Color",
            "WidgetPtr", "TextStyle", "Rect", "Point", "Size", "Key", "BuildContext"
        };

        static const std::unordered_set<std::string> py_keywords = {
            "def", "class", "import", "from", "as", "return", "if", "elif", "else",
            "for", "while", "break", "continue", "pass", "try", "except", "finally",
            "raise", "with", "yield", "lambda", "global", "nonlocal", "True", "False", "None"
        };

        static const std::unordered_set<std::string> json_keywords = {
            "true", "false", "null"
        };

        std::vector<std::vector<Token>> lines;
        std::vector<Token> current_line;
        size_t i = 0;
        size_t n = code.length();

        while (i < n) {
            char c = code[i];

            // Handle newline
            if (c == '\n') {
                lines.push_back(std::move(current_line));
                current_line.clear();
                i++;
                continue;
            }

            // Handle single-line comments
            if (c == '/' && i + 1 < n && code[i + 1] == '/') {
                size_t start = i;
                while (i < n && code[i] != '\n') i++;
                current_line.push_back({code.substr(start, i - start), TokenType::Comment});
                continue;
            }

            // Python / Bash single-line comments
            if (c == '#' && (language == "py" || language == "python" || language == "bash" || language == "cmake")) {
                size_t start = i;
                while (i < n && code[i] != '\n') i++;
                current_line.push_back({code.substr(start, i - start), TokenType::Comment});
                continue;
            }

            // C Preprocessor directives
            if (c == '#' && (language == "cpp" || language == "c")) {
                size_t start = i;
                while (i < n && !std::isspace(static_cast<unsigned char>(code[i]))) i++;
                current_line.push_back({code.substr(start, i - start), TokenType::Preprocessor});
                continue;
            }

            // Multi-line comments
            if (c == '/' && i + 1 < n && code[i + 1] == '*') {
                size_t start = i;
                i += 2;
                while (i + 1 < n && !(code[i] == '*' && code[i + 1] == '/')) {
                    if (code[i] == '\n') {
                        current_line.push_back({code.substr(start, i - start), TokenType::Comment});
                        lines.push_back(std::move(current_line));
                        current_line.clear();
                        start = i + 1;
                    }
                    i++;
                }
                if (i + 1 < n) i += 2;
                current_line.push_back({code.substr(start, i - start), TokenType::Comment});
                continue;
            }

            // String literals
            if (c == '"' || c == '\'') {
                char quote = c;
                size_t start = i;
                i++;
                while (i < n && code[i] != quote && code[i] != '\n') {
                    if (code[i] == '\\' && i + 1 < n) i++;
                    i++;
                }
                if (i < n && code[i] == quote) i++;
                current_line.push_back({code.substr(start, i - start), TokenType::String});
                continue;
            }

            // Numbers
            if (std::isdigit(static_cast<unsigned char>(c)) || (c == '0' && i + 1 < n && (code[i + 1] == 'x' || code[i + 1] == 'X'))) {
                size_t start = i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(code[i])) || code[i] == '.' || code[i] == '_')) {
                    i++;
                }
                current_line.push_back({code.substr(start, i - start), TokenType::Number});
                continue;
            }

            // Identifiers / Keywords
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                size_t start = i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(code[i])) || code[i] == '_')) {
                    i++;
                }
                std::string word = code.substr(start, i - start);

                // Check function call
                size_t peek = i;
                while (peek < n && std::isspace(static_cast<unsigned char>(code[peek]))) peek++;
                bool is_func = (peek < n && code[peek] == '(');

                TokenType type = TokenType::Plain;
                if (cpp_keywords.count(word) || py_keywords.count(word) || json_keywords.count(word)) {
                    type = TokenType::Keyword;
                } else if (cpp_types.count(word)) {
                    type = TokenType::Type;
                } else if (is_func) {
                    type = TokenType::Function;
                }

                current_line.push_back({std::move(word), type});
                continue;
            }

            // Operators
            if (std::ispunct(static_cast<unsigned char>(c))) {
                current_line.push_back({std::string(1, c), TokenType::Operator});
                i++;
                continue;
            }

            // Spaces / Plain characters
            size_t start = i;
            while (i < n && std::isspace(static_cast<unsigned char>(code[i])) && code[i] != '\n') {
                i++;
            }
            if (i > start) {
                current_line.push_back({code.substr(start, i - start), TokenType::Plain});
            } else {
                current_line.push_back({std::string(1, c), TokenType::Plain});
                i++;
            }
        }

        if (!current_line.empty() || lines.empty()) {
            lines.push_back(std::move(current_line));
        }

        return lines;
    }
};

} // namespace

class CodeBlockState : public State {
    bool copied_ = false;
    std::string cached_code_;
    std::string cached_lang_;
    std::vector<std::vector<Token>> cached_tokens_;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const CodeBlockWidget*>(widget());
        const auto& p = w->props;
        const auto& theme = p.theme;

        if (cached_code_ != p.code || cached_lang_ != p.language) {
            cached_code_ = p.code;
            cached_lang_ = p.language;
            cached_tokens_ = Lexer::highlight(p.code, p.language);
        }

        const auto& token_lines = cached_tokens_;

        // ── 1. Header Bar (Window controls + Language + Copy) ──
        WidgetPtr header = nullptr;
        if (p.show_header) {
            std::string lang_upper = p.language;
            for (auto& c : lang_upper) c = std::toupper(static_cast<unsigned char>(c));

            auto copy_btn = button(
                text(copied_ ? "✓ Copied!" : "Copy", {
                    .color = copied_ ? 0xFF34D399 : 0xFFCBD5E1,
                    .font_size = 11.0f,
                    .font_weight = FontWeight::Bold,
                }),
                [this, code_text = p.code]() {
                    if (auto* plt = Platform::instance()) {
                        plt->setClipboardText(code_text);
                    }
                    setState([this]() { copied_ = true; });
                },
                ButtonProps{
                    .normal_color = 0x25FFFFFF,
                    .hover_color = 0x40FFFFFF,
                    .pressed_color = 0x15FFFFFF,
                    .border_radius = 4.0f,
                    .padding = EdgeInsets::symmetric(4.0f, 10.0f),
                    .shadow_blur = 0.0f,
                }
            );

            auto dots_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    container({ .color = 0xFFEF4444, .border_radius = BorderRadius::circular(5.0f), .width = StyleValue::point(10.0f), .height = StyleValue::point(10.0f) }),
                    container({ .color = 0xFFF59E0B, .border_radius = BorderRadius::circular(5.0f), .width = StyleValue::point(10.0f), .height = StyleValue::point(10.0f) }),
                    container({ .color = 0xFF10B981, .border_radius = BorderRadius::circular(5.0f), .width = StyleValue::point(10.0f), .height = StyleValue::point(10.0f) }),
                    sizedBox(6.0f, 0.0f),
                    text(lang_upper, {
                        .color = theme.line_number_fg,
                        .font_size = 11.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                },
            });

            header = container({
                .color = theme.line_number_bg,
                .border_radius = BorderRadius::only(theme.border_radius.top_left, theme.border_radius.top_right, 0.0f, 0.0f),
                .padding = StyleInsets::symmetric(8.0f, 12.0f),
                .child = row({
                    .justify_content = Justify::SpaceBetween,
                    .align_items = Align::Center,
                    .children = {
                        dots_row,
                        p.show_copy_button ? WidgetPtr(copy_btn) : WidgetPtr(sizedBox(0.0f, 0.0f)),
                    },
                }),
            });
        }

        // ── 2. Code Body (Line numbers + Syntax Spans) ─────────
        std::vector<WidgetPtr> line_rows;
        line_rows.reserve(token_lines.size());

        for (size_t line_idx = 0; line_idx < token_lines.size(); ++line_idx) {
            int line_num = static_cast<int>(line_idx + 1);
            bool is_highlighted = std::find(p.highlighted_lines.begin(), p.highlighted_lines.end(), line_num) != p.highlighted_lines.end();

            std::vector<std::shared_ptr<InlineSpan>> spans;
            for (const auto& tok : token_lines[line_idx]) {
                Color tok_color = theme.default_text;
                FontStyle tok_style = FontStyle::Normal;

                switch (tok.type) {
                    case TokenType::Keyword:      tok_color = theme.keyword; break;
                    case TokenType::Type:         tok_color = theme.type; break;
                    case TokenType::String:       tok_color = theme.string; break;
                    case TokenType::Number:       tok_color = theme.number; break;
                    case TokenType::Comment:      tok_color = theme.comment; tok_style = FontStyle::Italic; break;
                    case TokenType::Function:     tok_color = theme.function; break;
                    case TokenType::Preprocessor: tok_color = theme.preprocessor; break;
                    case TokenType::Operator:     tok_color = theme.operator_color; break;
                    case TokenType::Plain:        tok_color = theme.default_text; break;
                }

                spans.push_back(span({
                    .text = tok.text,
                    .style = TextStyle{
                        .color = tok_color,
                        .font_size = p.font_size,
                        .font_style = tok_style,
                        .font_family = p.font_family,
                    },
                }));
            }

            auto line_rich_text = richText(span({
                .children = std::move(spans),
            }));

            std::vector<WidgetPtr> row_items;
            if (p.show_line_numbers) {
                row_items.push_back(container({
                    .color = is_highlighted ? theme.highlight_line_bg : theme.line_number_bg,
                    .align = Alignment::CenterRight,
                    .width = StyleValue::point(40.0f),
                    .padding = StyleInsets::symmetric(0.0f, 10.0f),
                    .child = text(std::to_string(line_num), {
                        .color = theme.line_number_fg,
                        .font_size = p.font_size,
                        .font_family = p.font_family,
                    }),
                }));
            }

            row_items.push_back(container({
                .padding = StyleInsets::symmetric(0.0f, 12.0f),
                .flex_grow = 1.0f,
                .child = line_rich_text,
            }));

            line_rows.push_back(container({
                .color = is_highlighted ? theme.highlight_line_bg : Colors::Transparent,
                .child = row({
                    .align_items = Align::Center,
                    .children = std::move(row_items),
                }),
            }));
        }

        auto code_content = column({
            .gap = StyleValue::point(2.0f),
            .padding = StyleInsets::symmetric(10.0f, 0.0f),
            .children = std::move(line_rows),
        });

        std::vector<WidgetPtr> block_children;
        if (header) block_children.push_back(header);
        block_children.push_back(code_content);

        return container({
            .color = theme.background,
            .border_radius = theme.border_radius,
            .child = column({
                .children = std::move(block_children),
            }),
        });
    }
};

std::unique_ptr<State> CodeBlockWidget::createState() {
    return std::make_unique<CodeBlockState>();
}

} // namespace enki
