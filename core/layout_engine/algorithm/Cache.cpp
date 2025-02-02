/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/algorithm/Cache.h>
#include <layout_engine/algorithm/PixelGrid.h>
#include <layout_engine/numeric/Comparison.h>

namespace facebook::anu {

static inline bool sizeIsExactAndMatchesOldMeasuredSize(
    SizingMode sizeMode,
    float size,
    float lastComputedSize) {
  return sizeMode == SizingMode::StretchFit &&
      anu::inexactEquals(size, lastComputedSize);
}

static inline bool oldSizeIsMaxContentAndStillFits(
    SizingMode sizeMode,
    float size,
    SizingMode lastSizeMode,
    float lastComputedSize) {
  return sizeMode == SizingMode::FitContent &&
      lastSizeMode == SizingMode::MaxContent &&
      (size >= lastComputedSize || anu::inexactEquals(size, lastComputedSize));
}

static inline bool newSizeIsStricterAndStillValid(
    SizingMode sizeMode,
    float size,
    SizingMode lastSizeMode,
    float lastSize,
    float lastComputedSize) {
  return lastSizeMode == SizingMode::FitContent &&
      sizeMode == SizingMode::FitContent && anu::isDefined(lastSize) &&
      anu::isDefined(size) && anu::isDefined(lastComputedSize) &&
      lastSize > size &&
      (lastComputedSize <= size || anu::inexactEquals(size, lastComputedSize));
}

bool canUseCachedMeasurement(
    const SizingMode widthMode,
    const float availableWidth,
    const SizingMode heightMode,
    const float availableHeight,
    const SizingMode lastWidthMode,
    const float lastAvailableWidth,
    const SizingMode lastHeightMode,
    const float lastAvailableHeight,
    const float lastComputedWidth,
    const float lastComputedHeight,
    const float marginRow,
    const float marginColumn,
    const anu::Config* const config) {
  if ((anu::isDefined(lastComputedHeight) && lastComputedHeight < 0) ||
      ((anu::isDefined(lastComputedWidth)) && lastComputedWidth < 0)) {
    return false;
  }

  const float pointScaleFactor = config->getPointScaleFactor();

  bool useRoundedComparison = config != nullptr && pointScaleFactor != 0;
  const float effectiveWidth = useRoundedComparison
      ? roundValueToPixelGrid(availableWidth, pointScaleFactor, false, false)
      : availableWidth;
  const float effectiveHeight = useRoundedComparison
      ? roundValueToPixelGrid(availableHeight, pointScaleFactor, false, false)
      : availableHeight;
  const float effectiveLastWidth = useRoundedComparison
      ? roundValueToPixelGrid(
            lastAvailableWidth, pointScaleFactor, false, false)
      : lastAvailableWidth;
  const float effectiveLastHeight = useRoundedComparison
      ? roundValueToPixelGrid(
            lastAvailableHeight, pointScaleFactor, false, false)
      : lastAvailableHeight;

  const bool hasSameWidthSpec = lastWidthMode == widthMode &&
      anu::inexactEquals(effectiveLastWidth, effectiveWidth);
  const bool hasSameHeightSpec = lastHeightMode == heightMode &&
      anu::inexactEquals(effectiveLastHeight, effectiveHeight);

  const bool widthIsCompatible =
      hasSameWidthSpec ||
      sizeIsExactAndMatchesOldMeasuredSize(
          widthMode, availableWidth - marginRow, lastComputedWidth) ||
      oldSizeIsMaxContentAndStillFits(
          widthMode,
          availableWidth - marginRow,
          lastWidthMode,
          lastComputedWidth) ||
      newSizeIsStricterAndStillValid(
          widthMode,
          availableWidth - marginRow,
          lastWidthMode,
          lastAvailableWidth,
          lastComputedWidth);

  const bool heightIsCompatible = hasSameHeightSpec ||
      sizeIsExactAndMatchesOldMeasuredSize(
                                      heightMode,
                                      availableHeight - marginColumn,
                                      lastComputedHeight) ||
      oldSizeIsMaxContentAndStillFits(heightMode,
                                      availableHeight - marginColumn,
                                      lastHeightMode,
                                      lastComputedHeight) ||
      newSizeIsStricterAndStillValid(heightMode,
                                     availableHeight - marginColumn,
                                     lastHeightMode,
                                     lastAvailableHeight,
                                     lastComputedHeight);

  return widthIsCompatible && heightIsCompatible;
}

} // namespace facebook::anu
