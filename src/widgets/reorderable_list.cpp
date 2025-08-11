/// @file reorderable_list.cpp
/// @brief 600+ FPS Direct Skia floating drag-and-drop ReorderableList for ENKI Framework.

#include "enki/widgets/reorderable_list.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"

#include <vector>
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ReorderableDragHandle
// ════════════════════════════════════════════════════════════════

WidgetPtr ReorderableDragHandle::build(BuildContext&) {
    if (child) return child;
    auto grip = text("⋮⋮");
    grip->fontSize(16.0f).bold().color(0xFF94A3B8);
    auto grip_box = container(grip);
    grip_box->paddingSymmetric(4.0f, 6.0f).borderRadius(4.0f);
    return grip_box;
}

// ════════════════════════════════════════════════════════════════
// RenderReorderableList
// ════════════════════════════════════════════════════════════════

class RenderReorderableList : public RenderBox {
public:
    ReorderableListOptions options;

    int   dragging_index = -1;
    int   target_index   = -1;
    float drag_offset_y  = 0.0f;
    float start_local_y  = 0.0f;

    // Yoga-computed tops for each child (cached after syncLayout)
    std::vector<float> yoga_tops_;

    explicit RenderReorderableList(ReorderableListOptions opts)
        : options(std::move(opts))
    {
        // Column flex layout so Yoga sizes children normally
        ANUNodeStyleSetFlexDirection(anu_node_, ANUFlexDirectionColumn);
        ANUNodeStyleSetWidth(anu_node_, options.width);
        ANUNodeStyleSetGap(anu_node_, ANUGutterRow, options.gap);
    }

    void updateOptions(const ReorderableListOptions& opts) {
        options = opts;
        ANUNodeStyleSetWidth(anu_node_, options.width);
        ANUNodeStyleSetGap(anu_node_, ANUGutterRow, options.gap);
        markNeedsLayout();
    }

    // Override syncLayout to cache child tops after Yoga computes them
    void syncLayout() override {
        RenderBox::syncLayout();
        // Now children have their offset() populated
        yoga_tops_.resize(children_.size());
        for (size_t i = 0; i < children_.size(); ++i) {
            yoga_tops_[i] = children_[i] ? children_[i]->offset().y : static_cast<float>(i) * (options.item_height + options.gap);
        }
    }

    bool hitTestSelf(Point pt) const override {
        return pt.x >= 0 && pt.x <= size_.width &&
               pt.y >= 0 && pt.y <= size_.height;
    }

    SystemCursor cursor() const override {
        return dragging_index != -1 ? SystemCursor::Move : SystemCursor::Default;
    }

    void handlePointerDown(const PointerEvent& e) override {
        if (e.button != MouseButton::Left || yoga_tops_.empty()) return;
        int count = static_cast<int>(children_.size());
        for (int k = 0; k < count; ++k) {
            float top = yoga_tops_[k];
            float bot = top + options.item_height;
            if (e.localPosition.y >= top && e.localPosition.y <= bot) {
                dragging_index = k;
                target_index   = k;
                drag_offset_y  = 0.0f;
                start_local_y  = e.localPosition.y;
                markNeedsPaint();
                break;
            }
        }
    }

    void handlePointerMove(const PointerEvent& e) override {
        if (dragging_index < 0) return;
        drag_offset_y = e.localPosition.y - start_local_y;
        float stride  = options.item_height + options.gap;
        int delta     = static_cast<int>(std::round(drag_offset_y / stride));
        int new_tgt   = std::clamp(dragging_index + delta, 0, static_cast<int>(children_.size()) - 1);
        if (new_tgt != target_index) {
            target_index = new_tgt;
        }
        markNeedsPaint();
    }

    void handlePointerUp(const PointerEvent&) override {
        if (dragging_index < 0) return;
        int old_idx    = dragging_index;
        int new_idx    = target_index;
        dragging_index = -1;
        target_index   = -1;
        drag_offset_y  = 0.0f;
        markNeedsPaint();
        if (old_idx != new_idx && options.on_reorder)
            options.on_reorder(old_idx, new_idx);
    }

