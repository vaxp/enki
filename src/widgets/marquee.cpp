/// @file marquee.cpp
/// @brief Marquee Widget Implementation for ENKI Framework.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/marquee.hpp"
#include "enki/platform/platform.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/tree/build_context.hpp"
#include <include/core/SkColor.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <layout_engine/Anu.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

namespace enki {

namespace {

sk_sp<SkFontMgr> getSharedFontMgr() {
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
        fc->setDefaultFontManager(getSharedFontMgr());
        fc->enableFontFallback();
        return fc;
    }();
    return s_fc;
}

inline SkColor toSkColor(Color c) {
    return static_cast<SkColor>(c);
}

skia::textlayout::TextStyle toSkTextStyle(const TextStyle& s) {
    skia::textlayout::TextStyle sk;
    sk.setColor(toSkColor(s.color));
    sk.setFontSize(s.font_size);

    SkFontStyle::Slant slant = (s.font_style == FontStyle::Italic) ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
    SkFontStyle fontStyle(static_cast<int>(s.font_weight), SkFontStyle::kNormal_Width, slant);
    sk.setFontStyle(fontStyle);

    if (!s.font_family.empty()) {
        sk.setFontFamilies({SkString(s.font_family.c_str())});
    }
    if (s.letter_spacing != 0.0f) {
        sk.setLetterSpacing(s.letter_spacing);
    }
    if (s.word_spacing != 0.0f) {
        sk.setWordSpacing(s.word_spacing);
    }
    if (s.height.has_value()) {
        sk.setHeight(*s.height);
        sk.setHeightOverride(true);
    }
    return sk;
}

std::unique_ptr<skia::textlayout::Paragraph> buildSimpleParagraph(const std::string& text, const TextStyle& style) {
    skia::textlayout::ParagraphStyle pStyle;
    pStyle.setMaxLines(1);

    auto builder = skia::textlayout::ParagraphBuilder::make(pStyle, getSharedFontCollection());
    if (!builder) return nullptr;

    builder->pushStyle(toSkTextStyle(style));
    builder->addText(text.c_str(), text.length());
    builder->pop();

    return builder->Build();
}

} // namespace

class RenderMarquee : public RenderBox {
public:
    std::string      text_data_;
    TextStyle        style_;
    float            velocity_;
    float            blank_space_;
    MarqueeDirection direction_;
    bool             pause_on_hover_;
    float            fading_edge_length_;

    std::unique_ptr<skia::textlayout::Paragraph> paragraph_;
    float            offset_x_ = 0.0f;
    bool             is_hovered_ = false;
    double           last_time_ = 0.0;

    RenderMarquee(
        std::string text,
        TextStyle style,
        float velocity,
        float blank_space,
        MarqueeDirection dir,
        bool pause_hover,
        float fade_len)
        : text_data_(std::move(text)),
          style_(std::move(style)),
          velocity_(velocity),
          blank_space_(blank_space),
          direction_(dir),
          pause_on_hover_(pause_hover),
          fading_edge_length_(fade_len)
    {
        rebuildParagraph();
        ANUNodeSetContext(anu_node_, this);
        ANUNodeSetMeasureFunc(anu_node_, &RenderMarquee::measureCallback);
    }

    void update(
        std::string text,
        TextStyle style,
        float velocity,
        float blank_space,
        MarqueeDirection dir,
        bool pause_hover,
        float fade_len)
    {
        bool text_changed = (text_data_ != text);
        text_data_ = std::move(text);
        style_ = std::move(style);
        velocity_ = velocity;
        blank_space_ = blank_space;
        direction_ = dir;
        pause_on_hover_ = pause_hover;
        fading_edge_length_ = fade_len;

        if (text_changed) {
            rebuildParagraph();
            offset_x_ = 0.0f;
            markNeedsLayout();
        } else {
            markNeedsPaint();
        }
    }

    void rebuildParagraph() {
        paragraph_ = buildSimpleParagraph(text_data_, style_);
    }

    static ANUSize measureCallback(ANUNodeConstRef node, float width, ANUMeasureMode widthMode,
                                   float height, ANUMeasureMode heightMode) {
        auto* self = static_cast<RenderMarquee*>(ANUNodeGetContext(node));
        if (!self || !self->paragraph_) return {0.0f, 0.0f};

        self->paragraph_->layout(std::numeric_limits<float>::infinity());
        float maxIntrinsic = self->paragraph_->getMaxIntrinsicWidth();
        float mw = (widthMode == ANUMeasureModeExactly) ? width : maxIntrinsic;
        float mh = (heightMode == ANUMeasureModeExactly) ? height : self->paragraph_->getHeight();

        return {mw, mh};
    }

