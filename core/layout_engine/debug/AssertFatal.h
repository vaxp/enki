/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/Anu.h>

namespace facebook::anu {

class Node;
class Config;

[[noreturn]] void fatalWithMessage(const char* message);

void assertFatal(bool condition, const char* message);
void assertFatalWithNode(
    const anu::Node* node,
    bool condition,
    const char* message);
void assertFatalWithConfig(
    const anu::Config* config,
    bool condition,
    const char* message);

} // namespace facebook::anu
