/// @file stack.cpp
/// @brief Stack and Positioned layout widgets implementation.

#include "enki/widgets/stack.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderPositioned Implementation
// ════════════════════════════════════════════════════════════════

RenderPositioned::RenderPositioned() {
    applyStyleToNode();
}

RenderPositioned::RenderPositioned(PositionedStyle style) : style_(std::move(style)) {
    applyStyleToNode();
}

void RenderPositioned::setStyle(const PositionedStyle& style) {
    if (style_ == style) return;
    style_ = style;
    applyStyleToNode();
    markNeedsLayout();
}

void RenderPositioned::applyStyleToNode() {
    if (!anu_node_) return;

    // Positioned children are ALWAYS absolutely positioned in Anu
    ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);

    auto applyEdge = [this](ANUEdge edge, const std::optional<StyleValue>& val) {
        if (!val.has_value()) {
            ANUNodeStyleSetPosition(anu_node_, edge, ANUUndefined);
        } else if (val->isPercent()) {
            ANUNodeStyleSetPositionPercent(anu_node_, edge, val->value);
        } else if (val->isPoint()) {
            ANUNodeStyleSetPosition(anu_node_, edge, val->value);
        } else {
            ANUNodeStyleSetPosition(anu_node_, edge, ANUUndefined);
        }
    };

    applyEdge(ANUEdgeTop,    style_.top);
    applyEdge(ANUEdgeRight,  style_.right);
    applyEdge(ANUEdgeBottom, style_.bottom);
    applyEdge(ANUEdgeLeft,   style_.left);
    applyEdge(ANUEdgeStart,  style_.start);
    applyEdge(ANUEdgeEnd,    style_.end);

    // Dimensions
    if (style_.width.has_value()) {
        if (style_.width->isPercent()) {
            ANUNodeStyleSetWidthPercent(anu_node_, style_.width->value);
        } else if (style_.width->isPoint()) {
            ANUNodeStyleSetWidth(anu_node_, style_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }
    } else {
        ANUNodeStyleSetWidthAuto(anu_node_);
    }

    if (style_.height.has_value()) {
        if (style_.height->isPercent()) {
            ANUNodeStyleSetHeightPercent(anu_node_, style_.height->value);
        } else if (style_.height->isPoint()) {
            ANUNodeStyleSetHeight(anu_node_, style_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }
    } else {
        ANUNodeStyleSetHeightAuto(anu_node_);
    }
}

void RenderPositioned::paint(PaintContext& context) {
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }
}

bool RenderPositioned::hitTestChildren(HitTestResult& result, Point localPoint) {
    return RenderObject::hitTestChildren(result, localPoint);
}

// ════════════════════════════════════════════════════════════════
// Positioned Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Positioned::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderPositioned>(style);
}

void Positioned::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rp = dynamic_cast<RenderPositioned*>(&renderObject)) {
        rp->setStyle(style);
    }
}

// ════════════════════════════════════════════════════════════════
// RenderStack Implementation
// ════════════════════════════════════════════════════════════════

RenderStack::RenderStack() {
    applyStyleToNode();
}

RenderStack::RenderStack(StackStyle style) : style_(std::move(style)) {
    applyStyleToNode();
}

void RenderStack::setStyle(const StackStyle& style) {
    if (style_ == style) return;
    style_ = style;
    applyStyleToNode();
    markNeedsLayout();
}

void RenderStack::applyStyleToNode() {
    if (!anu_node_) return;

    // Stack is the reference container for absolutely positioned children
    ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeRelative);

    // Dimensions
    if (style_.width.has_value()) {
        if (style_.width->isPercent()) {
            ANUNodeStyleSetWidthPercent(anu_node_, style_.width->value);
        } else if (style_.width->isPoint()) {
            ANUNodeStyleSetWidth(anu_node_, style_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }
    } else if (style_.fit == StackFit::Expand) {
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    } else {
        ANUNodeStyleSetWidthAuto(anu_node_);
    }

    if (style_.height.has_value()) {
        if (style_.height->isPercent()) {
            ANUNodeStyleSetHeightPercent(anu_node_, style_.height->value);
        } else if (style_.height->isPoint()) {
            ANUNodeStyleSetHeight(anu_node_, style_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }
    } else if (style_.fit == StackFit::Expand) {
        ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
    } else {
        ANUNodeStyleSetHeightAuto(anu_node_);
    }

    if (style_.min_width.has_value() && style_.min_width->isPoint()) {
        ANUNodeStyleSetMinWidth(anu_node_, style_.min_width->value);
    } else {
        ANUNodeStyleSetMinWidth(anu_node_, ANUUndefined);
    }

    if (style_.min_height.has_value() && style_.min_height->isPoint()) {
        ANUNodeStyleSetMinHeight(anu_node_, style_.min_height->value);
    } else {
        ANUNodeStyleSetMinHeight(anu_node_, ANUUndefined);
    }

    if (style_.max_width.has_value() && style_.max_width->isPoint()) {
        ANUNodeStyleSetMaxWidth(anu_node_, style_.max_width->value);
    } else {
        ANUNodeStyleSetMaxWidth(anu_node_, ANUUndefined);
    }

    if (style_.max_height.has_value() && style_.max_height->isPoint()) {
        ANUNodeStyleSetMaxHeight(anu_node_, style_.max_height->value);
    } else {
        ANUNodeStyleSetMaxHeight(anu_node_, ANUUndefined);
    }

    // Alignment for non-positioned flow items
    if (style_.fit == StackFit::Expand) {
        ANUNodeStyleSetAlignItems(anu_node_, ANUAlignStretch);
        ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexStart);
    } else {
        switch (style_.alignment) {
            case Alignment::TopLeft:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexStart);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexStart);
                break;
            case Alignment::TopCenter:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexStart);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
                break;
            case Alignment::TopRight:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexStart);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexEnd);
                break;
            case Alignment::CenterLeft:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexStart);
                break;
            case Alignment::Center:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
                break;
            case Alignment::CenterRight:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyCenter);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexEnd);
                break;
            case Alignment::BottomLeft:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexEnd);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexStart);
                break;
            case Alignment::BottomCenter:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexEnd);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignCenter);
                break;
            case Alignment::BottomRight:
                ANUNodeStyleSetJustifyContent(anu_node_, ANUJustifyFlexEnd);
                ANUNodeStyleSetAlignItems(anu_node_, ANUAlignFlexEnd);
                break;
        }
    }
}

void RenderStack::paint(PaintContext& context) {
    bool clip = (style_.clip_behavior != Clip::None);
    if (clip) {
        context.canvas.save();
        context.canvas.clipRect(Rect::fromPointSize(context.offset, size_));
    }

    // Paint children in forward order (0 is background, N-1 is foreground)
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    if (clip) {
        context.canvas.restore();
    }
}

bool RenderStack::hitTestChildren(HitTestResult& result, Point localPoint) {
    // Hit-test in reverse order (N-1 is foreground, tested first)
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

// ════════════════════════════════════════════════════════════════
// Stack Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Stack::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderStack>(style);
}

void Stack::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rs = dynamic_cast<RenderStack*>(&renderObject)) {
        rs->setStyle(style);
    }
}

} // namespace enki
