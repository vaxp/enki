/// @file timeline.cpp
/// @brief Implementation of Advanced Timeline widget.

#include "enki/widgets/timeline.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkDashPathEffect.h>
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
#include <cmath>

namespace enki {

static sk_sp<skia::textlayout::FontCollection> getTimelineFontCollection() {
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
// Custom RenderBox for Timeline View
// ════════════════════════════════════════════════════════════════

class RenderTimelineBox : public RenderBox {
public:
    std::shared_ptr<TimelineController> controller;
    TimelineProps options;
    int hovered_item_index = -1;

    struct ItemLayoutInfo {
        Rect card_rect;
        Point node_center;
        
        float card_w = -1.0f;
        std::string title;
        std::string desc;
        std::string time;
        std::string details;
        std::string badge;
        std::string icon;
        bool is_expanded = false;
        Color title_color = 0;
        Color desc_color = 0;
        Color time_color = 0;

        std::unique_ptr<skia::textlayout::Paragraph> p_title;
        std::unique_ptr<skia::textlayout::Paragraph> p_time;
        std::unique_ptr<skia::textlayout::Paragraph> p_desc;
        std::unique_ptr<skia::textlayout::Paragraph> p_det;
        std::unique_ptr<skia::textlayout::Paragraph> p_badge;
        std::unique_ptr<skia::textlayout::Paragraph> p_icon;
        std::unique_ptr<skia::textlayout::Paragraph> p_num;
    };
    mutable std::vector<ItemLayoutInfo> layout_cache_;

    RenderTimelineBox(std::shared_ptr<TimelineController> ctrl, TimelineProps opt)
        : controller(std::move(ctrl)), options(std::move(opt)) {
        updateFlexboxStyle();
    }

    Color getStatusColor(TimelineItemStatus status, Color custom_color) const {
        if (custom_color != 0) return custom_color;
        switch (status) {
            case TimelineItemStatus::Completed: return options.completed_color;
            case TimelineItemStatus::Active:    return options.active_color;
            case TimelineItemStatus::Pending:   return options.pending_color;
            case TimelineItemStatus::Warning:   return options.warning_color;
            case TimelineItemStatus::Failed:    return options.failed_color;
        }
        return options.pending_color;
    }

    float computeItemHeight(const TimelineItem& item, float card_w, size_t index) const {
        if (layout_cache_.size() <= index) {
            layout_cache_.resize(index + 1);
        }
        auto& cache = layout_cache_[index];
        auto fc = getTimelineFontCollection();
        if (!fc) return 80.0f;

        bool dirty = cache.card_w != card_w || cache.title != item.title ||
                     cache.desc != item.description || cache.time != item.timestamp ||
                     cache.details != item.details || cache.badge != item.badge_text ||
                     cache.icon != item.icon || cache.is_expanded != item.is_expanded ||
                     cache.title_color != options.title_color ||
                     cache.desc_color != options.desc_color ||
                     cache.time_color != options.timestamp_color;

        if (dirty) {
            cache.card_w = card_w;
            cache.title = item.title;
            cache.desc = item.description;
            cache.time = item.timestamp;
            cache.details = item.details;
            cache.badge = item.badge_text;
            cache.icon = item.icon;
            cache.is_expanded = item.is_expanded;
            cache.title_color = options.title_color;
            cache.desc_color = options.desc_color;
            cache.time_color = options.timestamp_color;

            skia::textlayout::ParagraphStyle p_style;
            
            // Badge
            if (!item.badge_text.empty()) {
                auto b = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(10.5f);
                t.setColor(static_cast<SkColor>(item.badge_fg != 0 ? item.badge_fg : 0xFFFFFFFF));
                b->pushStyle(t);
                b->addText(item.badge_text.c_str(), item.badge_text.length());
                cache.p_badge = b->Build();
                cache.p_badge->layout(100.0f);
            } else {
                cache.p_badge = nullptr;
            }
            float badge_w = cache.p_badge ? cache.p_badge->getMaxIntrinsicWidth() + 14.0f : 0.0f;

            // Title
            if (!item.title.empty()) {
                auto b_title = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t_title;
                t_title.setFontSize(13.0f);
                t_title.setColor(static_cast<SkColor>(options.title_color));
                b_title->pushStyle(t_title);
                b_title->addText(item.title.c_str(), item.title.length());
                cache.p_title = b_title->Build();
                cache.p_title->layout(std::max(40.0f, card_w - 24.0f - badge_w));
            } else {
                cache.p_title = nullptr;
            }

            // Time
            if (!item.timestamp.empty()) {
                auto b_time = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(11.0f);
                t.setColor(static_cast<SkColor>(options.timestamp_color));
                b_time->pushStyle(t);
                b_time->addText(item.timestamp.c_str(), item.timestamp.length());
                cache.p_time = b_time->Build();
                cache.p_time->layout(card_w - 24.0f);
            } else {
                cache.p_time = nullptr;
            }

            // Desc
            if (!item.description.empty()) {
                auto b = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(12.0f);
                t.setColor(static_cast<SkColor>(options.desc_color));
                b->pushStyle(t);
                b->addText(item.description.c_str(), item.description.length());
                cache.p_desc = b->Build();
                cache.p_desc->layout(card_w - 24.0f);
            } else {
                cache.p_desc = nullptr;
            }

            // Details
            if (item.is_expanded && !item.details.empty()) {
                auto b = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(11.5f);
                t.setColor(0xFF94A3B8); // hardcoded in original
                b->pushStyle(t);
                b->addText(item.details.c_str(), item.details.length());
                cache.p_det = b->Build();
                cache.p_det->layout(std::max(40.0f, card_w - 40.0f));
            } else {
                cache.p_det = nullptr;
            }
            
            // Icon
            if (!item.icon.empty()) {
                skia::textlayout::ParagraphStyle ic_style;
                ic_style.setTextAlign(skia::textlayout::TextAlign::kCenter);
                auto b = skia::textlayout::ParagraphBuilder::make(ic_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(options.node_size * 0.55f);
                b->pushStyle(t);
                b->addText(item.icon.c_str(), item.icon.length());
                cache.p_icon = b->Build();
                cache.p_icon->layout(options.node_size);
            } else {
                cache.p_icon = nullptr;
            }

            // Number
            cache.p_num = nullptr;
            if (item.icon.empty() && (options.is_stepper || item.node_shape == TimelineNodeShape::Number)) {
                std::string num_str = std::to_string(index + 1);
                skia::textlayout::ParagraphStyle n_style;
                n_style.setTextAlign(skia::textlayout::TextAlign::kCenter);
                auto b = skia::textlayout::ParagraphBuilder::make(n_style, fc);
                skia::textlayout::TextStyle t;
                t.setFontSize(11.0f);
                t.setColor(item.status == TimelineItemStatus::Pending ? static_cast<SkColor>(options.pending_color) : 0xFF0F172A);
                b->pushStyle(t);
                b->addText(num_str.c_str(), num_str.length());
                cache.p_num = b->Build();
                cache.p_num->layout(options.node_size);
            }
        }

        float h = 12.0f; // top padding
        if (cache.p_title) h += cache.p_title->getHeight() + 4.0f;
        if (cache.p_time) h += cache.p_time->getHeight() + 4.0f;
        if (cache.p_desc) h += cache.p_desc->getHeight() + 6.0f;
        if (cache.p_det) h += cache.p_det->getHeight() + 20.0f;
        h += 12.0f; // bottom padding
        
        return std::max(h, options.node_size + 16.0f);
    }

    float computeTotalHeight() const {
        const auto& items = controller->getItems();
        if (items.empty()) return 60.0f;

        if (layout_cache_.size() != items.size()) {
            layout_cache_.resize(items.size());
        }

        if (options.orientation == TimelineOrientation::Horizontal) {
            float max_card_h = 100.0f;
            for (size_t i = 0; i < items.size(); ++i) {
                max_card_h = std::max(max_card_h, computeItemHeight(items[i], options.card_width, i));
            }
            return options.node_size + 20.0f + max_card_h + 20.0f;
        }

        float total_h = 0.0f;
        float card_w = (options.alignment == TimelineAlignment::Alternate) ? options.card_width : 480.0f;

        for (size_t i = 0; i < items.size(); ++i) {
            float item_h = computeItemHeight(items[i], card_w, i);
            total_h += item_h;
            if (i + 1 < items.size()) total_h += options.item_spacing;
        }

        return total_h + 20.0f;
    }

    void updateFlexboxStyle() {
        FlexboxStyle st;
        st.height = StyleValue::point(computeTotalHeight());
        st.width = StyleValue::percent(100.0f);
        applyFlexboxStyle(anuNode(), st);
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        auto fc = getTimelineFontCollection();
        if (!fc) return;

        const auto& items = controller->getItems();
        if (items.empty()) return;

        if (layout_cache_.size() != items.size()) {
            layout_cache_.resize(items.size());
        }

        if (options.orientation == TimelineOrientation::Horizontal) {
            paintHorizontal(sk_canvas, ctx.offset, fc);
        } else if (options.alignment == TimelineAlignment::Alternate) {
            paintVerticalAlternate(sk_canvas, ctx.offset, fc);
        } else {
            paintVerticalStandard(sk_canvas, ctx.offset, fc);
        }
    }

    sk_sp<SkPathEffect> getDashEffect() const {
        static auto effect = [](){
            const SkScalar intervals[] = {6.0f, 4.0f};
            return SkDashPathEffect::Make(intervals, 2, 0.0f);
        }();
        return effect;
    }

    void paintHorizontal(SkCanvas* sk_canvas, Point offset, sk_sp<skia::textlayout::FontCollection> fc) {
        const auto& items = controller->getItems();
        size_t n = items.size();
        float step_x = size_.width / static_cast<float>(n);
        float line_y = offset.y + options.node_size * 0.8f + 10.0f;

        SkPaint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);
        if (options.line_style == TimelineLineStyle::Dashed) {
            line_paint.setPathEffect(getDashEffect());
        }

        // 1. Connecting Track Lines
        for (size_t i = 0; i + 1 < n; ++i) {
            float x1 = offset.x + (static_cast<float>(i) + 0.5f) * step_x + options.node_size * 0.5f;
            float x2 = offset.x + (static_cast<float>(i + 1) + 0.5f) * step_x - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            sk_canvas->drawLine(x1, line_y, x2, line_y, line_paint);
        }

        // 2. Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            float cx = offset.x + (static_cast<float>(i) + 0.5f) * step_x;
            float cy = line_y;

            layout_cache_[i].node_center = Point{cx, cy};

            paintNode(sk_canvas, cx, cy, item, i, fc);

            // Content Card Below Node
            float card_w = std::min(step_x - 12.0f, options.card_width);
            float card_x = cx - card_w * 0.5f;
            float card_y = cy + options.node_size * 0.5f + 12.0f;
            float card_h = computeItemHeight(item, card_w, i);

            layout_cache_[i].card_rect = Rect{card_x, card_y, card_w, card_h};
            paintCard(sk_canvas, card_x, card_y, card_w, card_h, item, (hovered_item_index == static_cast<int>(i)), fc, i);
        }
    }

    void paintVerticalStandard(SkCanvas* sk_canvas, Point offset, sk_sp<skia::textlayout::FontCollection> fc) {
        const auto& items = controller->getItems();
        size_t n = items.size();

        float node_x = offset.x + options.node_size * 0.5f + 16.0f;
        float cur_y = offset.y + 10.0f;
        float card_x = node_x + options.node_size * 0.5f + 16.0f;
        float card_w = std::max(200.0f, size_.width - card_x - 16.0f);

        // Precompute centers
        for (size_t i = 0; i < n; ++i) {
            float card_h = computeItemHeight(items[i], card_w, i);
            float node_y = cur_y + options.node_size * 0.5f + 6.0f;

            layout_cache_[i].node_center = Point{node_x, node_y};
            layout_cache_[i].card_rect = Rect{card_x, cur_y, card_w, card_h};

            cur_y += card_h + options.item_spacing;
        }

        SkPaint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);
        if (options.line_style == TimelineLineStyle::Dashed) {
            line_paint.setPathEffect(getDashEffect());
        }

        // 1. Draw Connecting Track Lines
        for (size_t i = 0; i + 1 < n; ++i) {
            float y1 = layout_cache_[i].node_center.y + options.node_size * 0.5f;
            float y2 = layout_cache_[i + 1].node_center.y - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            sk_canvas->drawLine(node_x, y1, node_x, y2, line_paint);
        }

        // 2. Draw Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            const auto& cache = layout_cache_[i];

            paintNode(sk_canvas, cache.node_center.x, cache.node_center.y, item, i, fc);
            paintCard(sk_canvas, cache.card_rect.x, cache.card_rect.y, cache.card_rect.width, cache.card_rect.height,
                      item, (hovered_item_index == static_cast<int>(i)), fc, i);
        }
    }

    void paintVerticalAlternate(SkCanvas* sk_canvas, Point offset, sk_sp<skia::textlayout::FontCollection> fc) {
        const auto& items = controller->getItems();
        size_t n = items.size();

        float center_x = offset.x + size_.width * 0.5f;
        float cur_y = offset.y + 10.0f;
        float card_w = std::min(options.card_width, size_.width * 0.45f);

        for (size_t i = 0; i < n; ++i) {
            float card_h = computeItemHeight(items[i], card_w, i);
            float node_y = cur_y + options.node_size * 0.5f + 6.0f;

            bool is_left = (i % 2 == 0);
            float card_x = is_left ? (center_x - options.node_size * 0.5f - 16.0f - card_w)
                                   : (center_x + options.node_size * 0.5f + 16.0f);

            layout_cache_[i].node_center = Point{center_x, node_y};
            layout_cache_[i].card_rect = Rect{card_x, cur_y, card_w, card_h};

            cur_y += card_h + options.item_spacing;
        }

        SkPaint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);
        if (options.line_style == TimelineLineStyle::Dashed) {
            line_paint.setPathEffect(getDashEffect());
        }

        // 1. Track line
        for (size_t i = 0; i + 1 < n; ++i) {
            float y1 = layout_cache_[i].node_center.y + options.node_size * 0.5f;
            float y2 = layout_cache_[i + 1].node_center.y - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            sk_canvas->drawLine(center_x, y1, center_x, y2, line_paint);
        }

        // 2. Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            const auto& cache = layout_cache_[i];

            paintNode(sk_canvas, cache.node_center.x, cache.node_center.y, item, i, fc);
            paintCard(sk_canvas, cache.card_rect.x, cache.card_rect.y, cache.card_rect.width, cache.card_rect.height,
                      item, (hovered_item_index == static_cast<int>(i)), fc, i);
        }
    }

    void paintNode(SkCanvas* sk_canvas, float cx, float cy, const TimelineItem& item, size_t index,
                   sk_sp<skia::textlayout::FontCollection> fc) {
        float r = options.node_size * 0.5f;
        Color node_col = getStatusColor(item.status, item.custom_node_color);

        // Active Glowing Outer Ring
        if (item.status == TimelineItemStatus::Active) {
            SkPaint glow_paint;
            glow_paint.setAntiAlias(true);
            glow_paint.setColor(0x3338BDF8);
            sk_canvas->drawCircle(cx, cy, r + 5.0f, glow_paint);
        }

        // Base Circle / Square
        SkPaint node_paint;
        node_paint.setAntiAlias(true);
        node_paint.setColor(node_col);

        if (item.node_shape == TimelineNodeShape::Square) {
            SkRRect sq;
            sq.setRectXY(SkRect::MakeXYWH(cx - r, cy - r, options.node_size, options.node_size), 4.0f, 4.0f);
            sk_canvas->drawRRect(sq, node_paint);
        } else {
            if (item.status == TimelineItemStatus::Pending) {
                node_paint.setStyle(SkPaint::kStroke_Style);
                node_paint.setStrokeWidth(2.0f);
                sk_canvas->drawCircle(cx, cy, r - 1.0f, node_paint);
            } else {
                sk_canvas->drawCircle(cx, cy, r, node_paint);
            }
        }

        // Glyphs & Content inside Node
        if (item.status == TimelineItemStatus::Completed) {
            // Emerald checkmark ✓
            SkPaint chk_paint;
            chk_paint.setAntiAlias(true);
            chk_paint.setColor(0xFFFFFFFF);
            chk_paint.setStyle(SkPaint::kStroke_Style);
            chk_paint.setStrokeWidth(2.0f);
            SkPath p;
            p.moveTo(cx - 4.5f, cy);
            p.lineTo(cx - 1.0f, cy + 3.5f);
            p.lineTo(cx + 4.5f, cy - 3.0f);
            sk_canvas->drawPath(p, chk_paint);
        } else if (item.status == TimelineItemStatus::Failed) {
            // Red cross ✕
            SkPaint x_paint;
            x_paint.setAntiAlias(true);
            x_paint.setColor(0xFFFFFFFF);
            x_paint.setStrokeWidth(2.0f);
            float sz = 3.5f;
            sk_canvas->drawLine(cx - sz, cy - sz, cx + sz, cy + sz, x_paint);
            sk_canvas->drawLine(cx + sz, cy - sz, cx - sz, cy + sz, x_paint);
        } else {
            const auto& cache = layout_cache_[index];
            if (!item.icon.empty() && cache.p_icon) {
                cache.p_icon->paint(sk_canvas, cx - options.node_size * 0.5f, cy - cache.p_icon->getHeight() * 0.5f);
            } else if (cache.p_num) {
                cache.p_num->paint(sk_canvas, cx - options.node_size * 0.5f, cy - cache.p_num->getHeight() * 0.5f);
            }
        }
    }

    void paintCard(SkCanvas* sk_canvas, float x, float y, float w, float h, const TimelineItem& item,
                   bool is_hov, sk_sp<skia::textlayout::FontCollection> fc, size_t index) {
        SkRRect rrect;
        rrect.setRectXY(SkRect::MakeXYWH(x, y, w, h), options.card_border_radius, options.card_border_radius);

        // Card Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(is_hov ? 0xFF24324D : options.card_bg_color);
        sk_canvas->drawRRect(rrect, bg_paint);

        // Card Border
        SkPaint b_paint;
        b_paint.setAntiAlias(true);
        b_paint.setStyle(SkPaint::kStroke_Style);
        b_paint.setStrokeWidth(1.0f);
        b_paint.setColor(is_hov ? 0xFF475569 : options.card_border_color);
        sk_canvas->drawRRect(rrect, b_paint);

        float cur_y = y + 12.0f;
        float pad_x = x + 12.0f;
        const auto& cache = layout_cache_[index];

        // 1. Title Row
        if (cache.p_title) {
            cache.p_title->paint(sk_canvas, pad_x, cur_y);
            
            // Paint Badge on the right
            if (cache.p_badge) {
                float bw = cache.p_badge->getMaxIntrinsicWidth() + 10.0f;
                float bh = cache.p_badge->getHeight() + 2.0f;
                float bx = x + w - 12.0f - bw;
                float by = cur_y;

                SkRRect bd_rect;
                bd_rect.setRectXY(SkRect::MakeXYWH(bx, by, bw, bh), 3.0f, 3.0f);
                SkPaint bd_paint;
                bd_paint.setColor(item.badge_bg != 0 ? item.badge_bg : 0x2E38BDF8);
                sk_canvas->drawRRect(bd_rect, bd_paint);
                cache.p_badge->paint(sk_canvas, bx + 5.0f, by + 1.0f);
            }

            cur_y += cache.p_title->getHeight() + 4.0f;
        }

        // 2. Timestamp
        if (cache.p_time) {
            cache.p_time->paint(sk_canvas, pad_x, cur_y);
            cur_y += cache.p_time->getHeight() + 4.0f;
        }

        // 3. Description
        if (cache.p_desc) {
            cache.p_desc->paint(sk_canvas, pad_x, cur_y);
            cur_y += cache.p_desc->getHeight() + 6.0f;
        }

        // 4. Expandable Details Box
        if (cache.p_det) {
            float det_h = cache.p_det->getHeight() + 12.0f;
            float avail_w = w - 24.0f;
            SkRRect det_box;
            det_box.setRectXY(SkRect::MakeXYWH(pad_x, cur_y, avail_w, det_h), 4.0f, 4.0f);
            SkPaint det_bg;
            det_bg.setColor(options.details_bg_color);
            sk_canvas->drawRRect(det_box, det_bg);

            cache.p_det->paint(sk_canvas, pad_x + 8.0f, cur_y + 6.0f);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    int getHitItemIndex(float local_x, float local_y) const {
        for (size_t i = 0; i < layout_cache_.size(); ++i) {
            const auto& cache = layout_cache_[i];
            if (local_x >= cache.card_rect.x && local_x <= cache.card_rect.x + cache.card_rect.width &&
                local_y >= cache.card_rect.y && local_y <= cache.card_rect.y + cache.card_rect.height) {
                return static_cast<int>(i);
            }
            float dx = local_x - cache.node_center.x;
            float dy = local_y - cache.node_center.y;
            if ((dx * dx + dy * dy) <= (options.node_size * options.node_size)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};

static RenderTimelineBox* findTimelineBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderTimelineBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findTimelineBox(child)) return found;
    }
    return nullptr;
}

class RenderTimelineWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<TimelineController> controller;
    TimelineProps options;
    int hovered_item_index;

    RenderTimelineWidget(std::shared_ptr<TimelineController> ctrl, TimelineProps opt, int h_idx)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), hovered_item_index(h_idx) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderTimelineBox>(controller, options);
        ro->hovered_item_index = hovered_item_index;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderTimelineBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->hovered_item_index = hovered_item_index;
            ro->updateFlexboxStyle();
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderTimelineWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Timeline State Implementation
// ════════════════════════════════════════════════════════════════

