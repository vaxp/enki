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
// TextSelection Model
// ════════════════════════════════════════════════════════════════

struct TextSelection {
    size_t base_offset = 0;
    size_t extent_offset = 0;

    [[nodiscard]] size_t start() const { return std::min(base_offset, extent_offset); }
    [[nodiscard]] size_t end() const { return std::max(base_offset, extent_offset); }
    [[nodiscard]] bool isCollapsed() const { return base_offset == extent_offset; }
    [[nodiscard]] bool isValid() const { return base_offset != static_cast<size_t>(-1) && extent_offset != static_cast<size_t>(-1); }

    static constexpr TextSelection collapsed(size_t offset) {
        return TextSelection{offset, offset};
    }
    static constexpr TextSelection empty() {
        return TextSelection{static_cast<size_t>(-1), static_cast<size_t>(-1)};
    }

    constexpr bool operator==(const TextSelection&) const = default;
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
               decoration_thickness == o.decoration_thickness &&
               shadows == o.shadows;
    }
    bool operator!=(const TextStyle& o) const { return !(*this == o); }
};

// ════════════════════════════════════════════════════════════════
// Spans for RichText
// ════════════════════════════════════════════════════════════════

using TextSpanCallback = std::function<void()>;
using TextSpanHoverCallback = std::function<void(bool is_hovered)>;

class ParagraphBuilderContext;

/// @brief Abstract base class for an inline text span.
class InlineSpan {
public:
    virtual ~InlineSpan() = default;
    virtual void build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const = 0;
};

/// @brief Declarative props for TextSpan creation.
struct TextSpanProps {
    std::string                              text = "";
    std::optional<TextStyle>                 style = std::nullopt;
    std::vector<std::shared_ptr<InlineSpan>> children = {};
    TextSpanCallback                         on_click = nullptr;
    TextSpanHoverCallback                    on_hover = nullptr;
};

/// @brief An immutable text span with its own style and optional nested children spans.
class TextSpan : public InlineSpan, public std::enable_shared_from_this<TextSpan> {
public:
    std::string                              text;
    std::optional<TextStyle>                 style;
    std::vector<std::shared_ptr<InlineSpan>> children;
    TextSpanCallback                         on_click;
    TextSpanHoverCallback                    on_hover;

    TextSpan(std::string text = "",
             std::optional<TextStyle> style = std::nullopt,
             std::vector<std::shared_ptr<InlineSpan>> children = {},
             TextSpanCallback on_click = nullptr,
             TextSpanHoverCallback on_hover = nullptr)
        : text(std::move(text)), style(std::move(style)), children(std::move(children)),
          on_click(std::move(on_click)), on_hover(std::move(on_hover)) {}

    explicit TextSpan(TextSpanProps props)
        : text(std::move(props.text)), style(std::move(props.style)), children(std::move(props.children)),
          on_click(std::move(props.on_click)), on_hover(std::move(props.on_hover)) {}

    void build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const override;
};

inline std::shared_ptr<TextSpan> span(TextSpanProps props) {
    return std::make_shared<TextSpan>(std::move(props));
}

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
    void setSelectable(bool selectable);
    void setSelectionColor(Color color);
    void setOnSelectionChanged(std::function<void(TextSelection)> callback);

    [[nodiscard]] const std::string& getTextData() const { return text_data_; }
    [[nodiscard]] const TextStyle& getDefaultStyle() const { return default_style_; }
    [[nodiscard]] TextAlign getTextAlign() const { return text_align_; }
    [[nodiscard]] TextDirection getTextDirection() const { return text_direction_; }
    [[nodiscard]] TextOverflow getOverflow() const { return overflow_; }
    [[nodiscard]] const std::optional<size_t>& getMaxLines() const { return max_lines_; }
    [[nodiscard]] bool getSoftWrap() const { return soft_wrap_; }
    [[nodiscard]] bool isSelectable() const { return selectable_; }
    [[nodiscard]] Color getSelectionColor() const { return selection_color_; }
    [[nodiscard]] TextSelection getSelection() const { return selection_; }

    void selectAll();
    void clearSelection();
    [[nodiscard]] std::string getSelectedText() const;
    void copyToClipboard();

    void paint(PaintContext& ctx) override;

    /// Anu layout measurement callback.
    static ANUSize measureText(ANUNodeConstRef node, float width, ANUMeasureMode widthMode, float height, ANUMeasureMode heightMode);

    void rebuildParagraph();

    bool hitTestSelf(Point localPoint) const override;
    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerExit(const PointerEvent& e) override;
    SystemCursor cursor() const override;

protected:
    void layoutParagraph(float availableWidth);
    void* getNativeParagraph() const;

private:

    std::string                         text_data_;
    std::shared_ptr<InlineSpan>         span_;
    TextStyle                           default_style_;
    TextAlign                           text_align_;
    TextDirection                       text_direction_;
    TextOverflow                        overflow_;
    std::optional<size_t>               max_lines_;
    bool                                soft_wrap_;
    bool                                selectable_ = false;
    Color                               selection_color_ = 0x6038BDF8;
    std::function<void(TextSelection)>  on_selection_changed_ = nullptr;

    TextSelection                       selection_ = TextSelection::empty();
    bool                                is_dragging_ = false;
    size_t                              selection_anchor_ = 0;
    Point                               last_click_pos_{0.0f, 0.0f};
    std::chrono::steady_clock::time_point last_click_time_{};
    int                                 click_count_ = 0;

    struct Impl;
    std::unique_ptr<Impl>               impl_;
};

