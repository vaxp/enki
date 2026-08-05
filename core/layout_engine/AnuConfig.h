/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <layout_engine/AnuEnums.h>
#include <layout_engine/AnuMacros.h>

ANU_EXTERN_C_BEGIN

typedef struct ANUNode* ANUNodeRef;
typedef const struct ANUNode* ANUNodeConstRef;

/**
 * Handle to a mutable Anu configuration.
 */
typedef struct ANUConfig* ANUConfigRef;

/**
 * Handle to an immutable Anu configuration.
 */
typedef const struct ANUConfig* ANUConfigConstRef;

/**
 * Allocates a set of configuration options. The configuration may be applied to
 * multiple nodes (i.e. a single global config), or can be applied more
 * granularly per-node.
 */
ANU_EXPORT ANUConfigRef ANUConfigNew(void);

/**
 * Frees the associated Anu configuration.
 */
ANU_EXPORT void ANUConfigFree(ANUConfigRef config);

/**
 * Returns the default config values set by Anu.
 */
ANU_EXPORT ANUConfigConstRef ANUConfigGetDefault(void);

/**
 * Anu by default creates new nodes with style defaults different from flexbox
 * on web (e.g. `ANUFlexDirectionColumn` and `ANUPositionRelative`).
 * `UseWebDefaults` instructs Anu to instead use a default style consistent
 * with the web.
 */
ANU_EXPORT void ANUConfigSetUseWebDefaults(ANUConfigRef config, bool enabled);

/**
 * Whether the configuration is set to use web defaults.
 */
ANU_EXPORT bool ANUConfigGetUseWebDefaults(ANUConfigConstRef config);

/**
 * Anu will by default round final layout positions and dimensions to the
 * nearst point. `pointScaleFactor` controls the density of the grid used for
 * layout rounding (e.g. to round to the closest display pixel).
 *
 * May be set to 0.0f to avoid rounding the layout results.
 */
ANU_EXPORT void ANUConfigSetPointScaleFactor(
    ANUConfigRef config,
    float pixelsInPoint);

/**
 * Get the currently set point scale factor.
 */
ANU_EXPORT float ANUConfigGetPointScaleFactor(ANUConfigConstRef config);

/**
 * Configures how Anu balances W3C conformance vs compatibility with layouts
 * created against earlier versions of Anu.
 *
 * By default Anu will prioritize W3C conformance. `Errata` may be set to ask
 * Anu to produce specific incorrect behaviors. E.g. `ANUConfigSetErrata(config,
 * ANUErrataStretchFlexBasis)`.
 *
 * ANUErrata is a bitmask, and multiple errata may be set at once. Predefined
 * constants exist for convenience:
 * 1. ANUErrataNone: No errata
 * 2. ANUErrataClassic: Match layout behaviors of Anu 1.x
 * 3. ANUErrataAll: Match layout behaviors of Anu 1.x, including
 * `UseLegacyStretchBehaviour`
 */
ANU_EXPORT void ANUConfigSetErrata(ANUConfigRef config, ANUErrata errata);

/**
 * Get the currently set errata.
 */
ANU_EXPORT ANUErrata ANUConfigGetErrata(ANUConfigConstRef config);

/**
 * Function pointer type for ANUConfigSetLogger.
 */
typedef int (*ANULogger)(
    ANUConfigConstRef config,
    ANUNodeConstRef node,
    ANULogLevel level,
    const char* format,
    va_list args);

/**
 * Set a custom log function for to use when logging diagnostics or fatal.
 * errors.
 */
ANU_EXPORT void ANUConfigSetLogger(ANUConfigRef config, ANULogger logger);

/**
 * Sets an arbitrary context pointer on the config which may be read from during
 * callbacks.
 */
ANU_EXPORT void ANUConfigSetContext(ANUConfigRef config, void* context);

/**
 * Gets the currently set context.
 */
ANU_EXPORT void* ANUConfigGetContext(ANUConfigConstRef config);

/**
 * Function pointer type for ANUConfigSetCloneNodeFunc.
 */
typedef ANUNodeRef (*ANUCloneNodeFunc)(
    ANUNodeConstRef oldNode,
    ANUNodeConstRef owner,
    size_t childIndex);

/**
 * Enable an experimental/unsupported feature in Anu.
 */
ANU_EXPORT void ANUConfigSetExperimentalFeatureEnabled(
    ANUConfigRef config,
    ANUExperimentalFeature feature,
    bool enabled);

/**
 * Whether an experimental feature is set.
 */
ANU_EXPORT bool ANUConfigIsExperimentalFeatureEnabled(
    ANUConfigConstRef config,
    ANUExperimentalFeature feature);

/**
 * Sets a callback, called during layout, to create a new mutable Anu node if
 * Anu must write to it and its owner is not its parent observed during layout.
 */
ANU_EXPORT void ANUConfigSetCloneNodeFunc(
    ANUConfigRef config,
    ANUCloneNodeFunc callback);

ANU_EXTERN_C_END
