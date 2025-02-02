/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <bitset>

#include <layout_engine/Anu.h>
#include <layout_engine/enums/Errata.h>
#include <layout_engine/enums/ExperimentalFeature.h>
#include <layout_engine/enums/LogLevel.h>

// Tag struct used to form the opaque ANUConfigRef for the public C API
struct ANUConfig {};

namespace facebook::anu {

class Config;
class Node;

using ExperimentalFeatureSet = std::bitset<ordinalCount<ExperimentalFeature>()>;

// Whether moving a node from an old to new config should dirty previously
// calculated layout results.
bool configUpdateInvalidatesLayout(
    const Config& oldConfig,
    const Config& newConfig);

class ANU_EXPORT Config : public ::ANUConfig {
 public:
  explicit Config(ANULogger logger) : logger_{logger} {}

  void setUseWebDefaults(bool useWebDefaults);
  bool useWebDefaults() const;

  void setExperimentalFeatureEnabled(ExperimentalFeature feature, bool enabled);
  bool isExperimentalFeatureEnabled(ExperimentalFeature feature) const;
  ExperimentalFeatureSet getEnabledExperiments() const;

  void setErrata(Errata errata);
  void addErrata(Errata errata);
  void removeErrata(Errata errata);
  Errata getErrata() const;
  bool hasErrata(Errata errata) const;

  void setPointScaleFactor(float pointScaleFactor);
  float getPointScaleFactor() const;

  void setContext(void* context);
  void* getContext() const;

  uint32_t getVersion() const noexcept;

  void setLogger(ANULogger logger);
  void log(
      const anu::Node* node,
      LogLevel logLevel,
      const char* format,
      va_list args) const;

  void setCloneNodeCallback(ANUCloneNodeFunc cloneNode);
  ANUNodeRef
  cloneNode(ANUNodeConstRef node, ANUNodeConstRef owner, size_t childIndex) const;

  static const Config& getDefault();

 private:
  ANUCloneNodeFunc cloneNodeCallback_{nullptr};
  ANULogger logger_{};

  bool useWebDefaults_ : 1 = false;

  uint32_t version_ = 0;
  ExperimentalFeatureSet experimentalFeatures_{};
  Errata errata_ = Errata::None;
  float pointScaleFactor_ = 1.0f;
  void* context_ = nullptr;
};

inline Config* resolveRef(const ANUConfigRef ref) {
  return static_cast<Config*>(ref);
}

inline const Config* resolveRef(const ANUConfigConstRef ref) {
  return static_cast<const Config*>(ref);
}

} // namespace facebook::anu
