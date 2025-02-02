/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/event/event.h>
#include <layout_engine/node/Node.h>

namespace facebook::anu {

void layoutAbsoluteChild(
    const anu::Node* containingNode,
    const anu::Node* node,
    anu::Node* child,
    float containingBlockWidth,
    float containingBlockHeight,
    SizingMode widthMode,
    Direction direction,
    LayoutData& layoutMarkerData,
    uint32_t depth,
    uint32_t generationCount);

// Returns if some absolute descendant has new layout
bool layoutAbsoluteDescendants(
    anu::Node* containingNode,
    anu::Node* currentNode,
    SizingMode widthSizingMode,
    Direction currentNodeDirection,
    LayoutData& layoutMarkerData,
    uint32_t currentDepth,
    uint32_t generationCount,
    float currentNodeMainOffsetFromContainingBlock,
    float currentNodeCrossOffsetFromContainingBlock,
    float containingNodeAvailableInnerWidth,
    float containingNodeAvailableInnerHeight);

} // namespace facebook::anu
