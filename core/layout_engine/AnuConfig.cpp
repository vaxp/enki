/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>
#include <layout_engine/debug/AssertFatal.h>
#include <layout_engine/debug/Log.h>

using namespace facebook;
using namespace facebook::anu;

ANUConfigRef ANUConfigNew(void) {
  return new anu::Config(getDefaultLogger());
}

void ANUConfigFree(const ANUConfigRef config) {
  delete resolveRef(config);
}

ANUConfigConstRef ANUConfigGetDefault() {
  return &anu::Config::getDefault();
}

void ANUConfigSetUseWebDefaults(const ANUConfigRef config, const bool enabled) {
  resolveRef(config)->setUseWebDefaults(enabled);
}

bool ANUConfigGetUseWebDefaults(const ANUConfigConstRef config) {
  return resolveRef(config)->useWebDefaults();
}

void ANUConfigSetPointScaleFactor(
    const ANUConfigRef config,
    const float pixelsInPoint) {
  anu::assertFatalWithConfig(
      resolveRef(config),
      pixelsInPoint >= 0.0f,
      "Scale factor should not be less than zero");

  resolveRef(config)->setPointScaleFactor(pixelsInPoint);
}

float ANUConfigGetPointScaleFactor(const ANUConfigConstRef config) {
  return resolveRef(config)->getPointScaleFactor();
}

void ANUConfigSetErrata(ANUConfigRef config, ANUErrata errata) {
  resolveRef(config)->setErrata(scopedEnum(errata));
}

ANUErrata ANUConfigGetErrata(ANUConfigConstRef config) {
  return unscopedEnum(resolveRef(config)->getErrata());
}

void ANUConfigSetLogger(const ANUConfigRef config, ANULogger logger) {
  if (logger != nullptr) {
    resolveRef(config)->setLogger(logger);
  } else {
    resolveRef(config)->setLogger(getDefaultLogger());
  }
}

void ANUConfigSetContext(const ANUConfigRef config, void* context) {
  resolveRef(config)->setContext(context);
}

void* ANUConfigGetContext(const ANUConfigConstRef config) {
  return resolveRef(config)->getContext();
}

void ANUConfigSetExperimentalFeatureEnabled(
    const ANUConfigRef config,
    const ANUExperimentalFeature feature,
    const bool enabled) {
  resolveRef(config)->setExperimentalFeatureEnabled(
      scopedEnum(feature), enabled);
}

bool ANUConfigIsExperimentalFeatureEnabled(
    const ANUConfigConstRef config,
    const ANUExperimentalFeature feature) {
  return resolveRef(config)->isExperimentalFeatureEnabled(scopedEnum(feature));
}

void ANUConfigSetCloneNodeFunc(
    const ANUConfigRef config,
    const ANUCloneNodeFunc callback) {
  resolveRef(config)->setCloneNodeCallback(callback);
}
