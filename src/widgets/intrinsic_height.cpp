/// @file intrinsic_height.cpp
/// @brief Implementation of IntrinsicHeight widget and RenderIntrinsicHeight.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/intrinsic_height.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderIntrinsicHeight Implementation
// ════════════════════════════════════════════════════════════════

class RenderIntrinsicHeight : public RenderBox {
public:
    std::optional<float> step_height_;
    std::optional<float> step_width_;

    RenderIntrinsicHeight(std::optional<float> step_h, std::optional<float> step_w)
        : step_height_(step_h), step_width_(step_w) {
        applyStyleToNode();
    }

    void setStepHeight(std::optional<float> step_h) {
        if (step_height_ != step_h) {
            step_height_ = step_h;
            markNeedsLayout();
        }
    }

    void setStepWidth(std::optional<float> step_w) {
        if (step_width_ != step_w) {
            step_width_ = step_w;
            markNeedsLayout();
        }
    }

    void applyStyleToNode() {
        if (!anu_node_) return;
        ANUNodeStyleSetHeightAuto(anu_node_);
    }

    void syncLayout() override {
        if (children_.empty() || !children_[0]) {
            RenderBox::syncLayout();
            return;
        }

        auto* child = children_[0];
        ANUNodeRef child_node = child->anuNode();

        // 1. Let parent layout establish size_.width from Anu
        RenderBox::syncLayout();

        // Check if child has an explicit fixed width
        bool child_has_fixed_width = false;
        float child_fixed_w = 0.0f;
        if (child_node) {
            ANUValue orig_w = ANUNodeStyleGetWidth(child_node);
            if (orig_w.unit == ANUUnitPoint && orig_w.value > 0.0f) {
                child_has_fixed_width = true;
                child_fixed_w = orig_w.value;
            }
        }

        // Available width for measuring height
        float avail_w = child_has_fixed_width ? child_fixed_w : (size_.width > 0.0f ? size_.width : NAN);

        // 2. Measure natural intrinsic height using current resolved width
        float natural_h = 0.0f;
        ANUValue orig_h = {0.0f, ANUUnitAuto};
        if (child_node) {
            orig_h = ANUNodeStyleGetHeight(child_node);
            if (orig_h.unit == ANUUnitPoint && orig_h.value > 0.0f) {
                natural_h = orig_h.value;
            } else {
                ANUNodeCalculateLayout(child_node, avail_w, NAN, ANUDirectionLTR);
                natural_h = ANUNodeLayoutGetHeight(child_node);
            }
        }
        if (natural_h <= 0.0f) {
            natural_h = child->size().height;
        }

        float target_h = natural_h;
        if (step_height_.has_value() && *step_height_ > 0.0f) {
            float step = *step_height_;
            target_h = std::ceil(target_h / step) * step;
        }

        // 3. Set computed height on child node and lay it out so stretched row items match target_h
        if (child_node) {
            ANUNodeStyleSetHeight(child_node, target_h);
            ANUNodeCalculateLayout(child_node, avail_w, target_h, ANUDirectionLTR);
            child->syncLayout();

            // Restore original child style height so future frames measure fresh dynamic content
            if (orig_h.unit == ANUUnitPoint) {
                ANUNodeStyleSetHeight(child_node, orig_h.value);
            } else if (orig_h.unit == ANUUnitPercent) {
                ANUNodeStyleSetHeightPercent(child_node, orig_h.value);
            } else {
                ANUNodeStyleSetHeightAuto(child_node);
            }
        }

        // 4. Ensure our bounds reflect target_h and appropriate width
        size_.height = target_h;
        if (child_has_fixed_width) {
            size_.width = child_fixed_w;
            child->setSize({child_fixed_w, target_h});
        } else {
            child->setSize({child->size().width, target_h});
        }

        if (step_width_.has_value() && *step_width_ > 0.0f) {
            float step = *step_width_;
            size_.width = std::ceil(size_.width / step) * step;
            child->setSize({size_.width, target_h});
        }
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

std::unique_ptr<RenderObject> IntrinsicHeightWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderIntrinsicHeight>(step_height, step_width);
}

void IntrinsicHeightWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rh = dynamic_cast<RenderIntrinsicHeight*>(&renderObject)) {
        rh->setStepHeight(step_height);
        rh->setStepWidth(step_width);
    }
}

} // namespace enki
