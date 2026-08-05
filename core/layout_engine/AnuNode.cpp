/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <layout_engine/Anu.h>

#include <layout_engine/algorithm/Cache.h>
#include <layout_engine/algorithm/CalculateLayout.h>
#include <layout_engine/debug/AssertFatal.h>
#include <layout_engine/debug/Log.h>
#include <layout_engine/event/event.h>
#include <layout_engine/node/Node.h>

using namespace facebook;
using namespace facebook::anu;

ANUNodeRef ANUNodeNew(void) {
  return ANUNodeNewWithConfig(ANUConfigGetDefault());
}

ANUNodeRef ANUNodeNewWithConfig(const ANUConfigConstRef config) {
  auto* node = new anu::Node{resolveRef(config)};
  anu::assertFatal(
      config != nullptr, "Tried to construct ANUNode with null config");
  Event::publish<Event::NodeAllocation>(node, {config});

  return node;
}

ANUNodeRef ANUNodeClone(ANUNodeConstRef oldNodeRef) {
  auto oldNode = resolveRef(oldNodeRef);
  const auto node = new anu::Node(*oldNode);
  Event::publish<Event::NodeAllocation>(node, {node->getConfig()});
  node->setOwner(nullptr);
  return node;
}

void ANUNodeFree(const ANUNodeRef nodeRef) {
  const auto node = resolveRef(nodeRef);

  if (auto owner = node->getOwner()) {
    owner->removeChild(node);
    node->setOwner(nullptr);
  }

  const size_t childCount = node->getChildCount();
  for (size_t i = 0; i < childCount; i++) {
    auto child = node->getChild(i);
    child->setOwner(nullptr);
  }

  node->clearChildren();

  Event::publish<Event::NodeDeallocation>(node, {ANUNodeGetConfig(node)});
  delete resolveRef(node);
}

void ANUNodeFreeRecursive(ANUNodeRef rootRef) {
  const auto root = resolveRef(rootRef);

  size_t skipped = 0;
  while (root->getChildCount() > skipped) {
    const auto child = root->getChild(skipped);
    if (child->getOwner() != root) {
      // Don't free shared nodes that we don't own.
      skipped += 1;
    } else {
      ANUNodeRemoveChild(root, child);
      ANUNodeFreeRecursive(child);
    }
  }
  ANUNodeFree(root);
}

void ANUNodeFinalize(const ANUNodeRef node) {
  Event::publish<Event::NodeDeallocation>(node, {ANUNodeGetConfig(node)});
  delete resolveRef(node);
}

void ANUNodeReset(ANUNodeRef node) {
  resolveRef(node)->reset();
}

void ANUNodeCalculateLayout(
    const ANUNodeRef node,
    const float ownerWidth,
    const float ownerHeight,
    const ANUDirection ownerDirection) {
  anu::calculateLayout(
      resolveRef(node), ownerWidth, ownerHeight, scopedEnum(ownerDirection));
}

bool ANUNodeGetHasNewLayout(ANUNodeConstRef node) {
  return resolveRef(node)->getHasNewLayout();
}

void ANUNodeSetHasNewLayout(ANUNodeRef node, bool hasNewLayout) {
  resolveRef(node)->setHasNewLayout(hasNewLayout);
}

bool ANUNodeIsDirty(ANUNodeConstRef node) {
  return resolveRef(node)->isDirty();
}

void ANUNodeMarkDirty(const ANUNodeRef nodeRef) {
  const auto node = resolveRef(nodeRef);

  anu::assertFatalWithNode(
      node,
      node->hasMeasureFunc(),
      "Only leaf nodes with custom measure functions "
      "should manually mark themselves as dirty");

  node->markDirtyAndPropagate();
}

void ANUNodeSetDirtiedFunc(ANUNodeRef node, ANUDirtiedFunc dirtiedFunc) {
  resolveRef(node)->setDirtiedFunc(dirtiedFunc);
}

ANUDirtiedFunc ANUNodeGetDirtiedFunc(ANUNodeConstRef node) {
  return resolveRef(node)->getDirtiedFunc();
}

