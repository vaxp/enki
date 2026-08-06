/// @file text.cpp
/// @brief Advanced Typography & Text Widgets implementation using SkParagraph and Anu Layout.

#include "enki/widgets/text.hpp"
#include "enki/rendering/canvas.hpp"
#include <include/core/SkColor.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextShadow.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <layout_engine/Anu.h>
#include <algorithm>
#include <limits>
#include <iostream>

namespace enki {

namespace {

// ════════════════════════════════════════════════════════════════
// Font Management & Helpers
// ════════════════════════════════════════════════════════════════

sk_sp<SkFontMgr> getTextFontMgr() {
    static sk_sp<SkFontMgr> s_mgr = []() {
        auto m = SkFontMgr_New_FontConfig(nullptr);
        if (!m) m = SkFontMgr::RefDefault();
        return m;
    }();
    return s_mgr;
}

sk_sp<skia::textlayout::FontCollection> getSharedFontCollection() {
    static sk_sp<skia::textlayout::FontCollection> s_fc = []() {
        auto fc = sk_make_sp<skia::textlayout::FontCollection>();
        fc->setDefaultFontManager(getTextFontMgr());
        fc->enableFontFallback();
        return fc;
    }();
    return s_fc;
}

inline SkColor toSkColor(Color c) {
    return static_cast<SkColor>(c);
}

skia::textlayout::TextAlign toSkTextAlign(TextAlign align) {
    switch (align) {
        case TextAlign::Left:    return skia::textlayout::TextAlign::kLeft;
        case TextAlign::Right:   return skia::textlayout::TextAlign::kRight;
        case TextAlign::Center:  return skia::textlayout::TextAlign::kCenter;
        case TextAlign::Justify: return skia::textlayout::TextAlign::kJustify;
        case TextAlign::Start:   return skia::textlayout::TextAlign::kStart;
        case TextAlign::End:     return skia::textlayout::TextAlign::kEnd;
    }
    return skia::textlayout::TextAlign::kStart;
}

skia::textlayout::TextDirection toSkTextDirection(TextDirection dir) {
    switch (dir) {
        case TextDirection::LTR: return skia::textlayout::TextDirection::kLtr;
        case TextDirection::RTL: return skia::textlayout::TextDirection::kRtl;
    }
    return skia::textlayout::TextDirection::kLtr;
}

skia::textlayout::TextDecoration toSkTextDecoration(TextDecoration d) {
    unsigned int res = skia::textlayout::TextDecoration::kNoDecoration;
    if (d & TextDecoration::Underline)   res |= skia::textlayout::TextDecoration::kUnderline;
    if (d & TextDecoration::Overline)    res |= skia::textlayout::TextDecoration::kOverline;
    if (d & TextDecoration::LineThrough) res |= skia::textlayout::TextDecoration::kLineThrough;
    return static_cast<skia::textlayout::TextDecoration>(res);
}

skia::textlayout::TextDecorationStyle toSkTextDecorationStyle(TextDecorationStyle s) {
    switch (s) {
        case TextDecorationStyle::Solid:  return skia::textlayout::TextDecorationStyle::kSolid;
        case TextDecorationStyle::Double: return skia::textlayout::TextDecorationStyle::kDouble;
        case TextDecorationStyle::Dotted: return skia::textlayout::TextDecorationStyle::kDotted;
        case TextDecorationStyle::Dashed: return skia::textlayout::TextDecorationStyle::kDashed;
        case TextDecorationStyle::Wavy:   return skia::textlayout::TextDecorationStyle::kWavy;
    }
    return skia::textlayout::TextDecorationStyle::kSolid;
}

skia::textlayout::TextStyle toSkTextStyle(const TextStyle& s) {
    skia::textlayout::TextStyle sk;
    sk.setColor(toSkColor(s.color));
    sk.setFontSize(s.font_size);

    // Font Style (Weight + Slant)
    SkFontStyle::Slant slant = (s.font_style == FontStyle::Italic) ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
    SkFontStyle fontStyle(static_cast<int>(s.font_weight), SkFontStyle::kNormal_Width, slant);
    sk.setFontStyle(fontStyle);

    // Font Families
    std::vector<SkString> families;
    if (!s.font_family.empty()) {
        families.emplace_back(s.font_family.c_str());
    }
    for (const auto& fam : s.font_families) {
        families.emplace_back(fam.c_str());
    }
    // Fallback standard families
    families.emplace_back("Inter");
    families.emplace_back("Roboto");
    families.emplace_back("Ubuntu");
    families.emplace_back("DejaVu Sans");
    families.emplace_back("sans-serif");
    sk.setFontFamilies(families);

    // Spacing
    sk.setLetterSpacing(s.letter_spacing);
    sk.setWordSpacing(s.word_spacing);

    // Height
    if (s.height.has_value()) {
        sk.setHeight(*s.height);
        sk.setHeightOverride(true);
    }

    // Decoration
    sk.setDecoration(toSkTextDecoration(s.decoration));
    sk.setDecorationColor(toSkColor(s.decoration_color));
    sk.setDecorationStyle(toSkTextDecorationStyle(s.decoration_style));
    sk.setDecorationThicknessMultiplier(s.decoration_thickness);

    // Shadows
    for (const auto& shadow : s.shadows) {
        sk.addShadow(skia::textlayout::TextShadow(
            toSkColor(shadow.color),
            SkPoint::Make(shadow.offset.x, shadow.offset.y),
            shadow.blur_radius * 0.5f // Convert blur radius to sigma
        ));
    }

    return sk;
}

TextStyle mergeStyles(const TextStyle& parent, const std::optional<TextStyle>& child) {
    if (!child) return parent;
    TextStyle res = *child;
    if (res.font_family.empty()) res.font_family = parent.font_family;
    if (res.font_families.empty()) res.font_families = parent.font_families;
    return res;
}

} // namespace

// ════════════════════════════════════════════════════════════════
// ParagraphBuilderContext Implementation
// ════════════════════════════════════════════════════════════════

class ParagraphBuilderContext {
public:
    explicit ParagraphBuilderContext(skia::textlayout::ParagraphBuilder* b) : builder(b) {}

