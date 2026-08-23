/// @file password_field.cpp
/// @brief Implementation of Advanced PasswordField widget.

#include "enki/widgets/password_field.hpp"
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
#include <cctype>
#include <cmath>

namespace enki {

class PasswordFieldState;
static PasswordFieldState* g_focused_passwordfield = nullptr;

static sk_sp<skia::textlayout::FontCollection> getPasswordFieldFontCollection() {
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
// Password Validation Rules Implementation
// ════════════════════════════════════════════════════════════════

bool PasswordValidationRules::checkUppercase(std::string_view p) const {
    return std::any_of(p.begin(), p.end(), [](unsigned char c) { return std::isupper(c); });
}

bool PasswordValidationRules::checkLowercase(std::string_view p) const {
    return std::any_of(p.begin(), p.end(), [](unsigned char c) { return std::islower(c); });
}

bool PasswordValidationRules::checkDigit(std::string_view p) const {
    return std::any_of(p.begin(), p.end(), [](unsigned char c) { return std::isdigit(c); });
}

bool PasswordValidationRules::checkSpecial(std::string_view p) const {
    return std::any_of(p.begin(), p.end(), [](unsigned char c) { return std::ispunct(c); });
}

bool PasswordValidationRules::meetsAll(std::string_view p) const {
    if (!checkMinLength(p)) return false;
    if (require_uppercase && !checkUppercase(p)) return false;
    if (require_lowercase && !checkLowercase(p)) return false;
    if (require_digit && !checkDigit(p)) return false;
    if (require_special && !checkSpecial(p)) return false;
    return true;
}

// ════════════════════════════════════════════════════════════════
// PasswordFieldController Implementation
// ════════════════════════════════════════════════════════════════

double PasswordFieldController::calculateEntropy() const {
    if (password_.empty()) return 0.0;

    int pool_size = 0;
    if (rules_.checkLowercase(password_)) pool_size += 26;
    if (rules_.checkUppercase(password_)) pool_size += 26;
    if (rules_.checkDigit(password_)) pool_size += 10;
    if (rules_.checkSpecial(password_)) pool_size += 33;
    if (pool_size == 0) pool_size = 26;

    return static_cast<double>(password_.length()) * (std::log2(static_cast<double>(pool_size)));
}

PasswordStrengthLevel PasswordFieldController::calculateStrength() const {
    if (password_.empty()) return PasswordStrengthLevel::Empty;

    double entropy = calculateEntropy();
    if (entropy < 28.0 || password_.length() < 6) return PasswordStrengthLevel::VeryWeak;
    if (entropy < 45.0 || password_.length() < 8) return PasswordStrengthLevel::Weak;
    if (entropy < 65.0) return PasswordStrengthLevel::Medium;
    if (entropy < 85.0 || !meetsAllRules()) return PasswordStrengthLevel::Strong;
    return PasswordStrengthLevel::VeryStrong;
}

void PasswordFieldController::generateStrongPassword(size_t length, bool use_symbols) {
    const std::string lower = "abcdefghijklmnopqrstuvwxyz";
    const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";
    const std::string symbols = "!@#$%^&*()-_=+[]{}|;:,.<>?";

    std::string charset = lower + upper + digits;
    if (use_symbols) charset += symbols;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist_charset(0, charset.size() - 1);
    std::uniform_int_distribution<size_t> dist_lower(0, lower.size() - 1);
    std::uniform_int_distribution<size_t> dist_upper(0, upper.size() - 1);
    std::uniform_int_distribution<size_t> dist_digit(0, digits.size() - 1);
    std::uniform_int_distribution<size_t> dist_symbol(0, symbols.size() - 1);

    std::string result;
    result.reserve(length);

    // Ensure at least one of each required category
    result += lower[dist_lower(gen)];
    result += upper[dist_upper(gen)];
    result += digits[dist_digit(gen)];
    if (use_symbols) result += symbols[dist_symbol(gen)];

    while (result.length() < length) {
        result += charset[dist_charset(gen)];
    }

    std::shuffle(result.begin(), result.end(), gen);
    password_ = result;
}

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for PasswordField Box
// ════════════════════════════════════════════════════════════════

class RenderPasswordFieldBox : public RenderBox {
public:
    std::shared_ptr<PasswordFieldController> controller;
    PasswordFieldProps options;
    bool is_focused = false;
    bool is_hovered = false;
    bool show_cursor = false;
    bool is_capslock_on = false;
    size_t cursor_pos = 0;
    size_t selection_start = 0;
    size_t selection_end = 0;

    int hovered_btn = 0; // 0=none, 1=eye, 2=generator, 3=clear

    std::unique_ptr<skia::textlayout::Paragraph> text_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> lock_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> capslock_paragraph_;

    RenderPasswordFieldBox(std::shared_ptr<PasswordFieldController> ctrl, PasswordFieldProps opt)
        : controller(std::move(ctrl)), options(std::move(opt)) {
        
        FlexboxStyle st;
        st.height = StyleValue::point(42.0f);
        applyFlexboxStyle(anuNode(), st);
    }

    void layoutParagraphs() {
        auto fc = getPasswordFieldFontCollection();
        if (!fc) return;

        float font_sz = options.style.font_size > 0 ? options.style.font_size : 14.0f;

        skia::textlayout::ParagraphStyle p_style;
        p_style.setTextAlign(skia::textlayout::TextAlign::kLeft);

        // 1. Lock Icon
        if (options.show_lock_icon) {
            auto b_lock = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_ic;
            t_ic.setFontSize(font_sz * 1.05f);
            t_ic.setColor(static_cast<SkColor>(options.icon_color));
            b_lock->pushStyle(t_ic);
            b_lock->addText("🔒", 4);
            lock_paragraph_ = b_lock->Build();
            lock_paragraph_->layout(30.0f);
        } else {
            lock_paragraph_.reset();
        }

        // 2. Main Password (Obscured or Plain) / Placeholder
        auto b_main = skia::textlayout::ParagraphBuilder::make(p_style, fc);
        skia::textlayout::TextStyle t_main;
        t_main.setFontSize(font_sz);

        if (!options.style.font_family.empty()) {
            std::vector<SkString> font_fams = {SkString(options.style.font_family.c_str())};
            t_main.setFontFamilies(font_fams);
        }

        const std::string& pwd = controller->getPassword();
        if (pwd.empty() && !is_focused) {
            t_main.setColor(static_cast<SkColor>(options.placeholder_color));
            b_main->pushStyle(t_main);
            b_main->addText(options.placeholder.c_str(), options.placeholder.length());
        } else {
            t_main.setColor(static_cast<SkColor>(options.style.color != 0 ? options.style.color : 0xFFF1F5F9));
            b_main->pushStyle(t_main);

            if (controller->isObscured()) {
                std::string masked = "";
                for (size_t i = 0; i < pwd.length(); ++i) {
                    masked += options.mask_char;
                }
                b_main->addText(masked.c_str(), masked.length());
            } else {
                b_main->addText(pwd.c_str(), pwd.length());
            }
        }
        text_paragraph_ = b_main->Build();
        text_paragraph_->layout(size_.width > 0 ? size_.width : 300.0f);

        // 3. CapsLock Warning Badge
        if (options.show_capslock_warning && is_capslock_on && is_focused) {
            auto b_caps = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_cp;
            t_cp.setFontSize(10.5f);
            t_cp.setColor(static_cast<SkColor>(options.warning_color));
            b_caps->pushStyle(t_cp);
            b_caps->addText("⇪ CAPS", 8);
            capslock_paragraph_ = b_caps->Build();
            capslock_paragraph_->layout(60.0f);
        } else {
            capslock_paragraph_.reset();
        }
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        layoutParagraphs();

        SkRect bounds = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(bounds, options.border_radius, options.border_radius);

        // 1. Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        sk_canvas->drawRRect(rrect, bg_paint);

        sk_canvas->save();
        sk_canvas->clipRRect(rrect, true);

        // 2. Lock Icon
        float content_x = ctx.offset.x + options.padding.left;
        float cy = ctx.offset.y + size_.height * 0.5f;

        if (lock_paragraph_) {
            float lock_y = cy - lock_paragraph_->getHeight() * 0.5f;
            lock_paragraph_->paint(sk_canvas, content_x, lock_y);
            content_x += lock_paragraph_->getMaxIntrinsicWidth() + 8.0f;
        }

        // 3. Text & Caret & Selection
        if (text_paragraph_) {
            float text_y = cy - text_paragraph_->getHeight() * 0.5f;

            if (is_focused && selection_start != selection_end) {
                size_t s = std::min(selection_start, selection_end);
                size_t e = std::max(selection_start, selection_end);
                auto rects = text_paragraph_->getRectsForRange(s, e,
                    skia::textlayout::RectHeightStyle::kTight,
                    skia::textlayout::RectWidthStyle::kTight);

                SkPaint sel_paint;
                sel_paint.setColor(static_cast<SkColor>(options.selection_color));
                sk_canvas->save();
                sk_canvas->translate(content_x, text_y);
                for (const auto& r : rects) {
                    sk_canvas->drawRect(r.rect, sel_paint);
                }
                sk_canvas->restore();
            }

            text_paragraph_->paint(sk_canvas, content_x, text_y);

            // Blinking Caret
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
                sk_canvas->drawRect(SkRect::MakeXYWH(content_x + c_x, text_y, 2.0f, c_h), cur_paint);
            }
        }

        // 4. Trailing Action Buttons
        float trail_right = ctx.offset.x + size_.width - options.padding.right;

        // Eye Visibility Toggle [ 👁 / 🙈 ]
        if (options.show_visibility_toggle) {
            float eye_cx = trail_right - 12.0f;
            float eye_cy = cy;

            if (hovered_btn == 1) {
                SkPaint hov_paint;
                hov_paint.setAntiAlias(true);
                hov_paint.setColor(0x3394A3B8);
                sk_canvas->drawCircle(eye_cx, eye_cy, 12.0f, hov_paint);
            }

            auto fc = getPasswordFieldFontCollection();
            skia::textlayout::ParagraphStyle p_style;
            auto b_eye = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_eye;
            t_eye.setFontSize(13.0f);
            t_eye.setColor(hovered_btn == 1 ? 0xFFFFFFFF : 0xFF94A3B8);
            b_eye->pushStyle(t_eye);
            b_eye->addText(controller->isObscured() ? "👁" : "🙈", 4);
            auto eye_p = b_eye->Build();
            eye_p->layout(24.0f);
            eye_p->paint(sk_canvas, eye_cx - 7.0f, eye_cy - eye_p->getHeight() * 0.5f);

            trail_right -= 28.0f;
        }

        // Clear Button [ ✕ ]
        if (options.show_clear_button && !controller->getPassword().empty() && !options.read_only) {
            float btn_cx = trail_right - 10.0f;
            float btn_cy = cy;

            if (hovered_btn == 3) {
                SkPaint hov_paint;
                hov_paint.setAntiAlias(true);
                hov_paint.setColor(0x3394A3B8);
                sk_canvas->drawCircle(btn_cx, btn_cy, 10.0f, hov_paint);
            }

            SkPaint x_paint;
            x_paint.setAntiAlias(true);
            x_paint.setColor(hovered_btn == 3 ? 0xFFFFFFFF : 0xFF94A3B8);
            x_paint.setStrokeWidth(1.6f);
            float sz = 4.0f;
            sk_canvas->drawLine(btn_cx - sz, btn_cy - sz, btn_cx + sz, btn_cy + sz, x_paint);
            sk_canvas->drawLine(btn_cx + sz, btn_cy - sz, btn_cx - sz, btn_cy + sz, x_paint);

            trail_right -= 24.0f;
        }

        // Password Generator Button [ 🎲 ]
        if (options.show_generator_button && !options.read_only) {
            float gen_cx = trail_right - 12.0f;
            float gen_cy = cy;

            if (hovered_btn == 2) {
                SkPaint hov_paint;
                hov_paint.setAntiAlias(true);
                hov_paint.setColor(0x3394A3B8);
                sk_canvas->drawCircle(gen_cx, gen_cy, 12.0f, hov_paint);
            }

            auto fc = getPasswordFieldFontCollection();
            skia::textlayout::ParagraphStyle p_style;
            auto b_gen = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_gen;
            t_gen.setFontSize(13.0f);
            t_gen.setColor(hovered_btn == 2 ? 0xFFFFFFFF : 0xFF94A3B8);
            b_gen->pushStyle(t_gen);
            b_gen->addText("🎲", 4);
            auto gen_p = b_gen->Build();
            gen_p->layout(24.0f);
            gen_p->paint(sk_canvas, gen_cx - 7.0f, gen_cy - gen_p->getHeight() * 0.5f);

            trail_right -= 28.0f;
        }

        // CapsLock Warning Badge
        if (capslock_paragraph_) {
            float w = capslock_paragraph_->getMaxIntrinsicWidth() + 10.0f;
            float h = capslock_paragraph_->getHeight() + 4.0f;
            float bx = trail_right - w;
            float by = cy - h * 0.5f;

            SkRRect cb_rrect;
            cb_rrect.setRectXY(SkRect::MakeXYWH(bx, by, w, h), 4.0f, 4.0f);
            SkPaint cb_bg;
            cb_bg.setAntiAlias(true);
            cb_bg.setColor(0x2BF59E0B);
            sk_canvas->drawRRect(cb_rrect, cb_bg);

            capslock_paragraph_->paint(sk_canvas, bx + 5.0f, by + 2.0f);
        }

        sk_canvas->restore();

        // 5. Border
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

    // 0=text, 1=eye, 2=gen, 3=clear
    int getHitComponent(float local_x, float local_y) const {
        float trail_right = size_.width - options.padding.right;

        if (options.show_visibility_toggle) {
            float eye_cx = trail_right - 12.0f;
            float eye_cy = size_.height * 0.5f;
            float dx = local_x - eye_cx;
            float dy = local_y - eye_cy;
            if ((dx * dx + dy * dy) <= (14.0f * 14.0f)) return 1;
            trail_right -= 28.0f;
        }

        if (options.show_clear_button && !controller->getPassword().empty() && !options.read_only) {
            float clr_cx = trail_right - 10.0f;
            float clr_cy = size_.height * 0.5f;
            float dx = local_x - clr_cx;
            float dy = local_y - clr_cy;
            if ((dx * dx + dy * dy) <= (12.0f * 12.0f)) return 3;
            trail_right -= 24.0f;
        }

        if (options.show_generator_button && !options.read_only) {
            float gen_cx = trail_right - 12.0f;
            float gen_cy = size_.height * 0.5f;
            float dx = local_x - gen_cx;
            float dy = local_y - gen_cy;
            if ((dx * dx + dy * dy) <= (14.0f * 14.0f)) return 2;
            trail_right -= 28.0f;
        }

        return 0; // Text area
    }

    size_t getIndexAtCoordinate(float local_x) {
        layoutParagraphs();
        if (!text_paragraph_) return 0;

        float content_x = options.padding.left;
        if (lock_paragraph_) content_x += lock_paragraph_->getMaxIntrinsicWidth() + 8.0f;

        float target_x = std::max(0.0f, local_x - content_x);
        auto pos = text_paragraph_->getGlyphPositionAtCoordinate(target_x, 5.0f);
        return std::min(static_cast<size_t>(pos.position), controller->getPassword().length());
    }
};

static RenderPasswordFieldBox* findPasswordFieldBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderPasswordFieldBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findPasswordFieldBox(child)) return found;
    }
    return nullptr;
}

class RenderPasswordFieldWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<PasswordFieldController> controller;
    PasswordFieldProps options;
    bool is_focused;
    bool is_hovered;
    bool show_cursor;
    bool is_capslock_on;
    size_t cursor_pos;
    size_t selection_start;
    size_t selection_end;
    int hovered_btn;

    RenderPasswordFieldWidget(std::shared_ptr<PasswordFieldController> ctrl, PasswordFieldProps opt,
                              bool focused, bool hovered, bool cursor, bool caps, size_t cur_pos,
                              size_t sel_s, size_t sel_e, int h_btn)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), is_focused(focused), is_hovered(hovered),
          show_cursor(cursor), is_capslock_on(caps), cursor_pos(cur_pos),
          selection_start(sel_s), selection_end(sel_e), hovered_btn(h_btn) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderPasswordFieldBox>(controller, options);
        ro->is_focused = is_focused;
        ro->is_hovered = is_hovered;
        ro->show_cursor = show_cursor;
        ro->is_capslock_on = is_capslock_on;
        ro->cursor_pos = cursor_pos;
        ro->selection_start = selection_start;
        ro->selection_end = selection_end;
        ro->hovered_btn = hovered_btn;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderPasswordFieldBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->is_focused = is_focused;
            ro->is_hovered = is_hovered;
            ro->show_cursor = show_cursor;
            ro->is_capslock_on = is_capslock_on;
            ro->cursor_pos = cursor_pos;
            ro->selection_start = selection_start;
            ro->selection_end = selection_end;
            ro->hovered_btn = hovered_btn;
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderPasswordFieldWidget"; }
};

