/// @file number_field.cpp
/// @brief Implementation of Advanced NumberField widget.

#include "enki/widgets/number_field.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/clipboard.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/DartTypes.h>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <cmath>

namespace enki {

class NumberFieldState;
static NumberFieldState* g_focused_numberfield = nullptr;

static sk_sp<skia::textlayout::FontCollection> getNumberFieldFontCollection() {
    static sk_sp<skia::textlayout::FontCollection> s_fc = []() {
        auto m = SkFontMgr_New_FontConfig(nullptr);
        if (!m) m = SkFontMgr::RefDefault();
        auto fc = sk_make_sp<skia::textlayout::FontCollection>();
        fc->setDefaultFontManager(m);
        fc->enableFontFallback();
        return fc;
    }();
    return s_fc;
}

// ════════════════════════════════════════════════════════════════
// Expression Evaluation Engine (Supports +, -, *, /, (), %, decimals)
// ════════════════════════════════════════════════════════════════

class ExpressionParser {
private:
    std::string src_;
    size_t pos_ = 0;

    void skipWhitespace() {
        while (pos_ < src_.length() && std::isspace(static_cast<unsigned char>(src_[pos_]))) {
            pos_++;
        }
    }

    double parseNumber() {
        skipWhitespace();
        size_t start = pos_;
        bool has_dot = false;
        if (pos_ < src_.length() && (src_[pos_] == '+' || src_[pos_] == '-')) pos_++;
        while (pos_ < src_.length()) {
            char c = src_[pos_];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                pos_++;
            } else if (c == '.' && !has_dot) {
                has_dot = true;
                pos_++;
            } else {
                break;
            }
        }
        if (start == pos_) return 0.0;
        try {
            double val = std::stod(src_.substr(start, pos_ - start));
            skipWhitespace();
            if (pos_ < src_.length() && src_[pos_] == '%') {
                val /= 100.0;
                pos_++;
            }
            return val;
        } catch (...) {
            return 0.0;
        }
    }

    double parseFactor() {
        skipWhitespace();
        if (pos_ < src_.length() && src_[pos_] == '(') {
            pos_++; // Consume '('
            double val = parseExpression();
            skipWhitespace();
            if (pos_ < src_.length() && src_[pos_] == ')') {
                pos_++; // Consume ')'
            }
            return val;
        }
        return parseNumber();
    }

    double parseTerm() {
        double val = parseFactor();
        while (pos_ < src_.length()) {
            skipWhitespace();
            if (pos_ >= src_.length()) break;
            char op = src_[pos_];
            if (op == '*' || op == '/') {
                pos_++;
                double next = parseFactor();
                if (op == '*') val *= next;
                else if (next != 0.0) val /= next;
            } else {
                break;
            }
        }
        return val;
    }

    double parseExpression() {
        double val = parseTerm();
        while (pos_ < src_.length()) {
            skipWhitespace();
            if (pos_ >= src_.length()) break;
            char op = src_[pos_];
            if (op == '+' || op == '-') {
                pos_++;
                double next = parseTerm();
                if (op == '+') val += next;
                else val -= next;
            } else {
                break;
            }
        }
        return val;
    }

public:
    static bool evaluate(const std::string& expr, double& out_result) {
        if (expr.empty()) return false;
        ExpressionParser p;
        p.src_ = expr;
        p.pos_ = 0;
        try {
            out_result = p.parseExpression();
            return !std::isnan(out_result) && !std::isinf(out_result);
        } catch (...) {
            return false;
        }
    }
};

// ════════════════════════════════════════════════════════════════
// Number Formatting Helper
// ════════════════════════════════════════════════════════════════

static std::string formatNumberValue(double val, const NumberFieldProps& opts) {
    if (opts.custom_formatter) {
        return opts.custom_formatter(val);
    }

    std::ostringstream ss;
    if (opts.precision >= 0) {
        ss << std::fixed << std::setprecision(opts.precision) << val;
    } else {
        ss << val;
    }
    std::string s = ss.str();

    // Thousands grouping if enabled
    if (opts.show_thousands_separator) {
        size_t dot_pos = s.find('.');
        std::string int_part = (dot_pos != std::string::npos) ? s.substr(0, dot_pos) : s;
        std::string frac_part = (dot_pos != std::string::npos) ? s.substr(dot_pos) : "";

        bool neg = (!int_part.empty() && int_part[0] == '-');
        if (neg) int_part = int_part.substr(1);

        std::string formatted_int = "";
        int count = 0;
        for (int i = static_cast<int>(int_part.length()) - 1; i >= 0; --i) {
            formatted_int = int_part[i] + formatted_int;
            if (++count % 3 == 0 && i > 0) {
                formatted_int = "," + formatted_int;
            }
        }
        if (neg) formatted_int = "-" + formatted_int;
        s = formatted_int + frac_part;
    }

    return s;
}

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for NumberField
// ════════════════════════════════════════════════════════════════

