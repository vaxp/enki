/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <layout_engine/enums/Edge.h>

namespace facebook::anu {

enum class PhysicalEdge : uint32_t {
  Left = anu::to_underlying(Edge::Left),
  Top = anu::to_underlying(Edge::Top),
  Right = anu::to_underlying(Edge::Right),
  Bottom = anu::to_underlying(Edge::Bottom),
};

} // namespace facebook::anu
