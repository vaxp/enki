/// @file text_area.cpp
/// @brief Implementation of Advanced multi-line TextArea widget.

#include "enki/widgets/text_area.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/clipboard.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
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
#include <cctype>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// TextAreaController Clipboard Implementations
// ════════════════════════════════════════════════════════════════

bool TextAreaController::copyToClipboard() {
    if (!Platform::instance()) return false;
    std::string to_copy;
    if (hasSelection()) {
        to_copy = getSelectedText();
    } else {
        to_copy = text;
    }
    if (to_copy.empty()) return false;

    ClipboardData data;
    data.setText(to_copy);
    Platform::instance()->setClipboardData(data, ClipboardType::Clipboard);
    Platform::instance()->setClipboardText(to_copy, ClipboardType::Clipboard);
    return true;
}

bool TextAreaController::cutToClipboard() {
    if (!Platform::instance()) return false;
    if (!hasSelection()) return false;

    std::string to_copy = getSelectedText();
    ClipboardData data;
    data.setText(to_copy);
    Platform::instance()->setClipboardData(data, ClipboardType::Clipboard);
    Platform::instance()->setClipboardText(to_copy, ClipboardType::Clipboard);

    size_t start = std::min(selection_start, selection_end);
    size_t end = std::max(selection_start, selection_end);
    pushUndo(text);
    text.erase(start, end - start);
    selection_start = start;
    clearSelection();
    return true;
}

bool TextAreaController::pasteFromClipboard() {
    if (!Platform::instance()) return false;
    std::string paste_str = Platform::instance()->getClipboardText(ClipboardType::Clipboard);
    if (paste_str.empty()) {
        auto cd = Platform::instance()->getClipboardData(ClipboardType::Clipboard);
        if (cd.hasText()) paste_str = cd.getText();
    }
    if (paste_str.empty()) return false;

    if (hasSelection()) {
        size_t start = std::min(selection_start, selection_end);
        size_t end = std::max(selection_start, selection_end);
        pushUndo(text);
        text.erase(start, end - start);
        selection_start = start;
        clearSelection();
    } else {
        pushUndo(text);
    }

    text.insert(selection_start, paste_str);
    selection_start += paste_str.length();
    clearSelection();
    return true;
}

class TextAreaState;
static TextAreaState* g_focused_textarea = nullptr;

