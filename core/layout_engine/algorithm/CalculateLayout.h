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

void calculateLayout(
    anu::Node* node,
    float ownerWidth,
    float ownerHeight,
    Direction ownerDirection);

bool calculateLayoutInternal(
    anu::Node* node,
    float availableWidth,
    float availableHeight,
    Direction ownerDirection,
    SizingMode widthSizingMode,
    SizingMode heightSizingMode,
    float ownerWidth,
    float ownerHeight,
    bool performLayout,
    LayoutPassReason reason,
    LayoutData& layoutMarkerData,
    uint32_t depth,
    uint32_t generationCount);

} // namespace facebook::anu
