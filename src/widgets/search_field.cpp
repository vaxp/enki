/// @file search_field.cpp
/// @brief Implementation of Advanced SearchField widget with live suggestions, history, and command palette.

#include "enki/widgets/search_field.hpp"
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

class SearchFieldState;
static SearchFieldState* g_focused_searchfield = nullptr;

static sk_sp<skia::textlayout::FontCollection> getSearchFieldFontCollection() {
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

// Case-insensitive substring finder for highlighting matches
static std::vector<std::pair<size_t, size_t>> findMatchRanges(std::string_view text, std::string_view query) {
    std::vector<std::pair<size_t, size_t>> ranges;
    if (text.empty() || query.empty()) return ranges;

    std::string lower_text(text);
    std::string lower_query(query);
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

    size_t pos = 0;
    while ((pos = lower_text.find(lower_query, pos)) != std::string::npos) {
        ranges.emplace_back(pos, pos + query.length());
        pos += query.length();
    }
    return ranges;
}

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for SearchField Bar
// ════════════════════════════════════════════════════════════════

class RenderSearchFieldBox : public RenderBox {
public:
    std::shared_ptr<SearchFieldController> controller;
    SearchFieldProps props;
    bool is_focused = false;
    bool is_hovered = false;
    bool show_cursor = false;
    size_t cursor_pos = 0;
    size_t selection_start = 0;
    size_t selection_end = 0;
    bool hovered_clear = false;
    float spinner_angle = 0.0f;

    std::unique_ptr<skia::textlayout::Paragraph> text_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> icon_paragraph_;
    std::unique_ptr<skia::textlayout::Paragraph> badge_paragraph_;

    RenderSearchFieldBox(std::shared_ptr<SearchFieldController> ctrl, SearchFieldProps opt)
        : controller(std::move(ctrl)), props(std::move(opt)) {
        
        float h = 42.0f;
        if (props.size == SearchFieldSize::Small) h = 34.0f;
        else if (props.size == SearchFieldSize::Large) h = 52.0f;

        FlexboxStyle st;
        st.height = StyleValue::point(h);
        applyFlexboxStyle(anuNode(), st);
    }

    void layoutParagraphs() {
        auto fc = getSearchFieldFontCollection();
        if (!fc) return;

        float font_sz = props.style.font_size > 0 ? props.style.font_size : 14.0f;
        if (props.size == SearchFieldSize::Small && props.style.font_size <= 0) font_sz = 12.5f;
        if (props.size == SearchFieldSize::Large && props.style.font_size <= 0) font_sz = 16.0f;

        skia::textlayout::ParagraphStyle p_style;
        p_style.setTextAlign(skia::textlayout::TextAlign::kLeft);

        // 1. Search Icon
        if (props.show_search_icon) {
            auto b_icon = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_ic;
            t_ic.setFontSize(font_sz * 1.1f);
            t_ic.setColor(static_cast<SkColor>(props.icon_color));
            b_icon->pushStyle(t_ic);
            b_icon->addText("🔍", 4);
            icon_paragraph_ = b_icon->Build();
            icon_paragraph_->layout(30.0f);
        } else {
            icon_paragraph_.reset();
        }

        // 2. Main Query / Placeholder
        auto b_main = skia::textlayout::ParagraphBuilder::make(p_style, fc);
        skia::textlayout::TextStyle t_main;
        t_main.setFontSize(font_sz);

        if (!props.style.font_family.empty()) {
            std::vector<SkString> font_fams = {SkString(props.style.font_family.c_str())};
            t_main.setFontFamilies(font_fams);
        }

        const std::string& q = controller->getQuery();
        if (q.empty() && !is_focused) {
            t_main.setColor(static_cast<SkColor>(props.placeholder_color));
            b_main->pushStyle(t_main);
            b_main->addText(props.placeholder.c_str(), props.placeholder.length());
        } else {
            t_main.setColor(static_cast<SkColor>(props.style.color != 0 ? props.style.color : 0xFFF1F5F9));
            b_main->pushStyle(t_main);
            b_main->addText(q.c_str(), q.length());
        }
        text_paragraph_ = b_main->Build();
        text_paragraph_->layout(size_.width > 0 ? size_.width : 300.0f);

        // 3. Shortcut Badge
        if (props.show_shortcut_badge && !props.shortcut_hint.empty() && q.empty() && !is_focused) {
            auto b_badge = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_bg;
            t_bg.setFontSize(11.0f);
            t_bg.setColor(static_cast<SkColor>(props.badge_text_color));
            b_badge->pushStyle(t_bg);
            b_badge->addText(props.shortcut_hint.c_str(), props.shortcut_hint.length());
            badge_paragraph_ = b_badge->Build();
            badge_paragraph_->layout(80.0f);
        } else {
            badge_paragraph_.reset();
        }
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        layoutParagraphs();

        float radius = props.border_radius;
        if (props.variant == SearchFieldVariant::Pill) {
            radius = size_.height * 0.5f;
        }

        SkRect bounds = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(bounds, radius, radius);

        // 1. Draw Field Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        if (props.variant == SearchFieldVariant::Outlined) {
            bg_paint.setColor(0x00000000);
        } else {
            bg_paint.setColor(props.background_color);
        }
        sk_canvas->drawRRect(rrect, bg_paint);

        // Clip Content
        sk_canvas->save();
        sk_canvas->clipRRect(rrect, true);

        // 2. Draw Search Icon
        float content_x = ctx.offset.x + props.padding.left;
        float cy = ctx.offset.y + size_.height * 0.5f;

        if (icon_paragraph_) {
            float icon_y = cy - icon_paragraph_->getHeight() * 0.5f;
            icon_paragraph_->paint(sk_canvas, content_x, icon_y);
            content_x += icon_paragraph_->getMaxIntrinsicWidth() + 8.0f;
        }

        // 3. Draw Main Query / Placeholder & Selection & Caret
        const std::string& q = controller->getQuery();
        if (text_paragraph_) {
            float text_y = cy - text_paragraph_->getHeight() * 0.5f;

            // Draw selection rect
            if (is_focused && selection_start != selection_end) {
                size_t s = std::min(selection_start, selection_end);
                size_t e = std::max(selection_start, selection_end);
                auto rects = text_paragraph_->getRectsForRange(s, e,
                    skia::textlayout::RectHeightStyle::kTight,
                    skia::textlayout::RectWidthStyle::kTight);

                SkPaint sel_paint;
                sel_paint.setColor(static_cast<SkColor>(props.selection_color));
                sk_canvas->save();
                sk_canvas->translate(content_x, text_y);
                for (const auto& r : rects) {
                    sk_canvas->drawRect(r.rect, sel_paint);
                }
                sk_canvas->restore();
            }

            text_paragraph_->paint(sk_canvas, content_x, text_y);

            // Draw Blinking Caret
            if (is_focused && show_cursor && !props.read_only && selection_start == selection_end) {
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
                cur_paint.setColor(static_cast<SkColor>(props.cursor_color));
                sk_canvas->drawRect(SkRect::MakeXYWH(content_x + c_x, text_y, 2.0f, c_h), cur_paint);
            }
        }

        // 4. Draw Trailing Items (Clear Button / Loading Spinner / Shortcut Badge)
        float trail_right = ctx.offset.x + size_.width - props.padding.right;

        // Clear Button [ ✕ ]
        if (props.show_clear_button && !q.empty() && !props.read_only) {
            float btn_r = 10.0f;
            float btn_cx = trail_right - btn_r;
            float btn_cy = cy;

            if (hovered_clear) {
                SkPaint hov_paint;
                hov_paint.setAntiAlias(true);
                hov_paint.setColor(0x3394A3B8);
                sk_canvas->drawCircle(btn_cx, btn_cy, btn_r + 2.0f, hov_paint);
            }

            SkPaint x_paint;
            x_paint.setAntiAlias(true);
            x_paint.setColor(hovered_clear ? 0xFFFFFFFF : 0xFF94A3B8);
            x_paint.setStrokeWidth(1.6f);
            float sz = 4.0f;
            sk_canvas->drawLine(btn_cx - sz, btn_cy - sz, btn_cx + sz, btn_cy + sz, x_paint);
            sk_canvas->drawLine(btn_cx + sz, btn_cy - sz, btn_cx - sz, btn_cy + sz, x_paint);

            trail_right -= (btn_r * 2.0f + 8.0f);
        }

        // Loading Spinner
        if (controller->isLoading()) {
            float sp_r = 8.0f;
            float sp_cx = trail_right - sp_r;
            float sp_cy = cy;

            SkPaint sp_paint;
            sp_paint.setAntiAlias(true);
            sp_paint.setStyle(SkPaint::kStroke_Style);
            sp_paint.setStrokeWidth(2.0f);
            sp_paint.setColor(props.focus_border_color);

            SkRect sp_rect = SkRect::MakeXYWH(sp_cx - sp_r, sp_cy - sp_r, sp_r * 2.0f, sp_r * 2.0f);
            sk_canvas->drawArc(sp_rect, spinner_angle, 270.0f, false, sp_paint);

            trail_right -= (sp_r * 2.0f + 8.0f);
        }

        // Shortcut Badge
        if (badge_paragraph_) {
            float b_w = badge_paragraph_->getMaxIntrinsicWidth() + 10.0f;
            float b_h = badge_paragraph_->getHeight() + 4.0f;
            float b_x = trail_right - b_w;
            float b_y = cy - b_h * 0.5f;

            SkRRect b_rrect;
            b_rrect.setRectXY(SkRect::MakeXYWH(b_x, b_y, b_w, b_h), 4.0f, 4.0f);
            SkPaint b_bg;
            b_bg.setAntiAlias(true);
            b_bg.setColor(props.badge_bg_color);
            sk_canvas->drawRRect(b_rrect, b_bg);

            badge_paragraph_->paint(sk_canvas, b_x + 5.0f, b_y + 2.0f);
        }

        sk_canvas->restore();

        // 5. Outer Border (Focus Glow / Hover / Normal)
        SkPaint border_paint;
        border_paint.setAntiAlias(true);
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(is_focused ? 1.5f : 1.0f);
        border_paint.setColor(is_focused ? props.focus_border_color : (is_hovered ? 0xFF475569 : props.border_color));
        sk_canvas->drawRRect(rrect, border_paint);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    bool isHitClearButton(float local_x, float local_y) const {
        if (!props.show_clear_button || controller->getQuery().empty()) return false;
        float btn_r = 12.0f;
        float btn_cx = size_.width - props.padding.right - 10.0f;
        float btn_cy = size_.height * 0.5f;
        float dx = local_x - btn_cx;
        float dy = local_y - btn_cy;
        return (dx * dx + dy * dy) <= (btn_r * btn_r);
    }

    size_t getIndexAtCoordinate(float local_x) {
        layoutParagraphs();
        if (!text_paragraph_) return 0;

        float content_x = props.padding.left;
        if (icon_paragraph_) content_x += icon_paragraph_->getMaxIntrinsicWidth() + 8.0f;

        float target_x = std::max(0.0f, local_x - content_x);
        auto pos = text_paragraph_->getGlyphPositionAtCoordinate(target_x, 5.0f);
        return std::min(static_cast<size_t>(pos.position), controller->getQuery().length());
    }
};

static RenderSearchFieldBox* findSearchFieldBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderSearchFieldBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findSearchFieldBox(child)) return found;
    }
    return nullptr;
}

class RenderSearchFieldWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<SearchFieldController> controller;
    SearchFieldProps props;
    bool is_focused;
    bool is_hovered;
    bool show_cursor;
    size_t cursor_pos;
    size_t selection_start;
    size_t selection_end;
    bool hovered_clear;
    float spinner_angle;

    RenderSearchFieldWidget(std::shared_ptr<SearchFieldController> ctrl, SearchFieldProps opt,
                            bool focused, bool hovered, bool cursor, size_t cur_pos,
                            size_t sel_s, size_t sel_e, bool h_clr, float spin_ang)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          props(std::move(opt)), is_focused(focused), is_hovered(hovered),
          show_cursor(cursor), cursor_pos(cur_pos), selection_start(sel_s),
          selection_end(sel_e), hovered_clear(h_clr), spinner_angle(spin_ang) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderSearchFieldBox>(controller, props);
        ro->is_focused = is_focused;
        ro->is_hovered = is_hovered;
        ro->show_cursor = show_cursor;
        ro->cursor_pos = cursor_pos;
        ro->selection_start = selection_start;
        ro->selection_end = selection_end;
        ro->hovered_clear = hovered_clear;
        ro->spinner_angle = spinner_angle;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderSearchFieldBox*>(&renderObject)) {
            ro->controller = controller;
            ro->props = props;
            ro->is_focused = is_focused;
            ro->is_hovered = is_hovered;
            ro->show_cursor = show_cursor;
            ro->cursor_pos = cursor_pos;
            ro->selection_start = selection_start;
            ro->selection_end = selection_end;
            ro->hovered_clear = hovered_clear;
            ro->spinner_angle = spinner_angle;
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderSearchFieldWidget"; }
};

