#pragma once
/// @file palette.hpp
/// @brief Centralized design tokens and color scheme for ENKI Gallery.

#include "enki/core/types.hpp"

namespace enki::gallery::Palette {
    constexpr Color bg_dark        = 0xFF0B0F19; // Deep Obsidian
    constexpr Color bg_card        = 0xFF141D2E; // Elevated Slate Card
    constexpr Color bg_card_hover  = 0xFF1E293B; // Slate 800 Hover
    constexpr Color border_subtle  = 0x3038BDF8; // Subtle Cyan Border
    constexpr Color border_bright  = 0x8038BDF8; // Bright Cyan Accent
    constexpr Color primary        = 0xFF0284C7; // Sky 600
    constexpr Color primary_neon   = 0xFF38BDF8; // Sky 400
    constexpr Color emerald        = 0xFF10B981; // Success Green
    constexpr Color amber          = 0xFFF59E0B; // Warning Amber
    constexpr Color rose           = 0xFFEF4444; // Error / Favorite Rose
    constexpr Color text_white     = 0xFFF8FAFC;
    constexpr Color text_muted     = 0xFF94A3B8;
    constexpr Color fullscreen_bg  = 0xF8050811; // 97% Opaque Deep Obsidian Backdrop
}
