/// @file custom_multi_child_layout.cpp
/// @brief Implementation of CustomMultiChildLayout and MultiChildLayoutDelegate.

#include "enki/widgets/custom_multi_child_layout.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderLayoutId Implementation
// ════════════════════════════════════════════════════════════════

class RenderLayoutId : public RenderBox {
public:
    explicit RenderLayoutId(std::string id) : id_(std::move(id)) {
        if (anu_node_) {
            ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
            ANUNodeStyleSetAlignItems(anu_node_, ANUAlignStretch);
            ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexStart);
        }
    }

    ~RenderLayoutId() override = default;

    [[nodiscard]] const std::string& id() const { return id_; }
    void setId(std::string id) {
        if (id_ != id) {
            id_ = std::move(id);
            markNeedsLayout();
        }
    }

    void syncLayout() override {
        float new_w = ANUNodeLayoutGetWidth(anu_node_);
        float new_h = ANUNodeLayoutGetHeight(anu_node_);
        if (new_w > 0.0f || new_h > 0.0f) {
            size_.width  = new_w;
            size_.height = new_h;
        }

        needs_layout_ = false;

        for (auto* child : children_) {
            if (child) {
                child->syncLayout();
                if (size_.width <= 0.0f && child->size().width > 0.0f) {
                    size_.width = child->size().width;
                }
                if (size_.height <= 0.0f && child->size().height > 0.0f) {
                    size_.height = child->size().height;
                }
            }
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
        return RenderObject::hitTestChildren(result, localPoint);
    }

    bool hitTestSelf(Point /*localPoint*/) const override {
        // LayoutId is purely a layout metadata wrapper; hits pass through to children
        return false;
    }

private:
    std::string id_;
};

// ════════════════════════════════════════════════════════════════
// RenderCustomMultiChildLayout Implementation
// ════════════════════════════════════════════════════════════════

class RenderCustomMultiChildLayout : public RenderBox {
public:
    explicit RenderCustomMultiChildLayout(std::shared_ptr<MultiChildLayoutDelegate> delegate = nullptr)
        : delegate_(std::move(delegate)) {
        if (anu_node_) {
            ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeRelative);
        }
    }

    ~RenderCustomMultiChildLayout() override = default;

    void setDelegate(std::shared_ptr<MultiChildLayoutDelegate> delegate) {
        if (delegate_ == delegate) return;
        if (!delegate_ || !delegate || delegate->shouldRelayout(*delegate_)) {
            markNeedsLayout();
        }
        delegate_ = std::move(delegate);
    }

    [[nodiscard]] const std::shared_ptr<MultiChildLayoutDelegate>& delegate() const { return delegate_; }

    [[nodiscard]] bool hasChild(std::string_view id) const {
        return findChild(id) != nullptr;
    }

    Size layoutChild(std::string_view id, const BoxConstraints& constraints) {
        auto* child = findChild(id);
        if (!child) {
            std::cerr << "[CustomMultiChildLayout] Warning: child with id '" << id << "' not found." << std::endl;
            return Size{0.0f, 0.0f};
        }

        ANUNodeRef child_node = child->anuNode();
        float avail_w = constraints.hasBoundedWidth() ? constraints.max_width : NAN;
        float avail_h = constraints.hasBoundedHeight() ? constraints.max_height : NAN;

        if (child_node) {
            if (constraints.isTight()) {
                ANUNodeStyleSetWidth(child_node, constraints.max_width);
                ANUNodeStyleSetHeight(child_node, constraints.max_height);
                ANUNodeStyleSetMinWidth(child_node, constraints.min_width);
                ANUNodeStyleSetMaxWidth(child_node, constraints.max_width);
                ANUNodeStyleSetMinHeight(child_node, constraints.min_height);
                ANUNodeStyleSetMaxHeight(child_node, constraints.max_height);
            } else {
                if (constraints.min_width > 0.0f) {
                    ANUNodeStyleSetMinWidth(child_node, constraints.min_width);
                } else {
                    ANUNodeStyleSetMinWidth(child_node, ANUUndefined);
                }
                if (constraints.hasBoundedWidth()) {
                    ANUNodeStyleSetMaxWidth(child_node, constraints.max_width);
                } else {
                    ANUNodeStyleSetMaxWidth(child_node, ANUUndefined);
                }

                if (constraints.min_height > 0.0f) {
                    ANUNodeStyleSetMinHeight(child_node, constraints.min_height);
                } else {
                    ANUNodeStyleSetMinHeight(child_node, ANUUndefined);
                }
                if (constraints.hasBoundedHeight()) {
                    ANUNodeStyleSetMaxHeight(child_node, constraints.max_height);
                } else {
                    ANUNodeStyleSetMaxHeight(child_node, ANUUndefined);
                }

                ANUValue cur_w = ANUNodeStyleGetWidth(child_node);
                if (cur_w.unit == ANUUnitUndefined) {
                    ANUNodeStyleSetWidthAuto(child_node);
                }
                ANUValue cur_h = ANUNodeStyleGetHeight(child_node);
                if (cur_h.unit == ANUUnitUndefined) {
                    ANUNodeStyleSetHeightAuto(child_node);
                }
            }

            ANUNodeCalculateLayout(child_node, avail_w, avail_h, ANUDirectionLTR);
        }

        child->syncLayout();
        Size sz = constraints.constrain(child->size());
        child->setSize(sz);
        return sz;
    }

    void positionChild(std::string_view id, Point offset) {
        auto* child = findChild(id);
        if (!child) {
            std::cerr << "[CustomMultiChildLayout] Warning: positionChild called on missing id '" << id << "'." << std::endl;
            return;
        }

        child->setOffset(offset);
        if (child->anuNode()) {
            ANUNodeStyleSetPosition(child->anuNode(), ANUEdgeLeft, offset.x);
            ANUNodeStyleSetPosition(child->anuNode(), ANUEdgeTop, offset.y);
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
            delegate_->layout_ = this;
            Size computed_size = delegate_->getSize(incoming_constraints);
            if (computed_size.width > 0.0f || computed_size.height > 0.0f) {
                size_ = computed_size;
            } else {
                size_ = {new_w > 0.0f ? new_w : 0.0f, new_h > 0.0f ? new_h : 0.0f};
            }

            delegate_->performLayout(size_);
        } else {
            size_ = {new_w > 0.0f ? new_w : 0.0f, new_h > 0.0f ? new_h : 0.0f};
            for (auto* child : children_) {
                if (child) {
                    child->syncLayout();
                }
            }
        }

        needs_layout_ = false;
        markNeedsPaint();
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
        for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
            RenderObject* child = *it;
            if (!child) continue;

            Point childLocalPoint = {
                localPoint.x - child->offset().x,
                localPoint.y - child->offset().y
            };

            if (child->hitTest(result, childLocalPoint)) {
                return true;
            }
        }
        return false;
    }

