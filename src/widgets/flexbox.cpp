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
    ANUNodeStyleSetDirection(node, style.direction ? static_cast<ANUDirection>(*style.direction) : ANUDirectionInherit);
    ANUNodeStyleSetFlexDirection(node, style.flex_direction ? static_cast<ANUFlexDirection>(*style.flex_direction) : ANUFlexDirectionColumn);
    ANUNodeStyleSetJustifyContent(node, style.justify_content ? static_cast<ANUJustify>(*style.justify_content) : ANUJustifyFlexStart);
    ANUNodeStyleSetAlignContent(node, style.align_content ? static_cast<ANUAlign>(*style.align_content) : ANUAlignFlexStart);
    ANUNodeStyleSetAlignItems(node, style.align_items ? static_cast<ANUAlign>(*style.align_items) : ANUAlignStretch);
    ANUNodeStyleSetAlignSelf(node, style.align_self ? static_cast<ANUAlign>(*style.align_self) : ANUAlignAuto);
    ANUNodeStyleSetPositionType(node, style.position_type ? static_cast<ANUPositionType>(*style.position_type) : ANUPositionTypeStatic);
    ANUNodeStyleSetFlexWrap(node, style.flex_wrap ? static_cast<ANUWrap>(*style.flex_wrap) : ANUWrapNoWrap);
    ANUNodeStyleSetOverflow(node, style.overflow ? static_cast<ANUOverflow>(*style.overflow) : ANUOverflowVisible);
    ANUNodeStyleSetDisplay(node, style.display ? static_cast<ANUDisplay>(*style.display) : ANUDisplayFlex);
    ANUNodeStyleSetBoxSizing(node, style.box_sizing ? static_cast<ANUBoxSizing>(*style.box_sizing) : ANUBoxSizingBorderBox);

    // ── Flex Factors ───────────────────────────────────────────
    ANUNodeStyleSetFlex(node, style.flex ? *style.flex : ANUUndefined);
    ANUNodeStyleSetFlexGrow(node, style.flex_grow ? *style.flex_grow : ANUUndefined);
    ANUNodeStyleSetFlexShrink(node, style.flex_shrink ? *style.flex_shrink : ANUUndefined);

    if (style.flex_basis.isPercent()) {
        ANUNodeStyleSetFlexBasisPercent(node, style.flex_basis.value);
    } else if (style.flex_basis.isPoint()) {
        ANUNodeStyleSetFlexBasis(node, style.flex_basis.value);
    } else {
        ANUNodeStyleSetFlexBasisAuto(node);
    }

    // ── Gaps (Gutters) ─────────────────────────────────────────
    if (style.gap.isPercent()) {
        ANUNodeStyleSetGapPercent(node, ANUGutterAll, style.gap.value);
    } else if (style.gap.isPoint()) {
        ANUNodeStyleSetGap(node, ANUGutterAll, style.gap.value);
    } else {
        ANUNodeStyleSetGap(node, ANUGutterAll, ANUUndefined);
    }

    if (style.row_gap.isPercent()) {
        ANUNodeStyleSetGapPercent(node, ANUGutterRow, style.row_gap.value);
    } else if (style.row_gap.isPoint()) {
        ANUNodeStyleSetGap(node, ANUGutterRow, style.row_gap.value);
    } else {
        ANUNodeStyleSetGap(node, ANUGutterRow, ANUUndefined);
    }

    if (style.column_gap.isPercent()) {
        ANUNodeStyleSetGapPercent(node, ANUGutterColumn, style.column_gap.value);
    } else if (style.column_gap.isPoint()) {
        ANUNodeStyleSetGap(node, ANUGutterColumn, style.column_gap.value);
    } else {
        ANUNodeStyleSetGap(node, ANUGutterColumn, ANUUndefined);
    }

    // ── Dimensions & Constraints ───────────────────────────────
    if (style.width.isPercent()) {
        ANUNodeStyleSetWidthPercent(node, style.width.value);
    } else if (style.width.isPoint()) {
        ANUNodeStyleSetWidth(node, style.width.value);
    } else {
        ANUNodeStyleSetWidthAuto(node);
    }

    if (style.height.isPercent()) {
        ANUNodeStyleSetHeightPercent(node, style.height.value);
    } else if (style.height.isPoint()) {
        ANUNodeStyleSetHeight(node, style.height.value);
    } else {
        ANUNodeStyleSetHeightAuto(node);
    }

    if (style.min_width.isPercent()) {
        ANUNodeStyleSetMinWidthPercent(node, style.min_width.value);
    } else if (style.min_width.isPoint()) {
        ANUNodeStyleSetMinWidth(node, style.min_width.value);
    } else {
        ANUNodeStyleSetMinWidth(node, ANUUndefined);
    }

    if (style.min_height.isPercent()) {
        ANUNodeStyleSetMinHeightPercent(node, style.min_height.value);
    } else if (style.min_height.isPoint()) {
        ANUNodeStyleSetMinHeight(node, style.min_height.value);
    } else {
        ANUNodeStyleSetMinHeight(node, ANUUndefined);
    }

    if (style.max_width.isPercent()) {
        ANUNodeStyleSetMaxWidthPercent(node, style.max_width.value);
    } else if (style.max_width.isPoint()) {
        ANUNodeStyleSetMaxWidth(node, style.max_width.value);
    } else {
        ANUNodeStyleSetMaxWidth(node, ANUUndefined);
    }

    if (style.max_height.isPercent()) {
        ANUNodeStyleSetMaxHeightPercent(node, style.max_height.value);
    } else if (style.max_height.isPoint()) {
        ANUNodeStyleSetMaxHeight(node, style.max_height.value);
    } else {
        ANUNodeStyleSetMaxHeight(node, ANUUndefined);
    }

    ANUNodeStyleSetAspectRatio(node, style.aspect_ratio ? *style.aspect_ratio : ANUUndefined);

    // ── Margin ─────────────────────────────────────────────────
    auto applyMarginEdge = [](ANUNodeRef n, ANUEdge edge, const StyleValue& val) {
        if (val.isPercent()) {
            ANUNodeStyleSetMarginPercent(n, edge, val.value);
        } else if (val.isPoint()) {
            ANUNodeStyleSetMargin(n, edge, val.value);
        } else if (val.isAuto()) {
            ANUNodeStyleSetMarginAuto(n, edge);
        } else {
            ANUNodeStyleSetMargin(n, edge, ANUUndefined);
        }
    };
    applyMarginEdge(node, ANUEdgeTop, style.margin.top);
    applyMarginEdge(node, ANUEdgeRight, style.margin.right);
    applyMarginEdge(node, ANUEdgeBottom, style.margin.bottom);
    applyMarginEdge(node, ANUEdgeLeft, style.margin.left);
    applyMarginEdge(node, ANUEdgeStart, style.margin.start);
    applyMarginEdge(node, ANUEdgeEnd, style.margin.end);

    // ── Padding ────────────────────────────────────────────────
    auto applyPaddingEdge = [](ANUNodeRef n, ANUEdge edge, const StyleValue& val) {
        if (val.isPercent()) {
            ANUNodeStyleSetPaddingPercent(n, edge, val.value);
        } else if (val.isPoint()) {
            ANUNodeStyleSetPadding(n, edge, val.value);
        } else {
            ANUNodeStyleSetPadding(n, edge, ANUUndefined);
        }
    };
    applyPaddingEdge(node, ANUEdgeTop, style.padding.top);
    applyPaddingEdge(node, ANUEdgeRight, style.padding.right);
    applyPaddingEdge(node, ANUEdgeBottom, style.padding.bottom);
    applyPaddingEdge(node, ANUEdgeLeft, style.padding.left);
    applyPaddingEdge(node, ANUEdgeStart, style.padding.start);
    applyPaddingEdge(node, ANUEdgeEnd, style.padding.end);

    // ── Position / Insets ──────────────────────────────────────
    auto applyPositionEdge = [](ANUNodeRef n, ANUEdge edge, const StyleValue& val) {
        if (val.isPercent()) {
            ANUNodeStyleSetPositionPercent(n, edge, val.value);
        } else if (val.isPoint()) {
            ANUNodeStyleSetPosition(n, edge, val.value);
        } else if (val.isAuto()) {
            ANUNodeStyleSetPositionAuto(n, edge);
        } else {
            ANUNodeStyleSetPosition(n, edge, ANUUndefined);
        }
    };
    applyPositionEdge(node, ANUEdgeTop, style.position.top);
    applyPositionEdge(node, ANUEdgeRight, style.position.right);
    applyPositionEdge(node, ANUEdgeBottom, style.position.bottom);
    applyPositionEdge(node, ANUEdgeLeft, style.position.left);
    applyPositionEdge(node, ANUEdgeStart, style.position.start);
    applyPositionEdge(node, ANUEdgeEnd, style.position.end);

    // ── Border Widths ──────────────────────────────────────────
    if (style.border.all > 0.0f) {
        ANUNodeStyleSetBorder(node, ANUEdgeAll, style.border.all);
    } else {
        ANUNodeStyleSetBorder(node, ANUEdgeAll, ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeTop, style.border.top > 0.0f ? style.border.top : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeRight, style.border.right > 0.0f ? style.border.right : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeBottom, style.border.bottom > 0.0f ? style.border.bottom : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeLeft, style.border.left > 0.0f ? style.border.left : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeStart, style.border.start > 0.0f ? style.border.start : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeEnd, style.border.end > 0.0f ? style.border.end : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeHorizontal, style.border.horizontal > 0.0f ? style.border.horizontal : ANUUndefined);
        ANUNodeStyleSetBorder(node, ANUEdgeVertical, style.border.vertical > 0.0f ? style.border.vertical : ANUUndefined);
    }
}

// ════════════════════════════════════════════════════════════════
// RenderFlex Implementation
// ════════════════════════════════════════════════════════════════

RenderFlex::RenderFlex(FlexboxStyle style) : style_(std::move(style)) {
    applyFlexboxStyle(anu_node_, style_);
}

void RenderFlex::setStyle(const FlexboxStyle& style) {
    if (style_ == style) return;
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
    if (style_ == style) return;
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