// ════════════════════════════════════════════════════════════════
// Text Widget — Simple High-Level Text Component
// ════════════════════════════════════════════════════════════════

/// @brief A widget that displays a string of text with a single style.
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

    explicit Text(std::string text, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), data(std::move(text)) {}

    Text(std::string text, TextStyle s, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), data(std::move(text)), style(std::move(s)) {}

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

    explicit RichText(std::shared_ptr<InlineSpan> span, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key)), text_span(std::move(span)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "RichText"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Props (C++20 Designated Initializers Support)
// ════════════════════════════════════════════════════════════════

struct TextProps {
    std::string text;
    
    // ── Common TextStyle Overrides ─────────────────────────────
    std::optional<Color>                color;
    std::optional<float>                font_size;
    std::optional<FontWeight>           font_weight;
    std::optional<FontStyle>            font_style;
    std::optional<std::string>          font_family;
    std::optional<float>                letter_spacing;
    std::optional<float>                word_spacing;
    std::optional<float>                height;
    std::vector<BoxShadow>              shadows;
    
    // ── Full Style Override ────────────────────────────────────
    std::optional<TextStyle>            style;

    // ── Paragraph Layout ───────────────────────────────────────
    std::optional<TextAlign>            text_align;
    std::optional<TextDirection>        text_direction;
    std::optional<TextOverflow>         overflow;
    std::optional<size_t>               max_lines;
    std::optional<bool>                 soft_wrap;

    // ── Selection Support ──────────────────────────────────────
    std::optional<bool>                 selectable;
    std::optional<Color>                selection_color;
    std::function<void(TextSelection)>  on_selection_changed = nullptr;

    Key key = Key::none();
};

struct RichTextProps {
    std::shared_ptr<InlineSpan>         text_span;
    std::optional<TextStyle>            default_style;
    std::optional<TextAlign>            text_align;
    std::optional<TextDirection>        text_direction;
    std::optional<TextOverflow>         overflow;
    std::optional<size_t>               max_lines;
    std::optional<bool>                 soft_wrap;
    std::optional<bool>                 selectable;
    std::optional<Color>                selection_color;
    std::function<void(TextSelection)>  on_selection_changed = nullptr;

    Key key = Key::none();
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

inline std::shared_ptr<Text> text(TextProps props) {
    auto t = std::make_shared<Text>(std::move(props.text));
    if (props.key != Key::none()) t->key = props.key;
    if (props.style) t->style = *props.style;
    
    // Apply Common Style Overrides
    if (props.color) t->style.color = *props.color;
    if (props.font_size) t->style.font_size = *props.font_size;
    if (props.font_weight) t->style.font_weight = *props.font_weight;
    if (props.font_style) t->style.font_style = *props.font_style;
    if (props.font_family) t->style.font_family = *props.font_family;
    if (props.letter_spacing) t->style.letter_spacing = *props.letter_spacing;
    if (props.word_spacing) t->style.word_spacing = *props.word_spacing;
    if (props.height) t->style.height = *props.height;
    for (const auto& s : props.shadows) t->style.shadows.push_back(s);
    
    // Apply Paragraph Configuration
    if (props.text_align) t->text_align = *props.text_align;
    if (props.text_direction) t->text_direction = *props.text_direction;
    if (props.overflow) t->overflow = *props.overflow;
    if (props.max_lines) t->max_lines = *props.max_lines;
    if (props.soft_wrap) t->soft_wrap = *props.soft_wrap;

    // Apply Selection Configuration
    if (props.selectable) t->selectable = *props.selectable;
    if (props.selection_color) t->selection_color = *props.selection_color;
    if (props.on_selection_changed) t->on_selection_changed = props.on_selection_changed;
    
    return t;
}

inline std::shared_ptr<RichText> richText(std::shared_ptr<InlineSpan> span) {
    return std::make_shared<RichText>(std::move(span));
}

inline std::shared_ptr<RichText> richText(RichTextProps props) {
    auto r = std::make_shared<RichText>(std::move(props.text_span));
    if (props.key != Key::none()) r->key = props.key;
    if (props.default_style) r->default_style = *props.default_style;
    if (props.text_align) r->text_align = *props.text_align;
    if (props.text_direction) r->text_direction = *props.text_direction;
    if (props.overflow) r->overflow = *props.overflow;
    if (props.max_lines) r->max_lines = *props.max_lines;
    if (props.soft_wrap) r->soft_wrap = *props.soft_wrap;
    if (props.selectable) r->selectable = *props.selectable;
    if (props.selection_color) r->selection_color = *props.selection_color;
    if (props.on_selection_changed) r->on_selection_changed = props.on_selection_changed;
    return r;
}
 
} // namespace enki
