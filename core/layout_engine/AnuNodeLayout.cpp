/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>
#include <layout_engine/debug/AssertFatal.h>
#include <layout_engine/enums/Edge.h>
#include <layout_engine/node/Node.h>

using namespace facebook;
using namespace facebook::anu;

namespace {

template <auto LayoutMember>
float getResolvedLayoutProperty(const ANUNodeConstRef nodeRef, const Edge edge) {
  const auto node = resolveRef(nodeRef);
  anu::assertFatalWithNode(
      node,
      edge <= Edge::End,
      "Cannot get layout properties of multi-edge shorthands");

  if (edge == Edge::Start) {
    if (node->getLayout().direction() == Direction::RTL) {
      return (node->getLayout().*LayoutMember)(PhysicalEdge::Right);
    } else {
      return (node->getLayout().*LayoutMember)(PhysicalEdge::Left);
    }
  }

  if (edge == Edge::End) {
    if (node->getLayout().direction() == Direction::RTL) {
      return (node->getLayout().*LayoutMember)(PhysicalEdge::Left);
    } else {
      return (node->getLayout().*LayoutMember)(PhysicalEdge::Right);
    }
  }

  return (node->getLayout().*LayoutMember)(static_cast<PhysicalEdge>(edge));
}

} // namespace

float ANUNodeLayoutGetLeft(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().position(PhysicalEdge::Left);
}

float ANUNodeLayoutGetTop(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().position(PhysicalEdge::Top);
}

float ANUNodeLayoutGetRight(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().position(PhysicalEdge::Right);
}

float ANUNodeLayoutGetBottom(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().position(PhysicalEdge::Bottom);
}

float ANUNodeLayoutGetWidth(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().dimension(Dimension::Width);
}

float ANUNodeLayoutGetHeight(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().dimension(Dimension::Height);
}

ANUDirection ANUNodeLayoutGetDirection(const ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->getLayout().direction());
}

bool ANUNodeLayoutGetHadOverflow(const ANUNodeConstRef node) {
  return resolveRef(node)->getLayout().hadOverflow();
}

float ANUNodeLayoutGetMargin(ANUNodeConstRef node, ANUEdge edge) {
  return getResolvedLayoutProperty<&LayoutResults::margin>(
      node, scopedEnum(edge));
}

float ANUNodeLayoutGetBorder(ANUNodeConstRef node, ANUEdge edge) {
  return getResolvedLayoutProperty<&LayoutResults::border>(
      node, scopedEnum(edge));
}

float ANUNodeLayoutGetPadding(ANUNodeConstRef node, ANUEdge edge) {
  return getResolvedLayoutProperty<&LayoutResults::padding>(
      node, scopedEnum(edge));
}