private:
    RenderBox* findChild(std::string_view id) const {
        for (auto* child : children_) {
            if (!child) continue;
            if (auto* lid = dynamic_cast<RenderLayoutId*>(child)) {
                if (lid->id() == id) {
                    return lid;
                }
            }
            if (child->ownerElement() && child->ownerElement()->widget()) {
                const auto& k = child->ownerElement()->widget()->key;
                if (k.type() == Key::Type::String && k.stringValue() == id) {
                    return dynamic_cast<RenderBox*>(child);
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<MultiChildLayoutDelegate> delegate_;
};

// ════════════════════════════════════════════════════════════════
// MultiChildLayoutDelegate Method Implementations
// ════════════════════════════════════════════════════════════════

bool MultiChildLayoutDelegate::hasChild(std::string_view id) const {
    return layout_ ? layout_->hasChild(id) : false;
}

Size MultiChildLayoutDelegate::layoutChild(std::string_view id, const BoxConstraints& constraints) {
    return layout_ ? layout_->layoutChild(id, constraints) : Size{0.0f, 0.0f};
}

void MultiChildLayoutDelegate::positionChild(std::string_view id, Point offset) {
    if (layout_) {
        layout_->positionChild(id, offset);
    }
}

// ════════════════════════════════════════════════════════════════
// LayoutIdWidget RenderObject Creation & Update
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> LayoutIdWidget::createRenderObject(BuildContext& /*ctx*/) {
    return std::make_unique<RenderLayoutId>(id);
}

void LayoutIdWidget::updateRenderObject(BuildContext& /*ctx*/, RenderObject& renderObject) {
    if (auto* r = dynamic_cast<RenderLayoutId*>(&renderObject)) {
        r->setId(id);
    }
}

// ════════════════════════════════════════════════════════════════
// CustomMultiChildLayoutWidget RenderObject Creation & Update
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> CustomMultiChildLayoutWidget::createRenderObject(BuildContext& /*ctx*/) {
    return std::make_unique<RenderCustomMultiChildLayout>(delegate);
}

void CustomMultiChildLayoutWidget::updateRenderObject(BuildContext& /*ctx*/, RenderObject& renderObject) {
    if (auto* r = dynamic_cast<RenderCustomMultiChildLayout*>(&renderObject)) {
        r->setDelegate(delegate);
    }
}

} // namespace enki
