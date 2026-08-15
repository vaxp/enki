#include "enki/widgets/slider.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/effects/SkImageFilters.h>
#include <layout_engine/Anu.h>
#include <algorithm>

namespace enki {

class RenderSlider : public RenderBox {
public:
    float value_;
    SliderOptions options_;
    SliderCallback on_change_;

    PanGestureRecognizer pan_recognizer_;
    bool is_hovered_ = false;
    bool is_dragging_ = false;

    RenderSlider(float value, SliderOptions opt, SliderCallback on_change)
        : value_(value), options_(std::move(opt)), on_change_(std::move(on_change)) 
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
            float h = std::max(options_.track_height, options_.thumb_radius * 2.0f);
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

    void setOptions(const SliderOptions& opt) {
        options_ = opt;
        updateAnuStyles();
        markNeedsLayout();
    }

    void setOnChange(SliderCallback cb) {
        on_change_ = std::move(cb);
    }

    void updateValueFromLocalX(float local_x) {
        float usable_width = size_.width - (options_.thumb_radius * 2.0f);
        if (usable_width <= 0.0f) return;
        
        float x = local_x - options_.thumb_radius;
        float fraction = std::clamp(x / usable_width, 0.0f, 1.0f);
        float new_val = options_.min_value + fraction * (options_.max_value - options_.min_value);
        
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
        
        bool hover = (e.localPosition.x >= 0 && e.localPosition.x <= size_.width &&
                      e.localPosition.y >= 0 && e.localPosition.y <= size_.height);
        if (hover != is_hovered_) {
            is_hovered_ = hover;
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
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(context.canvas.getNativeHandle());
        if (!sk_canvas) return;

        sk_canvas->save();
        sk_canvas->translate(context.offset.x, context.offset.y);

        float w = size_.width;
        float h = size_.height;
        
        float usable_w = w - (options_.thumb_radius * 2.0f);
        float fraction = 0.0f;
        if (options_.max_value > options_.min_value) {
            fraction = std::clamp((value_ - options_.min_value) / (options_.max_value - options_.min_value), 0.0f, 1.0f);
        }
        
        float thumb_x = options_.thumb_radius + (usable_w * fraction);
        float thumb_y = h / 2.0f;
        float track_y = (h - options_.track_height) / 2.0f;
        
        // 1. Inactive Track
        SkPaint inactive_paint;
        inactive_paint.setAntiAlias(true);
        inactive_paint.setColor(options_.inactive_color);
        
        SkRect inactive_rect = SkRect::MakeXYWH(options_.thumb_radius, track_y, usable_w, options_.track_height);
        sk_canvas->drawRoundRect(inactive_rect, options_.track_height / 2.0f, options_.track_height / 2.0f, inactive_paint);
        
        // 2. Active Track
        SkPaint active_paint;
        active_paint.setAntiAlias(true);
        active_paint.setColor(options_.active_color);
        
        float active_w = thumb_x - options_.thumb_radius;
        if (active_w > 0.0f) {
            SkRect active_rect = SkRect::MakeXYWH(options_.thumb_radius, track_y, active_w, options_.track_height);
            sk_canvas->drawRoundRect(active_rect, options_.track_height / 2.0f, options_.track_height / 2.0f, active_paint);
        }
        
        // 3. Hover Ring (if hovered or dragging)
        if (is_hovered_ || is_dragging_) {
            SkPaint hover_paint;
            hover_paint.setAntiAlias(true);
            SkColor base_active = options_.active_color;
            // 20% opacity of active color for hover
            hover_paint.setColor(SkColorSetA(base_active, 51));
            
            float hover_radius = options_.thumb_radius * 1.5f;
            sk_canvas->drawCircle(thumb_x, thumb_y, hover_radius, hover_paint);
        }

        // 4. Thumb with shadow
        SkPaint thumb_paint;
        thumb_paint.setAntiAlias(true);
        thumb_paint.setColor(options_.thumb_color);
        
        if (options_.shadow_blur > 0.0f) {
            thumb_paint.setImageFilter(SkImageFilters::DropShadow(
                0.0f, options_.shadow_offset_dy,   // dx, dy
                options_.shadow_blur, options_.shadow_blur,   // sigmaX, sigmaY
                options_.shadow_color, // color
                nullptr       // input
            ));
        }
        
        sk_canvas->drawCircle(thumb_x, thumb_y, options_.thumb_radius, thumb_paint);

        sk_canvas->restore();
    }
};

std::unique_ptr<RenderObject> Slider::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderSlider>(value, options, on_change);
}

void Slider::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    auto* r = static_cast<RenderSlider*>(&renderObject);
    r->setOptions(options);
    r->setOnChange(on_change);
    r->setValue(value);
}

} // namespace enki
