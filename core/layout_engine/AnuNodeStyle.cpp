/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>
#include <layout_engine/debug/AssertFatal.h>
#include <layout_engine/node/Node.h>

using namespace facebook;
using namespace facebook::anu;

namespace {

template <auto GetterT, auto SetterT, typename ValueT>
void updateStyle(ANUNodeRef node, ValueT value) {
  auto& style = resolveRef(node)->style();
  if ((style.*GetterT)() != value) {
    (style.*SetterT)(value);
    resolveRef(node)->markDirtyAndPropagate();
  }
}

template <auto GetterT, auto SetterT, typename IdxT, typename ValueT>
void updateStyle(ANUNodeRef node, IdxT idx, ValueT value) {
  auto& style = resolveRef(node)->style();
  if ((style.*GetterT)(idx) != value) {
    (style.*SetterT)(idx, value);
    resolveRef(node)->markDirtyAndPropagate();
  }
}

} // namespace

void ANUNodeCopyStyle(ANUNodeRef dstNode, ANUNodeConstRef srcNode) {
  auto dst = resolveRef(dstNode);
  auto src = resolveRef(srcNode);

  if (dst->style() != src->style()) {
    dst->setStyle(src->style());
    dst->markDirtyAndPropagate();
  }
}

void ANUNodeStyleSetDirection(const ANUNodeRef node, const ANUDirection value) {
  updateStyle<&Style::direction, &Style::setDirection>(node, scopedEnum(value));
}

ANUDirection ANUNodeStyleGetDirection(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().direction());
}

void ANUNodeStyleSetFlexDirection(
    const ANUNodeRef node,
    const ANUFlexDirection flexDirection) {
  updateStyle<&Style::flexDirection, &Style::setFlexDirection>(
      node, scopedEnum(flexDirection));
}

ANUFlexDirection ANUNodeStyleGetFlexDirection(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().flexDirection());
}

void ANUNodeStyleSetJustifyContent(
    const ANUNodeRef node,
    const ANUJustify justifyContent) {
  updateStyle<&Style::justifyContent, &Style::setJustifyContent>(
      node, scopedEnum(justifyContent));
}

ANUJustify ANUNodeStyleGetJustifyContent(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().justifyContent());
}

void ANUNodeStyleSetAlignContent(
    const ANUNodeRef node,
    const ANUAlign alignContent) {
  updateStyle<&Style::alignContent, &Style::setAlignContent>(
      node, scopedEnum(alignContent));
}

ANUAlign ANUNodeStyleGetAlignContent(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().alignContent());
}

void ANUNodeStyleSetAlignItems(const ANUNodeRef node, const ANUAlign alignItems) {
  updateStyle<&Style::alignItems, &Style::setAlignItems>(
      node, scopedEnum(alignItems));
}

ANUAlign ANUNodeStyleGetAlignItems(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().alignItems());
}

void ANUNodeStyleSetAlignSelf(const ANUNodeRef node, const ANUAlign alignSelf) {
  updateStyle<&Style::alignSelf, &Style::setAlignSelf>(
      node, scopedEnum(alignSelf));
}

ANUAlign ANUNodeStyleGetAlignSelf(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().alignSelf());
}

void ANUNodeStyleSetPositionType(
    const ANUNodeRef node,
    const ANUPositionType positionType) {
  updateStyle<&Style::positionType, &Style::setPositionType>(
      node, scopedEnum(positionType));
}

ANUPositionType ANUNodeStyleGetPositionType(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().positionType());
}

void ANUNodeStyleSetFlexWrap(const ANUNodeRef node, const ANUWrap flexWrap) {
  updateStyle<&Style::flexWrap, &Style::setFlexWrap>(
      node, scopedEnum(flexWrap));
}

ANUWrap ANUNodeStyleGetFlexWrap(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().flexWrap());
}

void ANUNodeStyleSetOverflow(const ANUNodeRef node, const ANUOverflow overflow) {
  updateStyle<&Style::overflow, &Style::setOverflow>(
      node, scopedEnum(overflow));
}

ANUOverflow ANUNodeStyleGetOverflow(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().overflow());
}

void ANUNodeStyleSetDisplay(const ANUNodeRef node, const ANUDisplay display) {
  updateStyle<&Style::display, &Style::setDisplay>(node, scopedEnum(display));
}

ANUDisplay ANUNodeStyleGetDisplay(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().display());
}

void ANUNodeStyleSetFlex(const ANUNodeRef node, const float flex) {
  updateStyle<&Style::flex, &Style::setFlex>(node, FloatOptional{flex});
}

float ANUNodeStyleGetFlex(const ANUNodeConstRef nodeRef) {
  const auto node = resolveRef(nodeRef);
  return node->style().flex().isUndefined() ? ANUUndefined
                                            : node->style().flex().unwrap();
}

void ANUNodeStyleSetFlexGrow(const ANUNodeRef node, const float flexGrow) {
  updateStyle<&Style::flexGrow, &Style::setFlexGrow>(
      node, FloatOptional{flexGrow});
}

