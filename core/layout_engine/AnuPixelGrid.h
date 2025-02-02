/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdbool.h>

#include <layout_engine/AnuConfig.h>
#include <layout_engine/AnuEnums.h>
#include <layout_engine/AnuMacros.h>

ANU_EXTERN_C_BEGIN

/**
 * Rounds a point value to the nearest whole pixel, given a pointScaleFactor
 * describing pixel density.
 * @returns the rounded value in points
 */
ANU_EXPORT float ANURoundValueToPixelGrid(
    double value,
    double pointScaleFactor,
    bool forceCeil,
    bool forceFloor);

ANU_EXTERN_C_END
