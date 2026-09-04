/// @file intrinsic_width.cpp
/// @brief Implementation of IntrinsicWidth widget and RenderIntrinsicWidth.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/intrinsic_width.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderIntrinsicWidth Implementation
// ════════════════════════════════════════════════════════════════

class RenderIntrinsicWidth : public RenderBox {
public:
    std::optional<float> step_width_;
    std::optional<float> step_height_;

    RenderIntrinsicWidth(std::optional<float> step_w, std::optional<float> step_h)
        : step_width_(step_w), step_height_(step_h) {
        applyStyleToNode();
    }

    void setStepWidth(std::optional<float> step_w) {
        if (step_width_ != step_w) {
            step_width_ = step_w;
            markNeedsLayout();
        }
    }

    void setStepHeight(std::optional<float> step_h) {
        if (step_height_ != step_h) {
            step_height_ = step_h;
            markNeedsLayout();
        }
    }

    void applyStyleToNode() {
        if (!anu_node_) return;
        // IntrinsicWidth sizes to its content width, without stretching to parent width
        ANUNodeStyleSetAlignSelf(anu_node_, ANUAlignFlexStart);
        ANUNodeStyleSetWidthAuto(anu_node_);
        ANUNodeStyleSetFlexGrow(anu_node_, 0.0f);
        ANUNodeStyleSetFlexShrink(anu_node_, 1.0f);
    }

    void syncLayout() override {
        if (children_.empty() || !children_[0]) {
            RenderBox::syncLayout();
            return;
        }

        auto* child = children_[0];
        ANUNodeRef child_node = child->anuNode();

        // 1. Measure natural intrinsic width
        float natural_w = 0.0f;
        if (child_node) {
            ANUValue orig_w = ANUNodeStyleGetWidth(child_node);
            if (orig_w.unit == ANUUnitPoint && orig_w.value > 0.0f) {
                natural_w = orig_w.value;
            } else {
                ANUNodeCalculateLayout(child_node, NAN, NAN, ANUDirectionLTR);
                natural_w = ANUNodeLayoutGetWidth(child_node);
            }
        }
        if (natural_w <= 0.0f) {
            natural_w = child->size().width;
        }

        float target_w = natural_w;
        if (step_width_.has_value() && *step_width_ > 0.0f) {
            float step = *step_width_;
            target_w = std::ceil(target_w / step) * step;
        }

        // 2. Set computed width on child node and lay it out so stretched items match target_w
        if (child_node) {
            ANUNodeStyleSetWidth(child_node, target_w);
            ANUNodeCalculateLayout(child_node, target_w, NAN, ANUDirectionLTR);
        }

        // 3. Normal sync layout down the tree
        RenderBox::syncLayout();

        // 4. Force container bounds to reflect intrinsic width and child height
        float ch = child->size().height;
        float target_h = ch;
        if (step_height_.has_value() && *step_height_ > 0.0f) {
            float step = *step_height_;
            target_h = std::ceil(target_h / step) * step;
        }

        size_.width = target_w;
        size_.height = target_h;
        child->setSize({target_w, ch});
    }

    void paint(PaintContext& context) override {
        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }
    }

    bool hitTestChildren(HitTestResult& result, Point localPoint) override {
        return RenderBox::hitTestChildren(result, localPoint);
    }
};

std::unique_ptr<RenderObject> IntrinsicWidthWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderIntrinsicWidth>(step_width, step_height);
}

void IntrinsicWidthWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rw = dynamic_cast<RenderIntrinsicWidth*>(&renderObject)) {
        rw->setStepWidth(step_width);
        rw->setStepHeight(step_height);
    }
}

} // namespace enki
