/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieShaper_DEFINED
#define SkottieShaper_DEFINED

#include <cstdint>

namespace skottie {
namespace Shaper {

enum class VAlign : uint8_t {
    kTop,
    kTopBaseline,
    kVisualCenter,
    kVisualBottom
};

enum class ResizePolicy : uint8_t {
    kNone,
    kScaleToFit,
    kDownscaleToFit
};

enum class LinebreakPolicy : uint8_t {
    kExplicit,
    kParagraph
};

enum class Direction : uint8_t {
    kLTR,
    kRTL
};

enum class Capitalization : uint8_t {
    kNone,
    kUpperCase,
    kLowerCase,
    kTitleCase
};

} // namespace Shaper
} // namespace skottie

#endif // SkottieShaper_DEFINED