    void paint(PaintContext& ctx) override {
        int   count  = static_cast<int>(children_.size());
        float card_w = size_.width > 0 ? size_.width : options.width;
        float card_h = options.item_height;
        float stride = card_h + options.gap;
        bool  active = (dragging_index >= 0 && dragging_index < count);

        // Ensure cache is valid
        while ((int)yoga_tops_.size() < count) {
            yoga_tops_.push_back((float)yoga_tops_.size() * stride);
        }

        // ── Pass 1: Non-dragged items (displaced) ────────────────
        for (int k = 0; k < count; ++k) {
            if (k == dragging_index) {
                // Ghost slot at original position
                float ghost_y = yoga_tops_[k];
                Rect  slot    = Rect::fromLTWH(ctx.offset.x, ctx.offset.y + ghost_y, card_w, card_h);
                Paint bp; bp.setColor(0x1538BDF8);
                ctx.canvas.drawRRect(slot, BorderRadius::circular(10.0f), bp);
                Paint bp2; bp2.setColor(0x5538BDF8); bp2.setStyle(PaintStyle::Stroke); bp2.setStrokeWidth(1.5f);
                ctx.canvas.drawRRect(slot, BorderRadius::circular(10.0f), bp2);
                continue;
            }

            // Compute displacement
            float disp_y = yoga_tops_[k];
            if (active && dragging_index != target_index) {
                if (dragging_index < target_index) {
                    if (k > dragging_index && k <= target_index) disp_y -= stride;
                } else {
                    if (k >= target_index && k < dragging_index) disp_y += stride;
                }
            }

            if (children_[k]) {
                PaintContext child_ctx{ctx.canvas, Point(ctx.offset.x, ctx.offset.y + disp_y), ctx.clip_rect, ctx.opacity};
                children_[k]->paint(child_ctx);
            }
        }

        // ── Pass 2: Drop indicator line ──────────────────────────
        if (active && options.show_drop_indicator && target_index != dragging_index) {
            float tgt_base = yoga_tops_[target_index];
            if (dragging_index < target_index) tgt_base -= stride;
            float line_y = ctx.offset.y + tgt_base +
                           (target_index > dragging_index ? (card_h + options.gap * 0.5f - 1.5f)
                                                          : (-options.gap * 0.5f - 1.5f));
            Rect  dl = Rect::fromLTWH(ctx.offset.x, line_y, card_w, 3.0f);
            Paint dp; dp.setColor(options.drop_indicator_color);
            ctx.canvas.drawRRect(dl, BorderRadius::circular(1.5f), dp);
        }

        // ── Pass 3: Floating dragged card on top ─────────────────
        if (active && children_[dragging_index]) {
            float base_y  = yoga_tops_[dragging_index];
            float float_y = std::clamp(base_y + drag_offset_y, 0.0f, size_.height - card_h);
            Rect  card    = Rect::fromLTWH(ctx.offset.x, ctx.offset.y + float_y, card_w, card_h);

            // Shadow
            Rect  sh = Rect::fromLTWH(ctx.offset.x + 2, ctx.offset.y + float_y + 8, card_w - 4, card_h);
            Paint sp; sp.setColor(0x88000000);
            sp.setImageFilter(ImageFilter::blur(12.0f, 12.0f));
            ctx.canvas.drawRRect(sh, BorderRadius::circular(10.0f), sp);

            // Card background
            Paint bg; bg.setColor(0xFF1E293B);
            ctx.canvas.drawRRect(card, BorderRadius::circular(10.0f), bg);

            // Glow border
            Paint gb; gb.setColor(options.drop_indicator_color);
            gb.setStyle(PaintStyle::Stroke); gb.setStrokeWidth(2.0f);
            ctx.canvas.drawRRect(card, BorderRadius::circular(10.0f), gb);

            // Child content
            PaintContext fc{ctx.canvas, Point(ctx.offset.x, ctx.offset.y + float_y), ctx.clip_rect, ctx.opacity};
            children_[dragging_index]->paint(fc);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// ReorderableList Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> ReorderableList::createRenderObject(BuildContext&) {
    return std::make_unique<RenderReorderableList>(options);
}

void ReorderableList::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rrl = dynamic_cast<RenderReorderableList*>(&renderObject)) {
        rrl->updateOptions(options);
    }
}

} // namespace enki
