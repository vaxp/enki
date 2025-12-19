/// @file limited_box.cpp
/// @brief Implementation of LimitedBox widget and RenderLimitedBox.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/limited_box.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/rendering/canvas.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderLimitedBox Implementation
// ════════════════════════════════════════════════════════════════

class RenderLimitedBox : public RenderBox {
public:
    std::optional<float> max_width_;
    std::optional<float> max_height_;

    RenderLimitedBox(std::optional<float> max_w, std::optional<float> max_h)
        : max_width_(max_w), max_height_(max_h) {
        applyStyleToNode();
    }

    void setMaxWidth(std::optional<float> max_w) {
        if (max_width_ != max_w) {
            max_width_ = max_w;
            applyConstraintsToChild();
            markNeedsLayout();
        }
    }

    void setMaxHeight(std::optional<float> max_h) {
        if (max_height_ != max_h) {
            max_height_ = max_h;
            applyConstraintsToChild();
            markNeedsLayout();
        }
    }

    void applyConstraintsToChild() {
        if (children_.empty() || !children_[0]) return;
        auto* child = children_[0];
        ANUNodeRef child_node = child->anuNode();
        if (!child_node) return;

        bool unconstrained_h = isHeightUnconstrained();
        bool unconstrained_w = isWidthUnconstrained();

        if (unconstrained_h && max_height_.has_value() && *max_height_ > 0.0f) {
            ANUValue cur = ANUNodeStyleGetMaxHeight(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *max_height_) {
                ANUNodeStyleSetMaxHeight(child_node, *max_height_);
            }
        } else {
            ANUValue cur = ANUNodeStyleGetMaxHeight(child_node);
            if (cur.unit == ANUUnitPoint) {
                ANUNodeStyleSetMaxHeight(child_node, NAN);
            }
        }

        if (unconstrained_w && max_width_.has_value() && *max_width_ > 0.0f) {
            ANUValue cur = ANUNodeStyleGetMaxWidth(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *max_width_) {
                ANUNodeStyleSetMaxWidth(child_node, *max_width_);
            }
        } else {
            ANUValue cur = ANUNodeStyleGetMaxWidth(child_node);
            if (cur.unit == ANUUnitPoint) {
                ANUNodeStyleSetMaxWidth(child_node, NAN);
            }
        }
    }

    void applyStyleToNode() {
        if (!anu_node_) return;
        // LimitedBox defaults to auto size and content pass-through
        ANUNodeStyleSetWidthAuto(anu_node_);
        ANUNodeStyleSetHeightAuto(anu_node_);
        ANUNodeStyleSetFlexShrink(anu_node_, 1.0f);
        ANUNodeStyleSetFlexGrow(anu_node_, 0.0f);
    }

    /// @brief Determine if incoming height constraint from parent is unconstrained (unbounded).
    [[nodiscard]] bool isHeightUnconstrained() const {
        // 1. Check parent RenderObject
        if (!parent_) {
            // Root node without parent: bounded by window layout size
            return false;
        }

        // Inside a vertical ScrollView: height is explicitly unconstrained
        if (const auto* sv = dynamic_cast<const RenderScrollView*>(parent_)) {
            if (sv->options.direction == Axis::Vertical) {
                return true;
            }
        }

        // 2. Inspect parent Anu layout node
        ANUNodeRef p_node = ANUNodeGetParent(anu_node_);
        if (p_node) {
            // If parent has scroll overflow in Column direction: height is unconstrained
            if (ANUNodeStyleGetOverflow(p_node) == ANUOverflowScroll) {
                ANUFlexDirection dir = ANUNodeStyleGetFlexDirection(p_node);
                if (dir == ANUFlexDirectionColumn || dir == ANUFlexDirectionColumnReverse) {
                    return true;
                }
            }

            // Check if parent has no finite height constraint
            ANUValue ph = ANUNodeStyleGetHeight(p_node);
            ANUValue pmh = ANUNodeStyleGetMaxHeight(p_node);
            ANUFlexDirection dir = ANUNodeStyleGetFlexDirection(p_node);

            bool is_col_main = (dir == ANUFlexDirectionColumn || dir == ANUFlexDirectionColumnReverse);
            if (is_col_main && (ph.unit == ANUUnitAuto || ph.unit == ANUUnitUndefined) && pmh.unit == ANUUnitUndefined) {
                return true;
            }
        }

        return false;
    }