class RenderNumberFieldBox : public RenderBox {
public:
    std::shared_ptr<NumberFieldController> controller;
    NumberFieldProps options;
    std::string edit_text;
    bool is_focused = false;
    bool is_hovered = false;
    bool show_cursor = false;
    size_t cursor_pos = 0;
    size_t selection_start = 0;
    size_t selection_end = 0;

    int hovered_button = 0; // 0=none, 1=up/plus, 2=down/minus

    std::unique_ptr<skia::textlayout::Paragraph> text_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> prefix_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> suffix_paragraph_;

    RenderNumberFieldBox(std::shared_ptr<NumberFieldController> ctrl, NumberFieldProps opt, std::string text_val)
        : controller(std::move(ctrl)), options(std::move(opt)), edit_text(std::move(text_val)) {
        
        float h = 40.0f;
        if (options.size == NumberFieldSize::Small) h = 32.0f;
        else if (options.size == NumberFieldSize::Large) h = 48.0f;

        FlexboxStyle st;
        st.height = StyleValue::point(h);
        applyFlexboxStyle(anuNode(), st);
    }

    void layoutParagraphs() {
        auto fc = getNumberFieldFontCollection();
        if (!fc) return;

        float font_sz = options.style.font_size > 0 ? options.style.font_size : 14.0f;
        if (options.size == NumberFieldSize::Small && options.style.font_size <= 0) font_sz = 12.5f;
        if (options.size == NumberFieldSize::Large && options.style.font_size <= 0) font_sz = 16.0f;

        skia::textlayout::ParagraphStyle p_style;
        p_style.setTextAlign(skia::textlayout::TextAlign::kLeft);

        // 1. Prefix Paragraph
        if (!options.prefix_text.empty()) {
            auto b_prefix = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_pref;
            t_pref.setFontSize(font_sz);
            t_pref.setColor(static_cast<SkColor>(options.prefix_suffix_color));
            b_prefix->pushStyle(t_pref);
            b_prefix->addText(options.prefix_text.c_str(), options.prefix_text.length());
            prefix_paragraph_ = b_prefix->Build();
            prefix_paragraph_->layout(100.0f);
        } else {
            prefix_paragraph_.reset();
        }

        // 2. Main Numeric / Edit Text Paragraph
        auto b_main = skia::textlayout::ParagraphBuilder::make(p_style, fc);
        skia::textlayout::TextStyle t_main;
        t_main.setFontSize(font_sz);
        t_main.setColor(static_cast<SkColor>(options.style.color != 0 ? options.style.color : 0xFFF1F5F9));
        if (!options.style.font_family.empty()) {
            std::vector<SkString> font_fams = {SkString(options.style.font_family.c_str())};
            t_main.setFontFamilies(font_fams);
        }
        b_main->pushStyle(t_main);
        std::string display_str = is_focused ? edit_text : formatNumberValue(controller->getValue(), options);
        b_main->addText(display_str.c_str(), display_str.length());
        text_paragraph_ = b_main->Build();
        text_paragraph_->layout(size_.width > 0 ? size_.width : 300.0f);

        // 3. Suffix Paragraph
        if (!options.suffix_text.empty()) {
            auto b_suffix = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_suff;
            t_suff.setFontSize(font_sz);
            t_suff.setColor(static_cast<SkColor>(options.prefix_suffix_color));
            b_suffix->pushStyle(t_suff);
            b_suffix->addText(options.suffix_text.c_str(), options.suffix_text.length());
            suffix_paragraph_ = b_suffix->Build();
            suffix_paragraph_->layout(100.0f);
        } else {
            suffix_paragraph_.reset();
        }
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        layoutParagraphs();

        SkRect bounds = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(bounds, options.border_radius, options.border_radius);

        // 1. Draw Field Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        sk_canvas->drawRRect(rrect, bg_paint);

        // Clip Content to rounded rect
        sk_canvas->save();
        sk_canvas->clipRRect(rrect, true);

        // Compute Stepper Buttons Geometry
        float btn_w = 28.0f;
        if (options.size == NumberFieldSize::Small) btn_w = 24.0f;
        else if (options.size == NumberFieldSize::Large) btn_w = 34.0f;

        float content_x = ctx.offset.x + options.padding.left;
        float avail_w = size_.width - options.padding.horizontal();

        // 2. Draw Side Steppers or Right Steppers
        if (options.stepper_position == NumberFieldStepperPosition::Sides && !options.read_only && !options.disabled) {
            // Left Decrement Button [ − ]
            SkRect left_btn = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, btn_w, size_.height);
            SkPaint btn_bg;
            btn_bg.setColor(hovered_button == 2 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(left_btn, btn_bg);

            // Minus icon
            SkPaint icon_paint;
            icon_paint.setAntiAlias(true);
            icon_paint.setColor(options.button_icon_color);
            icon_paint.setStrokeWidth(2.0f);
            float cy = ctx.offset.y + size_.height * 0.5f;
            float cx = ctx.offset.x + btn_w * 0.5f;
            sk_canvas->drawLine(cx - 5.0f, cy, cx + 5.0f, cy, icon_paint);

            // Left divider line
            SkPaint div_paint;
            div_paint.setColor(options.border_color);
            sk_canvas->drawLine(ctx.offset.x + btn_w, ctx.offset.y, ctx.offset.x + btn_w, ctx.offset.y + size_.height, div_paint);

            // Right Increment Button [ + ]
            SkRect right_btn = SkRect::MakeXYWH(ctx.offset.x + size_.width - btn_w, ctx.offset.y, btn_w, size_.height);
            btn_bg.setColor(hovered_button == 1 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(right_btn, btn_bg);

            // Plus icon
            cx = ctx.offset.x + size_.width - btn_w * 0.5f;
            sk_canvas->drawLine(cx - 5.0f, cy, cx + 5.0f, cy, icon_paint);
            sk_canvas->drawLine(cx, cy - 5.0f, cx, cy + 5.0f, icon_paint);

            // Right divider line
            sk_canvas->drawLine(ctx.offset.x + size_.width - btn_w, ctx.offset.y, ctx.offset.x + size_.width - btn_w, ctx.offset.y + size_.height, div_paint);

            content_x += btn_w;
            avail_w -= btn_w * 2.0f;
        } else if (options.stepper_position == NumberFieldStepperPosition::RightVertical && !options.read_only && !options.disabled) {
            float sb_x = ctx.offset.x + size_.width - btn_w;
            float half_h = size_.height * 0.5f;

            // Top Button [ ▲ ]
            SkRect top_btn = SkRect::MakeXYWH(sb_x, ctx.offset.y, btn_w, half_h);
            SkPaint btn_bg;
            btn_bg.setColor(hovered_button == 1 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(top_btn, btn_bg);

            // Up triangle
            SkPaint icon_paint;
            icon_paint.setAntiAlias(true);
            icon_paint.setColor(options.button_icon_color);
            icon_paint.setStyle(SkPaint::kFill_Style);
            SkPath up_path;
            float t_cx = sb_x + btn_w * 0.5f;
            float t_cy = ctx.offset.y + half_h * 0.5f;
            up_path.moveTo(t_cx, t_cy - 3.5f);
            up_path.lineTo(t_cx - 4.5f, t_cy + 3.0f);
            up_path.lineTo(t_cx + 4.5f, t_cy + 3.0f);
            up_path.close();
            sk_canvas->drawPath(up_path, icon_paint);

            // Bottom Button [ ▼ ]
            SkRect bot_btn = SkRect::MakeXYWH(sb_x, ctx.offset.y + half_h, btn_w, half_h);
            btn_bg.setColor(hovered_button == 2 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(bot_btn, btn_bg);

            // Down triangle
            SkPath dn_path;
            float b_cy = ctx.offset.y + half_h + half_h * 0.5f;
            dn_path.moveTo(t_cx, b_cy + 3.5f);
            dn_path.lineTo(t_cx - 4.5f, b_cy - 3.0f);
            dn_path.lineTo(t_cx + 4.5f, b_cy - 3.0f);
            dn_path.close();
            sk_canvas->drawPath(dn_path, icon_paint);

            // Divider lines
            SkPaint div_paint;
            div_paint.setColor(options.border_color);
            sk_canvas->drawLine(sb_x, ctx.offset.y, sb_x, ctx.offset.y + size_.height, div_paint);
            sk_canvas->drawLine(sb_x, ctx.offset.y + half_h, sb_x + btn_w, ctx.offset.y + half_h, div_paint);

            avail_w -= btn_w;
        } else if (options.stepper_position == NumberFieldStepperPosition::RightHorizontal && !options.read_only && !options.disabled) {
            float pair_w = btn_w * 2.0f;
            float sb_x = ctx.offset.x + size_.width - pair_w;

            // Minus [ − ]
            SkRect minus_btn = SkRect::MakeXYWH(sb_x, ctx.offset.y, btn_w, size_.height);
            SkPaint btn_bg;
            btn_bg.setColor(hovered_button == 2 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(minus_btn, btn_bg);

            SkPaint icon_paint;
            icon_paint.setAntiAlias(true);
            icon_paint.setColor(options.button_icon_color);
            icon_paint.setStrokeWidth(2.0f);
            float cy = ctx.offset.y + size_.height * 0.5f;
            float cx = sb_x + btn_w * 0.5f;
            sk_canvas->drawLine(cx - 4.0f, cy, cx + 4.0f, cy, icon_paint);

            // Plus [ + ]
            SkRect plus_btn = SkRect::MakeXYWH(sb_x + btn_w, ctx.offset.y, btn_w, size_.height);
            btn_bg.setColor(hovered_button == 1 ? options.button_hover_color : options.button_color);
            sk_canvas->drawRect(plus_btn, btn_bg);

            cx = sb_x + btn_w + btn_w * 0.5f;
            sk_canvas->drawLine(cx - 4.0f, cy, cx + 4.0f, cy, icon_paint);
            sk_canvas->drawLine(cx, cy - 4.0f, cx, cy + 4.0f, icon_paint);

            // Dividers
            SkPaint div_paint;
            div_paint.setColor(options.border_color);
            sk_canvas->drawLine(sb_x, ctx.offset.y, sb_x, ctx.offset.y + size_.height, div_paint);
            sk_canvas->drawLine(sb_x + btn_w, ctx.offset.y, sb_x + btn_w, ctx.offset.y + size_.height, div_paint);

            avail_w -= pair_w;
        }

        // 3. Draw Prefix
        float text_x = content_x;
        float text_y = ctx.offset.y + (size_.height - (text_paragraph_ ? text_paragraph_->getHeight() : 16.0f)) * 0.5f;

        if (prefix_paragraph_) {
            prefix_paragraph_->paint(sk_canvas, text_x, text_y);
            text_x += prefix_paragraph_->getMaxIntrinsicWidth() + 3.0f;
        }

        // 4. Draw Main Numeric Text & Selection Highlight
        if (text_paragraph_) {
            if (is_focused && selection_start != selection_end) {
                size_t s = std::min(selection_start, selection_end);
                size_t e = std::max(selection_start, selection_end);
                auto rects = text_paragraph_->getRectsForRange(s, e,
                    skia::textlayout::RectHeightStyle::kTight,
                    skia::textlayout::RectWidthStyle::kTight);

                SkPaint sel_paint;
                sel_paint.setColor(static_cast<SkColor>(options.selection_color));
                sk_canvas->save();
                sk_canvas->translate(text_x, text_y);
                for (const auto& r : rects) {
                    sk_canvas->drawRect(r.rect, sel_paint);
                }
                sk_canvas->restore();
            }

            text_paragraph_->paint(sk_canvas, text_x, text_y);

            // 5. Draw Blinking Cursor
            if (is_focused && show_cursor && !options.read_only && selection_start == selection_end) {
                float c_x = 0;
                float c_h = text_paragraph_->getHeight();
                if (cursor_pos > 0) {
                    auto prev_rects = text_paragraph_->getRectsForRange(cursor_pos - 1, cursor_pos,
                        skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);
                    if (!prev_rects.empty()) {
                        c_x = prev_rects.back().rect.fRight;
                    }
                }
                SkPaint cur_paint;
                cur_paint.setColor(static_cast<SkColor>(options.cursor_color));
                sk_canvas->drawRect(SkRect::MakeXYWH(text_x + c_x, text_y, 2.0f, c_h), cur_paint);
            }

            text_x += text_paragraph_->getMaxIntrinsicWidth() + 4.0f;
        }

        // 6. Draw Suffix
        if (suffix_paragraph_) {
            suffix_paragraph_->paint(sk_canvas, text_x, text_y);
        }

        sk_canvas->restore();

        // 7. Draw Outer Rounded Border (Normal / Hover / Focused Glow)
        SkPaint border_paint;
        border_paint.setAntiAlias(true);
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(is_focused ? 1.5f : 1.0f);
        border_paint.setColor(is_focused ? options.focus_border_color : (is_hovered ? 0xFF475569 : options.border_color));
        sk_canvas->drawRRect(rrect, border_paint);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    // 0=text/scrub, 1=up/plus, 2=down/minus
    int getHitComponent(float local_x, float local_y) const {
        float btn_w = 28.0f;
        if (options.size == NumberFieldSize::Small) btn_w = 24.0f;
        else if (options.size == NumberFieldSize::Large) btn_w = 34.0f;

        if (options.stepper_position == NumberFieldStepperPosition::Sides) {
            if (local_x <= btn_w) return 2; // Left minus
            if (local_x >= size_.width - btn_w) return 1; // Right plus
        } else if (options.stepper_position == NumberFieldStepperPosition::RightVertical) {
            if (local_x >= size_.width - btn_w) {
                return (local_y < size_.height * 0.5f) ? 1 : 2; // Top up, bottom down
            }
        } else if (options.stepper_position == NumberFieldStepperPosition::RightHorizontal) {
            if (local_x >= size_.width - btn_w * 2.0f) {
                return (local_x < size_.width - btn_w) ? 2 : 1; // Minus then Plus
            }
        }
        return 0; // Text area
    }

    size_t getIndexAtCoordinate(float local_x) {
        layoutParagraphs();
        if (!text_paragraph_) return 0;

        float btn_w = 28.0f;
        if (options.size == NumberFieldSize::Small) btn_w = 24.0f;
        else if (options.size == NumberFieldSize::Large) btn_w = 34.0f;

        float content_x = options.padding.left;
        if (options.stepper_position == NumberFieldStepperPosition::Sides) content_x += btn_w;
        if (prefix_paragraph_) content_x += prefix_paragraph_->getMaxIntrinsicWidth() + 3.0f;

        float target_x = std::max(0.0f, local_x - content_x);
        auto pos = text_paragraph_->getGlyphPositionAtCoordinate(target_x, 5.0f);
        return std::min(static_cast<size_t>(pos.position), edit_text.length());
    }
};

static RenderNumberFieldBox* findNumberFieldBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderNumberFieldBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findNumberFieldBox(child)) return found;
    }
    return nullptr;
}

class RenderNumberFieldWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<NumberFieldController> controller;
    NumberFieldProps options;
    std::string edit_text;
    bool is_focused;
    bool is_hovered;
    bool show_cursor;
    size_t cursor_pos;
    size_t selection_start;
    size_t selection_end;
    int hovered_button;

    RenderNumberFieldWidget(std::shared_ptr<NumberFieldController> ctrl, NumberFieldProps opt,
                            std::string txt, bool focused, bool hovered, bool cursor, size_t cur_pos,
                            size_t sel_s, size_t sel_e, int h_btn)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), edit_text(std::move(txt)), is_focused(focused),
          is_hovered(hovered), show_cursor(cursor), cursor_pos(cur_pos),
          selection_start(sel_s), selection_end(sel_e), hovered_button(h_btn) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderNumberFieldBox>(controller, options, edit_text);
        ro->is_focused = is_focused;
        ro->is_hovered = is_hovered;
        ro->show_cursor = show_cursor;
        ro->cursor_pos = cursor_pos;
        ro->selection_start = selection_start;
        ro->selection_end = selection_end;
        ro->hovered_button = hovered_button;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderNumberFieldBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->edit_text = edit_text;
            ro->is_focused = is_focused;
            ro->is_hovered = is_hovered;
            ro->show_cursor = show_cursor;
            ro->cursor_pos = cursor_pos;
            ro->selection_start = selection_start;
            ro->selection_end = selection_end;
            ro->hovered_button = hovered_button;
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderNumberFieldWidget"; }
};

