#include "enki/widgets/radio.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderRadio (Custom Drawing)
// ════════════════════════════════════════════════════════════════

class RenderRadio : public RenderBox {
public:
    bool is_selected;
    bool hovered;
    
    float size;
    float inner_size;
    float border_width;
    Color active_color;
    Color inactive_color;
    Color hover_color;
    Color bg_color;
    bool disabled;
    Color disabled_color;

    RenderRadio(bool sel, bool hov, const RadioWidget* options) 
        : is_selected(sel), hovered(hov),
          size(options->size),
          inner_size(options->inner_size),
          border_width(options->border_width),
          active_color(options->active_color),
          inactive_color(options->inactive_color),
          hover_color(options->hover_color),
          bg_color(options->bg_color),
          disabled(options->disabled),
          disabled_color(options->disabled_color) {
        ANUNodeStyleSetWidth(anu_node_, size);
        ANUNodeStyleSetHeight(anu_node_, size);
    }

    void update(bool new_sel, bool new_hovered, const RadioWidget* new_options) {
        if (size != new_options->size) {
            ANUNodeStyleSetWidth(anu_node_, new_options->size);
            ANUNodeStyleSetHeight(anu_node_, new_options->size);
            markNeedsLayout();
        }
        if (is_selected != new_sel || hovered != new_hovered || 
            active_color != new_options->active_color ||
            inactive_color != new_options->inactive_color) {
            markNeedsPaint();
        }
        is_selected = new_sel;
        hovered = new_hovered;
        
        size = new_options->size;
        inner_size = new_options->inner_size;
        border_width = new_options->border_width;
        active_color = new_options->active_color;
        inactive_color = new_options->inactive_color;
        hover_color = new_options->hover_color;
        bg_color = new_options->bg_color;
        disabled = new_options->disabled;
        disabled_color = new_options->disabled_color;
    }

    void paint(PaintContext& context) override {
        float center_x = context.offset.x + (size / 2.0f);
        float center_y = context.offset.y + (size / 2.0f);
        
        Point center{center_x, center_y};
        float outer_radius = size / 2.0f;

        // 1. Draw outer circle (background if any)
        if (bg_color != 0x00000000) {
            Paint bg_paint;
            bg_paint.setStyle(PaintStyle::Fill);
            bg_paint.setColor(bg_color);
            context.canvas.drawCircle(center, outer_radius, bg_paint);
        }

        // 2. Draw outer border
        Paint border_paint;
        border_paint.setStyle(PaintStyle::Stroke);
        border_paint.setStrokeWidth(border_width);
        
        if (disabled) {
            border_paint.setColor(disabled_color);
        } else if (is_selected) {
            border_paint.setColor(active_color);
        } else {
            border_paint.setColor(hovered ? hover_color : inactive_color);
        }
        
        // Draw the outer border circle with a radius adjusted by half the stroke width
        // so it fits exactly inside the `size` bounding box.
        float adjusted_outer_radius = outer_radius - (border_width / 2.0f);
        if (adjusted_outer_radius > 0) {
            context.canvas.drawCircle(center, adjusted_outer_radius, border_paint);
        }

        // 3. Draw inner selected circle
        if (is_selected) {
            Paint inner_paint;
            inner_paint.setStyle(PaintStyle::Fill);
            
            if (disabled) {
                inner_paint.setColor(disabled_color);
            } else {
                inner_paint.setColor(active_color);
            }

            float inner_rad = inner_size / 2.0f;
            if (inner_rad > 0) {
                context.canvas.drawCircle(center, inner_rad, inner_paint);
            }
        }
    }
};

// ════════════════════════════════════════════════════════════════
// Internal RadioRenderWidget
// ════════════════════════════════════════════════════════════════

class RadioRenderWidget : public SingleChildRenderObjectWidget {
public:
    bool is_selected;
    bool hovered;
    const RadioWidget* options;

    RadioRenderWidget(bool sel, bool hov, const RadioWidget* opt)
        : is_selected(sel), hovered(hov), options(opt) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderRadio>(is_selected, hovered, options);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        auto& renderRadio = static_cast<RenderRadio&>(renderObject);
        renderRadio.update(is_selected, hovered, options);
    }
    
    [[nodiscard]] std::string_view typeName() const override { return "RadioRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// RadioState
// ════════════════════════════════════════════════════════════════

class RadioState : public State {
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const RadioWidget*>(widget());

        bool is_selected = (w->value == w->group_value);
        auto render_widget = std::make_shared<RadioRenderWidget>(is_selected, hovered_, w);

        if (w->disabled) {
            return render_widget;
        }

        auto on_changed = w->on_changed;
        int value_to_send = w->value;

        return gestureDetector({
            .child = render_widget,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [on_changed, value_to_send, is_selected] {
                // Radio buttons usually only trigger if they are not already selected
                if (on_changed && !is_selected) {
                    on_changed(value_to_send);
                }
            },
            .on_hover_enter = [this](const PointerEvent&) { setState([this] { hovered_ = true; }); },
            .on_hover_exit  = [this](const PointerEvent&) { setState([this] { hovered_ = false; }); },
        });
    }
};

std::unique_ptr<State> RadioWidget::createState() {
    return std::make_unique<RadioState>();
}

} // namespace enki
