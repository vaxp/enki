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
    CheckboxOptions options;

    RenderCheckbox(bool val, bool hov, CheckboxOptions opt) 
        : value(val), hovered(hov), options(std::move(opt)) {
        ANUNodeStyleSetWidth(anu_node_, options.size);
        ANUNodeStyleSetHeight(anu_node_, options.size);
    }

    void update(bool new_value, bool new_hovered, const CheckboxOptions& new_options) {
        if (options.size != new_options.size) {
            ANUNodeStyleSetWidth(anu_node_, new_options.size);
            ANUNodeStyleSetHeight(anu_node_, new_options.size);
            markNeedsLayout();
        }
        if (value != new_value || hovered != new_hovered || 
            options.active_color != new_options.active_color ||
            options.border_color != new_options.border_color) {
            markNeedsPaint();
        }
        value = new_value;
        hovered = new_hovered;
        options = new_options;
    }

    void paint(PaintContext& context) override {
        Rect rect{
            context.offset.x,
            context.offset.y,
            options.size,
            options.size
        };

        BorderRadius radius = BorderRadius::circular(options.border_radius);

        Paint paint;
        paint.setStyle(PaintStyle::Fill);
        
        // Draw background
        if (value) {
            paint.setColor(options.disabled ? options.disabled_color : options.active_color);
        } else {
            paint.setColor(options.inactive_bg_color);
        }
        
        context.canvas.drawRRect(rect, radius, paint);

        // Draw border
        if (!value) {
            paint.setStyle(PaintStyle::Stroke);
            paint.setStrokeWidth(options.border_width);
            if (options.disabled) {
                paint.setColor(options.disabled_color);
            } else if (hovered) {
                paint.setColor(options.hover_border_color);
            } else {
                paint.setColor(options.border_color);
            }
            context.canvas.drawRRect(rect, radius, paint);
        }

        // Draw checkmark
        if (value) {
            Paint check_paint;
            check_paint.setColor(options.check_color);
            check_paint.setStyle(PaintStyle::Stroke);
            check_paint.setStrokeWidth(options.size * 0.15f); // Scale stroke with size
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
    CheckboxOptions options;

    CheckboxRenderWidget(bool val, bool hov, CheckboxOptions opt)
        : value(val), hovered(hov), options(std::move(opt)) {}

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
        auto* w = static_cast<const Checkbox*>(widget());

        auto render_widget = std::make_shared<CheckboxRenderWidget>(w->value, hovered_, w->options);

        if (w->options.disabled) {
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

std::unique_ptr<State> Checkbox::createState() {
    return std::make_unique<CheckboxState>();
}

} // namespace enki