// ════════════════════════════════════════════════════════════════
// NumberField State Implementation
// ════════════════════════════════════════════════════════════════

class NumberFieldState : public State {
private:
    std::shared_ptr<NumberFieldController> controller_;
    std::string edit_text_ = "";
    bool is_focused_ = false;
    bool is_hovered_ = false;
    bool show_cursor_ = true;
    double last_blink_time_ = 0.0;

    size_t cursor_pos_ = 0;
    size_t selection_start_ = 0;
    size_t selection_end_ = 0;
    int hovered_button_ = 0;

    // Drag Scrubbing
    bool is_scrubbing_ = false;
    float scrub_start_x_ = 0.0f;
    double scrub_start_val_ = 0.0;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;

    void resetBlink() {
        show_cursor_ = true;
        if (Platform::instance()) last_blink_time_ = Platform::instance()->getTime();
    }

    double clampValue(double val) {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        if (!nf) return val;

        if (nf->options.wrap_mode == NumberFieldWrapMode::Wrap &&
            nf->options.min_value != -std::numeric_limits<double>::infinity() &&
            nf->options.max_value != std::numeric_limits<double>::infinity()) {
            double range = nf->options.max_value - nf->options.min_value;
            if (range > 0.0) {
                while (val < nf->options.min_value) val += range;
                while (val > nf->options.max_value) val -= range;
            }
        } else {
            val = std::clamp(val, nf->options.min_value, nf->options.max_value);
        }
        return val;
    }

