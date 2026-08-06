/// @file flexbox.cpp
/// @brief Comprehensive Flexbox widget implementation with direct Anu integration.

#include "enki/widgets/flexbox.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// applyFlexboxStyle
// ════════════════════════════════════════════════════════════════

void applyFlexboxStyle(ANUNodeRef node, const FlexboxStyle& style) {
    if (!node) return;

    // ── Direction & Flow ───────────────────────────────────────
    if (style.direction) {
        ANUNodeStyleSetDirection(node, static_cast<ANUDirection>(*style.direction));
    }
    if (style.flex_direction) {
        ANUNodeStyleSetFlexDirection(node, static_cast<ANUFlexDirection>(*style.flex_direction));
    }
    if (style.justify_content) {
        ANUNodeStyleSetJustifyContent(node, static_cast<ANUJustify>(*style.justify_content));
    }
    if (style.align_content) {
        ANUNodeStyleSetAlignContent(node, static_cast<ANUAlign>(*style.align_content));
    }
    if (style.align_items) {
        ANUNodeStyleSetAlignItems(node, static_cast<ANUAlign>(*style.align_items));
    }
    if (style.align_self) {
        ANUNodeStyleSetAlignSelf(node, static_cast<ANUAlign>(*style.align_self));
    }
    if (style.position_type) {
        ANUNodeStyleSetPositionType(node, static_cast<ANUPositionType>(*style.position_type));
    }
    if (style.flex_wrap) {
        ANUNodeStyleSetFlexWrap(node, static_cast<ANUWrap>(*style.flex_wrap));
    }
    if (style.overflow) {
        ANUNodeStyleSetOverflow(node, static_cast<ANUOverflow>(*style.overflow));
    }
    if (style.display) {
        ANUNodeStyleSetDisplay(node, static_cast<ANUDisplay>(*style.display));
    }
    if (style.box_sizing) {
        ANUNodeStyleSetBoxSizing(node, static_cast<ANUBoxSizing>(*style.box_sizing));
    }

    // ── Flex Factors ───────────────────────────────────────────
    if (style.flex) {
        ANUNodeStyleSetFlex(node, *style.flex);
    }
    if (style.flex_grow) {
        ANUNodeStyleSetFlexGrow(node, *style.flex_grow);
    }
    if (style.flex_shrink) {
        ANUNodeStyleSetFlexShrink(node, *style.flex_shrink);
    }
    if (style.flex_basis.isDefined()) {
        if (style.flex_basis.isPercent()) {
            ANUNodeStyleSetFlexBasisPercent(node, style.flex_basis.value);
        } else if (style.flex_basis.isPoint()) {
            ANUNodeStyleSetFlexBasis(node, style.flex_basis.value);
        } else if (style.flex_basis.isAuto()) {
            ANUNodeStyleSetFlexBasisAuto(node);
        }
    }

    // ── Gaps (Gutters) ─────────────────────────────────────────
    if (style.gap.isDefined()) {
        if (style.gap.isPercent()) {
            ANUNodeStyleSetGapPercent(node, ANUGutterAll, style.gap.value);
        } else {
            ANUNodeStyleSetGap(node, ANUGutterAll, style.gap.value);
        }
    }
    if (style.row_gap.isDefined()) {
        if (style.row_gap.isPercent()) {
            ANUNodeStyleSetGapPercent(node, ANUGutterRow, style.row_gap.value);
        } else {
            ANUNodeStyleSetGap(node, ANUGutterRow, style.row_gap.value);
        }
    }
    if (style.column_gap.isDefined()) {
        if (style.column_gap.isPercent()) {
            ANUNodeStyleSetGapPercent(node, ANUGutterColumn, style.column_gap.value);
        } else {
            ANUNodeStyleSetGap(node, ANUGutterColumn, style.column_gap.value);
        }
    }

    // ── Dimensions & Constraints ───────────────────────────────
    if (style.width.isDefined()) {
        if (style.width.isPercent()) {
            ANUNodeStyleSetWidthPercent(node, style.width.value);
        } else if (style.width.isPoint()) {
            ANUNodeStyleSetWidth(node, style.width.value);
        } else if (style.width.isAuto()) {
            ANUNodeStyleSetWidthAuto(node);
        }
    }
    if (style.height.isDefined()) {
        if (style.height.isPercent()) {
            ANUNodeStyleSetHeightPercent(node, style.height.value);
        } else if (style.height.isPoint()) {
            ANUNodeStyleSetHeight(node, style.height.value);
        } else if (style.height.isAuto()) {
            ANUNodeStyleSetHeightAuto(node);
        }
    }

    if (style.min_width.isDefined()) {
        if (style.min_width.isPercent()) {
            ANUNodeStyleSetMinWidthPercent(node, style.min_width.value);
        } else {
            ANUNodeStyleSetMinWidth(node, style.min_width.value);
        }
    }
    if (style.min_height.isDefined()) {
        if (style.min_height.isPercent()) {
            ANUNodeStyleSetMinHeightPercent(node, style.min_height.value);
        } else {
            ANUNodeStyleSetMinHeight(node, style.min_height.value);
        }
    }

    if (style.max_width.isDefined()) {
        if (style.max_width.isPercent()) {
            ANUNodeStyleSetMaxWidthPercent(node, style.max_width.value);
        } else {
            ANUNodeStyleSetMaxWidth(node, style.max_width.value);
        }
    }
    if (style.max_height.isDefined()) {
        if (style.max_height.isPercent()) {
            ANUNodeStyleSetMaxHeightPercent(node, style.max_height.value);
        } else {
            ANUNodeStyleSetMaxHeight(node, style.max_height.value);
        }
    }

    if (style.aspect_ratio) {
        ANUNodeStyleSetAspectRatio(node, *style.aspect_ratio);
    }

    // ── Inset Application Helper ───────────────────────────────
    auto applyEdge = [](ANUNodeRef n, ANUEdge edge, const StyleValue& val,
                        auto fnPoint, auto fnPercent, auto fnAuto) {
        if (!val.isDefined()) return;
        if (val.isPercent()) {
            fnPercent(n, edge, val.value);
        } else if (val.isPoint()) {
            fnPoint(n, edge, val.value);
        } else if (val.isAuto()) {
            fnAuto(n, edge);
        }
    };

    // ── Margin ─────────────────────────────────────────────────
    applyEdge(node, ANUEdgeTop, style.margin.top, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);
    applyEdge(node, ANUEdgeRight, style.margin.right, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);
    applyEdge(node, ANUEdgeBottom, style.margin.bottom, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);
    applyEdge(node, ANUEdgeLeft, style.margin.left, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);
    applyEdge(node, ANUEdgeStart, style.margin.start, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);
    applyEdge(node, ANUEdgeEnd, style.margin.end, ANUNodeStyleSetMargin, ANUNodeStyleSetMarginPercent, ANUNodeStyleSetMarginAuto);

    // ── Padding ────────────────────────────────────────────────
    auto dummyAuto = [](ANUNodeRef, ANUEdge) {};
    applyEdge(node, ANUEdgeTop, style.padding.top, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);
    applyEdge(node, ANUEdgeRight, style.padding.right, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);
    applyEdge(node, ANUEdgeBottom, style.padding.bottom, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);
    applyEdge(node, ANUEdgeLeft, style.padding.left, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);
    applyEdge(node, ANUEdgeStart, style.padding.start, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);
    applyEdge(node, ANUEdgeEnd, style.padding.end, ANUNodeStyleSetPadding, ANUNodeStyleSetPaddingPercent, dummyAuto);

    // ── Position / Insets ──────────────────────────────────────
    applyEdge(node, ANUEdgeTop, style.position.top, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);
    applyEdge(node, ANUEdgeRight, style.position.right, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);
    applyEdge(node, ANUEdgeBottom, style.position.bottom, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);
    applyEdge(node, ANUEdgeLeft, style.position.left, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);
    applyEdge(node, ANUEdgeStart, style.position.start, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);
    applyEdge(node, ANUEdgeEnd, style.position.end, ANUNodeStyleSetPosition, ANUNodeStyleSetPositionPercent, ANUNodeStyleSetPositionAuto);

    // ── Border Widths ──────────────────────────────────────────
    if (style.border.all > 0.0f) {
        ANUNodeStyleSetBorder(node, ANUEdgeAll, style.border.all);
    } else {
        if (style.border.top > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeTop, style.border.top);
        if (style.border.right > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeRight, style.border.right);
        if (style.border.bottom > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeBottom, style.border.bottom);
        if (style.border.left > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeLeft, style.border.left);
        if (style.border.start > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeStart, style.border.start);
        if (style.border.end > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeEnd, style.border.end);
        if (style.border.horizontal > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeHorizontal, style.border.horizontal);
        if (style.border.vertical > 0.0f) ANUNodeStyleSetBorder(node, ANUEdgeVertical, style.border.vertical);
    }
}

