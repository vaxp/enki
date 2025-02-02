/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>

#include <layout_engine/algorithm/PixelGrid.h>

using namespace facebook;
using namespace facebook::anu;

float ANURoundValueToPixelGrid(
    const double value,
    const double pointScaleFactor,
    const bool forceCeil,
    const bool forceFloor) {
  return anu::roundValueToPixelGrid(
      value, pointScaleFactor, forceCeil, forceFloor);
}