    void commitValue(double new_val, bool notify = true) {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        if (!nf || nf->options.read_only || nf->options.disabled) return;

        new_val = clampValue(new_val);
        controller_->setValue(new_val);
        edit_text_ = formatNumberValue(new_val, nf->options);
        cursor_pos_ = edit_text_.length();
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;

        if (notify && nf->options.on_changed) {
            nf->options.on_changed(new_val);
        }
        setState([] {});
    }

    void commitEditText() {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        if (!nf) return;

        double parsed_val = 0.0;
        if (nf->options.allow_expressions && ExpressionParser::evaluate(edit_text_, parsed_val)) {
            commitValue(parsed_val);
        } else {
            try {
                parsed_val = std::stod(edit_text_);
                commitValue(parsed_val);
            } catch (...) {
                // Revert to controller value
                edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
                setState([] {});
            }
        }

        if (nf->options.on_submitted) {
            nf->options.on_submitted(controller_->getValue());
        }
    }

    void applyStep(double direction, double multiplier = 1.0) {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        if (!nf || nf->options.read_only || nf->options.disabled) return;

        double current = controller_->getValue();
        double step_sz = nf->options.step * multiplier;
        commitValue(current + direction * step_sz);
    }

    void deleteSelection() {
        if (selection_start_ == selection_end_) return;
        size_t s = std::min(selection_start_, selection_end_);
        size_t e = std::max(selection_start_, selection_end_);
        edit_text_.erase(s, e - s);
        cursor_pos_ = s;
        selection_start_ = s;
        selection_end_ = s;
    }

