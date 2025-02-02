/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <layout_engine/AnuConfig.h>
#include <layout_engine/AnuEnums.h>
#include <layout_engine/AnuMacros.h>

ANU_EXTERN_C_BEGIN

/**
 * Handle to a mutable Anu Node.
 */
typedef struct ANUNode* ANUNodeRef;

/**
 * Handle to an immutable Anu Node.
 */
typedef const struct ANUNode* ANUNodeConstRef;

/**
 * Heap allocates and returns a new Anu node using Anu settings.
 */
ANU_EXPORT ANUNodeRef ANUNodeNew(void);

/**
 * Heap allocates and returns a new Anu node, with customized settings.
 */
ANU_EXPORT ANUNodeRef ANUNodeNewWithConfig(ANUConfigConstRef config);

/**
 * Returns a mutable copy of an existing node, with the same context and
 * children, but no owner set. Does not call the function set by
 * ANUConfigSetCloneNodeFunc().
 */
ANU_EXPORT ANUNodeRef ANUNodeClone(ANUNodeConstRef node);

/**
 * Frees the Anu node, disconnecting it from its owner and children.
 */
ANU_EXPORT void ANUNodeFree(ANUNodeRef node);

/**
 * Frees the subtree of Anu nodes rooted at the given node.
 */
ANU_EXPORT void ANUNodeFreeRecursive(ANUNodeRef node);

/**
 * Frees the Anu node without disconnecting it from its owner or children.
 * Allows garbage collecting Anu nodes in parallel when the entire tree is
 * unreachable.
 */
ANU_EXPORT void ANUNodeFinalize(ANUNodeRef node);

/**
 * Resets the node to its default state.
 */
ANU_EXPORT void ANUNodeReset(ANUNodeRef node);

/**
 * Calculates the layout of the tree rooted at the given node.
 *
 * Layout results may be read after calling ANUNodeCalculateLayout() using
 * functions like ANUNodeLayoutGetLeft(), ANUNodeLayoutGetTop(), etc.
 *
 * ANUNodeGetHasNewLayout() may be read to know if the layout of the node or its
 * subtrees may have changed since the last time ANUNodeCalculate() was called.
 */
ANU_EXPORT void ANUNodeCalculateLayout(
    ANUNodeRef node,
    float availableWidth,
    float availableHeight,
    ANUDirection ownerDirection);

/**
 * Whether the given node may have new layout results. Must be reset by calling
 * ANUNodeSetHasNewLayout().
 */
ANU_EXPORT bool ANUNodeGetHasNewLayout(ANUNodeConstRef node);

/**
 * Sets whether a nodes layout is considered new.
 */
ANU_EXPORT void ANUNodeSetHasNewLayout(ANUNodeRef node, bool hasNewLayout);

/**
 * Whether the node's layout results are dirty due to it or its children
 * changing.
 */
ANU_EXPORT bool ANUNodeIsDirty(ANUNodeConstRef node);

/**
 * Marks a node with custom measure function as dirty.
 */
ANU_EXPORT void ANUNodeMarkDirty(ANUNodeRef node);

typedef void (*ANUDirtiedFunc)(ANUNodeConstRef node);

/**
 * Called when a change is made to the Anu tree which dirties this node.
 */
ANU_EXPORT void ANUNodeSetDirtiedFunc(ANUNodeRef node, ANUDirtiedFunc dirtiedFunc);

/**
 * Returns a dirtied func if set.
 */
ANU_EXPORT ANUDirtiedFunc ANUNodeGetDirtiedFunc(ANUNodeConstRef node);

/**
 * Inserts a child node at the given index.
 */
ANU_EXPORT void ANUNodeInsertChild(ANUNodeRef node, ANUNodeRef child, size_t index);

/**
 * Replaces the child node at a given index with a new one.
 */
ANU_EXPORT void ANUNodeSwapChild(ANUNodeRef node, ANUNodeRef child, size_t index);

/**
 * Removes the given child node.
 */
ANU_EXPORT void ANUNodeRemoveChild(ANUNodeRef node, ANUNodeRef child);

/**
 * Removes all children nodes.
 */
ANU_EXPORT void ANUNodeRemoveAllChildren(ANUNodeRef node);

/**
 * Sets children according to the given list of nodes.
 */
ANU_EXPORT void
ANUNodeSetChildren(ANUNodeRef owner, const ANUNodeRef* children, size_t count);

/**
 * Get the child node at a given index.
 */
ANU_EXPORT ANUNodeRef ANUNodeGetChild(ANUNodeRef node, size_t index);

/**
 * The number of child nodes.
 */
ANU_EXPORT size_t ANUNodeGetChildCount(ANUNodeConstRef node);

