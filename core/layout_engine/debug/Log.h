/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/Anu.h>

#include <layout_engine/config/Config.h>
#include <layout_engine/enums/LogLevel.h>
#include <layout_engine/node/Node.h>

namespace facebook::anu {

void log(LogLevel level, const char* format, ...) noexcept;

void log(
    const anu::Node* node,
    LogLevel level,
    const char* format,
    ...) noexcept;

void log(
    const anu::Config* config,
    LogLevel level,
    const char* format,
    ...) noexcept;

ANULogger getDefaultLogger();

} // namespace facebook::anu
