/// @file placeholder.cpp
/// @brief Implementation of Advanced Placeholder & Skeleton Shimmer for ENKI Framework.

#include "enki/widgets/placeholder.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderPlaceholder
// ════════════════════════════════════════════════════════════════

class RenderPlaceholder : public RenderBox {
public:
    PlaceholderProps options;
    float anim_phase_ = 0.0f;

    explicit RenderPlaceholder(PlaceholderProps opt) : options(std::move(opt)) {
        ANUNodeStyleSetWidth(anu_node_, options.width);
        ANUNodeStyleSetHeight(anu_node_, options.height);
    }

    void updateOptions(const PlaceholderProps& new_opts) {
        if (options.width != new_opts.width || options.height != new_opts.height) {
            ANUNodeStyleSetWidth(anu_node_, new_opts.width);
            ANUNodeStyleSetHeight(anu_node_, new_opts.height);
            markNeedsLayout();
        }
        options = new_opts;
        markNeedsPaint();
    }

    void tick(double /*now*/) override {
        if (options.style == PlaceholderStyle::Skeleton && options.animated_shimmer) {
            anim_phase_ += 0.03f;
            if (anim_phase_ > 2.0f) anim_phase_ -= 2.0f;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        Rect bounds = Rect::fromLTWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        BorderRadius rad = BorderRadius::circular(options.corner_radius);

        if (options.style == PlaceholderStyle::Skeleton) {
            // ── Skeleton Loader ───────────────────────────────────
            Paint base_paint;
            base_paint.setColor(options.background_color != 0x22334155 ? options.background_color : 0xFF1E293B);
            ctx.canvas.drawRRect(bounds, rad, base_paint);

            if (options.animated_shimmer) {
                float start_x = bounds.x + (anim_phase_ - 1.0f) * bounds.width;
                float end_x   = start_x + bounds.width * 0.75f;

                Paint shimmer_paint;
                shimmer_paint.setShader(Gradient::linear(
                    Point(start_x, bounds.y),
                    Point(end_x, bounds.y),
                    {0x0038BDF8, options.shimmer_color, 0x0038BDF8}
                ));
                ctx.canvas.drawRRect(bounds, rad, shimmer_paint);
            }
            return;
        }

        if (options.style == PlaceholderStyle::MediaSlot) {
            // ── Dashed Media Slot / Drop Zone ─────────────────────
            Paint bg_paint;
            bg_paint.setColor(options.background_color);
            ctx.canvas.drawRRect(bounds, rad, bg_paint);

            // Border
            Paint border_paint;
            border_paint.setColor(options.stroke_color);
            border_paint.setStyle(PaintStyle::Stroke);
            border_paint.setStrokeWidth(options.stroke_width);
            ctx.canvas.drawRRect(bounds, rad, border_paint);

            // Draw Icon & Label
            float center_x = bounds.x + bounds.width / 2.0f;
            float center_y = bounds.y + bounds.height / 2.0f;

            if (!options.icon.empty()) {
                Paint icon_paint;
                icon_paint.setColor(0xFF38BDF8);
                float icon_w = ctx.canvas.measureText(options.icon, 22.0f);
                Point icon_pt(center_x - icon_w / 2.0f, center_y - 8.0f);
                ctx.canvas.drawText(options.icon, icon_pt, icon_paint, 22.0f);
            }

            std::string slot_label = options.label.empty() ? "Click or Drag Media Here" : options.label;
            Paint text_paint;
            text_paint.setColor(options.text_color);
            float text_w = ctx.canvas.measureText(slot_label, 12.0f, nullptr, true);
            Point text_pt(center_x - text_w / 2.0f, center_y + 18.0f);
            ctx.canvas.drawText(slot_label, text_pt, text_paint, 12.0f, nullptr, true);
            return;
        }

        // ── Blueprint / Crosshair Wireframe ───────────────────────
        // 1. Background Fill
        Paint bg_paint;
        bg_paint.setColor(options.background_color);
        ctx.canvas.drawRRect(bounds, rad, bg_paint);

        // 2. Diagonal Crosshair Lines
        Paint line_paint;
        line_paint.setColor(options.crosshair_color);
        line_paint.setStyle(PaintStyle::Stroke);
        line_paint.setStrokeWidth(1.0f);

        ctx.canvas.drawLine(Point(bounds.x, bounds.y),
                            Point(bounds.x + bounds.width, bounds.y + bounds.height), line_paint);
        ctx.canvas.drawLine(Point(bounds.x + bounds.width, bounds.y),
                            Point(bounds.x, bounds.y + bounds.height), line_paint);

        // 3. Surrounding Border
        Paint border_paint;
        border_paint.setColor(options.stroke_color);
        border_paint.setStyle(PaintStyle::Stroke);
        border_paint.setStrokeWidth(options.stroke_width);
        ctx.canvas.drawRRect(bounds, rad, border_paint);

        // 4. Center Dimension Badge
        if (options.show_dimensions || !options.label.empty()) {
            std::ostringstream ss;
            if (!options.label.empty()) {
                ss << options.label << " (";
            }
            ss << static_cast<int>(size_.width) << " × " << static_cast<int>(size_.height) << " px";
            if (!options.label.empty()) {
                ss << ")";
            }

            std::string badge_str = ss.str();
            float txt_w = ctx.canvas.measureText(badge_str, 11.0f, nullptr, true);
            float badge_w = txt_w + 16.0f;
            float badge_h = 24.0f;

            float badge_x = bounds.x + (bounds.width - badge_w) / 2.0f;
            float badge_y = bounds.y + (bounds.height - badge_h) / 2.0f;
            Rect badge_rect = Rect::fromLTWH(badge_x, badge_y, badge_w, badge_h);

            // Badge Background
            Paint badge_bg;
            badge_bg.setColor(options.badge_bg_color);
            ctx.canvas.drawRRect(badge_rect, BorderRadius::circular(4.0f), badge_bg);

            // Badge Border
            Paint badge_stroke;
            badge_stroke.setColor(options.stroke_color);
            badge_stroke.setStyle(PaintStyle::Stroke);
            badge_stroke.setStrokeWidth(1.0f);
            ctx.canvas.drawRRect(badge_rect, BorderRadius::circular(4.0f), badge_stroke);

            // Badge Text
            Paint txt_paint;
            txt_paint.setColor(options.text_color);
            Point txt_pos(badge_x + 8.0f, badge_y + 16.0f);
            ctx.canvas.drawText(badge_str, txt_pos, txt_paint, 11.0f, nullptr, true);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// Widget Element & Factory
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Placeholder::createRenderObject(BuildContext&) {
    return std::make_unique<RenderPlaceholder>(options);
}

void Placeholder::updateRenderObject(BuildContext&, RenderObject& ro) {
    auto& r = static_cast<RenderPlaceholder&>(ro);
    r.updateOptions(options);
}

// ── Pre-composed Template Helpers ─────────────────────────────────

WidgetPtr placeholderCardSkeleton(float w) {
    auto avatar = placeholderSkeleton(44.0f, 44.0f, 22.0f);

    auto title_line = placeholderSkeleton(w * 0.45f, 14.0f, 4.0f);
    auto sub_line   = placeholderSkeleton(w * 0.30f, 10.0f, 4.0f);

    std::vector<WidgetPtr> h_lines = {title_line, sub_line};
    auto h_col = column(h_lines);
    h_col->gap(StyleValue::point(6.0f));

    std::vector<WidgetPtr> head_items = {avatar, h_col};
    auto head_row = row(head_items);
    head_row->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

    auto body_line1 = placeholderSkeleton(w - 40.0f, 12.0f, 4.0f);
    auto body_line2 = placeholderSkeleton(w - 70.0f, 12.0f, 4.0f);

    std::vector<WidgetPtr> card_items = {head_row, body_line1, body_line2};
    auto card_col = column(card_items);
    card_col->gap(StyleValue::point(14.0f));

    auto card_box = container(card_col);
    card_box->color(0xFF0F172A)
            .border(0xFF334155, 1.0f)
            .borderRadius(12.0f)
            .paddingAll(16.0f)
            .width(w);

    return card_box;
}

WidgetPtr placeholderListSkeleton(int rows, float w) {
    std::vector<WidgetPtr> row_widgets;

    for (int i = 0; i < rows; ++i) {
        auto icon_box = placeholderSkeleton(36.0f, 36.0f, 8.0f);
        auto t_line   = placeholderSkeleton(w * 0.5f, 12.0f, 4.0f);
        auto s_line   = placeholderSkeleton(w * 0.28f, 9.0f, 4.0f);

        std::vector<WidgetPtr> t_lines = {t_line, s_line};
        auto t_col = column(t_lines);
        t_col->gap(StyleValue::point(4.0f));

        auto badge_box = placeholderSkeleton(50.0f, 20.0f, 10.0f);

        std::vector<WidgetPtr> r_items = {icon_box, t_col, badge_box};
        auto r_row = row(r_items);
        r_row->justifyContent(Justify::SpaceBetween)
             .alignItems(Align::Center)
             .width(StyleValue::percent(100.0f));

        row_widgets.push_back(r_row);

        if (i < rows - 1) {
            auto div = container();
            div->color(0xFF1E293B).height(1.0f).width(StyleValue::percent(100.0f));
            row_widgets.push_back(div);
        }
    }

    auto list_col = column(row_widgets);
    list_col->gap(StyleValue::point(12.0f));

    auto list_box = container(list_col);
    list_box->color(0xFF0F172A)
            .border(0xFF334155, 1.0f)
            .borderRadius(12.0f)
            .paddingAll(16.0f)
            .width(w);

    return list_box;
}

} // namespace enki