// ════════════════════════════════════════════════════════════════
// SearchField State Implementation
// ════════════════════════════════════════════════════════════════

class SearchFieldState : public State {
private:
    std::shared_ptr<SearchFieldController> controller_;
    bool is_focused_ = false;
    bool is_hovered_ = false;
    bool show_cursor_ = true;
    double last_blink_time_ = 0.0;

    size_t cursor_pos_ = 0;
    size_t selection_start_ = 0;
    size_t selection_end_ = 0;
    bool hovered_clear_ = false;

    // Debounce & Spinner
    double last_input_time_ = 0.0;
    bool debounce_pending_ = false;
    float spinner_angle_ = 0.0f;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;
    SlotId mouse_down_conn_ = 0;

    void focus() {
        if (g_focused_searchfield && g_focused_searchfield != this) {
            g_focused_searchfield->unfocus();
        }
        g_focused_searchfield = this;
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
        if (g_focused_searchfield == this) {
            g_focused_searchfield = nullptr;
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
        std::string q = controller_->getQuery();
        q.erase(s, e - s);
        controller_->setQuery(q);
        cursor_pos_ = s;
        selection_start_ = s;
        selection_end_ = s;
    }

    void triggerQueryUpdate() {
        auto* sf = static_cast<const SearchFieldWidget*>(widget());
        if (!sf) return;

        resetBlink();
        if (Platform::instance()) last_input_time_ = Platform::instance()->getTime();
        debounce_pending_ = true;

        if (sf->props.on_changed) {
            sf->props.on_changed(controller_->getQuery());
        }

        // Live suggestions fetch
        if (sf->props.suggestions_provider) {
            controller_->setSuggestions(sf->props.suggestions_provider(controller_->getQuery()));
        }

        setState([] {});
    }

    void handleKey(int key, int mods) {
        auto* sf = static_cast<const SearchFieldWidget*>(widget());
        if (!sf) return;

        bool shift = (mods & 1) != 0;
        bool ctrl  = (mods & 2) != 0 || (mods & 4) != 0;

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
        const int KEY_ESCAPE    = 0xff1b;

        // Clipboard & Selection Shortcuts
        bool is_ctrl_c = (ctrl && (key == 'c' || key == 'C' || key == 0x63 || key == 0x43 || key == 54)) || (key == 0x03);
        bool is_ctrl_v = (ctrl && (key == 'v' || key == 'V' || key == 0x76 || key == 0x56 || key == 55)) || (key == 0x16);
        bool is_ctrl_x = (ctrl && (key == 'x' || key == 'X' || key == 0x78 || key == 0x58 || key == 53)) || (key == 0x18);
        bool is_ctrl_a = (ctrl && (key == 'a' || key == 'A' || key == 0x61 || key == 0x41 || key == 38)) || (key == 0x01);

        // Global Shortcut Ctrl+K to Focus
        if (ctrl && (key == 'k' || key == 'K' || key == 0x6b || key == 0x4b || key == 45)) {
            focus();
            cursor_pos_ = controller_->getQuery().length();
            selection_start_ = 0;
            selection_end_ = cursor_pos_;
            return;
        }

        const auto& suggestions = controller_->getSuggestions();

        if (key == KEY_DOWN && !suggestions.empty()) {
            int cur = controller_->getActiveSuggestionIndex();
            cur = (cur + 1) % static_cast<int>(suggestions.size());
            controller_->setActiveSuggestionIndex(cur);
            setState([] {});
        } else if (key == KEY_UP && !suggestions.empty()) {
            int cur = controller_->getActiveSuggestionIndex();
            cur = (cur <= 0) ? static_cast<int>(suggestions.size()) - 1 : cur - 1;
            controller_->setActiveSuggestionIndex(cur);
            setState([] {});
        } else if ((key == KEY_RETURN || key == 0xff8d)) {
            int active_idx = controller_->getActiveSuggestionIndex();
            if (active_idx >= 0 && active_idx < static_cast<int>(suggestions.size())) {
                const auto& item = suggestions[active_idx];
                controller_->addRecentSearch(item.title);
                controller_->setQuery(item.title);
                cursor_pos_ = item.title.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
                if (sf->props.on_suggestion_selected) sf->props.on_suggestion_selected(item);
                if (sf->props.on_submitted) sf->props.on_submitted(item.title);
            } else {
                controller_->addRecentSearch(controller_->getQuery());
                if (sf->props.on_submitted) sf->props.on_submitted(controller_->getQuery());
            }
            setState([] {});
        } else if (key == KEY_TAB && !suggestions.empty()) {
            int active_idx = controller_->getActiveSuggestionIndex();
            if (active_idx >= 0 && active_idx < static_cast<int>(suggestions.size())) {
                std::string fill_text = suggestions[active_idx].title;
                controller_->setQuery(fill_text);
                cursor_pos_ = fill_text.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
                triggerQueryUpdate();
            }
        } else if (key == KEY_ESCAPE) {
            controller_->clear();
            cursor_pos_ = 0;
            selection_start_ = 0;
            selection_end_ = 0;
            unfocus();
        } else if ((key == KEY_BACKSPACE || key == 0x08 || key == 0x7f) && !sf->props.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ > 0) {
                std::string q = controller_->getQuery();
                q.erase(cursor_pos_ - 1, 1);
                controller_->setQuery(q);
                cursor_pos_--;
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;
            }
            triggerQueryUpdate();
        } else if ((key == KEY_DELETE || key == 0xff9f) && !sf->props.read_only) {
            if (selection_start_ != selection_end_) {
                deleteSelection();
            } else if (cursor_pos_ < controller_->getQuery().length()) {
                std::string q = controller_->getQuery();
                q.erase(cursor_pos_, 1);
                controller_->setQuery(q);
            }
            triggerQueryUpdate();
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
            if (cursor_pos_ < controller_->getQuery().length()) cursor_pos_++;
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
            cursor_pos_ = controller_->getQuery().length();
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
            selection_end_ = controller_->getQuery().length();
            cursor_pos_ = selection_end_;
            setState([] {});
        } else if (is_ctrl_c && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = controller_->getQuery().substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
        } else if (is_ctrl_x && !sf->props.read_only && selection_start_ != selection_end_) {
            size_t s = std::min(selection_start_, selection_end_);
            size_t e = std::max(selection_start_, selection_end_);
            std::string sub = controller_->getQuery().substr(s, e - s);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(sub);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(sub);
            }
            deleteSelection();
            triggerQueryUpdate();
        } else if (is_ctrl_v && !sf->props.read_only) {
            if (Platform::instance()) {
                std::string paste = Platform::instance()->getClipboardText();
                if (!paste.empty()) {
                    deleteSelection();
                    std::string q = controller_->getQuery();
                    q.insert(cursor_pos_, paste);
                    controller_->setQuery(q);
                    cursor_pos_ += paste.length();
                    selection_start_ = cursor_pos_;
                    selection_end_ = cursor_pos_;
                    triggerQueryUpdate();
                }
            }
        }
    }