static sk_sp<skia::textlayout::FontCollection> getTextAreaFontCollection() {
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
// Custom RenderBox for Multi-line Text Area
// ════════════════════════════════════════════════════════════════

class RenderTextAreaBox : public RenderBox {
public:
    std::shared_ptr<TextAreaController> controller;
    TextAreaProps options;
    bool is_focused = false;
    bool show_cursor = false;
    float scroll_y = 0.0f;
    float total_content_height = 0.0f;

    std::unique_ptr<skia::textlayout::Paragraph> paragraph_;

    RenderTextAreaBox(std::shared_ptr<TextAreaController> ctrl, TextAreaProps opt, float scroll)
        : controller(std::move(ctrl)), options(std::move(opt)), scroll_y(scroll) {
        
        float line_h = (options.style.font_size > 0 ? options.style.font_size : 14.0f) * 1.45f;
        float min_h = options.min_lines * line_h + options.padding.vertical();
        FlexboxStyle st;
        st.height = StyleValue::point(min_h);
        applyFlexboxStyle(anuNode(), st);
    }

    void layoutParagraph(float available_width) {
        if (available_width <= 0.0f) return;

        auto fc = getTextAreaFontCollection();
        if (!fc) return;

        skia::textlayout::ParagraphStyle p_style;
        p_style.setTextAlign(skia::textlayout::TextAlign::kLeft);
        p_style.setTextDirection(skia::textlayout::TextDirection::kLtr);

        auto builder = skia::textlayout::ParagraphBuilder::make(p_style, fc);
        if (!builder) return;

        skia::textlayout::TextStyle t_style;
        t_style.setFontSize(options.style.font_size > 0 ? options.style.font_size : 14.0f);
        t_style.setColor(static_cast<SkColor>(options.style.color != 0 ? options.style.color : 0xFFF1F5F9));
        
        std::vector<SkString> font_families;
        if (!options.style.font_family.empty()) {
            font_families.emplace_back(options.style.font_family.c_str());
        } else {
            font_families.emplace_back("sans-serif");
        }
        t_style.setFontFamilies(font_families);

        builder->pushStyle(t_style);

        std::string display_str = controller->text;
        if (display_str.empty() && !is_focused) {
            t_style.setColor(static_cast<SkColor>(0xFF64748B));
            builder->pushStyle(t_style);
            builder->addText(options.hint_text.c_str(), options.hint_text.length());
        } else {
            builder->addText(display_str.c_str(), display_str.length());
        }

        paragraph_ = builder->Build();
        if (paragraph_) {
            paragraph_->layout(available_width);
            total_content_height = paragraph_->getHeight();
        }
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        float gutter_w = options.show_line_numbers ? 42.0f : 0.0f;
        float text_area_w = size_.width - gutter_w - options.padding.horizontal();
        float visible_h = size_.height - options.padding.vertical();

        layoutParagraph(text_area_w);

        // Clamp scroll_y
        float max_scroll = std::max(0.0f, total_content_height - visible_h);
        scroll_y = std::clamp(scroll_y, 0.0f, max_scroll);

        SkRect bounds_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(bounds_rect, options.border_radius, options.border_radius);

        // 1. Draw Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        sk_canvas->drawRRect(rrect, bg_paint);

        // 2. Draw Line Numbers Gutter
        if (options.show_line_numbers) {
            SkPaint gutter_paint;
            gutter_paint.setColor(options.line_number_bg);
            SkRect gutter_rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, gutter_w, size_.height);
            sk_canvas->drawRect(gutter_rect, gutter_paint);

            // Gutter Divider Line
            SkPaint div_paint;
            div_paint.setColor(options.border_color);
            sk_canvas->drawLine(ctx.offset.x + gutter_w, ctx.offset.y,
                                ctx.offset.x + gutter_w, ctx.offset.y + size_.height, div_paint);
        }

        // Clip Content to Inner Box
        sk_canvas->save();
        sk_canvas->clipRRect(rrect, true);

        // 3. Draw Line Numbers Text
        if (options.show_line_numbers && paragraph_) {
            float line_h = (options.style.font_size > 0 ? options.style.font_size : 14.0f) * 1.45f;
            size_t total_lines = controller->getLineCount();

            auto fc = getTextAreaFontCollection();
            skia::textlayout::ParagraphStyle p_style;
            p_style.setTextAlign(skia::textlayout::TextAlign::kRight);

            for (size_t i = 1; i <= total_lines; ++i) {
                float y_pos = ctx.offset.y + options.padding.top + (i - 1) * line_h - scroll_y;
                if (y_pos + line_h < ctx.offset.y || y_pos > ctx.offset.y + size_.height) continue;

                auto num_builder = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle num_style;
                num_style.setFontSize(11.0f);
                num_style.setColor(options.line_number_color);
                num_builder->pushStyle(num_style);
                std::string num_str = std::to_string(i);
                num_builder->addText(num_str.c_str(), num_str.length());
                auto num_para = num_builder->Build();
                num_para->layout(gutter_w - 10.0f);
                num_para->paint(sk_canvas, ctx.offset.x, y_pos);
            }
        }

        // 4. Draw Selection & Paragraph Text with Scroll Offset
        float text_origin_x = ctx.offset.x + gutter_w + options.padding.left;
        float text_origin_y = ctx.offset.y + options.padding.top - scroll_y;

        if (paragraph_) {
            // Draw Selection Highlight
            if (controller->hasSelection()) {
                size_t start = std::min(controller->selection_start, controller->selection_end);
                size_t end = std::max(controller->selection_start, controller->selection_end);

                auto rects = paragraph_->getRectsForRange(start, end,
                    skia::textlayout::RectHeightStyle::kTight,
                    skia::textlayout::RectWidthStyle::kTight);

                SkPaint sel_paint;
                sel_paint.setColor(static_cast<SkColor>(options.selection_color));

                sk_canvas->save();
                sk_canvas->translate(text_origin_x, text_origin_y);
                for (const auto& tb : rects) {
                    sk_canvas->drawRect(tb.rect, sel_paint);
                }
                sk_canvas->restore();
            }

            // Draw Paragraph Content
            paragraph_->paint(sk_canvas, text_origin_x, text_origin_y);

            // 5. Draw Blinking Cursor
            if (is_focused && show_cursor && !controller->hasSelection() && !options.read_only) {
                float cursor_x = 0;
                float cursor_y = 0;
                float cursor_h = (options.style.font_size > 0 ? options.style.font_size : 14.0f) * 1.3f;

                auto rects = paragraph_->getRectsForRange(controller->selection_start, controller->selection_start + 1,
                    skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);

                if (!rects.empty()) {
                    cursor_x = rects[0].rect.fLeft;
                    cursor_y = rects[0].rect.fTop;
                    cursor_h = rects[0].rect.height();
                } else if (controller->selection_start > 0) {
                    auto prev_rects = paragraph_->getRectsForRange(controller->selection_start - 1, controller->selection_start,
                        skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);
                    if (!prev_rects.empty()) {
                        cursor_x = prev_rects.back().rect.fRight;
                        cursor_y = prev_rects.back().rect.fTop;
                        cursor_h = prev_rects.back().rect.height();
                    }
                }

                SkPaint cursor_paint;
                cursor_paint.setColor(static_cast<SkColor>(options.cursor_color));
                sk_canvas->drawRect(SkRect::MakeXYWH(text_origin_x + cursor_x, text_origin_y + cursor_y, 2.0f, cursor_h), cursor_paint);
            }
        }

        // 6. Draw Scrollbar if scrollable
        if (total_content_height > visible_h && visible_h > 0) {
            float sb_w = 5.0f;
            float sb_x = ctx.offset.x + size_.width - sb_w - 3.0f;
            float thumb_h = std::max(24.0f, (visible_h / total_content_height) * visible_h);
            float thumb_y = ctx.offset.y + options.padding.top + (max_scroll > 0 ? (scroll_y / max_scroll) * (visible_h - thumb_h) : 0);

            SkPaint sb_paint;
            sb_paint.setAntiAlias(true);
            sb_paint.setColor(0x8094A3B8);
            sk_canvas->drawRoundRect(SkRect::MakeXYWH(sb_x, thumb_y, sb_w, thumb_h), 2.5f, 2.5f, sb_paint);
        }

        sk_canvas->restore();

        // 7. Draw Outer Border (Focused / Normal)
        SkPaint border_paint;
        border_paint.setAntiAlias(true);
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(is_focused ? 1.5f : 1.0f);
        border_paint.setColor(is_focused ? options.focus_border_color : options.border_color);
        sk_canvas->drawRRect(rrect, border_paint);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    size_t getIndexAtCoordinate(float local_x, float local_y) {
        float gutter_w = options.show_line_numbers ? 42.0f : 0.0f;
        float text_area_w = size_.width - gutter_w - options.padding.horizontal();
        layoutParagraph(text_area_w);

        if (!paragraph_) return 0;
        float target_x = std::max(0.0f, local_x - gutter_w - options.padding.left);
        float target_y = std::max(0.0f, local_y - options.padding.top + scroll_y);

        auto pos = paragraph_->getGlyphPositionAtCoordinate(target_x, target_y);
        return std::min(static_cast<size_t>(pos.position), controller->text.length());
    }
};

static RenderTextAreaBox* findTextAreaBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderTextAreaBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findTextAreaBox(child)) return found;
    }
    return nullptr;
}

class RenderTextAreaWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<TextAreaController> controller;
    TextAreaProps options;
    bool is_focused;
    bool show_cursor;
    float scroll_y;

    RenderTextAreaWidget(std::shared_ptr<TextAreaController> ctrl, TextAreaProps opt, bool focused, bool cursor, float scroll)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), is_focused(focused), show_cursor(cursor), scroll_y(scroll) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderTextAreaBox>(controller, options, scroll_y);
        ro->is_focused = is_focused;
        ro->show_cursor = show_cursor;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderTextAreaBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->is_focused = is_focused;
            ro->show_cursor = show_cursor;
            ro->scroll_y = scroll_y;
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderTextAreaWidget"; }
};

// ════════════════════════════════════════════════════════════════
// TextArea State Implementation
// ════════════════════════════════════════════════════════════════

class TextAreaState : public State {
private:
    bool is_focused_ = false;
    bool show_cursor_ = true;
    double last_blink_time_ = 0.0;
    float scroll_y_ = 0.0f;
    std::shared_ptr<TextAreaController> controller_;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;

    size_t getPrevCharBytes(size_t index) {
        if (index == 0) return 0;
        size_t num_bytes = 1;
        while (index >= num_bytes + 1 && (static_cast<unsigned char>(controller_->text[index - num_bytes]) & 0xC0) == 0x80) {
            num_bytes++;
        }
        return num_bytes;
    }

