#pragma once
/// @file theme.hpp
/// @brief Theme system — provides design tokens for UI components.

#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"
#include <string>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ColorScheme — material & dark neon color palette
// ════════════════════════════════════════════════════════════════

struct ColorScheme {
    Color primary        = 0xFF6200EE;
    Color onPrimary      = 0xFFFFFFFF;
    Color secondary      = 0xFF03DAC6;
    Color onSecondary    = 0xFF000000;
    Color background     = 0xFFF5F5F5;
    Color onBackground   = 0xFF000000;
    Color surface        = 0xFFFFFFFF;
    Color onSurface      = 0xFF000000;
    Color error          = 0xFFB00020;
    Color onError        = 0xFFFFFFFF;
    Color outline        = 0xFFE0E0E0;
    Color shadow         = 0x40000000;

    static ColorScheme light() {
        return ColorScheme{};
    }

    static ColorScheme dark() {
        return ColorScheme{
            .primary      = 0xFFBB86FC,
            .onPrimary    = 0xFF000000,
            .secondary    = 0xFF03DAC6,
            .onSecondary  = 0xFF000000,
            .background   = 0xFF121212,
            .onBackground = 0xFFFFFFFF,
            .surface      = 0xFF1E1E1E,
            .onSurface    = 0xFFFFFFFF,
            .error        = 0xFFCF6679,
            .onError      = 0xFF000000,
            .outline      = 0xFF333333,
            .shadow       = 0x60000000,
        };
    }
};

// ════════════════════════════════════════════════════════════════
// ThemeData — complete theme definition
// ════════════════════════════════════════════════════════════════

struct ThemeData {
    ColorScheme colorScheme;
    float defaultBorderRadius = 12.0f;
    EdgeInsets defaultPadding  = EdgeInsets::all(16.0f);
    float elevation            = 4.0f;

    static ThemeData light() {
        return ThemeData{ .colorScheme = ColorScheme::light() };
    }

    static ThemeData dark() {
        return ThemeData{ .colorScheme = ColorScheme::dark() };
    }
};

} // namespace enki
