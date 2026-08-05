/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/AnuValue.h>
#include <layout_engine/numeric/Comparison.h>

using namespace facebook;
using namespace facebook::anu;

const ANUValue ANUValueZero = {0, ANUUnitPoint};
const ANUValue ANUValueUndefined = {ANUUndefined, ANUUnitUndefined};
const ANUValue ANUValueAuto = {ANUUndefined, ANUUnitAuto};

bool ANUFloatIsUndefined(const float value) {
  return anu::isUndefined(value);
}
