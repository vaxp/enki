/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdbool.h>

#include <layout_engine/AnuConfig.h>
#include <layout_engine/AnuEnums.h>
#include <layout_engine/AnuMacros.h>

ANU_EXTERN_C_BEGIN

ANU_EXPORT float ANUNodeLayoutGetLeft(ANUNodeConstRef node);
ANU_EXPORT float ANUNodeLayoutGetTop(ANUNodeConstRef node);
ANU_EXPORT float ANUNodeLayoutGetRight(ANUNodeConstRef node);
ANU_EXPORT float ANUNodeLayoutGetBottom(ANUNodeConstRef node);
ANU_EXPORT float ANUNodeLayoutGetWidth(ANUNodeConstRef node);
ANU_EXPORT float ANUNodeLayoutGetHeight(ANUNodeConstRef node);
ANU_EXPORT ANUDirection ANUNodeLayoutGetDirection(ANUNodeConstRef node);
ANU_EXPORT bool ANUNodeLayoutGetHadOverflow(ANUNodeConstRef node);

// Get the computed values for these nodes after performing layout. If they were
// set using point values then the returned value will be the same as
// ANUNodeStyleGetXXX. However if they were set using a percentage value then the
// returned value is the computed value used during layout.
ANU_EXPORT float ANUNodeLayoutGetMargin(ANUNodeConstRef node, ANUEdge edge);
ANU_EXPORT float ANUNodeLayoutGetBorder(ANUNodeConstRef node, ANUEdge edge);
ANU_EXPORT float ANUNodeLayoutGetPadding(ANUNodeConstRef node, ANUEdge edge);

ANU_EXTERN_C_END
