/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cmath>

#include <layout_engine/Anu.h>

#include <layout_engine/algorithm/SizingMode.h>
#include <layout_engine/numeric/Comparison.h>

namespace facebook::anu {

struct CachedMeasurement {
  float availableWidth{-1};
  float availableHeight{-1};
  SizingMode widthSizingMode{SizingMode::MaxContent};
  SizingMode heightSizingMode{SizingMode::MaxContent};

  float computedWidth{-1};
  float computedHeight{-1};

  bool operator==(CachedMeasurement measurement) const {
    bool isEqual = widthSizingMode == measurement.widthSizingMode &&
        heightSizingMode == measurement.heightSizingMode;

    if (!anu::isUndefined(availableWidth) ||
        !anu::isUndefined(measurement.availableWidth)) {
      isEqual = isEqual && availableWidth == measurement.availableWidth;
    }
    if (!anu::isUndefined(availableHeight) ||
        !anu::isUndefined(measurement.availableHeight)) {
      isEqual = isEqual && availableHeight == measurement.availableHeight;
    }
    if (!anu::isUndefined(computedWidth) ||
        !anu::isUndefined(measurement.computedWidth)) {
      isEqual = isEqual && computedWidth == measurement.computedWidth;
    }
    if (!anu::isUndefined(computedHeight) ||
        !anu::isUndefined(measurement.computedHeight)) {
      isEqual = isEqual && computedHeight == measurement.computedHeight;
    }

    return isEqual;
  }
};

} // namespace facebook::anu
