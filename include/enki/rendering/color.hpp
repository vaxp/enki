#pragma once
/// @file color.hpp
/// @brief Color utilities.

#include "enki/core/types.hpp"

namespace enki {

/// Common colors.
namespace Colors {
    inline constexpr Color Transparent = 0x00000000;
    inline constexpr Color Black       = 0xFF000000;
    inline constexpr Color White       = 0xFFFFFFFF;
    inline constexpr Color Red         = 0xFFFF0000;
    inline constexpr Color Green       = 0xFF00FF00;
    inline constexpr Color Blue        = 0xFF0000FF;
    inline constexpr Color Yellow      = 0xFFFFFF00;
    inline constexpr Color Cyan        = 0xFF00FFFF;
    inline constexpr Color Magenta     = 0xFFFF00FF;
    inline constexpr Color Gray        = 0xFF808080;
}

/// Create a color from ARGB values (0-255).
constexpr Color makeColorARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(b);
}

/// Create a color from RGB values (0-255), with full opacity.
constexpr Color makeColorRGB(uint8_t r, uint8_t g, uint8_t b) {
    return makeColorARGB(255, r, g, b);
}

}  // namespace enki
