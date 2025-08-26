#include "enki/widgets/range_slider.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/effects/SkImageFilters.h>
#include <layout_engine/Anu.h>
#include <algorithm>
#include <cmath>

namespace enki {

class RenderRangeSlider : public RenderBox {
public:
    float start_value_;
    float end_value_;
    RangeSliderProps options_;
    RangeSliderCallback on_change_;

    PanGestureRecognizer pan_recognizer_;
    
    enum class Thumb { None, Start, End };
    Thumb hovered_thumb_ = Thumb::None;
    Thumb dragging_thumb_ = Thumb::None;

    RenderRangeSlider(float start, float end, RangeSliderProps opt, RangeSliderCallback on_change)
        : start_value_(start), end_value_(end), options_(std::move(opt)), on_change_(std::move(on_change)) 
    {
        pan_recognizer_.touch_slop = 2.0f; // Respond quickly
        
        pan_recognizer_.on_pan_start = [this](const DragStartDetails& d) {
            float start_x = getThumbX(start_value_);
            float end_x = getThumbX(end_value_);
            
            float dist_start = std::abs(d.local_position.x - start_x);
            float dist_end = std::abs(d.local_position.x - end_x);
            
            if (dist_start <= dist_end) {
                dragging_thumb_ = Thumb::Start;
            } else {
                dragging_thumb_ = Thumb::End;
            }
            
            updateValueFromLocalX(d.local_position.x);
            markNeedsPaint();
        };
        pan_recognizer_.on_pan_update = [this](const DragUpdateDetails& d) {
            updateValueFromLocalX(d.local_position.x);
        };
        pan_recognizer_.on_pan_end = [this](const DragEndDetails&) {
            dragging_thumb_ = Thumb::None;
            markNeedsPaint();
        };
        pan_recognizer_.on_pan_cancel = [this]() {
            dragging_thumb_ = Thumb::None;
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

    float getThumbX(float value) const {
        float usable_width = size_.width - (options_.thumb_radius * 2.0f);
        if (usable_width <= 0.0f) return options_.thumb_radius;
        
        float fraction = 0.0f;
        if (options_.max_value > options_.min_value) {
            fraction = std::clamp((value - options_.min_value) / (options_.max_value - options_.min_value), 0.0f, 1.0f);
        }
        return options_.thumb_radius + (usable_width * fraction);
    }

    void setValues(float start, float end) {
        if (start_value_ != start || end_value_ != end) {
            start_value_ = start;
            end_value_ = end;
            markNeedsPaint();
        }
    }

    void setOptions(const RangeSliderProps& opt) {
        options_ = opt;
        updateAnuStyles();
        markNeedsLayout();
    }

    void setOnChange(RangeSliderCallback cb) {
        on_change_ = std::move(cb);
    }

    void updateValueFromLocalX(float local_x) {
        float usable_width = size_.width - (options_.thumb_radius * 2.0f);
        if (usable_width <= 0.0f) return;
        
        float x = local_x - options_.thumb_radius;
        float fraction = std::clamp(x / usable_width, 0.0f, 1.0f);
        float new_val = options_.min_value + fraction * (options_.max_value - options_.min_value);
        
        bool changed = false;
        
        if (dragging_thumb_ == Thumb::Start) {
            new_val = std::clamp(new_val, options_.min_value, end_value_);
            if (new_val != start_value_) {
                start_value_ = new_val;
                changed = true;
            }
        } else if (dragging_thumb_ == Thumb::End) {
            new_val = std::clamp(new_val, start_value_, options_.max_value);
            if (new_val != end_value_) {
                end_value_ = new_val;
                changed = true;
            }
        }
        
        if (changed) {
            if (on_change_) on_change_(start_value_, end_value_);
            markNeedsPaint();
        }
    }

    void syncLayout() override {
        RenderBox::syncLayout();
    }

    void handlePointerDown(const PointerEvent& e) override {
        pan_recognizer_.handlePointerDown(e);
        
        float start_x = getThumbX(start_value_);
        float end_x = getThumbX(end_value_);
        
        float dist_start = std::abs(e.localPosition.x - start_x);
        float dist_end = std::abs(e.localPosition.x - end_x);
        
        if (dist_start <= dist_end) {
            dragging_thumb_ = Thumb::Start;
        } else {
            dragging_thumb_ = Thumb::End;
        }
        updateValueFromLocalX(e.localPosition.x);
    }

    void handlePointerMove(const PointerEvent& e) override {
        pan_recognizer_.handlePointerMove(e);
        
        bool is_in_bounds = (e.localPosition.x >= 0 && e.localPosition.x <= size_.width &&
                             e.localPosition.y >= 0 && e.localPosition.y <= size_.height);
        
        Thumb hover = Thumb::None;
        if (is_in_bounds) {
            float start_x = getThumbX(start_value_);
            float end_x = getThumbX(end_value_);
            float dist_start = std::abs(e.localPosition.x - start_x);
            float dist_end = std::abs(e.localPosition.x - end_x);
            
            if (dist_start <= options_.thumb_radius * 2.0f || dist_end <= options_.thumb_radius * 2.0f) {
                if (dist_start <= dist_end) {
                    hover = Thumb::Start;
                } else {
                    hover = Thumb::End;
                }
            }
        }
        
        if (hover != hovered_thumb_) {
            hovered_thumb_ = hover;
            markNeedsPaint();
        }
    }

    void handlePointerExit(const PointerEvent& e) override {
        if (hovered_thumb_ != Thumb::None) {
            hovered_thumb_ = Thumb::None;
            markNeedsPaint();
        }
    }

    void handlePointerUp(const PointerEvent& e) override {
        pan_recognizer_.handlePointerUp(e);
    }

    void handlePointerCancel() {
        pan_recognizer_.handlePointerCancel();
        if (hovered_thumb_ != Thumb::None) {
            hovered_thumb_ = Thumb::None;
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
        
        float start_x = getThumbX(start_value_);
        float end_x = getThumbX(end_value_);
        float thumb_y = h / 2.0f;
        float track_y = (h - options_.track_height) / 2.0f;
        
        // 1. Inactive Track
        SkPaint inactive_paint;
        inactive_paint.setAntiAlias(true);
        inactive_paint.setColor(options_.inactive_color);
        
        SkRect inactive_rect = SkRect::MakeXYWH(options_.thumb_radius, track_y, usable_w, options_.track_height);
        sk_canvas->drawRoundRect(inactive_rect, options_.track_height / 2.0f, options_.track_height / 2.0f, inactive_paint);
        
        // 2. Active Track (Between start and end)
        if (end_x > start_x) {
            SkPaint active_paint;
            active_paint.setAntiAlias(true);
            active_paint.setColor(options_.active_color);
            
            SkRect active_rect = SkRect::MakeXYWH(start_x, track_y, end_x - start_x, options_.track_height);
            sk_canvas->drawRoundRect(active_rect, options_.track_height / 2.0f, options_.track_height / 2.0f, active_paint);
        }
        
        auto drawThumb = [&](float x, Thumb type) {
            // Hover Ring
            if (hovered_thumb_ == type || dragging_thumb_ == type) {
                SkPaint hover_paint;
                hover_paint.setAntiAlias(true);
                SkColor base_active = options_.active_color;
                hover_paint.setColor(SkColorSetA(base_active, 51));
                sk_canvas->drawCircle(x, thumb_y, options_.thumb_radius * 1.5f, hover_paint);
            }
            
            // Thumb Shadow
            SkPaint thumb_paint;
            thumb_paint.setAntiAlias(true);
            thumb_paint.setColor(options_.thumb_color);
            
            if (options_.shadow_blur > 0.0f) {
                thumb_paint.setImageFilter(SkImageFilters::DropShadow(
                    0.0f, options_.shadow_offset_dy,
                    options_.shadow_blur, options_.shadow_blur,
                    options_.shadow_color,
                    nullptr
                ));
            }
            sk_canvas->drawCircle(x, thumb_y, options_.thumb_radius, thumb_paint);
        };
        
        // Draw inactive thumb first, active one on top
        if (dragging_thumb_ == Thumb::Start) {
            drawThumb(end_x, Thumb::End);
            drawThumb(start_x, Thumb::Start);
        } else {
            drawThumb(start_x, Thumb::Start);
            drawThumb(end_x, Thumb::End);
        }

        sk_canvas->restore();
    }
};

std::unique_ptr<RenderObject> RangeSliderWidget::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderRangeSlider>(start_value, end_value, options, on_change);
}

void RangeSliderWidget::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    auto* r = static_cast<RenderRangeSlider*>(&renderObject);
    r->setOptions(options);
    r->setOnChange(on_change);
    r->setValues(start_value, end_value);
}

} // namespace enki