    void pushStyle(const TextStyle& style) {
        builder->pushStyle(toSkTextStyle(style));
    }

    void popStyle() {
        builder->pop();
    }

    void addText(const std::string& text) {
        if (!text.empty()) {
            builder->addText(text.c_str(), text.size());
        }
    }

    skia::textlayout::ParagraphBuilder* builder;
};

void TextSpan::build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const {
    TextStyle effectiveStyle = mergeStyles(inheritedStyle, style);
    builder.pushStyle(effectiveStyle);

    if (!text.empty()) {
        builder.addText(text);
    }

    for (const auto& child : children) {
        if (child) {
            child->build(builder, effectiveStyle);
        }
    }

    builder.popStyle();
}

// ════════════════════════════════════════════════════════════════
// RenderParagraph Internal Implementation
// ════════════════════════════════════════════════════════════════

struct RenderParagraph::Impl {
    std::unique_ptr<skia::textlayout::Paragraph> paragraph;
    float current_layout_width = -1.0f;

    void build(const InlineSpan* rootSpan,
               const TextStyle& defaultStyle,
               TextAlign textAlign,
               TextDirection textDirection,
               TextOverflow overflow,
               std::optional<size_t> maxLines,
               bool softWrap) {
        skia::textlayout::ParagraphStyle pStyle;
        pStyle.setTextAlign(toSkTextAlign(textAlign));
        pStyle.setTextDirection(toSkTextDirection(textDirection));
        pStyle.setTextStyle(toSkTextStyle(defaultStyle));

        if (maxLines.has_value() && *maxLines > 0) {
            pStyle.setMaxLines(*maxLines);
        } else if (!softWrap) {
            pStyle.setMaxLines(1);
        }

        if (overflow == TextOverflow::Ellipsis) {
            pStyle.setEllipsis(SkString("..."));
        }

        auto builder = skia::textlayout::ParagraphBuilder::make(pStyle, getSharedFontCollection());
        if (!builder) {
            paragraph = nullptr;
            return;
        }

        ParagraphBuilderContext ctx(builder.get());
        if (rootSpan) {
            rootSpan->build(ctx, defaultStyle);
        }

        paragraph = builder->Build();
        current_layout_width = -1.0f;
    }