    size_t getNextCharBytes(size_t index) {
        if (index >= controller_->text.length()) return 0;
        size_t num_bytes = 1;
        while (index + num_bytes < controller_->text.length() && (static_cast<unsigned char>(controller_->text[index + num_bytes]) & 0xC0) == 0x80) {
            num_bytes++;
        }
        return num_bytes;
    }

    void notifyCursorMoved() {
        auto* ta = static_cast<const TextAreaWidget*>(widget());
        if (ta && ta->options.on_cursor_moved) {
            size_t r = 1, c = 1;
            controller_->getCursorPosition(r, c);
            ta->options.on_cursor_moved(r, c);
        }
    }

    void resetBlink() {
        show_cursor_ = true;
        if (Platform::instance()) last_blink_time_ = Platform::instance()->getTime();
    }

    void deleteSelection() {
        size_t start = std::min(controller_->selection_start, controller_->selection_end);
        size_t end = std::max(controller_->selection_start, controller_->selection_end);
        controller_->pushUndo(controller_->text);
        controller_->text.erase(start, end - start);
        controller_->selection_start = start;
        controller_->clearSelection();
    }

    void selectWordAt(size_t index) {
        const std::string& t = controller_->text;
        if (t.empty()) return;
        size_t idx = std::min(index, t.length());
        if (idx > 0 && (idx == t.length() || std::isspace(static_cast<unsigned char>(t[idx])))) {
            --idx;
        }
        if (std::isspace(static_cast<unsigned char>(t[idx]))) {
            controller_->selection_start = idx;
            controller_->selection_end = idx;
            return;
        }
        size_t start = idx;
        while (start > 0 && !std::isspace(static_cast<unsigned char>(t[start - 1])) &&
               (std::isalnum(static_cast<unsigned char>(t[start - 1])) || t[start - 1] == '_')) {
            --start;
        }
        size_t end = idx;
        while (end < t.length() && !std::isspace(static_cast<unsigned char>(t[end])) &&
               (std::isalnum(static_cast<unsigned char>(t[end])) || t[end] == '_')) {
            ++end;
        }
        controller_->selection_start = start;
        controller_->selection_end = end;
    }

    void selectLineAt(size_t index) {
        const std::string& t = controller_->text;
        if (t.empty()) return;
        size_t start = std::min(index, t.length());
        while (start > 0 && t[start - 1] != '\n') --start;
        size_t end = std::min(index, t.length());
        while (end < t.length() && t[end] != '\n') ++end;
        controller_->selection_start = start;
        controller_->selection_end = end;
    }

