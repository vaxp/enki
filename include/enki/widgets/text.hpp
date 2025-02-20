#pragma once
/// @file text.hpp
/// @brief Advanced Typography & Text Widgets (Text, RichText, TextStyle, TextSpan).
///
/// Fully powered by Skia's SkParagraph engine and integrated with the Anu Flexbox layout
/// engine for automatic intrinsic sizing, multi-line wrapping, ellipsis overflow, and rich formatting.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/container.hpp" // For BoxShadow
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Typography Enumerations
// ════════════════════════════════════════════════════════════════

/// @brief Font weight values (100 to 900).
enum class FontWeight : int {
    Thin       = 100,
    ExtraLight = 200,
    Light      = 300,
    Normal     = 400,
    Regular    = 400,
    Medium     = 500,
    SemiBold   = 600,
    Bold       = 700,
    ExtraBold  = 800,
    Black      = 900,
};

/// @brief Font style variants.
enum class FontStyle {
    Normal = 0,
    Italic = 1,
};

/// @brief Horizontal text alignment within its bounding box.
enum class TextAlign {
    Left,
    Right,
    Center,
    Justify,
    Start,
    End,
};

/// @brief Direction of text flow (LTR or RTL for BiDi support).
enum class TextDirection {
    LTR,
    RTL,
};

/// @brief How visual overflow should be handled when text exceeds available space.
enum class TextOverflow {
    Clip,     ///< Clip the overflowing text at the edge.
    Ellipsis, ///< Render an ellipsis ("...") at the end of the last visible line.
    Fade,     ///< Fade the overflowing edge.
};

/// @brief Text decoration lines.
enum class TextDecoration : unsigned int {
    None        = 0,
    Underline   = 1 << 0,
    Overline    = 1 << 1,
    LineThrough = 1 << 2,
};