// ════════════════════════════════════════════════════════════════
// PasswordField State Implementation
// ════════════════════════════════════════════════════════════════

class PasswordFieldState : public State {
private:
    std::shared_ptr<PasswordFieldController> controller_;
    bool is_focused_ = false;
    bool is_hovered_ = false;
    bool show_cursor_ = true;
    bool is_capslock_on_ = false;
    double last_blink_time_ = 0.0;

    size_t cursor_pos_ = 0;
    size_t selection_start_ = 0;
    size_t selection_end_ = 0;
    int hovered_btn_ = 0;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;
    SlotId mouse_down_conn_ = 0;

    void focus() {
        if (g_focused_passwordfield && g_focused_passwordfield != this) {
            g_focused_passwordfield->unfocus();
        }
        g_focused_passwordfield = this;
        is_focused_ = true;
        resetBlink();
        setState([] {});
    }

    void unfocus() {
        if (!is_focused_) return;
        is_focused_ = false;
        show_cursor_ = false;
        selection_start_ = 0;
        selection_end_ = 0;
        if (g_focused_passwordfield == this) {
            g_focused_passwordfield = nullptr;
        }
        setState([] {});
    }

    void resetBlink() {
        show_cursor_ = true;
        if (Platform::instance()) last_blink_time_ = Platform::instance()->getTime();
    }