void ANUNodeInsertChild(
    const ANUNodeRef ownerRef,
    const ANUNodeRef childRef,
    const size_t index) {
  auto owner = resolveRef(ownerRef);
  auto child = resolveRef(childRef);

  anu::assertFatalWithNode(
      owner,
      child->getOwner() == nullptr,
      "Child already has a owner, it must be removed first.");

  anu::assertFatalWithNode(
      owner,
      !owner->hasMeasureFunc(),
      "Cannot add child: Nodes with measure functions cannot have children.");

  owner->insertChild(child, index);
  child->setOwner(owner);
  owner->markDirtyAndPropagate();
}

void ANUNodeSwapChild(
    const ANUNodeRef ownerRef,
    const ANUNodeRef childRef,
    const size_t index) {
  auto owner = resolveRef(ownerRef);
  auto child = resolveRef(childRef);

  owner->replaceChild(child, index);
  child->setOwner(owner);
}

void ANUNodeRemoveChild(
    const ANUNodeRef ownerRef,
    const ANUNodeRef excludedChildRef) {
  auto owner = resolveRef(ownerRef);
  auto excludedChild = resolveRef(excludedChildRef);

  if (owner->getChildCount() == 0) {
    // This is an empty set. Nothing to remove.
    return;
  }

  // Children may be shared between parents, which is indicated by not having an
  // owner. We only want to reset the child completely if it is owned
  // exclusively by one node.
  auto childOwner = excludedChild->getOwner();
  if (owner->removeChild(excludedChild)) {
    if (owner == childOwner) {
      excludedChild->setLayout({}); // layout is no longer valid
      excludedChild->setOwner(nullptr);
    }
    owner->markDirtyAndPropagate();
  }
}

void ANUNodeRemoveAllChildren(const ANUNodeRef ownerRef) {
  auto owner = resolveRef(ownerRef);

  const size_t childCount = owner->getChildCount();
  if (childCount == 0) {
    // This is an empty set already. Nothing to do.
    return;
  }
  auto* firstChild = owner->getChild(0);
  if (firstChild->getOwner() == owner) {
    // If the first child has this node as its owner, we assume that this child
    // set is unique.
    for (size_t i = 0; i < childCount; i++) {
      anu::Node* oldChild = owner->getChild(i);
      oldChild->setLayout({}); // layout is no longer valid
      oldChild->setOwner(nullptr);
    }
    owner->clearChildren();
    owner->markDirtyAndPropagate();
    return;
  }
  // Otherwise, we are not the owner of the child set. We don't have to do
  // anything to clear it.
  owner->setChildren({});
  owner->markDirtyAndPropagate();
}

void ANUNodeSetChildren(
    const ANUNodeRef ownerRef,
    const ANUNodeRef* childrenRefs,
    const size_t count) {
  auto owner = resolveRef(ownerRef);
  auto children = reinterpret_cast<anu::Node* const*>(childrenRefs);

  if (owner == nullptr) {
    return;
  }

  const std::vector<anu::Node*> childrenVector = {children, children + count};
  if (childrenVector.empty()) {
    if (owner->getChildCount() > 0) {
      for (auto* child : owner->getChildren()) {
        child->setLayout({});
        child->setOwner(nullptr);
      }
      owner->setChildren({});
      owner->markDirtyAndPropagate();
    }
  } else {
    if (owner->getChildCount() > 0) {
      for (auto* oldChild : owner->getChildren()) {
        // Our new children may have nodes in common with the old children. We
        // don't reset these common nodes.
        if (std::find(childrenVector.begin(), childrenVector.end(), oldChild) ==
            childrenVector.end()) {
          oldChild->setLayout({});
          oldChild->setOwner(nullptr);
        }
      }
    }
    owner->setChildren(childrenVector);
    for (anu::Node* child : childrenVector) {
      child->setOwner(owner);
    }
    owner->markDirtyAndPropagate();
  }
}

