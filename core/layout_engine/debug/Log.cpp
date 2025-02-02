/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/debug/Log.h>

#ifdef ANDROID
#include <android/log.h>
#endif

namespace facebook::anu {

namespace {

void vlog(
    const anu::Config* config,
    const anu::Node* node,
    LogLevel level,
    const char* format,
    va_list args) {
  if (config == nullptr) {
    getDefaultLogger()(nullptr, node, unscopedEnum(level), format, args);
  } else {
    config->log(node, level, format, args);
  }
}
} // namespace

void log(LogLevel level, const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  vlog(nullptr, nullptr, level, format, args);
  va_end(args);
}

void log(
    const anu::Node* node,
    LogLevel level,
    const char* format,
    ...) noexcept {
  va_list args;
  va_start(args, format);
  vlog(
      node == nullptr ? nullptr : node->getConfig(), node, level, format, args);
  va_end(args);
}

void log(
    const anu::Config* config,
    LogLevel level,
    const char* format,
    ...) noexcept {
  va_list args;
  va_start(args, format);
  vlog(config, nullptr, level, format, args);
  va_end(args);
}

ANULogger getDefaultLogger() {
  return [](const ANUConfigConstRef /*config*/,
            const ANUNodeConstRef /*node*/,
            ANULogLevel level,
            const char* format,
            va_list args) -> int {
#ifdef ANDROID
    int androidLevel = ANULogLevelDebug;
    switch (level) {
      case ANULogLevelFatal:
        androidLevel = ANDROID_LOG_FATAL;
        break;
      case ANULogLevelError:
        androidLevel = ANDROID_LOG_ERROR;
        break;
      case ANULogLevelWarn:
        androidLevel = ANDROID_LOG_WARN;
        break;
      case ANULogLevelInfo:
        androidLevel = ANDROID_LOG_INFO;
        break;
      case ANULogLevelDebug:
        androidLevel = ANDROID_LOG_DEBUG;
        break;
      case ANULogLevelVerbose:
        androidLevel = ANDROID_LOG_VERBOSE;
        break;
    }
    return __android_log_vprint(androidLevel, "anu", format, args);
#else
    switch (level) {
      case ANULogLevelError:
      case ANULogLevelFatal:
        return vfprintf(stderr, format, args);
      case ANULogLevelWarn:
      case ANULogLevelInfo:
      case ANULogLevelDebug:
      case ANULogLevelVerbose:
      default:
        return vprintf(format, args);
    }
#endif
  };
}

} // namespace facebook::anu
