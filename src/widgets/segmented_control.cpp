#include "enki/widgets/segmented_control.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/rendering/canvas.hpp"
#include <include/core/SkCanvas.h>
#include <layout_engine/Anu.h>
#include <algorithm>
#include <cmath>

namespace enki {

class RenderSegmentedControl : public RenderBox {
public:
    SegmentedControlProps props_;
    std::unique_ptr<Ticker> ticker_;
    float anim_thumb_x_{0.0f};
    float anim_thumb_w_{0.0f};
    float target_thumb_x_{0.0f};
    float target_thumb_w_{0.0f};
    bool is_animating_{false};

    explicit RenderSegmentedControl(SegmentedControlProps props)
        : props_(std::move(props)) {
        applyStyle();
        ticker_ = createTicker([this]() {
            updateAnimation();
        });
    }

    ~RenderSegmentedControl() override {
        if (ticker_) {
            ticker_->stop();
            ticker_.reset();
        }
    }

    void update(const SegmentedControlProps& new_props) {
        bool selection_changed = (props_.selected_index != new_props.selected_index);
        props_ = new_props;
        applyStyle();
        if (selection_changed) {
            calculateTargets();
            if (ticker_) ticker_->start();
            is_animating_ = true;
        }
        markNeedsPaint();
    }

    void applyStyle() {
        if (!anu_node_) return;
        if (props_.width > 0.0f) {
            ANUNodeStyleSetWidth(anu_node_, props_.width);
        } else {
            ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        }
        ANUNodeStyleSetHeight(anu_node_, props_.height);
    }



    void calculateTargets() {
        size_t count = props_.items.size();
        if (count == 0 || size_.width <= 0.0f) return;

        float pad = props_.padding;
        float avail_w = size_.width - pad * 2.0f;
        float seg_w = avail_w / static_cast<float>(count);

        int idx = std::clamp(props_.selected_index, 0, static_cast<int>(count) - 1);
        target_thumb_x_ = pad + static_cast<float>(idx) * seg_w;
        target_thumb_w_ = seg_w;
    }

    void updateAnimation() {
        if (!is_animating_) return;

        float speed = 0.28f; // Smooth spring-like lerp factor
        float dx = target_thumb_x_ - anim_thumb_x_;
        float dw = target_thumb_w_ - anim_thumb_w_;

        if (std::abs(dx) < 0.5f && std::abs(dw) < 0.5f) {
            anim_thumb_x_ = target_thumb_x_;
            anim_thumb_w_ = target_thumb_w_;
            is_animating_ = false;
            if (ticker_) ticker_->stop();
        } else {
            anim_thumb_x_ += dx * speed;
            anim_thumb_w_ += dw * speed;
        }
        markNeedsPaint();
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0.0f && localPoint.x <= size_.width &&
               localPoint.y >= 0.0f && localPoint.y <= size_.height;
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return SystemCursor::Pointer;
    }

    void handlePointerDown(const PointerEvent& e) override {
        size_t count = props_.items.size();
        if (count == 0 || size_.width <= 0.0f) return;

        float pad = props_.padding;
        float avail_w = size_.width - pad * 2.0f;
        float seg_w = avail_w / static_cast<float>(count);

        float local_x = e.localPosition.x - pad;
        if (local_x >= 0.0f && local_x <= avail_w) {
            int clicked_idx = static_cast<int>(local_x / seg_w);
            clicked_idx = std::clamp(clicked_idx, 0, static_cast<int>(count) - 1);

            if (props_.items[clicked_idx].enabled && clicked_idx != props_.selected_index) {
                props_.selected_index = clicked_idx;
                calculateTargets();
                is_animating_ = true;
                if (ticker_) ticker_->start();
                if (props_.on_change) {
                    props_.on_change(clicked_idx);
                }
                markNeedsPaint();
            }
        }
    }

    void paint(PaintContext& ctx) override {
        calculateTargets();
        if (!is_animating_) {
            anim_thumb_x_ = target_thumb_x_;
            anim_thumb_w_ = target_thumb_w_;
        }

        auto* sk = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk) return;

        sk->save();
        sk->translate(ctx.offset.x, ctx.offset.y);

        auto& canvas = ctx.canvas;
        Rect bounds{0.0f, 0.0f, size_.width, size_.height};

        // 1. Background Track
        Paint track_paint;
        track_paint.setStyle(PaintStyle::Fill);
        track_paint.setAntiAlias(true);
        track_paint.setColor(props_.track_color);
        canvas.drawRRect(bounds, BorderRadius::circular(props_.border_radius), track_paint);

        // 2. Track Border
        if (props_.border_width > 0.0f && ((props_.border_color >> 24) > 0)) {
            Paint border_paint;
            border_paint.setStyle(PaintStyle::Stroke);
            border_paint.setStrokeWidth(props_.border_width);
            border_paint.setAntiAlias(true);
            border_paint.setColor(props_.border_color);
            canvas.drawRRect(bounds, BorderRadius::circular(props_.border_radius), border_paint);
        }

        size_t count = props_.items.size();
        if (count == 0) return;

        // 3. Sliding Active Pill / Thumb
        float pad = props_.padding;
        float thumb_h = size_.height - pad * 2.0f;
        Rect thumb_rect{anim_thumb_x_, pad, anim_thumb_w_, thumb_h};

        Paint thumb_paint;
        thumb_paint.setStyle(PaintStyle::Fill);
        thumb_paint.setAntiAlias(true);
        thumb_paint.setColor(props_.thumb_color);
        canvas.drawRRect(thumb_rect, BorderRadius::circular(props_.thumb_radius), thumb_paint);

        if ((props_.thumb_border_color >> 24) > 0) {
            Paint thumb_border;
            thumb_border.setStyle(PaintStyle::Stroke);
            thumb_border.setStrokeWidth(1.2f);
            thumb_border.setAntiAlias(true);
            thumb_border.setColor(props_.thumb_border_color);
            canvas.drawRRect(thumb_rect, BorderRadius::circular(props_.thumb_radius), thumb_border);
        }

        // 4. Segment Labels & Icons
        float avail_w = size_.width - pad * 2.0f;
        float seg_w = avail_w / static_cast<float>(count);

        for (size_t i = 0; i < count; ++i) {
            const auto& item = props_.items[i];
            bool is_active = (static_cast<int>(i) == props_.selected_index);

            float cx = pad + static_cast<float>(i) * seg_w + seg_w * 0.5f;
            float cy = size_.height * 0.5f;

            Color text_col = !item.enabled ? 0x4D94A3B8 : (is_active ? props_.active_text_color : props_.inactive_text_color);

            std::string display = item.icon.empty() ? item.label : (item.icon + " " + item.label);

            Paint p;
            p.setColor(text_col);
            p.setAntiAlias(true);

            // Approximate centering
            float approx_char_w = 7.0f;
            float text_w = static_cast<float>(display.length()) * approx_char_w;
            float tx = cx - text_w * 0.5f;
            float ty = cy + 4.0f;

            canvas.drawText(display, Point(tx, ty), p, 12.5f, nullptr, false);
        }

        sk->restore();
    }
};

SegmentedControlProps::operator WidgetPtr() const {
    return std::make_shared<SegmentedControlWidget>(*this);
}

std::unique_ptr<RenderObject> SegmentedControlWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderSegmentedControl>(props);
}

void SegmentedControlWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderSegmentedControl&>(renderObject);
    r.update(props);
}

} // namespace enki
