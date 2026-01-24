#pragma once
/// @file animation.hpp
/// @brief ENKI Master Animation Header.
///
/// Includes all animation engines, physics simulations, particle systems,
/// transition curves, and controllers provided by the ENKI framework:
///   - Ticker & VSync synchronization
///   - Curves & Easing functions
///   - Tween interpolation
///   - AnimationController & Timeline
///   - SpringController & SpringSimulation (Physics)
///   - ParticleSystem & ParticleEmitter
///   - PathMorph & SVG Morphing
///   - Staggered Animations
///   - Lottie Composition & Controller
///
/// Usage:
/// @code
///   #include "enki/animation.hpp"
///   // or #include "animation.hpp"
/// @endcode
///
/// @copyright ENKI Framework — MIT License

// ── Timing & Core Animation ─────────────────────────────────────
#include "enki/animation/ticker.hpp"
#include "enki/animation/curves.hpp"
#include "enki/animation/tween.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/timeline.hpp"
#include "enki/animation/stagger.hpp"

// ── Physics & Spring Simulations ────────────────────────────────
#include "enki/animation/spring_simulation.hpp"
#include "enki/animation/spring_controller.hpp"

// ── Particles, Vector Morphing & Vector Graphics ────────────────
#include "enki/animation/particle_system.hpp"
#include "enki/animation/path_morph.hpp"
#include "enki/animation/lottie_controller.hpp"