// ════════════════════════════════════════════════════════════════
// RenderFlex Implementation
// ════════════════════════════════════════════════════════════════

RenderFlex::RenderFlex(FlexboxStyle style) : style_(std::move(style)) {
    applyFlexboxStyle(anu_node_, style_);
}

void RenderFlex::setStyle(const FlexboxStyle& style) {
    style_ = style;
    applyFlexboxStyle(anu_node_, style_);
    markNeedsLayout();
}

void RenderFlex::paint(PaintContext& context) {
    bool clip = (style_.overflow && *style_.overflow == Overflow::Hidden);
    if (clip) {
        context.canvas.save();
        context.canvas.clipRect(Rect::fromPointSize(context.offset, size_));
    }

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

bool RenderFlex::hitTestChildren(HitTestResult& result, Point localPoint) {
    return RenderObject::hitTestChildren(result, localPoint);
}

// ════════════════════════════════════════════════════════════════
// RenderFlexItem Implementation
// ════════════════════════════════════════════════════════════════

RenderFlexItem::RenderFlexItem(FlexboxStyle style) : style_(std::move(style)) {
    applyFlexboxStyle(anu_node_, style_);
}

void RenderFlexItem::setStyle(const FlexboxStyle& style) {
    style_ = style;
    applyFlexboxStyle(anu_node_, style_);
    markNeedsLayout();
}

void RenderFlexItem::paint(PaintContext& context) {
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }
}

bool RenderFlexItem::hitTestChildren(HitTestResult& result, Point localPoint) {
    return RenderObject::hitTestChildren(result, localPoint);
}

// ════════════════════════════════════════════════════════════════
// Flexbox Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Flexbox::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderFlex>(style);
}

void Flexbox::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rf = dynamic_cast<RenderFlex*>(&renderObject)) {
        rf->setStyle(style);
    }
}

// ════════════════════════════════════════════════════════════════
// FlexItem Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> FlexItem::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderFlexItem>(style);
}

void FlexItem::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rfi = dynamic_cast<RenderFlexItem*>(&renderObject)) {
        rfi->setStyle(style);
    }
}

} // namespace enki
