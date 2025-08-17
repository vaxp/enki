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
    SwitchProps options;

    RenderSwitch(bool val, bool hov, SwitchProps opt) 
        : value(val), hovered(hov), options(std::move(opt)) {
        ANUNodeStyleSetWidth(anu_node_, options.width);
        ANUNodeStyleSetHeight(anu_node_, options.height);
    }

    void update(bool new_value, bool new_hovered, const SwitchProps& new_options) {
        if (options.width != new_options.width || options.height != new_options.height) {
            ANUNodeStyleSetWidth(anu_node_, new_options.width);
            ANUNodeStyleSetHeight(anu_node_, new_options.height);
            markNeedsLayout();
        }
        if (value != new_value || hovered != new_hovered || 
            options.active_color != new_options.active_color ||
            options.inactive_color != new_options.inactive_color) {
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
            options.width,
            options.height
        };

        BorderRadius radius = BorderRadius::circular(options.height / 2.0f);

        Paint paint;
        paint.setStyle(PaintStyle::Fill);
        
        // Draw track (background)
        if (options.disabled) {
            paint.setColor(options.disabled_color);
        } else if (value) {
            paint.setColor(hovered ? options.hover_color : options.active_color);
        } else {
            paint.setColor(hovered ? options.hover_inactive_color : options.inactive_color);
        }
        
        context.canvas.drawRRect(rect, radius, paint);

        // Draw thumb (circle)
        float thumb_size = options.height - (options.thumb_padding * 2.0f);
        float thumb_x = context.offset.x + options.thumb_padding;
        
        if (value) {
            // Move thumb to the right
            thumb_x = context.offset.x + options.width - thumb_size - options.thumb_padding;
        }

        Rect thumb_rect{
            thumb_x,
            context.offset.y + options.thumb_padding,
            thumb_size,
            thumb_size
        };

        BorderRadius thumb_radius = BorderRadius::circular(thumb_size / 2.0f);

        Paint thumb_paint;
        thumb_paint.setStyle(PaintStyle::Fill);
        
        if (options.disabled) {
            thumb_paint.setColor(options.disabled_thumb_color);
        } else if (value) {
            thumb_paint.setColor(options.active_thumb_color);
            thumb_paint.setShadow(0x40000000, 4.0f, 0.0f, 2.0f); // Add soft shadow
        } else {
            thumb_paint.setColor(options.inactive_thumb_color);
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
    SwitchProps options;

    SwitchRenderWidget(bool val, bool hov, SwitchProps opt)
        : value(val), hovered(hov), options(std::move(opt)) {}

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
        auto* w = static_cast<const Switch*>(widget());

        auto render_widget = std::make_shared<SwitchRenderWidget>(w->value, hovered_, w->options);

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

std::unique_ptr<State> Switch::createState() {
    return std::make_unique<SwitchState>();
}

} // namespace enki
