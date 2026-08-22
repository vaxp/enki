#include "enki/widgets/checkbox.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderCheckbox (Custom Drawing)
// ════════════════════════════════════════════════════════════════

class RenderCheckbox : public RenderBox {
public:
    bool value;
    bool hovered;
    
    float size;
    float border_width;
    float border_radius;
    Color active_color;
    Color check_color;
    Color border_color;
    Color hover_border_color;
    Color inactive_bg_color;
    bool disabled;
    Color disabled_color;

    RenderCheckbox(bool val, bool hov, const CheckboxWidget* options) 
        : value(val), hovered(hov),
          size(options->size),
          border_width(options->border_width),
          border_radius(options->border_radius),
          active_color(options->active_color),
          check_color(options->check_color),
          border_color(options->border_color),
          hover_border_color(options->hover_border_color),
          inactive_bg_color(options->inactive_bg_color),
          disabled(options->disabled),
          disabled_color(options->disabled_color) {
        ANUNodeStyleSetWidth(anu_node_, size);
        ANUNodeStyleSetHeight(anu_node_, size);
    }

    void update(bool new_value, bool new_hovered, const CheckboxWidget* new_options) {
        if (size != new_options->size) {
            ANUNodeStyleSetWidth(anu_node_, new_options->size);
            ANUNodeStyleSetHeight(anu_node_, new_options->size);
            markNeedsLayout();
        }
        if (value != new_value || hovered != new_hovered || 
            active_color != new_options->active_color ||
            border_color != new_options->border_color) {
            markNeedsPaint();
        }
        value = new_value;
        hovered = new_hovered;
        size = new_options->size;
        border_width = new_options->border_width;
        border_radius = new_options->border_radius;
        active_color = new_options->active_color;
        check_color = new_options->check_color;
        border_color = new_options->border_color;
        hover_border_color = new_options->hover_border_color;
        inactive_bg_color = new_options->inactive_bg_color;
        disabled = new_options->disabled;
        disabled_color = new_options->disabled_color;
    }

    void paint(PaintContext& context) override {
        Rect rect{
            context.offset.x,
            context.offset.y,
            size,
            size
        };

        BorderRadius radius = BorderRadius::circular(border_radius);

        Paint paint;
        paint.setStyle(PaintStyle::Fill);
        
        // Draw background
        if (value) {
            paint.setColor(disabled ? disabled_color : active_color);
        } else {
            paint.setColor(inactive_bg_color);
        }
        
        context.canvas.drawRRect(rect, radius, paint);

        // Draw border
        if (!value) {
            paint.setStyle(PaintStyle::Stroke);
            paint.setStrokeWidth(border_width);
            if (disabled) {
                paint.setColor(disabled_color);
            } else if (hovered) {
                paint.setColor(hover_border_color);
            } else {
                paint.setColor(border_color);
            }
            context.canvas.drawRRect(rect, radius, paint);
        }

        // Draw checkmark
        if (value) {
            Paint check_paint;
            check_paint.setColor(check_color);
            check_paint.setStyle(PaintStyle::Stroke);
            check_paint.setStrokeWidth(size * 0.15f); // Scale stroke with size
            // TODO: Proper stroke cap setting when enki's Paint supports it
            // check_paint.stroke_cap = StrokeCap::Round;

            // Checkmark path
            Path path;
            float start_x = rect.x + rect.width * 0.25f;
            float start_y = rect.y + rect.height * 0.5f;
            
            float mid_x = rect.x + rect.width * 0.45f;
            float mid_y = rect.y + rect.height * 0.7f;
            
            float end_x = rect.x + rect.width * 0.75f;
            float end_y = rect.y + rect.height * 0.3f;

            path.moveTo(start_x, start_y);
            path.lineTo(mid_x, mid_y);
            path.lineTo(end_x, end_y);

            context.canvas.drawPath(path, check_paint);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// Internal CheckboxRenderWidget
// ════════════════════════════════════════════════════════════════

class CheckboxRenderWidget : public SingleChildRenderObjectWidget {
public:
    bool value;
    bool hovered;
    const CheckboxWidget* options;

    CheckboxRenderWidget(bool val, bool hov, const CheckboxWidget* opt)
        : value(val), hovered(hov), options(opt) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderCheckbox>(value, hovered, options);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        auto& renderCheckbox = static_cast<RenderCheckbox&>(renderObject);
        renderCheckbox.update(value, hovered, options);
    }
    
    [[nodiscard]] std::string_view typeName() const override { return "CheckboxRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// CheckboxState
// ════════════════════════════════════════════════════════════════

class CheckboxState : public State {
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const CheckboxWidget*>(widget());

        auto render_widget = std::make_shared<CheckboxRenderWidget>(w->value, hovered_, w);

        if (w->disabled) {
            return render_widget;
        }

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type = SystemCursor::Pointer;

        detector->on_hover_enter = [this](const PointerEvent&) { setState([this] { hovered_ = true; }); };
        detector->on_hover_exit  = [this](const PointerEvent&) { setState([this] { hovered_ = false; }); };

        auto on_changed = w->on_changed;
        bool current_value = w->value;
        detector->on_tap = [on_changed, current_value] {
            if (on_changed) {
                on_changed(!current_value);
            }
        };

        detector->child = render_widget;
        return detector;
    }
};

std::unique_ptr<State> CheckboxWidget::createState() {
    return std::make_unique<CheckboxState>();
}

} // namespace enki