    void deleteSelection() {
        if (selection_start_ == selection_end_) return;
        size_t s = std::min(selection_start_, selection_end_);
        size_t e = std::max(selection_start_, selection_end_);
        std::string p = controller_->getPassword();
        p.erase(s, e - s);
        controller_->setPassword(p);
        cursor_pos_ = s;
        selection_start_ = s;
        selection_end_ = s;
    }

    void notifyPasswordChanged() {
        auto* pf = static_cast<const PasswordFieldWidget*>(widget());
        if (!pf) return;

        resetBlink();
        if (pf->options.on_changed) {
            pf->options.on_changed(controller_->getPassword());
        }
        if (pf->options.on_strength_changed) {
            pf->options.on_strength_changed(controller_->calculateStrength());
        }
        setState([] {});
    }

    void handleKey(int key, int mods) {
        auto* pf = static_cast<const PasswordFieldWidget*>(widget());
        if (!pf) return;

        bool shift = (mods & 1) != 0;
        bool ctrl  = (mods & 2) != 0 || (mods & 4) != 0;
        is_capslock_on_ = (mods & 2) != 0; // CapsLock modifier check

        const int KEY_BACKSPACE = 0xff08;
        const int KEY_DELETE    = 0xffff;
        const int KEY_LEFT      = 0xff51;
        const int KEY_RIGHT     = 0xff53;
        const int KEY_HOME      = 0xff50;
        const int KEY_END       = 0xff57;
        const int KEY_RETURN    = 0xff0d;
        const int KEY_ESCAPE    = 0xff1b;

        // Clipboard & Selection Shortcuts
        bool is_ctrl_c = (ctrl && (key == 'c' || key == 'C' || key == 0x63 || key == 0x43 || key == 54)) || (key == 0x03);
        bool is_ctrl_v = (ctrl && (key == 'v' || key == 'V' || key == 0x76 || key == 0x56 || key == 55)) || (key == 0x16);
        bool is_ctrl_x = (ctrl && (key == 'x' || key == 'X' || key == 0x78 || key == 0x58 || key == 53)) || (key == 0x18);
        bool is_ctrl_a = (ctrl && (key == 'a' || key == 'A' || key == 0x61 || key == 0x41 || key == 38)) || (key == 0x01);

        if ((key == KEY_RETURN || key == 0xff8d)) {
            if (pf->options.on_submitted) pf->options.on_submitted(controller_->getPassword());
        } else if (key == KEY_ESCAPE) {
            unfocus();
        } else if ((key == KEY_BACKSPACE || key == 0x08 || key == 0x7f) && !pf->options.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ > 0) {
                std::string p = controller_->getPassword();
                p.erase(cursor_pos_ - 1, 1);
                controller_->setPassword(p);
                cursor_pos_--;
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            }
            notifyPasswordChanged();
        } else if ((key == KEY_DELETE || key == 0xff9f) && !pf->options.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ < controller_->getPassword().length()) {
                std::string p = controller_->getPassword();
                p.erase(cursor_pos_, 1);
                controller_->setPassword(p);
            }
            notifyPasswordChanged();
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
            if (cursor_pos_ < controller_->getPassword().length()) cursor_pos_++;
            if (!shift) {
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            } else {
                selection_end_ = cursor_pos_;
            }
            resetBlink();
            setState([] {});
        } else if (key == KEY_HOME) {
            cursor_pos_ = 0;
            if (!shift) {
                selection_start_ = 0;
                selection_end_ = 0;
            } else {
                selection_end_ = 0;
            }
            resetBlink();
            setState([] {});
        } else if (key == KEY_END) {
            cursor_pos_ = controller_->getPassword().length();
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
            selection_end_ = controller_->getPassword().length();
            cursor_pos_ = selection_end_;
            setState([] {});
        } else if (is_ctrl_c && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = controller_->getPassword().substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
        } else if (is_ctrl_x && !pf->options.read_only && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = controller_->getPassword().substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
            deleteSelection();
            notifyPasswordChanged();
        } else if (is_ctrl_v && !pf->options.read_only) {
            if (Platform::instance()) {
                std::string paste = Platform::instance()->getClipboardText();
                if (!paste.empty()) {
                    deleteSelection();
                    std::string p = controller_->getPassword();
                    p.insert(cursor_pos_, paste);
                    controller_->setPassword(p);
                    cursor_pos_ += paste.length();
                    selection_start_ = cursor_pos_;
                    selection_end_ = cursor_pos_;
                    notifyPasswordChanged();
                }
            }
        }
    }