    bool hitTestSelf(Point localPoint) const override {
        return (localPoint.x >= 0 && localPoint.x <= size_.width &&
                localPoint.y >= 0 && localPoint.y <= size_.height);
    }

    void handlePointerEnter(const PointerEvent&) override {
        is_hovered_ = true;
    }

    void handlePointerExit(const PointerEvent&) override {
        is_hovered_ = false;
    }

    void paint(PaintContext& ctx) override {
        if (!paragraph_ || text_data_.empty() || size_.width <= 0.0f || size_.height <= 0.0f) return;

        paragraph_->layout(std::numeric_limits<float>::infinity());
        float text_width = paragraph_->getMaxIntrinsicWidth();
        float text_height = paragraph_->getHeight();
        float cycle_len = text_width + blank_space_;

        double current_time = 0.0;
        if (auto* p = Platform::instance()) {
            current_time = p->getTime();
        } else {
            static auto start = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            current_time = std::chrono::duration<double>(now - start).count();
        }

        if (last_time_ > 0.0) {
            double dt = current_time - last_time_;
            if (!(is_hovered_ && pause_on_hover_)) {
                float dx = static_cast<float>(velocity_ * dt);
                if (direction_ == MarqueeDirection::RightToLeft) {
                    offset_x_ += dx;
                    if (cycle_len > 0.0f && offset_x_ >= cycle_len) {
                        offset_x_ = std::fmod(offset_x_, cycle_len);
                    }
                } else {
                    offset_x_ -= dx;
                    if (cycle_len > 0.0f && offset_x_ < 0.0f) {
                        offset_x_ = cycle_len - std::fmod(-offset_x_, cycle_len);
                    }
                }
            }
        }
        last_time_ = current_time;

        Rect bounds = Rect::fromPointSize(ctx.offset, size_);

        ctx.canvas.save();
        ctx.canvas.clipRect(bounds);

        float fade = std::min(fading_edge_length_, size_.width * 0.4f);
        if (fade > 0.0f) {
            ctx.canvas.saveLayer(&bounds, nullptr);
        }

        float start_x = (direction_ == MarqueeDirection::RightToLeft) ? -offset_x_ : offset_x_;
        while (start_x < size_.width) {
            if (start_x + text_width > 0.0f) {
                float text_y = ctx.offset.y + (size_.height - text_height) * 0.5f;
                ctx.canvas.drawParagraph(paragraph_.get(), ctx.offset.x + start_x, text_y);
            }
            start_x += cycle_len;
            if (cycle_len <= 0.0f) break;
        }

        if (fade > 0.0f) {
            Paint left_mask;
            left_mask.setBlendMode(BlendMode::DstOut);
            left_mask.setShader(Gradient::linear(
                Point{ctx.offset.x, ctx.offset.y},
                Point{ctx.offset.x + fade, ctx.offset.y},
                {0xFF000000, 0x00000000}
            ));
            ctx.canvas.drawRect(Rect::fromLTWH(ctx.offset.x, ctx.offset.y, fade, size_.height), left_mask);

            Paint right_mask;
            right_mask.setBlendMode(BlendMode::DstOut);
            right_mask.setShader(Gradient::linear(
                Point{ctx.offset.x + size_.width - fade, ctx.offset.y},
                Point{ctx.offset.x + size_.width, ctx.offset.y},
                {0x00000000, 0xFF000000}
            ));
            ctx.canvas.drawRect(Rect::fromLTWH(ctx.offset.x + size_.width - fade, ctx.offset.y, fade, size_.height), right_mask);

            ctx.canvas.restore();
        }

        ctx.canvas.restore();
        markNeedsPaint();
    }
};

std::unique_ptr<RenderObject> MarqueeWidget::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderMarquee>(
        text, style, velocity, blank_space, direction, pause_on_hover, fading_edge_length
    );
}

void MarqueeWidget::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rm = dynamic_cast<RenderMarquee*>(&renderObject)) {
        rm->update(
            text, style, velocity, blank_space, direction, pause_on_hover, fading_edge_length
        );
    }
}

} // namespace enki
