#include "enki/widgets/text_field.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Internal Render Object & Widget
// ════════════════════════════════════════════════════════════════

class RenderTextField : public RenderParagraph {
public:
    std::shared_ptr<TextFieldController> controller;
    TextFieldProps options;
    bool is_focused = false;
    bool show_cursor = false;

    RenderTextField(std::shared_ptr<InlineSpan> span, std::shared_ptr<TextFieldController> ctrl, const TextFieldProps& opt)
        : RenderParagraph(span, opt.style, TextAlign::Start, TextDirection::LTR, TextOverflow::Clip, opt.max_lines, false),
          controller(ctrl), options(opt) {
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    }

    void paint(PaintContext& ctx) override {
        auto* p = static_cast<skia::textlayout::Paragraph*>(getNativeParagraph());
        if (!p) return;

        layoutParagraph(size_.width);
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());

        // 1. Draw Selection
        if (controller->hasSelection()) {
            size_t start = std::min(controller->selection_start, controller->selection_end);
            size_t end = std::max(controller->selection_start, controller->selection_end);
            
            auto rects = p->getRectsForRange(start, end, skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);
            std::cout << "[Selection] start=" << start << " end=" << end << " rects=" << rects.size() << std::endl;
            
            SkPaint sel_paint;
            sel_paint.setColor(static_cast<SkColor>(options.selection_color));
            
            sk_canvas->save();
            sk_canvas->translate(ctx.offset.x, ctx.offset.y);
            for (const auto& tb : rects) {
                sk_canvas->drawRect(tb.rect, sel_paint);
            }
            sk_canvas->restore();
        }

        // 2. Draw Text (via base class)
        ctx.canvas.drawParagraph(p, ctx.offset.x, ctx.offset.y);

        // 3. Draw Cursor
        if (is_focused && show_cursor && !controller->hasSelection() && !options.read_only) {
            auto rects = p->getRectsForRange(controller->selection_start, controller->selection_start + 1, skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);
            
            float cursor_x = 0;
            float cursor_y = 0;
            float cursor_h = options.style.font_size * 1.2f;

            if (!rects.empty()) {
                cursor_x = rects[0].rect.fLeft;
                cursor_y = rects[0].rect.fTop;
                cursor_h = rects[0].rect.height();
            } else if (controller->selection_start > 0) {
                // End of line/text
                auto prev_rects = p->getRectsForRange(controller->selection_start - 1, controller->selection_start, skia::textlayout::RectHeightStyle::kTight, skia::textlayout::RectWidthStyle::kTight);
                if (!prev_rects.empty()) {
                    cursor_x = prev_rects.back().rect.fRight;
                    cursor_y = prev_rects.back().rect.fTop;
                    cursor_h = prev_rects.back().rect.height();
                }
            }

            SkPaint cursor_paint;
            cursor_paint.setColor(static_cast<SkColor>(options.cursor_color));
            
            sk_canvas->save();
            sk_canvas->translate(ctx.offset.x, ctx.offset.y);
            sk_canvas->drawRect(SkRect::MakeXYWH(cursor_x, cursor_y, 2.0f, cursor_h), cursor_paint);
            sk_canvas->restore();
        }
    }

    size_t getIndexAtCoordinate(float x, float y) {
        auto* p = static_cast<skia::textlayout::Paragraph*>(getNativeParagraph());
        if (!p) return 0;
        layoutParagraph(size_.width);
        
        auto pos = p->getGlyphPositionAtCoordinate(x, y);
        return pos.position;
    }
};

class RenderTextFieldWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<TextFieldController> controller;
    TextFieldProps options;
    bool is_focused;
    bool show_cursor;
    std::string text_to_display;

    RenderTextFieldWidget(std::string text_to_display, std::shared_ptr<TextFieldController> ctrl, TextFieldProps opt, bool focused, bool cursor)
        : SingleChildRenderObjectWidget(Key::none()), controller(ctrl), options(opt), is_focused(focused), show_cursor(cursor), text_to_display(std::move(text_to_display)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto span = std::make_shared<TextSpan>(text_to_display, options.style);
        auto ro = std::make_unique<RenderTextField>(span, controller, options);
        ro->is_focused = is_focused;
        ro->show_cursor = show_cursor;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderTextField*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->is_focused = is_focused;
            ro->show_cursor = show_cursor;
            
            if (ro->getTextData() != text_to_display) {
                ro->setText(text_to_display, options.style, TextAlign::Start, TextDirection::LTR, TextOverflow::Clip, options.max_lines, false);
            } else {
                ro->markNeedsPaint(); // For cursor blink / selection update
            }
        }
    }
    
    std::string_view typeName() const override { return "RenderTextFieldWidget"; }
};

// ════════════════════════════════════════════════════════════════
// TextFieldState
// ════════════════════════════════════════════════════════════════

class TextFieldState;
static TextFieldState* g_focused_textfield = nullptr;