/**
 * Get the parent/owner currently set for a node.
 */
ANU_EXPORT ANUNodeRef ANUNodeGetOwner(ANUNodeRef node);

/**
 * Get the parent/owner currently set for a node.
 */
ANU_EXPORT ANUNodeRef ANUNodeGetParent(ANUNodeRef node);

/**
 * Set a new config for the node after creation.
 */
ANU_EXPORT void ANUNodeSetConfig(ANUNodeRef node, ANUConfigRef config);

/**
 * Get the config currently set on the node.
 */
ANU_EXPORT ANUConfigConstRef ANUNodeGetConfig(ANUNodeRef node);

/**
 * Sets extra data on the Anu node which may be read from during callbacks.
 */
ANU_EXPORT void ANUNodeSetContext(ANUNodeRef node, void* context);

/**
 * Returns the context or NULL if no context has been set.
 */
ANU_EXPORT void* ANUNodeGetContext(ANUNodeConstRef node);

typedef struct ANUSize {
  float width;
  float height;
} ANUSize;

/**
 * Returns the computed dimensions of the node, following the constraints of
 * `widthMode` and `heightMode`:
 *
 * ANUMeasureModeUndefined: The parent has not imposed any constraint on the
 * child. It can be whatever size it wants.
 *
 * ANUMeasureModeAtMost: The child can be as large as it wants up to the
 * specified size.
 *
 * ANUMeasureModeExactly: The parent has determined an exact size for the
 * child. The child is going to be given those bounds regardless of how big it
 * wants to be.
 *
 * @returns the size of the leaf node, measured under the given constraints.
 */
typedef ANUSize (*ANUMeasureFunc)(
    ANUNodeConstRef node,
    float width,
    ANUMeasureMode widthMode,
    float height,
    ANUMeasureMode heightMode);

/**
 * Allows providing custom measurements for a Anu leaf node (usually for
 * measuring text). ANUNodeMarkDirty() must be set if content effecting the
 * measurements of the node changes.
 */
ANU_EXPORT void ANUNodeSetMeasureFunc(ANUNodeRef node, ANUMeasureFunc measureFunc);

/**
 * Whether a measure function is set.
 */
ANU_EXPORT bool ANUNodeHasMeasureFunc(ANUNodeConstRef node);

/**
 * @returns a defined offset to baseline (ascent).
 */
typedef float (*ANUBaselineFunc)(ANUNodeConstRef node, float width, float height);

/**
 * Set a custom function for determining the text baseline for use in baseline
 * alignment.
 */
ANU_EXPORT void ANUNodeSetBaselineFunc(
    ANUNodeRef node,
    ANUBaselineFunc baselineFunc);

/**
 * Whether a baseline function is set.
 */
ANU_EXPORT bool ANUNodeHasBaselineFunc(ANUNodeConstRef node);

/**
 * Sets this node should be considered the reference baseline among siblings.
 */
ANU_EXPORT void ANUNodeSetIsReferenceBaseline(
    ANUNodeRef node,
    bool isReferenceBaseline);

/**
 * Whether this node is set as the reference baseline.
 */
ANU_EXPORT bool ANUNodeIsReferenceBaseline(ANUNodeConstRef node);

/**
 * Sets whether a leaf node's layout results may be truncated during layout
 * rounding.
 */
ANU_EXPORT void ANUNodeSetNodeType(ANUNodeRef node, ANUNodeType nodeType);

/**
 * Wwhether a leaf node's layout results may be truncated during layout
 * rounding.
 */
ANU_EXPORT ANUNodeType ANUNodeGetNodeType(ANUNodeConstRef node);

/**
 * Make it so that this node will always form a containing block for any
 * descendant nodes. This is useful for when a node has a property outside of
 * of Anu that will form a containing block. For example, transforms or some of
 * the others listed in
 * https://developer.mozilla.org/en-US/docs/Web/CSS/Containing_block
 */
ANU_EXPORT void ANUNodeSetAlwaysFormsContainingBlock(
    ANUNodeRef node,
    bool alwaysFormsContainingBlock);

/**
 * Whether the node will always form a containing block for any descendant. This
 * can happen in situation where the client implements something like a
 * transform that can affect containing blocks but is not handled by Anu
 * directly.
 */
ANU_EXPORT bool ANUNodeGetAlwaysFormsContainingBlock(ANUNodeConstRef node);

/**
 * @deprecated
 */
ANU_DEPRECATED(
    "ANUNodeCanUseCachedMeasurement may be removed in a future version of Anu")
ANU_EXPORT bool ANUNodeCanUseCachedMeasurement(
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
    ANUConfigRef config);

ANU_EXTERN_C_END
