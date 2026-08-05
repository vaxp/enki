/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>

#include <layout_engine/algorithm/Align.h>
#include <layout_engine/algorithm/Baseline.h>
#include <layout_engine/debug/AssertFatal.h>
#include <layout_engine/event/event.h>

namespace facebook::anu {

float calculateBaseline(const anu::Node* node) {
  if (node->hasBaselineFunc()) {
    Event::publish<Event::NodeBaselineStart>(node);

    const float baseline = node->baseline(
        node->getLayout().measuredDimension(Dimension::Width),
        node->getLayout().measuredDimension(Dimension::Height));

    Event::publish<Event::NodeBaselineEnd>(node);

    anu::assertFatalWithNode(
        node,
        !std::isnan(baseline),
        "Expect custom baseline function to not return NaN");
    return baseline;
  }

  anu::Node* baselineChild = nullptr;
  for (auto child : node->getLayoutChildren()) {
    if (child->getLineIndex() > 0) {
      break;
    }
    if (child->style().positionType() == PositionType::Absolute) {
      continue;
    }
    if (resolveChildAlignment(node, child) == Align::Baseline ||
        child->isReferenceBaseline()) {
      baselineChild = child;
      break;
    }

    if (baselineChild == nullptr) {
      baselineChild = child;
    }
  }

  if (baselineChild == nullptr) {
    return node->getLayout().measuredDimension(Dimension::Height);
  }

  const float baseline = calculateBaseline(baselineChild);
  return baseline + baselineChild->getLayout().position(PhysicalEdge::Top);
}

bool isBaselineLayout(const anu::Node* node) {
  if (isColumn(node->style().flexDirection())) {
    return false;
  }
  if (node->style().alignItems() == Align::Baseline) {
    return true;
  }
  for (auto child : node->getLayoutChildren()) {
    if (child->style().positionType() != PositionType::Absolute &&
        child->style().alignSelf() == Align::Baseline) {
      return true;
    }
  }

  return false;
}

} // namespace facebook::anu
