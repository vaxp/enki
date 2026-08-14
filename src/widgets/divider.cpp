#include "enki/widgets/divider.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDivider (Horizontal)
// ════════════════════════════════════════════════════════════════

class RenderDivider : public RenderBox {
public:
    DividerOptions options;

    RenderDivider(DividerOptions opt) : options(std::move(opt)) {
        // Set layout constraints
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        ANUNodeStyleSetHeight(anu_node_, options.height);
    }

    void updateOptions(const DividerOptions& new_options) {
        if (options.height != new_options.height) {
            ANUNodeStyleSetHeight(anu_node_, new_options.height);
            markNeedsLayout();
        }
        if (options.thickness != new_options.thickness ||
            options.indent != new_options.indent ||
            options.end_indent != new_options.end_indent ||
            options.color != new_options.color) {
            markNeedsPaint();
        }
        options = new_options;
    }

    void paint(PaintContext& context) override {
        if (options.color == 0x00000000 || options.thickness <= 0.0f) return;

        Paint paint;
        paint.setColor(options.color);
        paint.setStyle(PaintStyle::Fill);

        float center_y = size_.height / 2.0f;
        float top = center_y - (options.thickness / 2.0f);
        float bottom = center_y + (options.thickness / 2.0f);
        float left = options.indent;
        float right = size_.width - options.end_indent;

        if (left >= right) return;

        Rect rect{
            context.offset.x + left,
            context.offset.y + top,
            right - left,
            options.thickness
        };

        context.canvas.drawRect(rect, paint);
    }
};

// ════════════════════════════════════════════════════════════════
// RenderVerticalDivider
// ════════════════════════════════════════════════════════════════

class RenderVerticalDivider : public RenderBox {
public:
    DividerOptions options;

    RenderVerticalDivider(DividerOptions opt) : options(std::move(opt)) {
        // Set layout constraints
        ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
        ANUNodeStyleSetWidth(anu_node_, options.height); // Here 'height' means total width occupied
    }

    void updateOptions(const DividerOptions& new_options) {
        if (options.height != new_options.height) {
            ANUNodeStyleSetWidth(anu_node_, new_options.height);
            markNeedsLayout();
        }
        if (options.thickness != new_options.thickness ||
            options.indent != new_options.indent ||
            options.end_indent != new_options.end_indent ||
            options.color != new_options.color) {
            markNeedsPaint();
        }
        options = new_options;
    }

    void paint(PaintContext& context) override {
        if (options.color == 0x00000000 || options.thickness <= 0.0f) return;

        Paint paint;
        paint.setColor(options.color);
        paint.setStyle(PaintStyle::Fill);

        float center_x = size_.width / 2.0f;
        float left = center_x - (options.thickness / 2.0f);
        float top = options.indent;
        float bottom = size_.height - options.end_indent;

        if (top >= bottom) return;

        Rect rect{
            context.offset.x + left,
            context.offset.y + top,
            options.thickness,
            bottom - top
        };

        context.canvas.drawRect(rect, paint);
    }
};

// ════════════════════════════════════════════════════════════════
// Widget Implementations
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Divider::createRenderObject(BuildContext&) {
    return std::make_unique<RenderDivider>(options);
}

void Divider::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& renderDivider = static_cast<RenderDivider&>(renderObject);
    renderDivider.updateOptions(options);
}

std::unique_ptr<RenderObject> VerticalDivider::createRenderObject(BuildContext&) {
    return std::make_unique<RenderVerticalDivider>(options);
}

void VerticalDivider::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& renderVerticalDivider = static_cast<RenderVerticalDivider&>(renderObject);
    renderVerticalDivider.updateOptions(options);
}

} // namespace enki