class TimelineState : public State {
private:
    std::shared_ptr<TimelineController> controller_;
    int hovered_item_index_ = -1;

public:
    void initState() override {
        State::initState();
        auto* tl = static_cast<const TimelineWidget*>(widget());
        controller_ = tl->props.controller;
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* tl = static_cast<const TimelineWidget*>(widget());
        controller_ = tl->props.controller;
    }

    WidgetPtr build(BuildContext&) override {
        auto* tl = static_cast<const TimelineWidget*>(widget());

        auto timeline_render = std::make_shared<RenderTimelineWidget>(
            controller_, tl->props, hovered_item_index_
        );

        return gestureDetector({
            .child = timeline_render,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = (hovered_item_index_ >= 0) ? SystemCursor::Pointer : SystemCursor::Default,
            .on_tap_down = [this, tl](const TapDownDetails& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findTimelineBox(ro)) {
                        int hit = box->getHitItemIndex(e.local_position.x, e.local_position.y);
                        if (hit >= 0 && static_cast<size_t>(hit) < controller_->getItems().size()) {
                            const auto& item = controller_->getItems()[hit];

                            if (tl->props.is_stepper) {
                                controller_->setActiveStep(hit);
                                if (tl->props.on_step_changed) {
                                    tl->props.on_step_changed(hit);
                                }
                            } else {
                                if (!item.details.empty()) {
                                    controller_->toggleExpand(item.id);
                                    if (tl->props.on_item_expanded) {
                                        tl->props.on_item_expanded(item.id, !item.is_expanded);
                                    }
                                }
                            }

                            if (tl->props.on_item_tap) {
                                tl->props.on_item_tap(item);
                            }
                            setState([] {});
                        }
                    }
                }
            },
            .on_hover_exit = [this](const PointerEvent&) {
                setState([this] { hovered_item_index_ = -1; });
            },
            .on_hover_move = [this](const PointerEvent& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findTimelineBox(ro)) {
                        int hit = box->getHitItemIndex(e.localPosition.x, e.localPosition.y);
                        if (hit != hovered_item_index_) {
                            hovered_item_index_ = hit;
                            setState([] {});
                        }
                    }
                }
            },
        });
    }
};

std::unique_ptr<State> TimelineWidget::createState() {
    return std::make_unique<TimelineState>();
}

} // namespace enki
