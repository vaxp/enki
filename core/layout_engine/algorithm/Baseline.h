/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/Anu.h>
#include <layout_engine/node/Node.h>

namespace facebook::anu {

// Calculate baseline represented as an offset from the top edge of the node.
float calculateBaseline(const anu::Node* node);

// Whether any of the children of this node participate in baseline alignment
bool isBaselineLayout(const anu::Node* node);

} // namespace facebook::anu
