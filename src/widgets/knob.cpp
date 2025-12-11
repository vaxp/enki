#include "enki/widgets/knob.hpp"
#include "enki/rendering/canvas.hpp"
#include <layout_engine/Anu.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace enki {

class RenderKnob : public RenderBox {
public:
    KnobProps props_;
    bool is_dragging_{false};
    float last_mouse_y_{0.0f};

    explicit RenderKnob(KnobProps props)
        : props_(std::move(props)) {
        applyStyle();
    }

    void update(const KnobProps& new_props) {
        bool dims_changed = (props_.size != new_props.size || props_.show_value != new_props.show_value || props_.label != new_props.label);
        props_ = new_props;
        if (dims_changed) {
            applyStyle();
        }
        markNeedsPaint();
    }

    void applyStyle() {
        if (!anu_node_) return;
        float total_h = props_.size + (props_.show_value ? 28.0f : 0.0f) + (!props_.label.empty() ? 16.0f : 0.0f);
        ANUNodeStyleSetWidth(anu_node_, props_.size);
        ANUNodeStyleSetHeight(anu_node_, total_h);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0.0f && localPoint.x <= size_.width &&
               localPoint.y >= 0.0f && localPoint.y <= size_.height;
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return SystemCursor::ResizeVertical;
    }

    void handlePointerDown(const PointerEvent& e) override {
        is_dragging_ = true;
        last_mouse_y_ = e.localPosition.y;
    }

    void handlePointerMove(const PointerEvent& e) override {
        if (!is_dragging_) return;

        float dy = last_mouse_y_ - e.localPosition.y;
        last_mouse_y_ = e.localPosition.y;

        float range = props_.max_value - props_.min_value;
        if (range <= 0.0f) return;

        // 150 pixels drag = full range travel
        float delta = dy * (range / 150.0f);
        float new_val = std::clamp(props_.value + delta, props_.min_value, props_.max_value);

        if (props_.step > 0.0f) {
            new_val = std::round((new_val - props_.min_value) / props_.step) * props_.step + props_.min_value;
            new_val = std::clamp(new_val, props_.min_value, props_.max_value);
        }

        if (new_val != props_.value) {
            props_.value = new_val;
            if (props_.on_value_changed) {
                props_.on_value_changed(new_val);
            }
            markNeedsPaint();
        }
    }

    void handlePointerUp(const PointerEvent&) override {
        is_dragging_ = false;
    }

    void paint(PaintContext& ctx) override {
        auto& canvas = ctx.canvas;
        auto* sk = static_cast<SkCanvas*>(canvas.getNativeHandle());
        if (!sk) return;

        sk->save();
        sk->translate(ctx.offset.x, ctx.offset.y);

        float d = props_.size;
        float cx = d * 0.5f;
        float cy = d * 0.5f;
        float r = d * 0.5f;

        float range = props_.max_value - props_.min_value;
        float frac = (range > 0.0f) ? std::clamp((props_.value - props_.min_value) / range, 0.0f, 1.0f) : 0.0f;

        // 1. Background Arc Track (270 degrees from 135 deg)
        float arc_margin = 6.0f;
        SkRect oval = SkRect::MakeXYWH(arc_margin, arc_margin, d - arc_margin * 2.0f, d - arc_margin * 2.0f);

        SkPaint track_paint;
        track_paint.setStyle(SkPaint::kStroke_Style);
        track_paint.setStrokeWidth(3.5f);
        track_paint.setStrokeCap(SkPaint::kRound_Cap);
        track_paint.setAntiAlias(true);
        track_paint.setColor(props_.track_color);
        sk->drawArc(oval, 135.0f, 270.0f, false, track_paint);

        // 2. Active Progress Arc
        SkPaint active_arc_paint;
        active_arc_paint.setStyle(SkPaint::kStroke_Style);
        active_arc_paint.setStrokeWidth(3.5f);
        active_arc_paint.setStrokeCap(SkPaint::kRound_Cap);
        active_arc_paint.setAntiAlias(true);
        active_arc_paint.setColor(props_.active_color);

        if (!props_.is_bipolar) {
            float active_sweep = frac * 270.0f;
            if (active_sweep > 0.5f) {
                sk->drawArc(oval, 135.0f, active_sweep, false, active_arc_paint);
            }
        } else {
            // Bipolar: sweeps left/right from top center (270 deg)
            float start_deg = 270.0f;
            float sweep_deg = (frac - 0.5f) * 270.0f;
            sk->drawArc(oval, start_deg, sweep_deg, false, active_arc_paint);
        }

        // 3. Dial Body (Inner Circle)
        float dial_radius = r - 12.0f;

        Paint dial_fill;
        dial_fill.setStyle(PaintStyle::Fill);
        dial_fill.setAntiAlias(true);
        dial_fill.setColor(props_.dial_color);
        canvas.drawCircle(Point(cx, cy), dial_radius, dial_fill);

        Paint dial_border;
        dial_border.setStyle(PaintStyle::Stroke);
        dial_border.setStrokeWidth(1.2f);
        dial_border.setAntiAlias(true);
        dial_border.setColor(props_.track_color);
        canvas.drawCircle(Point(cx, cy), dial_radius, dial_border);

        // 4. Indicator Pointer Notch
        float current_angle_deg = 135.0f + frac * 270.0f;
        constexpr float kPi = 3.14159265358979323846f;
        float rad = current_angle_deg * kPi / 180.0f;

        float r1 = dial_radius * 0.35f;
        float r2 = dial_radius * 0.85f;
        float x1 = cx + r1 * std::cos(rad);
        float y1 = cy + r1 * std::sin(rad);
        float x2 = cx + r2 * std::cos(rad);
        float y2 = cy + r2 * std::sin(rad);

        Paint notch_paint;
        notch_paint.setStyle(PaintStyle::Stroke);
        notch_paint.setStrokeWidth(2.5f);
        notch_paint.setAntiAlias(true);
        notch_paint.setColor(props_.pointer_color);
        canvas.drawLine(Point(x1, y1), Point(x2, y2), notch_paint);

        // 5. Value Readout & Label
        float cur_y = d + 12.0f;

        if (props_.show_value) {
            char val_str[32];
            if (std::abs(props_.value - std::round(props_.value)) < 0.01f) {
                std::snprintf(val_str, sizeof(val_str), "%.0f %s", props_.value, props_.unit.c_str());
            } else {
                std::snprintf(val_str, sizeof(val_str), "%.1f %s", props_.value, props_.unit.c_str());
            }

            Paint val_paint;
            val_paint.setAntiAlias(true);
            val_paint.setColor(props_.active_color);

            float approx_w = static_cast<float>(std::string(val_str).length()) * 6.5f;
            canvas.drawText(val_str, Point(cx - approx_w * 0.5f, cur_y), val_paint, 11.5f, nullptr, false);
            cur_y += 14.0f;
        }

        if (!props_.label.empty()) {
            Paint label_paint;
            label_paint.setAntiAlias(true);
            label_paint.setColor(props_.text_color);

            float approx_w = static_cast<float>(props_.label.length()) * 6.0f;
            canvas.drawText(props_.label, Point(cx - approx_w * 0.5f, cur_y), label_paint, 10.5f, nullptr, false);
        }

        sk->restore();
    }
};

KnobProps::operator WidgetPtr() const {
    return std::make_shared<KnobWidget>(*this);
}

std::unique_ptr<RenderObject> KnobWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderKnob>(props);
}

void KnobWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderKnob&>(renderObject);
    r.update(props);
}

} // namespace enki