inline TextDecoration operator|(TextDecoration a, TextDecoration b) {
    return static_cast<TextDecoration>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline bool operator&(TextDecoration a, TextDecoration b) {
    return (static_cast<unsigned int>(a) & static_cast<unsigned int>(b)) != 0;
}

/// @brief Style of the text decoration stroke.
enum class TextDecorationStyle {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy,
};

// ════════════════════════════════════════════════════════════════
// TextStyle — Detailed Typography Configuration
// ════════════════════════════════════════════════════════════════

/// @brief Complete visual styling for a run of text.
struct TextStyle {
    Color                      color = 0xFFFFFFFF;                     ///< Text color (default: white).
    float                      font_size = 14.0f;                      ///< Font size in logical pixels (default: 14).
    FontWeight                 font_weight = FontWeight::Normal;       ///< Font weight (default: Normal).
    FontStyle                  font_style = FontStyle::Normal;         ///< Font style (Normal or Italic).
    std::string                font_family = "";                       ///< Primary font family name (e.g. "Inter", "Roboto").
    std::vector<std::string>   font_families;                          ///< Fallback font families in priority order.
    float                      letter_spacing = 0.0f;                  ///< Additional spacing between glyphs (in pixels).
    float                      word_spacing = 0.0f;                    ///< Additional spacing between words (in pixels).
    std::optional<float>       height;                                 ///< Line height multiplier (e.g. 1.2 = 120% font size).
    TextDecoration             decoration = TextDecoration::None;      ///< Text decoration lines (Underline, LineThrough...).
    Color                      decoration_color = 0xFFFFFFFF;          ///< Color for text decoration line.
    TextDecorationStyle        decoration_style = TextDecorationStyle::Solid; ///< Stroke style of decoration line.
    float                      decoration_thickness = 1.0f;            ///< Thickness of decoration line.
    std::vector<BoxShadow>     shadows;                                ///< Text shadows.

    // ── Fluent Builders ────────────────────────────────────────

    TextStyle& setColor(Color c) { color = c; return *this; }
    TextStyle& setFontSize(float s) { font_size = s; return *this; }
    TextStyle& setFontWeight(FontWeight w) { font_weight = w; return *this; }
    TextStyle& bold() { font_weight = FontWeight::Bold; return *this; }
    TextStyle& setFontStyle(FontStyle s) { font_style = s; return *this; }
    TextStyle& italic() { font_style = FontStyle::Italic; return *this; }
    TextStyle& setFontFamily(std::string family) { font_family = std::move(family); return *this; }
    TextStyle& addFontFamily(std::string family) { font_families.push_back(std::move(family)); return *this; }
    TextStyle& setLetterSpacing(float s) { letter_spacing = s; return *this; }
    TextStyle& setWordSpacing(float s) { word_spacing = s; return *this; }
    TextStyle& setHeight(float h) { height = h; return *this; }
    TextStyle& setDecoration(TextDecoration d, Color c = 0xFFFFFFFF, TextDecorationStyle s = TextDecorationStyle::Solid, float thickness = 1.0f) {
        decoration = d;
        decoration_color = c;
        decoration_style = s;
        decoration_thickness = thickness;
        return *this;
    }
    TextStyle& underline(Color c = 0xFFFFFFFF) { return setDecoration(TextDecoration::Underline, c); }
    TextStyle& lineThrough(Color c = 0xFFFFFFFF) { return setDecoration(TextDecoration::LineThrough, c); }
    TextStyle& addShadow(Color color, Point offset, float blurRadius) {
        shadows.push_back(BoxShadow(color, offset, blurRadius, 0));
        return *this;
    }

    bool operator==(const TextStyle& o) const {
        return color == o.color &&
               font_size == o.font_size &&
               font_weight == o.font_weight &&
               font_style == o.font_style &&
               font_family == o.font_family &&
               font_families == o.font_families &&
               letter_spacing == o.letter_spacing &&
               word_spacing == o.word_spacing &&
               height == o.height &&
               decoration == o.decoration &&
               decoration_color == o.decoration_color &&
               decoration_style == o.decoration_style &&
               decoration_thickness == o.decoration_thickness;
    }
    bool operator!=(const TextStyle& o) const { return !(*this == o); }
};

// ════════════════════════════════════════════════════════════════
// Spans for RichText
// ════════════════════════════════════════════════════════════════

class ParagraphBuilderContext;

/// @brief Abstract base class for an inline text span.
class InlineSpan {
public:
    virtual ~InlineSpan() = default;
    virtual void build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const = 0;
};

/// @brief An immutable text span with its own style and optional nested children spans.
class TextSpan : public InlineSpan {
public:
    std::string text;
    std::optional<TextStyle> style;
    std::vector<std::shared_ptr<InlineSpan>> children;

    TextSpan(std::string text = "",
             std::optional<TextStyle> style = std::nullopt,
             std::vector<std::shared_ptr<InlineSpan>> children = {})
        : text(std::move(text)), style(std::move(style)), children(std::move(children)) {}

    void build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const override;
};

inline std::shared_ptr<TextSpan> span(std::string text, std::optional<TextStyle> style = std::nullopt, std::vector<std::shared_ptr<InlineSpan>> children = {}) {
    return std::make_shared<TextSpan>(std::move(text), std::move(style), std::move(children));
}

// ════════════════════════════════════════════════════════════════
// RenderParagraph — Core Render Object for Text & RichText
// ════════════════════════════════════════════════════════════════

/// @brief Render object that lays out and paints SkParagraph text with Anu Flexbox integration.
class RenderParagraph : public RenderBox {
public:
    RenderParagraph(std::shared_ptr<InlineSpan> span,
                    TextStyle defaultStyle,
                    TextAlign textAlign = TextAlign::Start,
                    TextDirection textDirection = TextDirection::LTR,
                    TextOverflow overflow = TextOverflow::Clip,
                    std::optional<size_t> maxLines = std::nullopt,
                    bool softWrap = true);
    ~RenderParagraph() override;

    void setTextSpan(std::shared_ptr<InlineSpan> span);
    void setText(std::string data,
                 TextStyle style,
                 TextAlign align = TextAlign::Start,
                 TextDirection dir = TextDirection::LTR,
                 TextOverflow overflow = TextOverflow::Clip,
                 std::optional<size_t> maxLines = std::nullopt,
                 bool softWrap = true);

    void setDefaultStyle(TextStyle style);
    void setTextAlign(TextAlign align);
    void setTextDirection(TextDirection dir);
    void setOverflow(TextOverflow overflow);
    void setMaxLines(std::optional<size_t> maxLines);
    void setSoftWrap(bool softWrap);

    [[nodiscard]] const std::string& getTextData() const { return text_data_; }
    [[nodiscard]] const TextStyle& getDefaultStyle() const { return default_style_; }
    [[nodiscard]] TextAlign getTextAlign() const { return text_align_; }
    [[nodiscard]] TextDirection getTextDirection() const { return text_direction_; }
    [[nodiscard]] TextOverflow getOverflow() const { return overflow_; }
    [[nodiscard]] const std::optional<size_t>& getMaxLines() const { return max_lines_; }
    [[nodiscard]] bool getSoftWrap() const { return soft_wrap_; }

    void paint(PaintContext& ctx) override;

    /// Anu layout measurement callback.
    static ANUSize measureText(ANUNodeConstRef node, float width, ANUMeasureMode widthMode, float height, ANUMeasureMode heightMode);

private:
    void rebuildParagraph();
    void layoutParagraph(float availableWidth);

    std::string                 text_data_;
    std::shared_ptr<InlineSpan> span_;
    TextStyle                   default_style_;
    TextAlign                   text_align_;
    TextDirection               text_direction_;
    TextOverflow                overflow_;
    std::optional<size_t>       max_lines_;
    bool                        soft_wrap_;

    struct Impl;
    std::unique_ptr<Impl>       impl_;
};

// ════════════════════════════════════════════════════════════════
// Text Widget — Simple High-Level Text Component
// ════════════════════════════════════════════════════════════════

/// @brief A widget that displays a string of text with a single style.
class Text : public SingleChildRenderObjectWidget {
public:
    std::string           data;
    TextStyle             style;
    TextAlign             text_align = TextAlign::Start;
    TextDirection         text_direction = TextDirection::LTR;
    TextOverflow          overflow = TextOverflow::Clip;
    std::optional<size_t> max_lines;
    bool                  soft_wrap = true;

    explicit Text(std::string text, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), data(std::move(text)) {}

    Text(std::string text, TextStyle s, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), data(std::move(text)), style(std::move(s)) {}

    // ── Fluent Configuration ───────────────────────────────────

    Text& setStyle(TextStyle s) { style = std::move(s); return *this; }
    Text& color(Color c) { style.color = c; return *this; }
    Text& fontSize(float s) { style.font_size = s; return *this; }
    Text& fontWeight(FontWeight w) { style.font_weight = w; return *this; }
    Text& bold() { style.font_weight = FontWeight::Bold; return *this; }
    Text& italic() { style.font_style = FontStyle::Italic; return *this; }
    Text& fontFamily(std::string f) { style.font_family = std::move(f); return *this; }
    Text& letterSpacing(float s) { style.letter_spacing = s; return *this; }
    Text& height(float h) { style.height = h; return *this; }
    Text& shadow(Color c, Point offset = {0, 2}, float blurRadius = 4.0f) {
        style.addShadow(c, offset, blurRadius);
        return *this;
    }

    Text& textAlign(TextAlign a) { text_align = a; return *this; }
    Text& textDirection(TextDirection d) { text_direction = d; return *this; }
    Text& textOverflow(TextOverflow o) { overflow = o; return *this; }
    Text& ellipsis() { overflow = TextOverflow::Ellipsis; return *this; }
    Text& maxLines(size_t m) { max_lines = m; return *this; }
    Text& softWrap(bool w) { soft_wrap = w; return *this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Text"; }
};

// ════════════════════════════════════════════════════════════════
// RichText Widget — Rich Formatted Text Component
// ════════════════════════════════════════════════════════════════

/// @brief A widget that displays text using multiple spans with individual styles.
class RichText : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<InlineSpan> text_span;
    TextStyle                   default_style;
    TextAlign                   text_align = TextAlign::Start;
    TextDirection               text_direction = TextDirection::LTR;
    TextOverflow                overflow = TextOverflow::Clip;
    std::optional<size_t>       max_lines;
    bool                        soft_wrap = true;

    explicit RichText(std::shared_ptr<InlineSpan> span, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), text_span(std::move(span)) {}

    RichText& defaultStyle(TextStyle s) { default_style = std::move(s); return *this; }
    RichText& textAlign(TextAlign a) { text_align = a; return *this; }
    RichText& textDirection(TextDirection d) { text_direction = d; return *this; }
    RichText& textOverflow(TextOverflow o) { overflow = o; return *this; }
    RichText& ellipsis() { overflow = TextOverflow::Ellipsis; return *this; }
    RichText& maxLines(size_t m) { max_lines = m; return *this; }
    RichText& softWrap(bool w) { soft_wrap = w; return *this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "RichText"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Text> text(std::string text) {
    return std::make_shared<Text>(std::move(text));
}

inline std::shared_ptr<Text> text(std::string text, TextStyle style) {
    return std::make_shared<Text>(std::move(text), std::move(style));
}

inline std::shared_ptr<RichText> richText(std::shared_ptr<InlineSpan> span) {
    return std::make_shared<RichText>(std::move(span));
}

} // namespace enki