public:
    void initState() override {
        State::initState();
        auto* sf = static_cast<const SearchFieldWidget*>(widget());
        controller_ = sf->props.controller;
        cursor_pos_ = controller_->getQuery().length();
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;

        is_focused_ = sf->props.auto_focus;
        if (is_focused_) {
            g_focused_searchfield = this;
        }

        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (g_focused_searchfield != this || !is_focused_) return;
                auto* current_sf = static_cast<const SearchFieldWidget*>(widget());
                if (!current_sf || current_sf->props.read_only) return;

                // Ignore control characters
                if (!text.empty() && text.length() == 1) {
                    char c = text[0];
                    if (c >= 0 && c < 32) return;
                    if (c == 127) return;
                }

                deleteSelection();
                std::string q = controller_->getQuery();
                q.insert(cursor_pos_, text);
                controller_->setQuery(q);
                cursor_pos_ += text.length();
                selection_start_ = cursor_pos_;
                selection_end_ = cursor_pos_;

                triggerQueryUpdate();
            });

            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (g_focused_searchfield != this || !is_focused_) return;
                handleKey(key, mods);
            });

            mouse_down_conn_ = Platform::instance()->onMouseDown().connect([this](float gx, float gy, int /*btn*/) {
                if (!is_focused_) return;
                if (auto* ro = context().element()->findRenderObject()) {
                    Rect b = ro->globalBounds();
                    auto* current_sf = static_cast<const SearchFieldWidget*>(widget());
                    bool has_sug = current_sf && current_sf->props.show_suggestions &&
                        (!controller_->getSuggestions().empty() || (!controller_->getRecentSearches().empty() && controller_->getQuery().empty()));
                    float extra_h = has_sug ? 300.0f : 0.0f;
                    if (gx < b.x || gx > (b.x + b.width) || gy < b.y || gy > (b.y + b.height + extra_h)) {
                        unfocus();
                    }
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* sf = static_cast<const SearchFieldWidget*>(widget());
        controller_ = sf->props.controller;
    }

    void dispose() override {
        if (g_focused_searchfield == this) g_focused_searchfield = nullptr;
        if (Platform::instance()) {
            if (text_input_conn_) Platform::instance()->onTextInput().disconnect(text_input_conn_);
            if (key_down_conn_) Platform::instance()->onKeyDown().disconnect(key_down_conn_);
            if (mouse_down_conn_) Platform::instance()->onMouseDown().disconnect(mouse_down_conn_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* sf = static_cast<const SearchFieldWidget*>(widget());

        // Process Debounce Timer
        if (debounce_pending_ && Platform::instance()) {
            double cur = Platform::instance()->getTime();
            if (cur - last_input_time_ >= (sf->props.debounce_ms / 1000.0)) {
                debounce_pending_ = false;
                if (sf->props.on_search) {
                    sf->props.on_search(controller_->getQuery());
                }
            }
        }

        // Caret Blink
        if (is_focused_ && Platform::instance()) {
            double cur = Platform::instance()->getTime();
            if (cur - last_blink_time_ > 0.53) {
                show_cursor_ = !show_cursor_;
                last_blink_time_ = cur;
            }
            if (controller_->isLoading()) {
                spinner_angle_ += 12.0f;
                if (spinner_angle_ >= 360.0f) spinner_angle_ -= 360.0f;
            }
        } else {
            show_cursor_ = false;
        }

        auto search_box = std::make_shared<RenderSearchFieldWidget>(
            controller_, sf->props, is_focused_, is_hovered_, show_cursor_,
            cursor_pos_, selection_start_, selection_end_, hovered_clear_, spinner_angle_
        );

        auto detector = gestureDetector({
            .child = search_box,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Text,
            .on_tap_down = [this](const TapDownDetails& e) {
                focus();

                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findSearchFieldBox(ro)) {
                        if (box->isHitClearButton(e.local_position.x, e.local_position.y)) {
                            controller_->clear();
                            cursor_pos_ = 0;
                            selection_start_ = 0;
                            selection_end_ = 0;
                            triggerQueryUpdate();
                            return;
                        }

                        cursor_pos_ = box->getIndexAtCoordinate(e.local_position.x);
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                    }
                }
                setState([] {});
            },
            .on_hover_enter = [this](const PointerEvent&) { setState([this] { is_hovered_ = true; }); },
            .on_hover_exit  = [this](const PointerEvent&) { setState([this] { is_hovered_ = false; hovered_clear_ = false; }); },
            .on_hover_move = [this](const PointerEvent& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findSearchFieldBox(ro)) {
                        bool clr = box->isHitClearButton(e.localPosition.x, e.localPosition.y);
                        if (clr != hovered_clear_) {
                            hovered_clear_ = clr;
                            setState([] {});
                        }
                    }
                }
            },
        });

        // Build Suggestions Dropdown Overlay if active and visible
        const auto& suggestions = controller_->getSuggestions();
        const auto& recent = controller_->getRecentSearches();

        bool has_suggestions = sf->props.show_suggestions && is_focused_ && (!suggestions.empty() || (!recent.empty() && controller_->getQuery().empty()));

        if (!has_suggestions) {
            return detector;
        }

        std::vector<WidgetPtr> pop_items;

        // 1. Suggestions List
        if (!suggestions.empty()) {
            std::string last_cat = "";
            int limit = std::min(static_cast<int>(suggestions.size()), sf->props.max_visible_suggestions);

            for (int i = 0; i < limit; ++i) {
                const auto& item = suggestions[i];
                bool is_active = (controller_->getActiveSuggestionIndex() == i);

                // Category Header if changed
                if (!item.category.empty() && item.category != last_cat) {
                    last_cat = item.category;
                    auto cat_txt = text({
                        .text = item.category,
                        .color = 0xFF64748B,
                        .font_size = 11.0f,
                        .font_weight = FontWeight::Bold,
                });
                    auto cat_box = container(cat_txt);
                    cat_box->paddingSymmetric(4.0f, 8.0f);
                    pop_items.push_back(cat_box);
                }

                // Suggestion Item Row
                auto ic = text({
                    .text = item.icon_char.empty() ? "🔍" : item.icon_char,
                    .font_size = 13.5f,
                });

                Color t_color = is_active ? sf->props.focus_border_color : 0xFFF1F5F9;
                auto t = text({
                    .text = item.title,
                    .color = t_color,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                });

                std::vector<WidgetPtr> row_content = {ic, t};

                if (!item.subtitle.empty()) {
                    auto sub = text({
                        .text = item.subtitle,
                        .color = 0xFF94A3B8,
                        .font_size = 11.5f,
                    });
                    row_content.push_back(sub);
                }

                auto row_widget = row(row_content);
                row_widget->gap(StyleValue::point(8.0f))
                          .alignItems(Align::Center);

                if (!item.badge.empty()) {
                    auto bg_t = text({
                        .text = item.badge,
                        .color = 0xFF94A3B8,
                        .font_size = 10.5f,
                    });
                    auto bg_box = container(bg_t);
                    bg_box->color(0xFF1E293B)
                          .borderRadius(4.0f)
                          .paddingSymmetric(2.0f, 6.0f);

                    std::vector<WidgetPtr> full_row_items = {row_widget, bg_box};
                    auto full_row = row(full_row_items);
                    full_row->justifyContent(Justify::SpaceBetween)
                            .alignItems(Align::Center);
                    row_widget = full_row;
                }

                auto item_box = container(row_widget);
                item_box->color(is_active ? sf->props.item_hover_color : 0x00000000)
                        .borderRadius(6.0f)
                        .paddingSymmetric(6.0f, 10.0f);

                auto item_detector = gestureDetector({
                    .child = item_box,
                    .hit_test_behavior = HitTestBehavior::Opaque,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = [this, item, sf] {
                        controller_->addRecentSearch(item.title);
                        controller_->setQuery(item.title);
                        cursor_pos_ = item.title.length();
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                        if (sf->props.on_suggestion_selected) sf->props.on_suggestion_selected(item);
                        if (sf->props.on_submitted) sf->props.on_submitted(item.title);
                        setState([] {});
                    },
                });
                pop_items.push_back(item_detector);
            }
        }
        // 2. Recent Searches if Query is Empty
        else if (!recent.empty() && controller_->getQuery().empty()) {
            auto rec_header = text({
                .text = "RECENT SEARCHES",
                .color = 0xFF64748B,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            });
            auto rec_header_box = container(rec_header);
            rec_header_box->paddingSymmetric(4.0f, 8.0f);
            pop_items.push_back(rec_header_box);

            int rec_limit = std::min(static_cast<int>(recent.size()), 5);
            for (int i = 0; i < rec_limit; ++i) {
                std::string r_query = recent[i];

                auto ic = text({
                    .text = "🕒",
                    .font_size = 12.5f,
                });

                auto r_txt = text({
                    .text = r_query,
                    .color = 0xFFE2E8F0,
                    .font_size = 12.5f,
                });

                std::vector<WidgetPtr> r_items = {ic, r_txt};
                auto r_row = row(r_items);
                r_row->gap(StyleValue::point(8.0f))
                     .alignItems(Align::Center);

                auto r_box = container(r_row);
                r_box->borderRadius(6.0f)
                     .paddingSymmetric(6.0f, 10.0f);

                auto r_det = gestureDetector({
                    .child = r_box,
                    .hit_test_behavior = HitTestBehavior::Opaque,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = [this, r_query, sf] {
                        controller_->setQuery(r_query);
                        cursor_pos_ = r_query.length();
                        selection_start_ = cursor_pos_;
                        selection_end_ = cursor_pos_;
                        triggerQueryUpdate();
                        if (sf->props.on_submitted) sf->props.on_submitted(r_query);
                    },
                });
                pop_items.push_back(r_det);
            }
        }

        auto pop_col = column(pop_items);
        pop_col->gap(StyleValue::point(2.0f));

        auto popup_container = container(pop_col);
        popup_container->color(sf->props.popup_bg_color)
                       .borderRadius(sf->props.border_radius)
                       .border(sf->props.border_color, 1.0f)
                       .paddingAll(6.0f);

        std::vector<WidgetPtr> combined_items = {detector, popup_container};
        auto combined_col = column(combined_items);
        combined_col->gap(StyleValue::point(6.0f));

        return combined_col;
    }
};

std::unique_ptr<State> SearchFieldWidget::createState() {
    return std::make_unique<SearchFieldState>();
}

} // namespace enki
