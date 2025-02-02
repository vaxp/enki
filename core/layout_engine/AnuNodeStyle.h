/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stddef.h>

#include <layout_engine/AnuNode.h>
#include <layout_engine/AnuValue.h>

ANU_EXTERN_C_BEGIN

ANU_EXPORT void ANUNodeCopyStyle(ANUNodeRef dstNode, ANUNodeConstRef srcNode);

ANU_EXPORT void ANUNodeStyleSetDirection(ANUNodeRef node, ANUDirection direction);
ANU_EXPORT ANUDirection ANUNodeStyleGetDirection(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlexDirection(
    ANUNodeRef node,
    ANUFlexDirection flexDirection);
ANU_EXPORT ANUFlexDirection ANUNodeStyleGetFlexDirection(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetJustifyContent(
    ANUNodeRef node,
    ANUJustify justifyContent);
ANU_EXPORT ANUJustify ANUNodeStyleGetJustifyContent(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetAlignContent(ANUNodeRef node, ANUAlign alignContent);
ANU_EXPORT ANUAlign ANUNodeStyleGetAlignContent(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetAlignItems(ANUNodeRef node, ANUAlign alignItems);
ANU_EXPORT ANUAlign ANUNodeStyleGetAlignItems(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetAlignSelf(ANUNodeRef node, ANUAlign alignSelf);
ANU_EXPORT ANUAlign ANUNodeStyleGetAlignSelf(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetPositionType(
    ANUNodeRef node,
    ANUPositionType positionType);
ANU_EXPORT ANUPositionType ANUNodeStyleGetPositionType(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlexWrap(ANUNodeRef node, ANUWrap flexWrap);
ANU_EXPORT ANUWrap ANUNodeStyleGetFlexWrap(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetOverflow(ANUNodeRef node, ANUOverflow overflow);
ANU_EXPORT ANUOverflow ANUNodeStyleGetOverflow(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetDisplay(ANUNodeRef node, ANUDisplay display);
ANU_EXPORT ANUDisplay ANUNodeStyleGetDisplay(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlex(ANUNodeRef node, float flex);
ANU_EXPORT float ANUNodeStyleGetFlex(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlexGrow(ANUNodeRef node, float flexGrow);
ANU_EXPORT float ANUNodeStyleGetFlexGrow(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlexShrink(ANUNodeRef node, float flexShrink);
ANU_EXPORT float ANUNodeStyleGetFlexShrink(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetFlexBasis(ANUNodeRef node, float flexBasis);
ANU_EXPORT void ANUNodeStyleSetFlexBasisPercent(ANUNodeRef node, float flexBasis);
ANU_EXPORT void ANUNodeStyleSetFlexBasisAuto(ANUNodeRef node);
ANU_EXPORT ANUValue ANUNodeStyleGetFlexBasis(ANUNodeConstRef node);

ANU_EXPORT void
ANUNodeStyleSetPosition(ANUNodeRef node, ANUEdge edge, float position);
ANU_EXPORT void
ANUNodeStyleSetPositionPercent(ANUNodeRef node, ANUEdge edge, float position);
ANU_EXPORT ANUValue ANUNodeStyleGetPosition(ANUNodeConstRef node, ANUEdge edge);
ANU_EXPORT void ANUNodeStyleSetPositionAuto(ANUNodeRef node, ANUEdge edge);

ANU_EXPORT
void ANUNodeStyleSetMargin(ANUNodeRef node, ANUEdge edge, float margin);
ANU_EXPORT void
ANUNodeStyleSetMarginPercent(ANUNodeRef node, ANUEdge edge, float margin);
ANU_EXPORT void ANUNodeStyleSetMarginAuto(ANUNodeRef node, ANUEdge edge);
ANU_EXPORT ANUValue ANUNodeStyleGetMargin(ANUNodeConstRef node, ANUEdge edge);

ANU_EXPORT void
ANUNodeStyleSetPadding(ANUNodeRef node, ANUEdge edge, float padding);
ANU_EXPORT void
ANUNodeStyleSetPaddingPercent(ANUNodeRef node, ANUEdge edge, float padding);
ANU_EXPORT ANUValue ANUNodeStyleGetPadding(ANUNodeConstRef node, ANUEdge edge);

ANU_EXPORT void ANUNodeStyleSetBorder(ANUNodeRef node, ANUEdge edge, float border);
ANU_EXPORT float ANUNodeStyleGetBorder(ANUNodeConstRef node, ANUEdge edge);

ANU_EXPORT void
ANUNodeStyleSetGap(ANUNodeRef node, ANUGutter gutter, float gapLength);
ANU_EXPORT void
ANUNodeStyleSetGapPercent(ANUNodeRef node, ANUGutter gutter, float gapLength);
ANU_EXPORT float ANUNodeStyleGetGap(ANUNodeConstRef node, ANUGutter gutter);

ANU_EXPORT void ANUNodeStyleSetBoxSizing(ANUNodeRef node, ANUBoxSizing boxSizing);
ANU_EXPORT ANUBoxSizing ANUNodeStyleGetBoxSizing(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetWidth(ANUNodeRef node, float width);
ANU_EXPORT void ANUNodeStyleSetWidthPercent(ANUNodeRef node, float width);
ANU_EXPORT void ANUNodeStyleSetWidthAuto(ANUNodeRef node);
ANU_EXPORT ANUValue ANUNodeStyleGetWidth(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetHeight(ANUNodeRef node, float height);
ANU_EXPORT void ANUNodeStyleSetHeightPercent(ANUNodeRef node, float height);
ANU_EXPORT void ANUNodeStyleSetHeightAuto(ANUNodeRef node);
ANU_EXPORT ANUValue ANUNodeStyleGetHeight(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetMinWidth(ANUNodeRef node, float minWidth);
ANU_EXPORT void ANUNodeStyleSetMinWidthPercent(ANUNodeRef node, float minWidth);
ANU_EXPORT ANUValue ANUNodeStyleGetMinWidth(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetMinHeight(ANUNodeRef node, float minHeight);
ANU_EXPORT void ANUNodeStyleSetMinHeightPercent(ANUNodeRef node, float minHeight);
ANU_EXPORT ANUValue ANUNodeStyleGetMinHeight(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetMaxWidth(ANUNodeRef node, float maxWidth);
ANU_EXPORT void ANUNodeStyleSetMaxWidthPercent(ANUNodeRef node, float maxWidth);
ANU_EXPORT ANUValue ANUNodeStyleGetMaxWidth(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetMaxHeight(ANUNodeRef node, float maxHeight);
ANU_EXPORT void ANUNodeStyleSetMaxHeightPercent(ANUNodeRef node, float maxHeight);
ANU_EXPORT ANUValue ANUNodeStyleGetMaxHeight(ANUNodeConstRef node);

ANU_EXPORT void ANUNodeStyleSetAspectRatio(ANUNodeRef node, float aspectRatio);
ANU_EXPORT float ANUNodeStyleGetAspectRatio(ANUNodeConstRef node);

ANU_EXTERN_C_END
