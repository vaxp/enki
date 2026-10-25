#include "enki/widgets/slider.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include "enki/rendering/canvas.hpp"

#include <layout_engine/Anu.h>
#include <algorithm>

namespace enki {

class RenderSlider : public RenderBox {
public:
    float value_;
    SliderCallback on_change_;
    
    Color active_color_;
    Color inactive_color_;
    Color thumb_color_;
    float track_height_;
    float thumb_radius_;
    float min_value_;
    float max_value_;
    Color shadow_color_;
    float shadow_blur_;
    float shadow_offset_dy_;

    PanGestureRecognizer pan_recognizer_;
    bool is_hovered_ = false;
    bool is_dragging_ = false;

    RenderSlider(float value, SliderCallback on_change, const SliderWidget* opt)
        : value_(value), on_change_(std::move(on_change)),
          active_color_(opt->active_color),
          inactive_color_(opt->inactive_color),
          thumb_color_(opt->thumb_color),
          track_height_(opt->track_height),
          thumb_radius_(opt->thumb_radius),
          min_value_(opt->min_value),
          max_value_(opt->max_value),
          shadow_color_(opt->shadow_color),
          shadow_blur_(opt->shadow_blur),
          shadow_offset_dy_(opt->shadow_offset_dy)
    {
        pan_recognizer_.touch_slop = 2.0f; // Respond quickly but allow tiny taps
        
        pan_recognizer_.on_pan_start = [this](const DragStartDetails& d) {
            is_dragging_ = true;
            updateValueFromLocalX(d.local_position.x);
            markNeedsPaint();
        };
        pan_recognizer_.on_pan_update = [this](const DragUpdateDetails& d) {
            updateValueFromLocalX(d.local_position.x);
        };
        pan_recognizer_.on_pan_end = [this](const DragEndDetails&) {
            is_dragging_ = false;
            markNeedsPaint();
        };
        pan_recognizer_.on_pan_cancel = [this]() {
            is_dragging_ = false;
            markNeedsPaint();
        };

        updateAnuStyles();
    }

    void updateAnuStyles() {
        if (anu_node_) {
            float h = std::max(track_height_, thumb_radius_ * 2.0f);
            ANUNodeStyleSetHeight(anu_node_, h);
            ANUNodeStyleSetMinWidth(anu_node_, 100.0f);
        }
    }

    void setValue(float v) {
        if (value_ != v) {
            value_ = v;
            markNeedsPaint();
        }
    }

    void setOptions(const SliderWidget* opt) {
        active_color_ = opt->active_color;
        inactive_color_ = opt->inactive_color;
        thumb_color_ = opt->thumb_color;
        track_height_ = opt->track_height;
        thumb_radius_ = opt->thumb_radius;
        min_value_ = opt->min_value;
        max_value_ = opt->max_value;
        shadow_color_ = opt->shadow_color;
        shadow_blur_ = opt->shadow_blur;
        shadow_offset_dy_ = opt->shadow_offset_dy;
        
        updateAnuStyles();
        markNeedsLayout();
    }

    void setOnChange(SliderCallback cb) {
        on_change_ = std::move(cb);
    }

    void updateValueFromLocalX(float local_x) {
        float usable_width = size_.width - (thumb_radius_ * 2.0f);
        if (usable_width <= 0.0f) return;
        
        float x = local_x - thumb_radius_;
        float fraction = std::clamp(x / usable_width, 0.0f, 1.0f);
        float new_val = min_value_ + fraction * (max_value_ - min_value_);
        
        if (new_val != value_) {
            value_ = new_val;
            if (on_change_) on_change_(value_);
            markNeedsPaint();
        }
    }

    void syncLayout() override {
        RenderBox::syncLayout();
    }

    void handlePointerDown(const PointerEvent& e) override {
        pan_recognizer_.handlePointerDown(e);
        updateValueFromLocalX(e.localPosition.x);
    }

    void handlePointerMove(const PointerEvent& e) override {
        pan_recognizer_.handlePointerMove(e);
    }

    void handlePointerEnter(const PointerEvent& e) override {
        if (!is_hovered_) {
            is_hovered_ = true;
            markNeedsPaint();
        }
    }

    void handlePointerExit(const PointerEvent& e) override {
        if (is_hovered_) {
            is_hovered_ = false;
            markNeedsPaint();
        }
    }

    void handlePointerUp(const PointerEvent& e) override {
        pan_recognizer_.handlePointerUp(e);
    }

    void handlePointerCancel() {
        pan_recognizer_.handlePointerCancel();
        if (is_hovered_) {
            is_hovered_ = false;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& context) override {
        auto& canvas = context.canvas;
        canvas.save();
        canvas.translate(context.offset.x, context.offset.y);

        float w = size_.width;
        float h = size_.height;
        
        float usable_w = w - (thumb_radius_ * 2.0f);
        float fraction = 0.0f;
        if (max_value_ > min_value_) {
            fraction = std::clamp((value_ - min_value_) / (max_value_ - min_value_), 0.0f, 1.0f);
        }
        
        float thumb_x = thumb_radius_ + (usable_w * fraction);
        float thumb_y = h / 2.0f;
        float track_y = (h - track_height_) / 2.0f;
        
        // 1. Inactive Track
        Paint inactive_paint;
        inactive_paint.color(inactive_color_);
        canvas.drawRRect(Rect{thumb_radius_, track_y, usable_w, track_height_}, 
                         BorderRadius::all(track_height_ / 2.0f), inactive_paint);
        
        // 2. Active Track
        float active_w = thumb_x - thumb_radius_;
        if (active_w > 0.0f) {
            Paint active_paint;
            active_paint.color(active_color_);
            canvas.drawRRect(Rect{thumb_radius_, track_y, active_w, track_height_}, 
                             BorderRadius::all(track_height_ / 2.0f), active_paint);
        }
        
        // 3. Hover Ring (if hovered or dragging)
        if (is_hovered_ || is_dragging_) {
            Paint hover_paint;
            // 20% opacity of active color for hover
            Color hover_color = (active_color_ & 0x00FFFFFF) | 0x33000000;
            hover_paint.color(hover_color);
            float hover_radius = thumb_radius_ * 1.5f;
            canvas.drawCircle(Point{thumb_x, thumb_y}, hover_radius, hover_paint);
        }

        // 4. Thumb with shadow
        if (shadow_blur_ > 0.0f) {
            canvas.drawCircleShadow(Point{thumb_x, thumb_y}, thumb_radius_, 
                                    shadow_color_, shadow_blur_, Point{0.0f, shadow_offset_dy_});
        }
        
        Paint thumb_paint;
        thumb_paint.color(thumb_color_);
        canvas.drawCircle(Point{thumb_x, thumb_y}, thumb_radius_, thumb_paint);

        canvas.restore();
    }
};

std::unique_ptr<RenderObject> SliderWidget::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderSlider>(value, on_change, this);
}

void SliderWidget::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    auto* r = static_cast<RenderSlider*>(&renderObject);
    r->setOptions(this);
    r->setOnChange(on_change);
    r->setValue(value);
}

} // namespace enki