float ANUNodeStyleGetFlexGrow(const ANUNodeConstRef nodeRef) {
  const auto node = resolveRef(nodeRef);
  return node->style().flexGrow().isUndefined()
      ? Style::DefaultFlexGrow
      : node->style().flexGrow().unwrap();
}

void ANUNodeStyleSetFlexShrink(const ANUNodeRef node, const float flexShrink) {
  updateStyle<&Style::flexShrink, &Style::setFlexShrink>(
      node, FloatOptional{flexShrink});
}

float ANUNodeStyleGetFlexShrink(const ANUNodeConstRef nodeRef) {
  const auto node = resolveRef(nodeRef);
  return node->style().flexShrink().isUndefined()
      ? (node->getConfig()->useWebDefaults() ? Style::WebDefaultFlexShrink
                                             : Style::DefaultFlexShrink)
      : node->style().flexShrink().unwrap();
}

void ANUNodeStyleSetFlexBasis(const ANUNodeRef node, const float flexBasis) {
  updateStyle<&Style::flexBasis, &Style::setFlexBasis>(
      node, StyleLength::points(flexBasis));
}

void ANUNodeStyleSetFlexBasisPercent(
    const ANUNodeRef node,
    const float flexBasisPercent) {
  updateStyle<&Style::flexBasis, &Style::setFlexBasis>(
      node, StyleLength::percent(flexBasisPercent));
}

void ANUNodeStyleSetFlexBasisAuto(const ANUNodeRef node) {
  updateStyle<&Style::flexBasis, &Style::setFlexBasis>(
      node, StyleLength::ofAuto());
}

ANUValue ANUNodeStyleGetFlexBasis(const ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().flexBasis();
}

void ANUNodeStyleSetPosition(ANUNodeRef node, ANUEdge edge, float points) {
  updateStyle<&Style::position, &Style::setPosition>(
      node, scopedEnum(edge), StyleLength::points(points));
}

void ANUNodeStyleSetPositionPercent(ANUNodeRef node, ANUEdge edge, float percent) {
  updateStyle<&Style::position, &Style::setPosition>(
      node, scopedEnum(edge), StyleLength::percent(percent));
}

void ANUNodeStyleSetPositionAuto(ANUNodeRef node, ANUEdge edge) {
  updateStyle<&Style::position, &Style::setPosition>(
      node, scopedEnum(edge), StyleLength::ofAuto());
}

ANUValue ANUNodeStyleGetPosition(ANUNodeConstRef node, ANUEdge edge) {
  return (ANUValue)resolveRef(node)->style().position(scopedEnum(edge));
}

void ANUNodeStyleSetMargin(ANUNodeRef node, ANUEdge edge, float points) {
  updateStyle<&Style::margin, &Style::setMargin>(
      node, scopedEnum(edge), StyleLength::points(points));
}

void ANUNodeStyleSetMarginPercent(ANUNodeRef node, ANUEdge edge, float percent) {
  updateStyle<&Style::margin, &Style::setMargin>(
      node, scopedEnum(edge), StyleLength::percent(percent));
}

void ANUNodeStyleSetMarginAuto(ANUNodeRef node, ANUEdge edge) {
  updateStyle<&Style::margin, &Style::setMargin>(
      node, scopedEnum(edge), StyleLength::ofAuto());
}

ANUValue ANUNodeStyleGetMargin(ANUNodeConstRef node, ANUEdge edge) {
  return (ANUValue)resolveRef(node)->style().margin(scopedEnum(edge));
}

void ANUNodeStyleSetPadding(ANUNodeRef node, ANUEdge edge, float points) {
  updateStyle<&Style::padding, &Style::setPadding>(
      node, scopedEnum(edge), StyleLength::points(points));
}

void ANUNodeStyleSetPaddingPercent(ANUNodeRef node, ANUEdge edge, float percent) {
  updateStyle<&Style::padding, &Style::setPadding>(
      node, scopedEnum(edge), StyleLength::percent(percent));
}

ANUValue ANUNodeStyleGetPadding(ANUNodeConstRef node, ANUEdge edge) {
  return (ANUValue)resolveRef(node)->style().padding(scopedEnum(edge));
}

void ANUNodeStyleSetBorder(
    const ANUNodeRef node,
    const ANUEdge edge,
    const float border) {
  updateStyle<&Style::border, &Style::setBorder>(
      node, scopedEnum(edge), StyleLength::points(border));
}

float ANUNodeStyleGetBorder(const ANUNodeConstRef node, const ANUEdge edge) {
  auto border = resolveRef(node)->style().border(scopedEnum(edge));
  if (border.isUndefined() || border.isAuto()) {
    return ANUUndefined;
  }

  return static_cast<ANUValue>(border).value;
}

void ANUNodeStyleSetGap(
    const ANUNodeRef node,
    const ANUGutter gutter,
    const float gapLength) {
  updateStyle<&Style::gap, &Style::setGap>(
      node, scopedEnum(gutter), StyleLength::points(gapLength));
}