    void handleKey(int key, int mods) {
        auto* ta = static_cast<const TextAreaWidget*>(widget());
        if (!ta) return;

        // XKB Keysyms
        const int KEY_BACKSPACE = 0xff08;
        const int KEY_DELETE    = 0xffff;
        const int KEY_LEFT      = 0xff51;
        const int KEY_UP        = 0xff52;
        const int KEY_RIGHT     = 0xff53;
        const int KEY_DOWN      = 0xff54;
        const int KEY_HOME      = 0xff50;
        const int KEY_END       = 0xff57;
        const int KEY_RETURN    = 0xff0d;
        const int KEY_TAB       = 0xff09;

        bool shift = (mods & 1) != 0;
        bool ctrl  = (mods & 2) != 0 || (mods & 4) != 0;
        bool changed = false;

        // Clipboard & Selection Shortcuts
        bool is_ctrl_c = (ctrl && (key == 'c' || key == 'C' || key == 0x63 || key == 0x43 || key == 54)) || (key == 0x03);
        bool is_ctrl_v = (ctrl && (key == 'v' || key == 'V' || key == 0x76 || key == 0x56 || key == 55)) || (key == 0x16);
        bool is_ctrl_x = (ctrl && (key == 'x' || key == 'X' || key == 0x78 || key == 0x58 || key == 53)) || (key == 0x18);
        bool is_ctrl_a = (ctrl && (key == 'a' || key == 'A' || key == 0x61 || key == 0x41 || key == 38)) || (key == 0x01);
        bool is_ctrl_z = (ctrl && !shift && (key == 'z' || key == 'Z' || key == 0x7a || key == 0x5a || key == 52)) || (key == 0x1a && !shift);
        bool is_ctrl_y = (ctrl && (key == 'y' || key == 'Y' || key == 0x79 || key == 0x59 || key == 29 || key == 0x19)) ||
                         (ctrl && shift && (key == 'z' || key == 'Z' || key == 0x7a || key == 0x5a || key == 52 || key == 0x1a));

        if ((key == KEY_BACKSPACE || key == 0x08 || key == 0x7f) && !ta->options.read_only) {
            if (controller_->hasSelection()) {
                deleteSelection();
                changed = true;
            } else if (controller_->selection_start > 0) {
                controller_->pushUndo(controller_->text);
                size_t num_bytes = getPrevCharBytes(controller_->selection_start);
                controller_->selection_start -= num_bytes;
                controller_->text.erase(controller_->selection_start, num_bytes);
                controller_->clearSelection();
                changed = true;
            }
        } else if ((key == KEY_DELETE || key == 0xff9f) && !ta->options.read_only) {
            if (controller_->hasSelection()) {
                deleteSelection();
                changed = true;
            } else if (controller_->selection_start < controller_->text.length()) {
                controller_->pushUndo(controller_->text);
                size_t num_bytes = getNextCharBytes(controller_->selection_start);
                controller_->text.erase(controller_->selection_start, num_bytes);
                changed = true;
            }
        } else if ((key == KEY_RETURN || key == 0xff8d) && !ta->options.read_only) {
            if (controller_->hasSelection()) deleteSelection();
            controller_->pushUndo(controller_->text);
            controller_->text.insert(controller_->selection_start, "\n");
            controller_->selection_start += 1;
            controller_->clearSelection();
            changed = true;
            if (ta->options.on_submitted) ta->options.on_submitted(controller_->text);
        } else if (key == KEY_TAB && !ta->options.read_only) {
            if (controller_->hasSelection()) deleteSelection();
            controller_->pushUndo(controller_->text);
            controller_->text.insert(controller_->selection_start, "    ");
            controller_->selection_start += 4;
            controller_->clearSelection();
            changed = true;
        } else if (key == KEY_LEFT) {
            if (!shift && controller_->hasSelection()) {
                controller_->selection_start = std::min(controller_->selection_start, controller_->selection_end);
                controller_->clearSelection();
            } else if (controller_->selection_start > 0) {
                controller_->selection_start -= getPrevCharBytes(controller_->selection_start);
                if (!shift) controller_->clearSelection();
            }
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        } else if (key == KEY_RIGHT) {
            if (!shift && controller_->hasSelection()) {
                controller_->selection_start = std::max(controller_->selection_start, controller_->selection_end);
                controller_->clearSelection();
            } else if (controller_->selection_start < controller_->text.length()) {
                controller_->selection_start += getNextCharBytes(controller_->selection_start);
                if (!shift) controller_->clearSelection();
            }
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        } else if (key == KEY_UP || key == KEY_DOWN) {
            size_t r, c;
            controller_->getCursorPosition(r, c);
            if (key == KEY_UP && r > 1) {
                size_t target_r = r - 1;
                size_t curr_r = 1;
                size_t new_idx = 0;
                for (size_t i = 0; i < controller_->text.length(); ++i) {
                    if (curr_r == target_r) {
                        new_idx = std::min(controller_->text.length(), i + c - 1);
                        break;
                    }
                    if (controller_->text[i] == '\n') ++curr_r;
                }
                controller_->selection_start = new_idx;
                if (!shift) controller_->clearSelection();
            } else if (key == KEY_DOWN && r < controller_->getLineCount()) {
                size_t target_r = r + 1;
                size_t curr_r = 1;
                size_t new_idx = controller_->text.length();
                for (size_t i = 0; i < controller_->text.length(); ++i) {
                    if (controller_->text[i] == '\n') {
                        ++curr_r;
                        if (curr_r == target_r) {
                            new_idx = std::min(controller_->text.length(), i + 1 + c - 1);
                            break;
                        }
                    }
                }
                controller_->selection_start = new_idx;
                if (!shift) controller_->clearSelection();
            }
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        } else if (key == KEY_HOME) {
            if (ctrl) {
                controller_->selection_start = 0;
            } else {
                size_t idx = controller_->selection_start;
                while (idx > 0 && controller_->text[idx - 1] != '\n') --idx;
                controller_->selection_start = idx;
            }
            if (!shift) controller_->clearSelection();
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        } else if (key == KEY_END) {
            if (ctrl) {
                controller_->selection_start = controller_->text.length();
            } else {
                size_t idx = controller_->selection_start;
                while (idx < controller_->text.length() && controller_->text[idx] != '\n') ++idx;
                controller_->selection_start = idx;
            }
            if (!shift) controller_->clearSelection();
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        } else if (is_ctrl_a) {
            controller_->selectAll();
            setState([]{});
        } else if (is_ctrl_c) {
            controller_->copyToClipboard();
        } else if (is_ctrl_x && !ta->options.read_only) {
            if (controller_->cutToClipboard()) {
                changed = true;
            }
        } else if (is_ctrl_v && !ta->options.read_only) {
            if (controller_->pasteFromClipboard()) {
                changed = true;
            }
        } else if (is_ctrl_z && !ta->options.read_only) {
            if (controller_->undo()) {
                changed = true;
            }
        } else if (is_ctrl_y && !ta->options.read_only) {
            if (controller_->redo()) {
                changed = true;
            }
        }

        if (changed) {
            if (ta->options.on_changed) ta->options.on_changed(controller_->text);
            notifyCursorMoved();
            resetBlink();
            setState([]{});
        }
    }

public:
    void initState() override {
        State::initState();
        auto* ta = static_cast<const TextAreaWidget*>(widget());
        controller_ = ta->controller;
        is_focused_ = ta->options.auto_focus;

        if (is_focused_) g_focused_textarea = this;

        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (g_focused_textarea != this) return;
                auto* current_ta = static_cast<const TextAreaWidget*>(widget());
                if (!current_ta || current_ta->options.read_only) return;

                // Ignore control characters (ASCII < 32 and DEL 127) exactly like TextField
                if (!text.empty() && text.length() == 1) {
                    char c = text[0];
                    if (c >= 0 && c < 32) return;
                    if (c == 127) return;
                }

                if (controller_->hasSelection()) deleteSelection();
                controller_->pushUndo(controller_->text);
                controller_->text.insert(controller_->selection_start, text);
                controller_->selection_start += text.length();
                controller_->clearSelection();

                if (current_ta->options.on_changed) current_ta->options.on_changed(controller_->text);
                notifyCursorMoved();
                resetBlink();
                setState([]{});
            });

            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (g_focused_textarea != this) return;
                handleKey(key, mods);
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* ta = static_cast<const TextAreaWidget*>(widget());
        controller_ = ta->controller;
    }

    void dispose() override {
        if (g_focused_textarea == this) g_focused_textarea = nullptr;
        if (Platform::instance()) {
            if (text_input_conn_) Platform::instance()->onTextInput().disconnect(text_input_conn_);
            if (key_down_conn_) Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* ta = static_cast<const TextAreaWidget*>(widget());

        if (is_focused_ && Platform::instance()) {
            double cur = Platform::instance()->getTime();
            if (cur - last_blink_time_ > 0.53) {
                show_cursor_ = !show_cursor_;
                last_blink_time_ = cur;
            }
        } else {
            show_cursor_ = false;
        }

        auto render_box = std::make_shared<RenderTextAreaWidget>(
            controller_, ta->options, is_focused_, show_cursor_, scroll_y_
        );

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Text;

        // 1. Single Tap -> Move cursor to exact clicked coordinate
        detector->on_tap_down = [this](const TapDownDetails& e) {
            g_focused_textarea = this;
            is_focused_ = true;
            resetBlink();

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* rta = findTextAreaBox(ro)) {
                    size_t idx = rta->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                    controller_->selection_start = idx;
                    controller_->clearSelection();
                    notifyCursorMoved();
                }
            }
            setState([]{});
        };

        // 2. Double Tap -> Select Word
        detector->on_double_tap_down = [this](const TapDownDetails& e) {
            g_focused_textarea = this;
            is_focused_ = true;
            resetBlink();

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* rta = findTextAreaBox(ro)) {
                    size_t idx = rta->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                    selectWordAt(idx);
                    notifyCursorMoved();
                }
            }
            setState([]{});
        };

        // 3. Long Press -> Select Whole Line
        detector->on_long_press_start = [this](const LongPressStartDetails& e) {
            g_focused_textarea = this;
            is_focused_ = true;
            resetBlink();

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* rta = findTextAreaBox(ro)) {
                    size_t idx = rta->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                    selectLineAt(idx);
                    notifyCursorMoved();
                }
            }
            setState([]{});
        };

        // 4. Click & Drag -> Select Text Range
        detector->on_pan_start = [this](const DragStartDetails& e) {
            g_focused_textarea = this;
            is_focused_ = true;
            resetBlink();

            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* rta = findTextAreaBox(ro)) {
                    size_t idx = rta->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                    controller_->selection_start = idx;
                    controller_->selection_end   = idx;
                    notifyCursorMoved();
                }
            }
            setState([]{});
        };

        detector->on_pan_update = [this](const DragUpdateDetails& e) {
            if (auto* ro = context().element()->findRenderObject()) {
                if (auto* rta = findTextAreaBox(ro)) {
                    size_t idx = rta->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                    controller_->selection_end = idx;
                    notifyCursorMoved();
                }
            }
            setState([]{});
        };

        // 5. Mouse Wheel Scroll
        detector->on_scroll = [this](float, float dy) {
            scroll_y_ -= dy * 24.0f;
            if (scroll_y_ < 0.0f) scroll_y_ = 0.0f;
            setState([]{});
        };

        detector->child = render_box;

        // Optional Footer Bar with Word/Char Counter
        if (ta->options.show_counter) {
            size_t curr_row = 1, curr_col = 1;
            controller_->getCursorPosition(curr_row, curr_col);

            std::string pos_str = "Ln " + std::to_string(curr_row) + ", Col " + std::to_string(curr_col);
            auto pos_txt = text({
                .text = pos_str,
                .color = 0xFF94A3B8,
                .font_size = 11.0f,
            });

            std::string count_str = std::to_string(controller_->text.length()) +
                                    (ta->options.max_characters > 0 ? " / " + std::to_string(ta->options.max_characters) : "") +
                                    " chars • " + std::to_string(controller_->getWordCount()) + " words";
            auto count_txt = text({
                .text = count_str,
                .color = 0xFF94A3B8,
                .font_size = 11.0f,
            });

            std::vector<WidgetPtr> footer_items = {pos_txt, count_txt};
            auto footer_row = row(footer_items);
            footer_row->justifyContent(Justify::SpaceBetween);

            auto footer_box = container(footer_row);
            footer_box->paddingSymmetric(4.0f, 6.0f);

            std::vector<WidgetPtr> all_items = {detector, footer_box};
            auto res_col = column(all_items);
            return res_col;
        }

        return detector;
    }
};

std::unique_ptr<State> TextAreaWidget::createState() {
    return std::make_unique<TextAreaState>();
}

} // namespace enki
