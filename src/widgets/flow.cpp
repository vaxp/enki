/// @file flow.cpp
/// @brief Implementation of Flow widget, RenderFlow, and FlowPaintingContext.

#include "enki/widgets/flow.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderFlow Declaration
// ════════════════════════════════════════════════════════════════

class RenderFlow : public RenderBox {
public:
    explicit RenderFlow(std::shared_ptr<FlowDelegate> delegate = nullptr)
        : delegate_(std::move(delegate)) {
        if (anu_node_) {
            ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeRelative);
        }
    }

    ~RenderFlow() override = default;

    void setDelegate(std::shared_ptr<FlowDelegate> delegate) {
        if (delegate_ == delegate) return;
        if (!delegate_ || !delegate || delegate->shouldRelayout(*delegate_)) {
            markNeedsLayout();
        } else if (delegate->shouldRepaint(*delegate_)) {
            markNeedsPaint();
        }
        delegate_ = std::move(delegate);
    }

    [[nodiscard]] const std::shared_ptr<FlowDelegate>& delegate() const { return delegate_; }

    [[nodiscard]] const std::vector<Size>& childSizes() const { return child_sizes_; }
    [[nodiscard]] Size childSize(size_t index) const {
        if (index < child_sizes_.size()) {
            return child_sizes_[index];
        }
        return Size{0.0f, 0.0f};
    }

    void recordChildTransform(size_t index, const Matrix4& transform, float opacity) {
        if (index < child_transforms_.size()) {
            child_transforms_[index] = transform;
            child_opacities_[index]  = opacity;
            child_painted_[index]    = true;
        }
    }

    void syncLayout() override {
        float new_w = ANUNodeLayoutGetWidth(anu_node_);
        float new_h = ANUNodeLayoutGetHeight(anu_node_);
        float new_x = ANUNodeLayoutGetLeft(anu_node_);
        float new_y = ANUNodeLayoutGetTop(anu_node_);

        offset_.x = new_x;
        offset_.y = new_y;

        BoxConstraints incoming_constraints;
        if (new_w > 0.0f && new_h > 0.0f) {
            incoming_constraints = BoxConstraints::loose({new_w, new_h});
        } else if (new_w > 0.0f) {
            incoming_constraints = BoxConstraints{0.0f, new_w, 0.0f, std::numeric_limits<float>::infinity()};
        } else if (new_h > 0.0f) {
            incoming_constraints = BoxConstraints{0.0f, std::numeric_limits<float>::infinity(), 0.0f, new_h};
        }

        if (delegate_) {
            Size computed_size = delegate_->getSize(incoming_constraints);
            if (computed_size.width > 0.0f || computed_size.height > 0.0f) {
                size_ = computed_size;
            } else {
                size_ = {new_w > 0.0f ? new_w : 0.0f, new_h > 0.0f ? new_h : 0.0f};
            }
        } else {
            size_ = {new_w > 0.0f ? new_w : 0.0f, new_h > 0.0f ? new_h : 0.0f};
        }

        child_sizes_.resize(children_.size(), Size{0.0f, 0.0f});
        child_transforms_.resize(children_.size(), Matrix4::identity());
        child_opacities_.resize(children_.size(), 1.0f);
        child_painted_.resize(children_.size(), false);

        for (size_t i = 0; i < children_.size(); ++i) {
            RenderObject* child = children_[i];
            if (!child) continue;

            BoxConstraints child_constraints = delegate_
                ? delegate_->getConstraintsForChild(i, incoming_constraints)
                : BoxConstraints{0.0f, incoming_constraints.max_width, 0.0f, incoming_constraints.max_height};

            ANUNodeRef child_node = child->anuNode();
            if (child_node) {
                float avail_w = child_constraints.hasBoundedWidth() ? child_constraints.max_width : NAN;
                float avail_h = child_constraints.hasBoundedHeight() ? child_constraints.max_height : NAN;

                if (child_constraints.isTight()) {
                    ANUNodeStyleSetWidth(child_node, child_constraints.max_width);
                    ANUNodeStyleSetHeight(child_node, child_constraints.max_height);
                    ANUNodeStyleSetMinWidth(child_node, child_constraints.min_width);
                    ANUNodeStyleSetMaxWidth(child_node, child_constraints.max_width);
                    ANUNodeStyleSetMinHeight(child_node, child_constraints.min_height);
                    ANUNodeStyleSetMaxHeight(child_node, child_constraints.max_height);
                } else {
                    if (child_constraints.min_width > 0.0f) {
                        ANUNodeStyleSetMinWidth(child_node, child_constraints.min_width);
                    } else {
                        ANUNodeStyleSetMinWidth(child_node, ANUUndefined);
                    }
                    if (child_constraints.hasBoundedWidth()) {
                        ANUNodeStyleSetMaxWidth(child_node, child_constraints.max_width);
                    } else {
                        ANUNodeStyleSetMaxWidth(child_node, ANUUndefined);
                    }

                    if (child_constraints.min_height > 0.0f) {
                        ANUNodeStyleSetMinHeight(child_node, child_constraints.min_height);
                    } else {
                        ANUNodeStyleSetMinHeight(child_node, ANUUndefined);
                    }
                    if (child_constraints.hasBoundedHeight()) {
                        ANUNodeStyleSetMaxHeight(child_node, child_constraints.max_height);
                    } else {
                        ANUNodeStyleSetMaxHeight(child_node, ANUUndefined);
                    }
                }

                ANUNodeCalculateLayout(child_node, avail_w, avail_h, ANUDirectionLTR);
            }

            child->syncLayout();
            Size sz = child_constraints.constrain(child->size());
            child->setSize(sz);
            child_sizes_[i] = sz;
        }

        needs_layout_ = false;
        markNeedsPaint();
    }

    void paint(PaintContext& context) override {
        // Reset painted flags and transforms
        std::fill(child_painted_.begin(), child_painted_.end(), false);
        std::fill(child_transforms_.begin(), child_transforms_.end(), Matrix4::identity());
        std::fill(child_opacities_.begin(), child_opacities_.end(), 1.0f);

        FlowPaintingContext flow_context(*this, context);

        if (delegate_) {
            delegate_->paintChildren(flow_context);
        } else {
            // Default painting if no delegate is provided
            for (size_t i = 0; i < children_.size(); ++i) {
                flow_context.paintChild(i);
            }
        }
    }

    bool hitTestChildren(HitTestResult& result, Point localPoint) override {
        // Test children in reverse paint order (topmost painted child receives event first)
        for (int i = static_cast<int>(children_.size()) - 1; i >= 0; --i) {
            RenderObject* child = children_[static_cast<size_t>(i)];
            if (!child) continue;

            // Only test children that were actually painted
            if (static_cast<size_t>(i) < child_painted_.size() && !child_painted_[static_cast<size_t>(i)]) {
                continue;
            }

            // Map local point back to child's local coordinates via inverse transform
            const Matrix4& trans = child_transforms_[static_cast<size_t>(i)];
            Point childLocalPoint = trans.mapPointInverse(localPoint);

            Size sz = child_sizes_[static_cast<size_t>(i)];
            Rect childBounds{0.0f, 0.0f, sz.width, sz.height};

            if (childBounds.contains(childLocalPoint)) {
                if (child->hitTest(result, childLocalPoint)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool hitTestSelf(Point /*localPoint*/) const override {
        return false;
    }

private:
    friend class FlowPaintingContext;

    std::shared_ptr<FlowDelegate> delegate_;
    std::vector<Size>             child_sizes_;
    std::vector<Matrix4>          child_transforms_;
    std::vector<float>            child_opacities_;
    std::vector<bool>             child_painted_;
};

// ════════════════════════════════════════════════════════════════
// FlowPaintingContext Implementation
// ════════════════════════════════════════════════════════════════

FlowPaintingContext::FlowPaintingContext(RenderFlow& flow, PaintContext& paint_context)
    : flow_(flow), paint_context_(paint_context) {}

Size FlowPaintingContext::size() const {
    return flow_.size();
}

size_t FlowPaintingContext::childCount() const {
    return flow_.children().size();
}

Size FlowPaintingContext::getChildSize(size_t index) const {
    return flow_.childSize(index);
}

void FlowPaintingContext::paintChild(size_t index, Point offset, float opacity) {
    paintChild(index, Matrix4::translation(offset.x, offset.y), opacity);
}

void FlowPaintingContext::paintChild(size_t index, const Matrix4& transform, float opacity) {
    if (index >= flow_.children().size()) {
        std::cerr << "[Flow] Error: index " << index << " out of bounds (childCount="
                  << flow_.children().size() << ")" << std::endl;
        return;
    }

    RenderObject* child = flow_.children()[index];
    if (!child) return;

    flow_.recordChildTransform(index, transform, opacity);

    Canvas& canvas = paint_context_.canvas;
    canvas.save();

    // Translate to Flow's top-left origin on the canvas
    canvas.translate(paint_context_.offset.x, paint_context_.offset.y);

    // Apply the child's 3x3/4x4 transformation matrix
    float sk_matrix[9];
    transform.toSkMatrix9(sk_matrix);
    canvas.concat(sk_matrix);

    // Apply opacity via offscreen layer if needed
    bool has_alpha = (opacity < 0.999f && opacity >= 0.0f);
    if (has_alpha) {
        Size sz = flow_.childSize(index);
        Rect bounds{0.0f, 0.0f, sz.width > 0.0f ? sz.width : 10000.0f, sz.height > 0.0f ? sz.height : 10000.0f};
        canvas.saveLayerAlpha(opacity, &bounds);
    }

    // Paint the child in its local coordinate space (0, 0)
    PaintContext child_ctx{canvas, Point{0.0f, 0.0f}, paint_context_.clip_rect, paint_context_.opacity};
    child->paint(child_ctx);

    if (has_alpha) {
        canvas.restore();
    }

    canvas.restore();
}

void FlowPaintingContext::paintChild(size_t index, const float matrix9[9], float opacity) {
    Matrix4 m;
    m.storage[0]  = matrix9[0]; // scaleX
    m.storage[4]  = matrix9[1]; // skewX
    m.storage[12] = matrix9[2]; // transX
    m.storage[1]  = matrix9[3]; // skewY
    m.storage[5]  = matrix9[4]; // scaleY
    m.storage[13] = matrix9[5]; // transY
    m.storage[3]  = matrix9[6]; // persp0
    m.storage[7]  = matrix9[7]; // persp1
    m.storage[15] = matrix9[8]; // persp2
    paintChild(index, m, opacity);
}

// ════════════════════════════════════════════════════════════════
// FlowWidget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> FlowWidget::createRenderObject(BuildContext& /*ctx*/) {
    return std::make_unique<RenderFlow>(delegate);
}

void FlowWidget::updateRenderObject(BuildContext& /*ctx*/, RenderObject& renderObject) {
    auto& rf = static_cast<RenderFlow&>(renderObject);
    rf.setDelegate(delegate);
}

} // namespace enki