void ANUNodeStyleSetGapPercent(ANUNodeRef node, ANUGutter gutter, float percent) {
  updateStyle<&Style::gap, &Style::setGap>(
      node, scopedEnum(gutter), StyleLength::percent(percent));
}

float ANUNodeStyleGetGap(const ANUNodeConstRef node, const ANUGutter gutter) {
  auto gapLength = resolveRef(node)->style().gap(scopedEnum(gutter));
  if (gapLength.isUndefined() || gapLength.isAuto()) {
    return ANUUndefined;
  }

  return static_cast<ANUValue>(gapLength).value;
}

void ANUNodeStyleSetAspectRatio(const ANUNodeRef node, const float aspectRatio) {
  updateStyle<&Style::aspectRatio, &Style::setAspectRatio>(
      node, FloatOptional{aspectRatio});
}

float ANUNodeStyleGetAspectRatio(const ANUNodeConstRef node) {
  const FloatOptional op = resolveRef(node)->style().aspectRatio();
  return op.isUndefined() ? ANUUndefined : op.unwrap();
}

void ANUNodeStyleSetBoxSizing(ANUNodeRef node, ANUBoxSizing boxSizing) {
  updateStyle<&Style::boxSizing, &Style::setBoxSizing>(
      node, scopedEnum(boxSizing));
}

ANUBoxSizing ANUNodeStyleGetBoxSizing(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->style().boxSizing());
}

void ANUNodeStyleSetWidth(ANUNodeRef node, float points) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Width, StyleLength::points(points));
}

void ANUNodeStyleSetWidthPercent(ANUNodeRef node, float percent) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Width, StyleLength::percent(percent));
}

void ANUNodeStyleSetWidthAuto(ANUNodeRef node) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Width, StyleLength::ofAuto());
}

ANUValue ANUNodeStyleGetWidth(ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().dimension(Dimension::Width);
}

void ANUNodeStyleSetHeight(ANUNodeRef node, float points) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Height, StyleLength::points(points));
}

void ANUNodeStyleSetHeightPercent(ANUNodeRef node, float percent) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Height, StyleLength::percent(percent));
}

void ANUNodeStyleSetHeightAuto(ANUNodeRef node) {
  updateStyle<&Style::dimension, &Style::setDimension>(
      node, Dimension::Height, StyleLength::ofAuto());
}

ANUValue ANUNodeStyleGetHeight(ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().dimension(Dimension::Height);
}

void ANUNodeStyleSetMinWidth(const ANUNodeRef node, const float minWidth) {
  updateStyle<&Style::minDimension, &Style::setMinDimension>(
      node, Dimension::Width, StyleLength::points(minWidth));
}

void ANUNodeStyleSetMinWidthPercent(const ANUNodeRef node, const float minWidth) {
  updateStyle<&Style::minDimension, &Style::setMinDimension>(
      node, Dimension::Width, StyleLength::percent(minWidth));
}

ANUValue ANUNodeStyleGetMinWidth(const ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().minDimension(Dimension::Width);
}

void ANUNodeStyleSetMinHeight(const ANUNodeRef node, const float minHeight) {
  updateStyle<&Style::minDimension, &Style::setMinDimension>(
      node, Dimension::Height, StyleLength::points(minHeight));
}

void ANUNodeStyleSetMinHeightPercent(
    const ANUNodeRef node,
    const float minHeight) {
  updateStyle<&Style::minDimension, &Style::setMinDimension>(
      node, Dimension::Height, StyleLength::percent(minHeight));
}

ANUValue ANUNodeStyleGetMinHeight(const ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().minDimension(Dimension::Height);
}

void ANUNodeStyleSetMaxWidth(const ANUNodeRef node, const float maxWidth) {
  updateStyle<&Style::maxDimension, &Style::setMaxDimension>(
      node, Dimension::Width, StyleLength::points(maxWidth));
}

void ANUNodeStyleSetMaxWidthPercent(const ANUNodeRef node, const float maxWidth) {
  updateStyle<&Style::maxDimension, &Style::setMaxDimension>(
      node, Dimension::Width, StyleLength::percent(maxWidth));
}

ANUValue ANUNodeStyleGetMaxWidth(const ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().maxDimension(Dimension::Width);
}

void ANUNodeStyleSetMaxHeight(const ANUNodeRef node, const float maxHeight) {
  updateStyle<&Style::maxDimension, &Style::setMaxDimension>(
      node, Dimension::Height, StyleLength::points(maxHeight));
}

void ANUNodeStyleSetMaxHeightPercent(
    const ANUNodeRef node,
    const float maxHeight) {
  updateStyle<&Style::maxDimension, &Style::setMaxDimension>(
      node, Dimension::Height, StyleLength::percent(maxHeight));
}

ANUValue ANUNodeStyleGetMaxHeight(const ANUNodeConstRef node) {
  return (ANUValue)resolveRef(node)->style().maxDimension(Dimension::Height);
}