ANUNodeRef ANUNodeGetChild(const ANUNodeRef nodeRef, const size_t index) {
  const auto node = resolveRef(nodeRef);

  if (index < node->getChildren().size()) {
    return node->getChild(index);
  }
  return nullptr;
}

size_t ANUNodeGetChildCount(const ANUNodeConstRef node) {
  return resolveRef(node)->getChildren().size();
}

ANUNodeRef ANUNodeGetOwner(const ANUNodeRef node) {
  return resolveRef(node)->getOwner();
}

ANUNodeRef ANUNodeGetParent(const ANUNodeRef node) {
  return resolveRef(node)->getOwner();
}

void ANUNodeSetConfig(ANUNodeRef node, ANUConfigRef config) {
  resolveRef(node)->setConfig(resolveRef(config));
}

ANUConfigConstRef ANUNodeGetConfig(ANUNodeRef node) {
  return resolveRef(node)->getConfig();
}

void ANUNodeSetContext(ANUNodeRef node, void* context) {
  return resolveRef(node)->setContext(context);
}

void* ANUNodeGetContext(ANUNodeConstRef node) {
  return resolveRef(node)->getContext();
}

void ANUNodeSetMeasureFunc(ANUNodeRef node, ANUMeasureFunc measureFunc) {
  resolveRef(node)->setMeasureFunc(measureFunc);
}

bool ANUNodeHasMeasureFunc(ANUNodeConstRef node) {
  return resolveRef(node)->hasMeasureFunc();
}

void ANUNodeSetBaselineFunc(ANUNodeRef node, ANUBaselineFunc baselineFunc) {
  resolveRef(node)->setBaselineFunc(baselineFunc);
}

bool ANUNodeHasBaselineFunc(ANUNodeConstRef node) {
  return resolveRef(node)->hasBaselineFunc();
}

void ANUNodeSetIsReferenceBaseline(ANUNodeRef nodeRef, bool isReferenceBaseline) {
  const auto node = resolveRef(nodeRef);
  if (node->isReferenceBaseline() != isReferenceBaseline) {
    node->setIsReferenceBaseline(isReferenceBaseline);
    node->markDirtyAndPropagate();
  }
}

bool ANUNodeIsReferenceBaseline(ANUNodeConstRef node) {
  return resolveRef(node)->isReferenceBaseline();
}

void ANUNodeSetNodeType(ANUNodeRef node, ANUNodeType nodeType) {
  return resolveRef(node)->setNodeType(scopedEnum(nodeType));
}

ANUNodeType ANUNodeGetNodeType(ANUNodeConstRef node) {
  return unscopedEnum(resolveRef(node)->getNodeType());
}

void ANUNodeSetAlwaysFormsContainingBlock(
    ANUNodeRef node,
    bool alwaysFormsContainingBlock) {
  resolveRef(node)->setAlwaysFormsContainingBlock(alwaysFormsContainingBlock);
}

bool ANUNodeGetAlwaysFormsContainingBlock(ANUNodeConstRef node) {
  return resolveRef(node)->alwaysFormsContainingBlock();
}

// TODO: This leaks internal details to the public API. Remove after removing
// ComponentKit usage of it.
bool ANUNodeCanUseCachedMeasurement(
    ANUMeasureMode widthMode,
    float availableWidth,
    ANUMeasureMode heightMode,
    float availableHeight,
    ANUMeasureMode lastWidthMode,
    float lastAvailableWidth,
    ANUMeasureMode lastHeightMode,
    float lastAvailableHeight,
    float lastComputedWidth,
    float lastComputedHeight,
    float marginRow,
    float marginColumn,
    ANUConfigRef config) {
  return anu::canUseCachedMeasurement(
      sizingMode(scopedEnum(widthMode)),
      availableWidth,
      sizingMode(scopedEnum(heightMode)),
      availableHeight,
      sizingMode(scopedEnum(lastWidthMode)),
      lastAvailableWidth,
      sizingMode(scopedEnum(lastHeightMode)),
      lastAvailableHeight,
      lastComputedWidth,
      lastComputedHeight,
      marginRow,
      marginColumn,
      resolveRef(config));
}
