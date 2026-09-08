#pragma once
/// @file calculator_theme.hpp
/// @brief Cyber-Aero Quantum Glass Visual Assets, SkSL Shaders, and SVGs for ENKI Calculator.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"
#include <string>

namespace enki::calc {

// ════════════════════════════════════════════════════════════════
// SkSL Shaders
// ════════════════════════════════════════════════════════════════

/// 1. Window Background Ambient Aurora Shader
inline const std::string SHADER_WINDOW_BACKGROUND = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        vec2 center = uv - vec2(0.5, 0.4);
        float dist = length(center);

        // Subtle slow ambient breathing wave
        float wave = sin(dist * 14.0 - time * 1.2) * 0.5 + 0.5;

        // Dark OLED glass backdrop palette
        vec3 bgDeep   = vec3(0.035, 0.047, 0.082); // #090C15
        vec3 bgCard   = vec3(0.063, 0.082, 0.133); // #101522
        vec3 cyanGlow = vec3(0.00, 0.85, 1.00) * 0.08;
        vec3 purpGlow = vec3(0.70, 0.10, 0.95) * 0.07;

        vec3 color = mix(bgDeep, bgCard, uv.y * 1.2);
        vec3 aurora = mix(cyanGlow, purpGlow, sin(uv.x * 3.1415 + time * 0.6) * 0.5 + 0.5);
        color += aurora * wave;

        return vec4(color, 0.96);
    }
)";

/// 2. Outer Window Perimeter Neon Shader
inline const std::string SHADER_WINDOW_BORDER = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float angle = atan(uv.y - 0.5, uv.x - 0.5);
        float sweep = sin(angle * 2.0 + time * 2.0) * 0.5 + 0.5;

        vec3 cyan   = vec3(0.00, 0.90, 1.00); // #00E5FF
        vec3 purple = vec3(0.74, 0.15, 1.00); // #BD26FF
        vec3 color  = mix(cyan, purple, sweep);

        return vec4(color, 1.0);
    }
)";

/// 3. Hero Equals Key (=) SkSL Energy Core Shader
inline const std::string SHADER_EQUALS_KEY = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float t = time * 3.0;
        float pulse = sin(uv.x * 6.0 + uv.y * 4.0 - t) * 0.5 + 0.5;

        vec3 cyan   = vec3(0.00, 0.85, 0.98);
        vec3 magenta = vec3(0.92, 0.15, 0.82);
        vec3 color = mix(cyan, magenta, uv.x * 0.6 + pulse * 0.4);

        // Edge specular gloss
        float gloss = pow(1.0 - uv.y, 2.5) * 0.35;
        color += vec3(gloss);

        return vec4(color, 1.0);
    }
)";

/// 4. Clear Key (AC) Subtle Amber Plasma Shader
inline const std::string SHADER_CLEAR_KEY = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float pulse = sin(time * 2.5) * 0.5 + 0.5;
        vec3 deepRed = vec3(0.55, 0.08, 0.12);
        vec3 brightRed = vec3(0.90, 0.20, 0.22);
        return vec4(mix(deepRed, brightRed, pulse * 0.3 + uv.y * 0.4), 0.9);
    }
)";

// ════════════════════════════════════════════════════════════════
// Vector SVG Assets
// ════════════════════════════════════════════════════════════════

/// Sci-Fi HUD Chamfered Screen Border SVG
inline const std::string SVG_HUD_SCREEN_FRAME = R"(
<svg viewBox="0 0 400 130">
    <path d="M 0 16 L 16 0 H 384 L 400 16 V 114 L 384 130 H 16 L 0 114 Z"
          fill="#0C1220" stroke="#00E5FF" stroke-width="1.5"/>
    <path d="M 24 4 H 100" stroke="#00E5FF" stroke-width="2"/>
    <path d="M 376 126 H 300" stroke="#BD00FF" stroke-width="2"/>
    <polygon points="8,16 16,8 20,12 12,20" fill="#00E5FF"/>
    <polygon points="392,114 384,122 380,118 388,110" fill="#BD00FF"/>
</svg>
)";

/// Quantum Hexagon Icon SVG
inline const std::string SVG_QUANTUM_HEXAGON = R"(
<svg viewBox="0 0 40 40">
    <polygon points="20,2 36,11 36,29 20,38 4,29 4,11" fill="none" stroke="#00E5FF" stroke-width="2"/>
    <circle cx="20" cy="20" r="4" fill="#BD00FF"/>
</svg>
)";

// ════════════════════════════════════════════════════════════════
// Color Constants
// ════════════════════════════════════════════════════════════════

namespace colors {
    constexpr Color WindowBg          = 0x4D000000;
    constexpr Color DisplayBg         = 0x4D000000;
    constexpr Color KeypadBg          = 0x331E293B;

    // Keys
    constexpr Color KeyNumBg          = 0x331E293B; // 20% Slate Glass
    constexpr Color KeyNumBorder      = 0x2E475569;
    constexpr Color KeyNumText        = 0xFFF8FAFC;

    constexpr Color KeyOpBg           = 0x381E1B4B; // Indigo Tint
    constexpr Color KeyOpBorder       = 0x4D6366F1;
    constexpr Color KeyOpText         = 0xFF38BDF8;

    constexpr Color KeySciBg          = 0x2B151F32; // Cyan Slate
    constexpr Color KeySciBorder      = 0x3338BDF8;
    constexpr Color KeySciText        = 0xFF94A3B8;

    constexpr Color KeyActionBg       = 0x33450A0A; // Crimson Tint
    constexpr Color KeyActionBorder   = 0x66EF4444;
    constexpr Color KeyActionText     = 0xFFFCA5A5;

    constexpr Color KeyEqualsText     = 0xFFFFFFFF;
    constexpr Color AccentCyan        = 0xFF00E5FF;
    constexpr Color AccentMagenta     = 0xFFBD00FF;
    constexpr Color AccentGold        = 0xFFF59E0B;
}

} // namespace enki::calc