    /// @brief Determine if incoming width constraint from parent is unconstrained (unbounded).
    [[nodiscard]] bool isWidthUnconstrained() const {
        if (!parent_) {
            return false;
        }

        // Inside a horizontal ScrollView: width is explicitly unconstrained
        if (const auto* sv = dynamic_cast<const RenderScrollView*>(parent_)) {
            if (sv->options.direction == Axis::Horizontal) {
                return true;
            }
        }

        // 2. Inspect parent Anu layout node
        ANUNodeRef p_node = ANUNodeGetParent(anu_node_);
        if (p_node) {
            // If parent has scroll overflow in Row direction: width is unconstrained
            if (ANUNodeStyleGetOverflow(p_node) == ANUOverflowScroll) {
                ANUFlexDirection dir = ANUNodeStyleGetFlexDirection(p_node);
                if (dir == ANUFlexDirectionRow || dir == ANUFlexDirectionRowReverse) {
                    return true;
                }
            }

            // Check if parent has no finite width constraint
            ANUValue pw = ANUNodeStyleGetWidth(p_node);
            ANUValue pmw = ANUNodeStyleGetMaxWidth(p_node);
            ANUFlexDirection dir = ANUNodeStyleGetFlexDirection(p_node);

            bool is_row_main = (dir == ANUFlexDirectionRow || dir == ANUFlexDirectionRowReverse);
            if (is_row_main && (pw.unit == ANUUnitAuto || pw.unit == ANUUnitUndefined) && pmw.unit == ANUUnitUndefined) {
                return true;
            }
        }

        return false;
    }

    void syncLayout() override {
        bool unconstrained_h = isHeightUnconstrained();
        bool unconstrained_w = isWidthUnconstrained();

        if (children_.empty() || !children_[0]) {
            RenderBox::syncLayout();
            if (unconstrained_w && max_width_.has_value() && size_.width > *max_width_) {
                size_.width = *max_width_;
            }
            if (unconstrained_h && max_height_.has_value() && size_.height > *max_height_) {
                size_.height = *max_height_;
            }
            return;
        }

        auto* child = children_[0];
        applyConstraintsToChild();

        // Normal sync layout pass down the tree
        RenderBox::syncLayout();

        if (child) {
            Size cs = child->size();
            if (unconstrained_w && max_width_.has_value() && cs.width > *max_width_) {
                cs.width = *max_width_;
            }
            if (unconstrained_h && max_height_.has_value() && cs.height > *max_height_) {
                cs.height = *max_height_;
            }
            child->setSize(cs);
            size_ = cs;
        } else {
            Size s = size_;
            if (unconstrained_w && max_width_.has_value() && s.width > *max_width_) {
                s.width = *max_width_;
            }
            if (unconstrained_h && max_height_.has_value() && s.height > *max_height_) {
                s.height = *max_height_;
            }
            size_ = s;
        }
    }

    void paint(PaintContext& context) override {
        if (children_.empty() || !children_[0]) return;
        auto* child = children_[0];
        PaintContext child_ctx = context.withOffset(child->offset());
        child->paint(child_ctx);
    }
};

std::unique_ptr<RenderObject> LimitedBoxWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderLimitedBox>(max_width, max_height);
}

void LimitedBoxWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* ro = dynamic_cast<RenderLimitedBox*>(&renderObject)) {
        ro->setMaxWidth(max_width);
        ro->setMaxHeight(max_height);
    }
}

} // namespace enki
