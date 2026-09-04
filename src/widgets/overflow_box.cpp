/// @file overflow_box.cpp
/// @brief Implementation of OverflowBox widget and RenderOverflowBox.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/overflow_box.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

namespace {

float alignRatioX(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::CenterLeft:
        case Alignment::BottomLeft:
            return 0.0f;
        case Alignment::TopCenter:
        case Alignment::Center:
        case Alignment::BottomCenter:
            return 0.5f;
        case Alignment::TopRight:
        case Alignment::CenterRight:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

float alignRatioY(Alignment a) {
    switch (a) {
        case Alignment::TopLeft:
        case Alignment::TopCenter:
        case Alignment::TopRight:
            return 0.0f;
        case Alignment::CenterLeft:
        case Alignment::Center:
        case Alignment::CenterRight:
            return 0.5f;
        case Alignment::BottomLeft:
        case Alignment::BottomCenter:
        case Alignment::BottomRight:
            return 1.0f;
    }
    return 0.5f;
}

} // namespace

// ════════════════════════════════════════════════════════════════
// RenderOverflowBox Implementation
// ════════════════════════════════════════════════════════════════

class RenderOverflowBox : public RenderBox {
public:
    Alignment            alignment_;
    std::optional<float> min_width_;
    std::optional<float> max_width_;
    std::optional<float> min_height_;
    std::optional<float> max_height_;
    Clip                 clip_behavior_;

    RenderOverflowBox(Alignment align,
                      std::optional<float> min_w, std::optional<float> max_w,
                      std::optional<float> min_h, std::optional<float> max_h,
                      Clip clip)
        : alignment_(align),
          min_width_(min_w), max_width_(max_w),
          min_height_(min_h), max_height_(max_h),
          clip_behavior_(clip) {
        applyStyleToNode();
    }

    void setAlignment(Alignment align) {
        if (alignment_ != align) {
            alignment_ = align;
            markNeedsLayout();
        }
    }

    void setMinWidth(std::optional<float> min_w) {
        if (min_width_ != min_w) {
            min_width_ = min_w;
            markNeedsLayout();
        }
    }

    void setMaxWidth(std::optional<float> max_w) {
        if (max_width_ != max_w) {
            max_width_ = max_w;
            markNeedsLayout();
        }
    }

    void setMinHeight(std::optional<float> min_h) {
        if (min_height_ != min_h) {
            min_height_ = min_h;
            markNeedsLayout();
        }
    }

    void setMaxHeight(std::optional<float> max_h) {
        if (max_height_ != max_h) {
            max_height_ = max_h;
            markNeedsLayout();
        }
    }

    void setClipBehavior(Clip clip) {
        if (clip_behavior_ != clip) {
            clip_behavior_ = clip;
            markNeedsPaint();
        }
    }

    void applyStyleToNode() {
        if (!anu_node_) return;
        ANUNodeStyleSetOverflow(anu_node_, ANUOverflowVisible);
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
    }

    void applyChildConstraints() {
        if (children_.empty() || !children_[0]) return;
        ANUNodeRef child_node = children_[0]->anuNode();
        if (!child_node) return;

        // Ensure child does not get auto-shrunk by Flexbox when overflowing
        if (ANUNodeStyleGetFlexShrink(child_node) != 0.0f) {
            ANUNodeStyleSetFlexShrink(child_node, 0.0f);
        }

        if (min_width_.has_value()) {
            ANUValue cur = ANUNodeStyleGetMinWidth(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *min_width_) {
                ANUNodeStyleSetMinWidth(child_node, *min_width_);
            }
        }
        if (max_width_.has_value()) {
            ANUValue cur = ANUNodeStyleGetMaxWidth(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *max_width_) {
                ANUNodeStyleSetMaxWidth(child_node, *max_width_);
            }
        }
        if (min_width_.has_value() && max_width_.has_value() && *min_width_ == *max_width_) {
            ANUValue cur = ANUNodeStyleGetWidth(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *min_width_) {
                ANUNodeStyleSetWidth(child_node, *min_width_);
            }
        }

        if (min_height_.has_value()) {
            ANUValue cur = ANUNodeStyleGetMinHeight(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *min_height_) {
                ANUNodeStyleSetMinHeight(child_node, *min_height_);
            }
        }
        if (max_height_.has_value()) {
            ANUValue cur = ANUNodeStyleGetMaxHeight(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *max_height_) {
                ANUNodeStyleSetMaxHeight(child_node, *max_height_);
            }
        }
        if (min_height_.has_value() && max_height_.has_value() && *min_height_ == *max_height_) {
            ANUValue cur = ANUNodeStyleGetHeight(child_node);
            if (cur.unit != ANUUnitPoint || cur.value != *min_height_) {
                ANUNodeStyleSetHeight(child_node, *min_height_);
            }
        }
    }

    void syncLayout() override {
        applyChildConstraints();

        // 1. Sync size and offset of this box from Anu
        RenderBox::syncLayout();

        if (children_.empty() || !children_[0]) return;
        auto* child = children_[0];

        // 2. Adjust child size if min/max constraints were explicitly specified
        Size c_size = child->size();
        if (min_width_.has_value() && c_size.width < *min_width_)   c_size.width = *min_width_;
        if (max_width_.has_value() && c_size.width > *max_width_)   c_size.width = *max_width_;
        if (min_height_.has_value() && c_size.height < *min_height_) c_size.height = *min_height_;
        if (max_height_.has_value() && c_size.height > *max_height_) c_size.height = *max_height_;

        if (c_size.width != child->size().width || c_size.height != child->size().height) {
            child->setSize(c_size);
        }

        // 3. Compute alignment offset
        float dx = size_.width  - child->size().width;
        float dy = size_.height - child->size().height;

        float rx = alignRatioX(alignment_);
        float ry = alignRatioY(alignment_);

        Point offset = { dx * rx, dy * ry };
        child->setOffset(offset);
    }

    void paint(PaintContext& context) override {
        if (children_.empty() || !children_[0]) return;
        auto* child = children_[0];

        if (clip_behavior_ != Clip::None && size_.width > 0.0f && size_.height > 0.0f) {
            Rect bounds = Rect::fromPointSize(context.offset, size_);
            context.canvas.save();
            context.canvas.clipRect(bounds);

            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);

            context.canvas.restore();
        } else {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (clip_behavior_ != Clip::None) {
            if (localPoint.x < 0.0f || localPoint.x > size_.width ||
                localPoint.y < 0.0f || localPoint.y > size_.height) {
                return false;
            }
        }
        return RenderBox::hitTest(result, localPoint);
    }
};

std::unique_ptr<RenderObject> OverflowBoxWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderOverflowBox>(
        alignment, min_width, max_width, min_height, max_height, clip_behavior
    );
}

void OverflowBoxWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* ro = dynamic_cast<RenderOverflowBox*>(&renderObject)) {
        ro->setAlignment(alignment);
        ro->setMinWidth(min_width);
        ro->setMaxWidth(max_width);
        ro->setMinHeight(min_height);
        ro->setMaxHeight(max_height);
        ro->setClipBehavior(clip_behavior);
    }
}

} // namespace enki
