/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/Anu.h>
#include <layout_engine/algorithm/FlexDirection.h>
#include <layout_engine/event/event.h>
#include <layout_engine/node/Node.h>

namespace facebook::anu {

// Given an offset to an edge, returns the offset to the opposite edge on the
// same axis. This assumes that the width/height of both nodes is determined at
// this point.
inline float getPositionOfOppositeEdge(
    float position,
    FlexDirection axis,
    const anu::Node* const containingNode,
    const anu::Node* const node) {
  return containingNode->getLayout().measuredDimension(dimension(axis)) -
      node->getLayout().measuredDimension(dimension(axis)) - position;
}

inline void setChildTrailingPosition(
    const anu::Node* const node,
    anu::Node* const child,
    const FlexDirection axis) {
  child->setLayoutPosition(
      getPositionOfOppositeEdge(
          child->getLayout().position(flexStartEdge(axis)), axis, node, child),
      flexEndEdge(axis));
}

inline bool needsTrailingPosition(const FlexDirection axis) {
  return axis == FlexDirection::RowReverse ||
      axis == FlexDirection::ColumnReverse;
}

} // namespace facebook::anu