    void handleKey(int key, int mods) {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        if (!nf || nf->options.disabled) return;

        bool shift = (mods & 1) != 0;
        bool ctrl  = (mods & 2) != 0 || (mods & 4) != 0;
        bool alt   = (mods & 8) != 0 || (mods & 4) != 0;

        const int KEY_BACKSPACE = 0xff08;
        const int KEY_DELETE    = 0xffff;
        const int KEY_LEFT      = 0xff51;
        const int KEY_UP        = 0xff52;
        const int KEY_RIGHT     = 0xff53;
        const int KEY_DOWN      = 0xff54;
        const int KEY_PAGE_UP   = 0xff55;
        const int KEY_PAGE_DOWN = 0xff56;
        const int KEY_HOME      = 0xff50;
        const int KEY_END       = 0xff57;
        const int KEY_RETURN    = 0xff0d;
        const int KEY_ESCAPE    = 0xff1b;

        // Clipboard & Selection Shortcuts
        bool is_ctrl_c = (ctrl && (key == 'c' || key == 'C' || key == 0x63 || key == 0x43 || key == 54)) || (key == 0x03);
        bool is_ctrl_v = (ctrl && (key == 'v' || key == 'V' || key == 0x76 || key == 0x56 || key == 55)) || (key == 0x16);
        bool is_ctrl_x = (ctrl && (key == 'x' || key == 'X' || key == 0x78 || key == 0x58 || key == 53)) || (key == 0x18);
        bool is_ctrl_a = (ctrl && (key == 'a' || key == 'A' || key == 0x61 || key == 0x41 || key == 38)) || (key == 0x01);
        bool is_ctrl_z = (ctrl && !shift && (key == 'z' || key == 'Z' || key == 0x7a || key == 0x5a || key == 52)) || (key == 0x1a && !shift);
        bool is_ctrl_y = (ctrl && (key == 'y' || key == 'Y' || key == 0x79 || key == 0x59 || key == 29 || key == 0x19)) ||
                         (ctrl && shift && (key == 'z' || key == 'Z' || key == 0x7a || key == 0x5a || key == 52 || key == 0x1a));

        if (key == KEY_UP) {
            double mult = alt ? (nf->options.fine_step / nf->options.step) : (shift ? (nf->options.large_step / nf->options.step) : 1.0);
            applyStep(1.0, mult);
        } else if (key == KEY_DOWN) {
            double mult = alt ? (nf->options.fine_step / nf->options.step) : (shift ? (nf->options.large_step / nf->options.step) : 1.0);
            applyStep(-1.0, mult);
        } else if (key == KEY_PAGE_UP) {
            applyStep(1.0, nf->options.large_step / nf->options.step);
        } else if (key == KEY_PAGE_DOWN) {
            applyStep(-1.0, nf->options.large_step / nf->options.step);
        } else if (key == KEY_HOME && nf->options.min_value != -std::numeric_limits<double>::infinity()) {
            commitValue(nf->options.min_value);
        } else if (key == KEY_END && nf->options.max_value != std::numeric_limits<double>::infinity()) {
            commitValue(nf->options.max_value);
        } else if ((key == KEY_RETURN || key == 0xff8d)) {
            commitEditText();
        } else if (key == KEY_ESCAPE) {
            edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
            setState([] {});
        } else if ((key == KEY_BACKSPACE || key == 0x08 || key == 0x7f) && !nf->options.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ > 0) {
                edit_text_.erase(cursor_pos_ - 1, 1);
                cursor_pos_--;
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            }
            resetBlink();
            setState([] {});
        } else if ((key == KEY_DELETE || key == 0xff9f) && !nf->options.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ < edit_text_.length()) {
                edit_text_.erase(cursor_pos_, 1);
            }
            resetBlink();
            setState([] {});
        } else if (key == KEY_LEFT) {
            if (cursor_pos_ > 0) cursor_pos_--;
            if (!shift) {
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            } else {
                selection_end_ = cursor_pos_;
            }
            resetBlink();
            setState([] {});
        } else if (key == KEY_RIGHT) {
            if (cursor_pos_ < edit_text_.length()) cursor_pos_++;
            if (!shift) {
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            } else {
                selection_end_ = cursor_pos_;
            }
            resetBlink();
            setState([] {});
        } else if (is_ctrl_a) {
            selection_start_ = 0;
            selection_end_ = edit_text_.length();
            cursor_pos_ = selection_end_;
            setState([] {});
        } else if (is_ctrl_c && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = edit_text_.substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
        } else if (is_ctrl_x && !nf->options.read_only && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = edit_text_.substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
            deleteSelection();
            resetBlink();
            setState([] {});
        } else if (is_ctrl_v && !nf->options.read_only) {
            if (Platform::instance()) {
                std::string paste = Platform::instance()->getClipboardText();
                if (!paste.empty()) {
                    deleteSelection();
                    edit_text_.insert(cursor_pos_, paste);
                    cursor_pos_ += paste.length();
                    selection_start_ = cursor_pos_;
                    selection_end_ = cursor_pos_;
                    resetBlink();
                    setState([] {});
                }
            }
        } else if (is_ctrl_z && !nf->options.read_only) {
            if (controller_->undo()) {
                edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
                cursor_pos_ = edit_text_.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
                setState([] {});
            }
        } else if (is_ctrl_y && !nf->options.read_only) {
            if (controller_->redo()) {
                edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
                cursor_pos_ = edit_text_.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
                setState([] {});
            }
        }
    }

