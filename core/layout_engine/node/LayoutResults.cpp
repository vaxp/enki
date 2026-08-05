/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cmath>

#include <layout_engine/node/LayoutResults.h>
#include <layout_engine/numeric/Comparison.h>

namespace facebook::anu {

bool LayoutResults::operator==(LayoutResults layout) const {
  bool isEqual = anu::inexactEquals(position_, layout.position_) &&
      anu::inexactEquals(dimensions_, layout.dimensions_) &&
      anu::inexactEquals(margin_, layout.margin_) &&
      anu::inexactEquals(border_, layout.border_) &&
      anu::inexactEquals(padding_, layout.padding_) &&
      direction() == layout.direction() &&
      hadOverflow() == layout.hadOverflow() &&
      lastOwnerDirection == layout.lastOwnerDirection &&
      configVersion == layout.configVersion &&
      nextCachedMeasurementsIndex == layout.nextCachedMeasurementsIndex &&
      cachedLayout == layout.cachedLayout &&
      computedFlexBasis == layout.computedFlexBasis;

  for (uint32_t i = 0; i < LayoutResults::MaxCachedMeasurements && isEqual;
       ++i) {
    isEqual = isEqual && cachedMeasurements[i] == layout.cachedMeasurements[i];
  }

  if (!anu::isUndefined(measuredDimensions_[0]) ||
      !anu::isUndefined(layout.measuredDimensions_[0])) {
    isEqual =
        isEqual && (measuredDimensions_[0] == layout.measuredDimensions_[0]);
  }
  if (!anu::isUndefined(measuredDimensions_[1]) ||
      !anu::isUndefined(layout.measuredDimensions_[1])) {
    isEqual =
        isEqual && (measuredDimensions_[1] == layout.measuredDimensions_[1]);
  }

  return isEqual;
}

} // namespace facebook::anu
