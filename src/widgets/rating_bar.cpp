#include "enki/widgets/rating_bar.hpp"
#include "enki/rendering/canvas.hpp"
#include <layout_engine/Anu.h>
#include <include/core/SkPath.h>
#include <include/core/SkCanvas.h>
#include <algorithm>
#include <cmath>

namespace enki {

class RenderRatingBar : public RenderBox {
public:
    RatingBarProps props_;
    float hover_rating_{-1.0f};

    explicit RenderRatingBar(RatingBarProps props)
        : props_(std::move(props)) {
        applyStyle();
    }

    void update(const RatingBarProps& new_props) {
        props_ = new_props;
        applyStyle();
        markNeedsPaint();
    }

    void applyStyle() {
        if (!anu_node_) return;
        float total_w = props_.max_rating * props_.item_size + (props_.max_rating - 1) * props_.item_spacing;
        ANUNodeStyleSetWidth(anu_node_, total_w);
        ANUNodeStyleSetHeight(anu_node_, props_.item_size);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0.0f && localPoint.x <= size_.width &&
               localPoint.y >= 0.0f && localPoint.y <= size_.height;
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return props_.is_read_only ? SystemCursor::Default : SystemCursor::Pointer;
    }

    float calculateRatingFromX(float local_x) const {
        if (props_.max_rating <= 0 || size_.width <= 0.0f) return 0.0f;
        float stride = props_.item_size + props_.item_spacing;
        if (stride <= 0.0f) return 0.0f;

        int star_idx = static_cast<int>(local_x / stride);
        if (star_idx < 0) return 0.0f;
        if (star_idx >= props_.max_rating) return static_cast<float>(props_.max_rating);

        float star_local_x = local_x - static_cast<float>(star_idx) * stride;
        float frac = std::clamp(star_local_x / props_.item_size, 0.0f, 1.0f);

        if (props_.allow_half) {
            frac = (frac <= 0.5f) ? 0.5f : 1.0f;
        } else {
            frac = 1.0f;
        }

        return static_cast<float>(star_idx) + frac;
    }

    void handlePointerMove(const PointerEvent& e) override {
        if (props_.is_read_only) return;
        float r = calculateRatingFromX(e.localPosition.x);
        if (r != hover_rating_) {
            hover_rating_ = r;
            if (props_.on_hover) props_.on_hover(hover_rating_);
            markNeedsPaint();
        }
    }

    void handlePointerExit(const PointerEvent&) override {
        if (hover_rating_ >= 0.0f) {
            hover_rating_ = -1.0f;
            if (props_.on_hover) props_.on_hover(-1.0f);
            markNeedsPaint();
        }
    }

    void handlePointerDown(const PointerEvent& e) override {
        if (props_.is_read_only) return;
        float r = calculateRatingFromX(e.localPosition.x);
        props_.rating = r;
        if (props_.on_rating_changed) {
            props_.on_rating_changed(r);
        }
        markNeedsPaint();
    }

    static SkPath makeStarPath(float cx, float cy, float R) {
        SkPath path;
        float r = R * 0.40f;
        constexpr float kPi = 3.14159265358979323846f;
        for (int i = 0; i < 10; ++i) {
            float angle = static_cast<float>(i) * kPi / 5.0f - kPi / 2.0f;
            float rad = (i % 2 == 0) ? R : r;
            float x = cx + rad * std::cos(angle);
            float y = cy + rad * std::sin(angle);
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        path.close();
        return path;
    }

    void paint(PaintContext& ctx) override {
        auto& canvas = ctx.canvas;
        auto* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
        if (!sk_canvas) return;

        sk_canvas->save();
        sk_canvas->translate(ctx.offset.x, ctx.offset.y);

        float effective_rating = (hover_rating_ >= 0.0f) ? hover_rating_ : props_.rating;
        float stride = props_.item_size + props_.item_spacing;
        float radius = props_.item_size * 0.5f;

        SkPaint inactive_sk;
        inactive_sk.setStyle(SkPaint::kFill_Style);
        inactive_sk.setAntiAlias(true);
        inactive_sk.setColor(props_.inactive_color);

        SkPaint active_sk;
        active_sk.setStyle(SkPaint::kFill_Style);
        active_sk.setAntiAlias(true);
        active_sk.setColor(props_.active_color);

        for (int i = 0; i < props_.max_rating; ++i) {
            float star_x = static_cast<float>(i) * stride;
            float cx = star_x + radius;
            float cy = radius;

            SkPath star_path = makeStarPath(cx, cy, radius * 0.95f);

            // Always draw inactive background star
            sk_canvas->drawPath(star_path, inactive_sk);

            float star_value = effective_rating - static_cast<float>(i);
            if (star_value > 0.0f) {
                float fill_fraction = std::clamp(star_value, 0.0f, 1.0f);

                sk_canvas->save();
                SkRect clip_rect = SkRect::MakeXYWH(star_x, 0.0f, props_.item_size * fill_fraction, props_.item_size);
                sk_canvas->clipRect(clip_rect, true);
                sk_canvas->drawPath(star_path, active_sk);
                sk_canvas->restore();
            }
        }

        sk_canvas->restore();
    }
};

RatingBarProps::operator WidgetPtr() const {
    return std::make_shared<RatingBarWidget>(*this);
}

std::unique_ptr<RenderObject> RatingBarWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderRatingBar>(props);
}

void RatingBarWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderRatingBar&>(renderObject);
    r.update(props);
}

} // namespace enki