    void layout(float width) {
        if (paragraph && (current_layout_width != width || width <= 0.0f)) {
            paragraph->layout(std::max(0.0f, width));
            current_layout_width = width;
        }
    }
};

// ════════════════════════════════════════════════════════════════
// RenderParagraph Implementation
// ════════════════════════════════════════════════════════════════

RenderParagraph::RenderParagraph(std::shared_ptr<InlineSpan> span,
                                 TextStyle defaultStyle,
                                 TextAlign textAlign,
                                 TextDirection textDirection,
                                 TextOverflow overflow,
                                 std::optional<size_t> maxLines,
                                 bool softWrap)
    : span_(std::move(span)),
      default_style_(std::move(defaultStyle)),
      text_align_(textAlign),
      text_direction_(textDirection),
      overflow_(overflow),
      max_lines_(maxLines),
      soft_wrap_(softWrap),
      impl_(std::make_unique<Impl>()) {

    ANUNodeSetContext(anuNode(), this);
    ANUNodeSetMeasureFunc(anuNode(), &RenderParagraph::measureText);
    rebuildParagraph();
}

RenderParagraph::~RenderParagraph() {
    ANUNodeSetMeasureFunc(anuNode(), nullptr);
    ANUNodeSetContext(anuNode(), nullptr);
}

void RenderParagraph::setText(std::string data,
                              TextStyle style,
                              TextAlign align,
                              TextDirection dir,
                              TextOverflow overflow,
                              std::optional<size_t> maxLines,
                              bool softWrap) {
    if (text_data_ == data &&
        default_style_ == style &&
        text_align_ == align &&
        text_direction_ == dir &&
        overflow_ == overflow &&
        max_lines_ == maxLines &&
        soft_wrap_ == softWrap) {
        return; // Fast path: zero allocation, zero shaping, zero flexbox dirtying
    }

    bool layout_changed = (text_data_ != data ||
                           default_style_.font_size != style.font_size ||
                           default_style_.font_weight != style.font_weight ||
                           default_style_.font_family != style.font_family ||
                           default_style_.letter_spacing != style.letter_spacing ||
                           default_style_.height != style.height ||
                           text_direction_ != dir ||
                           max_lines_ != maxLines ||
                           soft_wrap_ != softWrap);

    text_data_ = std::move(data);
    default_style_ = std::move(style);
    text_align_ = align;
    text_direction_ = dir;
    overflow_ = overflow;
    max_lines_ = maxLines;
    soft_wrap_ = softWrap;

    span_ = std::make_shared<TextSpan>(text_data_, default_style_);
    rebuildParagraph();

    if (layout_changed) {
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    } else {
        markNeedsPaint();
    }
}

void RenderParagraph::setTextSpan(std::shared_ptr<InlineSpan> span) {
    span_ = std::move(span);
    rebuildParagraph();
    ANUNodeMarkDirty(anuNode());
    markNeedsLayout();
}

void RenderParagraph::setDefaultStyle(TextStyle style) {
    if (default_style_ != style) {
        default_style_ = std::move(style);
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setTextAlign(TextAlign align) {
    if (text_align_ != align) {
        text_align_ = align;
        rebuildParagraph();
        markNeedsPaint();
    }
}

void RenderParagraph::setTextDirection(TextDirection dir) {
    if (text_direction_ != dir) {
        text_direction_ = dir;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setOverflow(TextOverflow overflow) {
    if (overflow_ != overflow) {
        overflow_ = overflow;
        rebuildParagraph();
        markNeedsPaint();
    }
}

void RenderParagraph::setMaxLines(std::optional<size_t> maxLines) {
    if (max_lines_ != maxLines) {
        max_lines_ = maxLines;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setSoftWrap(bool softWrap) {
    if (soft_wrap_ != softWrap) {
        soft_wrap_ = softWrap;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::rebuildParagraph() {
    impl_->build(span_.get(), default_style_, text_align_, text_direction_, overflow_, max_lines_, soft_wrap_);
}

void RenderParagraph::layoutParagraph(float availableWidth) {
    impl_->layout(availableWidth);
}

ANUSize RenderParagraph::measureText(ANUNodeConstRef node,
                                     float width,
                                     ANUMeasureMode widthMode,
                                     float height,
                                     ANUMeasureMode heightMode) {
    auto* self = static_cast<RenderParagraph*>(ANUNodeGetContext(node));
    if (!self || !self->impl_ || !self->impl_->paragraph) {
        return {0.0f, 0.0f};
    }

    float constraintWidth = std::numeric_limits<float>::max();
    if (widthMode == ANUMeasureModeExactly || widthMode == ANUMeasureModeAtMost) {
        constraintWidth = width;
    }

    if (!self->soft_wrap_) {
        constraintWidth = std::numeric_limits<float>::max();
    }

    self->layoutParagraph(constraintWidth);

    float maxIntrinsicWidth = self->impl_->paragraph->getMaxIntrinsicWidth();
    float longestLineWidth = self->impl_->paragraph->getLongestLine();
    float contentWidth = (constraintWidth < maxIntrinsicWidth) ? longestLineWidth : maxIntrinsicWidth;

    float measuredWidth = contentWidth;
    if (widthMode == ANUMeasureModeExactly) {
        measuredWidth = width;
    } else if (widthMode == ANUMeasureModeAtMost) {
        measuredWidth = std::min(contentWidth, width);
    }

    float measuredHeight = self->impl_->paragraph->getHeight();
    if (heightMode == ANUMeasureModeExactly) {
        measuredHeight = height;
    } else if (heightMode == ANUMeasureModeAtMost) {
        measuredHeight = std::min(measuredHeight, height);
    }

    return {measuredWidth, measuredHeight};
}

void RenderParagraph::paint(PaintContext& ctx) {
    if (!impl_ || !impl_->paragraph) return;

    // Layout paragraph to the exact allocated box width
    layoutParagraph(size_.width);

    // Draw via canvas drawParagraph interface
    ctx.canvas.drawParagraph(impl_->paragraph.get(), ctx.offset.x, ctx.offset.y);
}

// ════════════════════════════════════════════════════════════════
// Text Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Text::createRenderObject(BuildContext&) {
    auto span = std::make_shared<TextSpan>(data, style);
    auto rp = std::make_unique<RenderParagraph>(
        span,
        style,
        text_align,
        text_direction,
        overflow,
        max_lines,
        soft_wrap
    );
    rp->setText(data, style, text_align, text_direction, overflow, max_lines, soft_wrap);
    return rp;
}

void Text::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rp = dynamic_cast<RenderParagraph*>(&renderObject)) {
        rp->setText(data, style, text_align, text_direction, overflow, max_lines, soft_wrap);
    }
}

// ════════════════════════════════════════════════════════════════
// RichText Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> RichText::createRenderObject(BuildContext&) {
    return std::make_unique<RenderParagraph>(
        text_span,
        default_style,
        text_align,
        text_direction,
        overflow,
        max_lines,
        soft_wrap
    );
}

void RichText::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rp = dynamic_cast<RenderParagraph*>(&renderObject)) {
        rp->setTextSpan(text_span);
        rp->setDefaultStyle(default_style);
        rp->setTextAlign(text_align);
        rp->setTextDirection(text_direction);
        rp->setOverflow(overflow);
        rp->setMaxLines(max_lines);
        rp->setSoftWrap(soft_wrap);
    }
}

} // namespace enki