class TextFieldState : public State {
    std::shared_ptr<TextFieldController> controller_;
    bool is_focused_ = false;
    bool show_cursor_ = true;
    double last_blink_time_ = 0;
    
    uint64_t text_input_conn_ = 0;
    uint64_t key_down_conn_ = 0;

    size_t getPrevCharBytes(size_t index) {
        if (index == 0) return 0;
        size_t num_bytes = 1;
        while (index >= num_bytes + 1 && (controller_->text[index - num_bytes] & 0xC0) == 0x80) {
            num_bytes++;
        }
        return num_bytes;
    }

    size_t getNextCharBytes(size_t index) {
        if (index >= controller_->text.length()) return 0;
        size_t num_bytes = 1;
        while (index + num_bytes < controller_->text.length() && (controller_->text[index + num_bytes] & 0xC0) == 0x80) {
            num_bytes++;
        }
        return num_bytes;
    }

public:
    void initState() override {
        State::initState();
        auto* tf = static_cast<const TextFieldWidget*>(widget());
        controller_ = tf->controller;
        is_focused_ = tf->options.auto_focus;
        if (is_focused_) g_focused_textfield = this;
        
        // Connect to Platform inputs
        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (g_focused_textfield != this) return;
                
                auto* current_tf = static_cast<const TextFieldWidget*>(widget());
                if (current_tf->options.read_only) return;
                
                // Ignore control characters
                if (!text.empty() && text.length() == 1) {
                    char c = text[0];
                    if (c >= 0 && c < 32) return;
                    if (c == 127) return;
                }
                
                // Delete selection first if any
                if (controller_->hasSelection()) {
                    deleteSelection();
                }
                
                size_t insert_pos = std::min(controller_->selection_start, controller_->text.length());
                controller_->text.insert(insert_pos, text);
                controller_->selection_start = insert_pos + text.length();
                controller_->clearSelection();
                
                if (current_tf->options.on_changed) current_tf->options.on_changed(controller_->text);
                resetBlink();
                setState([]{});
            });
            
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (g_focused_textfield != this) return;
                handleKey(key, mods);
            });
        }
    }

    // NEW: Handle Widget updates to retain correct controller instance
    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* tf = static_cast<const TextFieldWidget*>(widget());
        controller_ = tf->controller;
    }

    void dispose() override {
        if (g_focused_textfield == this) g_focused_textfield = nullptr;
        if (Platform::instance()) {
            Platform::instance()->onTextInput().disconnect(text_input_conn_);
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    void handleKey(int key, int mods) {
        auto* tf = static_cast<const TextFieldWidget*>(widget());
        
        // XKB Keysyms
        const int KEY_BACKSPACE = 0xff08;
        const int KEY_DELETE    = 0xffff;
        const int KEY_LEFT      = 0xff51;
        const int KEY_RIGHT     = 0xff53;
        const int KEY_RETURN    = 0xff0d;
        
        bool shift = (mods & 1) != 0; // Shift mod
        bool ctrl = (mods & 2) != 0 || (mods & 4) != 0;  // Ctrl mod (Wayland bit 2 / X11 bit 4)
        
        bool changed = false;
        
        if (key == KEY_BACKSPACE && !tf->options.read_only) { // Backspace
            if (controller_->hasSelection()) {
                deleteSelection();
                changed = true;
            } else if (controller_->selection_start > 0) {
                size_t num_bytes = getPrevCharBytes(controller_->selection_start);
                controller_->selection_start -= num_bytes;
                controller_->text.erase(controller_->selection_start, num_bytes);
                controller_->clearSelection();
                changed = true;
            }
        } else if (key == KEY_DELETE && !tf->options.read_only) { // Delete
            if (controller_->hasSelection()) {
                deleteSelection();
                changed = true;
            } else if (controller_->selection_start < controller_->text.length()) {
                size_t num_bytes = getNextCharBytes(controller_->selection_start);
                controller_->text.erase(controller_->selection_start, num_bytes);
                changed = true;
            }
        } else if (key == KEY_LEFT) { // Left Arrow
            if (!shift && controller_->hasSelection()) {
                controller_->selection_start = std::min(controller_->selection_start, controller_->selection_end);
                controller_->clearSelection();
            } else if (controller_->selection_start > 0) {
                controller_->selection_start -= getPrevCharBytes(controller_->selection_start);
                if (!shift) controller_->clearSelection();
            }
            resetBlink();
            setState([]{}); // Needs paint
        } else if (key == KEY_RIGHT) { // Right Arrow
            if (!shift && controller_->hasSelection()) {
                controller_->selection_start = std::max(controller_->selection_start, controller_->selection_end);
                controller_->clearSelection();
            } else if (controller_->selection_start < controller_->text.length()) {
                controller_->selection_start += getNextCharBytes(controller_->selection_start);
                if (!shift) controller_->clearSelection();
            }
            resetBlink();
            setState([]{}); // Needs paint
        } else if (key == KEY_RETURN) { // Enter (Return)
            if (tf->options.on_submitted) tf->options.on_submitted(controller_->text);
        } else if (ctrl && (key == 0x61 || key == 0x41 || key == 0x01)) { // Ctrl+A (a or A)
            controller_->selectAll();
            setState([]{});
        } else if (ctrl && (key == 0x63 || key == 0x43 || key == 0x03)) { // Ctrl+C
            if (controller_->hasSelection()) {
                size_t start = std::min(controller_->selection_start, controller_->selection_end);
                size_t end = std::max(controller_->selection_start, controller_->selection_end);
                Platform::instance()->setClipboardText(controller_->text.substr(start, end - start));
            }
        } else if (ctrl && (key == 0x76 || key == 0x56 || key == 0x16) && !tf->options.read_only) { // Ctrl+V
            std::string paste = Platform::instance()->getClipboardText();
            if (!paste.empty()) {
                if (controller_->hasSelection()) deleteSelection();
                controller_->text.insert(controller_->selection_start, paste);
                controller_->selection_start += paste.length();
                controller_->clearSelection();
                changed = true;
            }
        } else if (ctrl && (key == 0x78 || key == 0x58 || key == 0x18) && !tf->options.read_only) { // Ctrl+X
            if (controller_->hasSelection()) {
                size_t start = std::min(controller_->selection_start, controller_->selection_end);
                size_t end = std::max(controller_->selection_start, controller_->selection_end);
                Platform::instance()->setClipboardText(controller_->text.substr(start, end - start));
                deleteSelection();
                changed = true;
            }
        }
        
        if (changed) {
            if (tf->options.on_changed) {
                tf->options.on_changed(controller_->text);
            }
            resetBlink();
            setState([]{});
        }
    }

    void deleteSelection() {
        if (!controller_ || controller_->text.empty()) {
            if (controller_) controller_->clearSelection();
            return;
        }
        size_t start = std::min({controller_->selection_start, controller_->selection_end, controller_->text.length()});
        size_t end = std::min(std::max(controller_->selection_start, controller_->selection_end), controller_->text.length());
        if (start < end) {
            controller_->text.erase(start, end - start);
        }
        controller_->selection_start = start;
        controller_->clearSelection();
    }

    void unfocus() {
        if (is_focused_) {
            is_focused_ = false;
            show_cursor_ = false;
            if (controller_) controller_->clearSelection();
            setState([] {});
        }
    }

    void resetBlink() {
        show_cursor_ = true;
        if (Platform::instance()) last_blink_time_ = Platform::instance()->getTime();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* tf = static_cast<const TextFieldWidget*>(widget());
        
        if (is_focused_ && Platform::instance()) {
            double current_time = Platform::instance()->getTime();
            if (current_time - last_blink_time_ > 0.53) {
                show_cursor_ = !show_cursor_;
                last_blink_time_ = current_time;
                if (auto* ro = ctx.element()->findRenderObject()) {
                    ro->markNeedsPaint();
                }
            }
        }
        
        std::string display_text = controller_->text.empty() ? tf->options.hint_text : controller_->text;
        if (tf->options.obscure_text && !controller_->text.empty()) {
            display_text = std::string(controller_->text.length(), '*');
        }

        auto text_field_widget = std::make_shared<RenderTextFieldWidget>(
            display_text, 
            controller_, 
            tf->options, 
            is_focused_, 
            show_cursor_
        );

        return gestureDetector({
            .child = text_field_widget,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Text,
            .on_tap_down = [this, tf](const TapDownDetails& e) {
                if (g_focused_textfield && g_focused_textfield != this) {
                    g_focused_textfield->unfocus();
                }
                g_focused_textfield = this;
                is_focused_ = true;
                
                if (controller_->text.empty()) {
                    controller_->selection_start = 0;
                    controller_->selection_end = 0;
                } else if (auto* el = element()) {
                    if (auto* rtf = dynamic_cast<RenderTextField*>(el->findRenderObject())) {
                        size_t index = rtf->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                        index = std::min(index, controller_->text.length());
                        if (e.modifiers & 1) {
                            controller_->selection_end = index;
                        } else {
                            controller_->selection_start = index;
                            controller_->clearSelection();
                        }
                    }
                }
                
                resetBlink();
                setState([]{});
            },
            .on_pan_update = [this, tf](const DragUpdateDetails& e) {
                if (controller_->text.empty()) {
                    controller_->selection_start = 0;
                    controller_->selection_end = 0;
                } else if (auto* el = element()) {
                    if (auto* rtf = dynamic_cast<RenderTextField*>(el->findRenderObject())) {
                        size_t index = rtf->getIndexAtCoordinate(e.local_position.x, e.local_position.y);
                        index = std::min(index, controller_->text.length());
                        controller_->selection_end = index;
                        resetBlink();
                        setState([]{});
                    }
                }
            },
        });
    }
};

std::unique_ptr<State> TextFieldWidget::createState() {
    return std::make_unique<TextFieldState>();
}

} // namespace enki
