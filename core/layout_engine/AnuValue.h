/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdbool.h>

#include <layout_engine/AnuEnums.h>
#include <layout_engine/AnuMacros.h>

/**
 * Float value to represent "undefined" in style values.
 */
#ifdef __cplusplus
#include <limits>
constexpr float ANUUndefined = std::numeric_limits<float>::quiet_NaN();
#else
#include <math.h>
#define ANUUndefined NAN
#endif

ANU_EXTERN_C_BEGIN

/**
 * Structure used to represent a dimension in a style.
 */
typedef struct ANUValue {
  float value;
  ANUUnit unit;
} ANUValue;

/**
 * Constant for a dimension of "auto".
 */
ANU_EXPORT extern const ANUValue ANUValueAuto;

/**
 * Constant for a dimension which is not defined.
 */
ANU_EXPORT extern const ANUValue ANUValueUndefined;

/**
 * Constant for a dimension that is zero-length.
 */
ANU_EXPORT extern const ANUValue ANUValueZero;

/**
 * Whether a dimension represented as a float is defined.
 */
ANU_EXPORT bool ANUFloatIsUndefined(float value);

ANU_EXTERN_C_END

// Equality operators for comparison of ANUValue in C++
#ifdef __cplusplus
inline bool operator==(const ANUValue& lhs, const ANUValue& rhs) {
  if (lhs.unit != rhs.unit) {
    return false;
  }

  switch (lhs.unit) {
    case ANUUnitUndefined:
    case ANUUnitAuto:
      return true;
    case ANUUnitPoint:
    case ANUUnitPercent:
      return lhs.value == rhs.value;
  }

  return false;
}

inline bool operator!=(const ANUValue& lhs, const ANUValue& rhs) {
  return !(lhs == rhs);
}

inline ANUValue operator-(const ANUValue& value) {
  return {-value.value, value.unit};
}
#endif