public:
    void initState() override {
        State::initState();
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        controller_ = nf->controller;
        edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
        cursor_pos_ = edit_text_.length();
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;

        is_focused_ = nf->options.auto_focus;
        if (is_focused_) g_focused_numberfield = this;

        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (g_focused_numberfield != this) return;
                auto* current_nf = static_cast<const NumberFieldWidget*>(widget());
                if (!current_nf || current_nf->options.read_only || current_nf->options.disabled) return;

                // Ignore control characters
                if (!text.empty() && text.length() == 1) {
                    char c = text[0];
                    if (c >= 0 && c < 32) return;
                    if (c == 127) return;
                }

                // Filter allowed characters for number field
                for (char c : text) {
                    bool is_num = std::isdigit(static_cast<unsigned char>(c));
                    bool is_dot = (c == '.' && current_nf->options.allow_decimals);
                    bool is_neg = (c == '-' && current_nf->options.allow_negative);
                    bool is_expr = (current_nf->options.allow_expressions && (c == '+' || c == '*' || c == '/' || c == '(' || c == ')' || c == '%'));

                    if (is_num || is_dot || is_neg || is_expr) {
                        deleteSelection();
                        edit_text_.insert(cursor_pos_, 1, c);
                        cursor_pos_++;
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                    }
                }
                resetBlink();
                setState([] {});
            });

            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (g_focused_numberfield != this) return;
                handleKey(key, mods);
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* nf = static_cast<const NumberFieldWidget*>(widget());
        controller_ = nf->controller;
        if (!is_focused_) {
            edit_text_ = formatNumberValue(controller_->getValue(), nf->options);
        }
    }

    void dispose() override {
        if (g_focused_numberfield == this) g_focused_numberfield = nullptr;
        if (Platform::instance()) {
            if (text_input_conn_) Platform::instance()->onTextInput().disconnect(text_input_conn_);
            if (key_down_conn_) Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* nf = static_cast<const NumberFieldWidget*>(widget());

        if (is_focused_ && Platform::instance()) {
            double cur = Platform::instance()->getTime();
            if (cur - last_blink_time_ > 0.53) {
                show_cursor_ = !show_cursor_;
                last_blink_time_ = cur;
            }
        } else {
            show_cursor_ = false;
        }

        auto render_box = std::make_shared<RenderNumberFieldWidget>(
            controller_, nf->options, edit_text_, is_focused_, is_hovered_,
            show_cursor_, cursor_pos_, selection_start_, selection_end_, hovered_button_
        );

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type = is_scrubbing_ ? SystemCursor::ResizeHorizontal : SystemCursor::Text;

        detector->on_hover_enter = [this](const PointerEvent&) {
            setState([this] { is_hovered_ = true; });
        };
        detector->on_hover_exit = [this](const PointerEvent&) {
            setState([this] { is_hovered_ = false; hovered_button_ = 0; });
        };

        detector->on_hover_move = [this](const PointerEvent& e) {
            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* box = findNumberFieldBox(ro)) {
                    int hit = box->getHitComponent(e.localPosition.x, e.localPosition.y);
                    if (hit != hovered_button_) {
                        hovered_button_ = hit;
                        setState([] {});
                    }
                }
            }
        };

        // 1. Single Tap Down (Handle Steppers vs Text vs Focus)
        detector->on_tap_down = [this](const TapDownDetails& e) {
            auto* nf = static_cast<const NumberFieldWidget*>(widget());
            if (!nf || nf->options.disabled) return;

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* box = findNumberFieldBox(ro)) {
                    int hit = box->getHitComponent(e.local_position.x, e.local_position.y);
                    if (hit == 1) { // Up / Plus
                        applyStep(1.0);
                    } else if (hit == 2) { // Down / Minus
                        applyStep(-1.0);
                    } else { // Text Area
                        g_focused_numberfield = this;
                        is_focused_ = true;
                        resetBlink();
                        cursor_pos_ = box->getIndexAtCoordinate(e.local_position.x);
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                        setState([] {});
                    }
                }
            }
        };

        // 2. Drag Scrubbing (Horizontal Drag-to-Adjust)
        detector->on_pan_start = [this](const DragStartDetails& e) {
            auto* nf = static_cast<const NumberFieldWidget*>(widget());
            if (!nf || nf->options.read_only || nf->options.disabled || !nf->options.enable_scrubbing) return;

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* box = findNumberFieldBox(ro)) {
                    int hit = box->getHitComponent(e.local_position.x, e.local_position.y);
                    if (hit == 0) { // Text area drag scrub
                        is_scrubbing_ = true;
                        scrub_start_x_ = e.global_position.x;
                        scrub_start_val_ = controller_->getValue();
                        setState([] {});
                    }
                }
            }
        };

        detector->on_pan_update = [this](const DragUpdateDetails& e) {
            if (!is_scrubbing_) return;
            auto* nf = static_cast<const NumberFieldWidget*>(widget());
            if (!nf) return;

            float delta_x = e.global_position.x - scrub_start_x_;
            double step_size = nf->options.step;
            double scrub_delta = (delta_x / 5.0f) * step_size;
            commitValue(scrub_start_val_ + scrub_delta);
        };

        detector->on_pan_end = [this](const DragEndDetails&) {
            if (is_scrubbing_) {
                is_scrubbing_ = false;
                setState([] {});
            }
        };

        // 3. Mouse Wheel Scroll Stepping
        detector->on_scroll = [this](float, float dy) {
            auto* nf = static_cast<const NumberFieldWidget*>(widget());
            if (!nf || nf->options.read_only || nf->options.disabled) return;
            applyStep(dy > 0 ? 1.0 : -1.0);
        };

        detector->child = render_box;
        return detector;
    }
};

std::unique_ptr<State> NumberFieldWidget::createState() {
    return std::make_unique<NumberFieldState>();
}

} // namespace enki