public:
    void initState() override {
        State::initState();
        auto* pf = static_cast<const PasswordFieldWidget*>(widget());
        controller_ = pf->controller;
        cursor_pos_ = controller_->getPassword().length();
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;

        is_focused_ = pf->options.auto_focus;
        if (is_focused_) {
            g_focused_passwordfield = this;
        }

        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (g_focused_passwordfield != this || !is_focused_) return;
                auto* current_pf = static_cast<const PasswordFieldWidget*>(widget());
                if (!current_pf || current_pf->options.read_only) return;

                // Ignore control characters
                if (!text.empty() && text.length() == 1) {
                    char c = text[0];
                    if (c >= 0 && c < 32) return;
                    if (c == 127) return;
                }

                deleteSelection();
                std::string p = controller_->getPassword();
                p.insert(cursor_pos_, text);
                controller_->setPassword(p);
                cursor_pos_ += text.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;

                notifyPasswordChanged();
            });

            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (g_focused_passwordfield != this || !is_focused_) return;
                handleKey(key, mods);
            });

            mouse_down_conn_ = Platform::instance()->onMouseDown().connect([this](float gx, float gy, int /*btn*/) {
                if (!is_focused_) return;
                if (auto* ro = context().element()->findRenderObject()) {
                    Rect b = ro->globalBounds();
                    if (gx < b.x || gx > (b.x + b.width) || gy < b.y || gy > (b.y + b.height + 150.0f)) {
                        unfocus();
                    }
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* pf = static_cast<const PasswordFieldWidget*>(widget());
        controller_ = pf->controller;
    }

    void dispose() override {
        if (g_focused_passwordfield == this) g_focused_passwordfield = nullptr;
        if (Platform::instance()) {
            if (text_input_conn_) Platform::instance()->onTextInput().disconnect(text_input_conn_);
            if (key_down_conn_) Platform::instance()->onKeyDown().disconnect(key_down_conn_);
            if (mouse_down_conn_) Platform::instance()->onMouseDown().disconnect(mouse_down_conn_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* pf = static_cast<const PasswordFieldWidget*>(widget());

        // Caret Blink
        if (is_focused_ && Platform::instance()) {
            double cur = Platform::instance()->getTime();
            if (cur - last_blink_time_ > 0.53) {
                show_cursor_ = !show_cursor_;
                last_blink_time_ = cur;
            }
        } else {
            show_cursor_ = false;
        }

        auto field_box = std::make_shared<RenderPasswordFieldWidget>(
            controller_, pf->options, is_focused_, is_hovered_, show_cursor_,
            is_capslock_on_, cursor_pos_, selection_start_, selection_end_, hovered_btn_
        );

        auto detector = gestureDetector({
            .child = field_box,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Text,
            .on_tap_down = [this, pf](const TapDownDetails& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findPasswordFieldBox(ro)) {
                        int hit = box->getHitComponent(e.local_position.x, e.local_position.y);
                        if (hit == 1) { // Eye toggle
                            if (pf->options.hold_to_peek) {
                                controller_->setObscured(false);
                            } else {
                                controller_->toggleObscured();
                            }
                            setState([] {});
                            return;
                        } else if (hit == 2) { // Generate password
                            controller_->generateStrongPassword(16, true);
                            cursor_pos_ = controller_->getPassword().length();
                            selection_start_ = cursor_pos_;
                            selection_end_ = cursor_pos_;
                            notifyPasswordChanged();
                            return;
                        } else if (hit == 3) { // Clear
                            controller_->clear();
                            cursor_pos_ = 0;
                            selection_start_ = 0;
                            selection_end_ = 0;
                            notifyPasswordChanged();
                            return;
                        }

                        focus();
                        cursor_pos_ = box->getIndexAtCoordinate(e.local_position.x);
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                    }
                }
                setState([] {});
            },
            .on_tap_up = [this, pf](const TapUpDetails&) {
                if (pf->options.hold_to_peek && !controller_->isObscured()) {
                    controller_->setObscured(true);
                    setState([] {});
                }
            },
            .on_hover_enter = [this](const PointerEvent&) { setState([this] { is_hovered_ = true; }); },
            .on_hover_exit  = [this](const PointerEvent&) { setState([this] { is_hovered_ = false; hovered_btn_ = 0; }); },
            .on_hover_move = [this](const PointerEvent& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findPasswordFieldBox(ro)) {
                        int hit = box->getHitComponent(e.localPosition.x, e.localPosition.y);
                        if (hit != hovered_btn_) {
                            hovered_btn_ = hit;
                            setState([] {});
                        }
                    }
                }
            },
        });

        std::vector<WidgetPtr> main_col_items = {detector};

        // 1. Password Strength Meter Bar
        if (pf->options.show_strength_meter) {
            auto strength = controller_->calculateStrength();
            int score = 0;
            Color bar_color = 0xFF334155;
            std::string strength_label = "Strength: None";

            switch (strength) {
                case PasswordStrengthLevel::VeryWeak:
                    score = 1; bar_color = 0xFFEF4444; strength_label = "Strength: Very Weak"; break;
                case PasswordStrengthLevel::Weak:
                    score = 2; bar_color = 0xFFF97316; strength_label = "Strength: Weak"; break;
                case PasswordStrengthLevel::Medium:
                    score = 3; bar_color = 0xFFEAB308; strength_label = "Strength: Medium"; break;
                case PasswordStrengthLevel::Strong:
                    score = 4; bar_color = 0xFF22C55E; strength_label = "Strength: Strong"; break;
                case PasswordStrengthLevel::VeryStrong:
                    score = 4; bar_color = 0xFF10B981; strength_label = "Strength: Excellent 🔒"; break;
                default:
                    score = 0; bar_color = 0xFF334155; strength_label = "Strength: None"; break;
            }

            std::vector<WidgetPtr> segment_boxes;
            for (int i = 1; i <= 4; ++i) {
                auto seg = container();
                seg->color(i <= score ? bar_color : 0xFF334155)
                   .borderRadius(2.0f)
                   .height(4.0f)
                   .flexGrow(1.0f);
                segment_boxes.push_back(seg);
            }

            auto seg_row = row(segment_boxes);
            seg_row->gap(StyleValue::point(4.0f));

            auto st_txt = text({
                .text = strength_label,
                .color = bar_color,
                .font_size = 11.0f,
            });

            std::vector<WidgetPtr> meter_items = {seg_row, st_txt};
            auto meter_col = column(meter_items);
            meter_col->gap(StyleValue::point(4.0f));

            main_col_items.push_back(meter_col);
        }

        // 2. Rules Validation Checklist
        if (pf->options.show_rules_checklist) {
            const auto& rules = controller_->getRules();
            const std::string& pwd = controller_->getPassword();

            auto buildRuleRow = [](bool passed, const std::string& label) -> WidgetPtr {
                auto ic = text({
                    .text = passed ? "✓ " : "• ",
                    .color = passed ? 0xFF10B981 : 0xFF64748B,
                    .font_size = 11.5f,
                    .font_weight = FontWeight::Bold,
                });

                auto lbl = text({
                    .text = label,
                    .color = passed ? 0xFFE2E8F0 : 0xFF94A3B8,
                    .font_size = 11.5f,
                });

                std::vector<WidgetPtr> items = {ic, lbl};
                auto r = row(items);
                r->gap(StyleValue::point(4.0f)).alignItems(Align::Center);
                return r;
            };

            std::vector<WidgetPtr> rule_items;
            rule_items.push_back(buildRuleRow(rules.checkMinLength(pwd), "At least " + std::to_string(rules.min_length) + " characters"));
            if (rules.require_uppercase) rule_items.push_back(buildRuleRow(rules.checkUppercase(pwd), "Uppercase letter (A-Z)"));
            if (rules.require_lowercase) rule_items.push_back(buildRuleRow(rules.checkLowercase(pwd), "Lowercase letter (a-z)"));
            if (rules.require_digit)     rule_items.push_back(buildRuleRow(rules.checkDigit(pwd), "Number (0-9)"));
            if (rules.require_special)   rule_items.push_back(buildRuleRow(rules.checkSpecial(pwd), "Special symbol (!@#$%...)"));

            auto rules_col = column(rule_items);
            rules_col->gap(StyleValue::point(3.0f));

            auto rules_box = container(rules_col);
            rules_box->color(0xFF0F172A)
                     .borderRadius(6.0f)
                     .border(0xFF334155, 1.0f)
                     .paddingAll(8.0f);

            main_col_items.push_back(rules_box);
        }

        auto full_col = column(main_col_items);
        full_col->gap(StyleValue::point(6.0f));

        return full_col;
    }
};

std::unique_ptr<State> PasswordFieldWidget::createState() {
    return std::make_unique<PasswordFieldState>();
}

} // namespace enki
