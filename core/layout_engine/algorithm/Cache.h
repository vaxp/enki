/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/algorithm/SizingMode.h>
#include <layout_engine/config/Config.h>

namespace facebook::anu {

bool canUseCachedMeasurement(
    SizingMode widthMode,
    float availableWidth,
    SizingMode heightMode,
    float availableHeight,
    SizingMode lastWidthMode,
    float lastAvailableWidth,
    SizingMode lastHeightMode,
    float lastAvailableHeight,
    float lastComputedWidth,
    float lastComputedHeight,
    float marginRow,
    float marginColumn,
    const anu::Config* config);

} // namespace facebook::anu
