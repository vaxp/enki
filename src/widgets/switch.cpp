#include "enki/widgets/switch.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderSwitch (Custom Drawing)
// ════════════════════════════════════════════════════════════════

class RenderSwitch : public RenderBox {
public:
    bool value;
    bool hovered;
    
    float width;
    float height;
    float thumb_padding;
    Color active_color;
    Color active_thumb_color;
    Color inactive_color;
    Color inactive_thumb_color;
    Color hover_color;
    Color hover_inactive_color;
    bool disabled;
    Color disabled_color;
    Color disabled_thumb_color;

    RenderSwitch(bool val, bool hov, const SwitchWidget* options) 
        : value(val), hovered(hov),
          width(options->width),
          height(options->height),
          thumb_padding(options->thumb_padding),
          active_color(options->active_color),
          active_thumb_color(options->active_thumb_color),
          inactive_color(options->inactive_color),
          inactive_thumb_color(options->inactive_thumb_color),
          hover_color(options->hover_color),
          hover_inactive_color(options->hover_inactive_color),
          disabled(options->disabled),
          disabled_color(options->disabled_color),
          disabled_thumb_color(options->disabled_thumb_color) {
        ANUNodeStyleSetWidth(anu_node_, width);
        ANUNodeStyleSetHeight(anu_node_, height);
    }

    void update(bool new_value, bool new_hovered, const SwitchWidget* new_options) {
        if (width != new_options->width || height != new_options->height) {
            ANUNodeStyleSetWidth(anu_node_, new_options->width);
            ANUNodeStyleSetHeight(anu_node_, new_options->height);
            markNeedsLayout();
        }
        if (value != new_value || hovered != new_hovered || 
            active_color != new_options->active_color ||
            inactive_color != new_options->inactive_color) {
            markNeedsPaint();
        }
        value = new_value;
        hovered = new_hovered;
        
        width = new_options->width;
        height = new_options->height;
        thumb_padding = new_options->thumb_padding;
        active_color = new_options->active_color;
        active_thumb_color = new_options->active_thumb_color;
        inactive_color = new_options->inactive_color;
        inactive_thumb_color = new_options->inactive_thumb_color;
        hover_color = new_options->hover_color;
        hover_inactive_color = new_options->hover_inactive_color;
        disabled = new_options->disabled;
        disabled_color = new_options->disabled_color;
        disabled_thumb_color = new_options->disabled_thumb_color;
    }

    void paint(PaintContext& context) override {
        Rect rect{
            context.offset.x,
            context.offset.y,
            width,
            height
        };

        BorderRadius radius = BorderRadius::circular(height / 2.0f);

        Paint paint;
        paint.setStyle(PaintStyle::Fill);
        
        // Draw track (background)
        if (disabled) {
            paint.setColor(disabled_color);
        } else if (value) {
            paint.setColor(hovered ? hover_color : active_color);
        } else {
            paint.setColor(hovered ? hover_inactive_color : inactive_color);
        }
        
        context.canvas.drawRRect(rect, radius, paint);

        // Draw thumb (circle)
        float thumb_size = height - (thumb_padding * 2.0f);
        float thumb_x = context.offset.x + thumb_padding;
        
        if (value) {
            // Move thumb to the right
            thumb_x = context.offset.x + width - thumb_size - thumb_padding;
        }

        Rect thumb_rect{
            thumb_x,
            context.offset.y + thumb_padding,
            thumb_size,
            thumb_size
        };

        BorderRadius thumb_radius = BorderRadius::circular(thumb_size / 2.0f);

        Paint thumb_paint;
        thumb_paint.setStyle(PaintStyle::Fill);
        
        if (disabled) {
            thumb_paint.setColor(disabled_thumb_color);
        } else if (value) {
            thumb_paint.setColor(active_thumb_color);
            thumb_paint.setShadow(0x40000000, 4.0f, 0.0f, 2.0f); // Add soft shadow
        } else {
            thumb_paint.setColor(inactive_thumb_color);
            thumb_paint.setShadow(0x30000000, 4.0f, 0.0f, 2.0f); // Add soft shadow
        }

        context.canvas.drawRRect(thumb_rect, thumb_radius, thumb_paint);
    }
};

// ════════════════════════════════════════════════════════════════
// Internal SwitchRenderWidget
// ════════════════════════════════════════════════════════════════

class SwitchRenderWidget : public SingleChildRenderObjectWidget {
public:
    bool value;
    bool hovered;
    const SwitchWidget* options;

    SwitchRenderWidget(bool val, bool hov, const SwitchWidget* opt)
        : value(val), hovered(hov), options(opt) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderSwitch>(value, hovered, options);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        auto& renderSwitch = static_cast<RenderSwitch&>(renderObject);
        renderSwitch.update(value, hovered, options);
    }
    
    [[nodiscard]] std::string_view typeName() const override { return "SwitchRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// SwitchState
// ════════════════════════════════════════════════════════════════

class SwitchState : public State {
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const SwitchWidget*>(widget());

        auto render_widget = std::make_shared<SwitchRenderWidget>(w->value, hovered_, w);

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

std::unique_ptr<State> SwitchWidget::createState() {
    return std::make_unique<SwitchState>();
}

} // namespace enki
