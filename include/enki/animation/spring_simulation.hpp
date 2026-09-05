#pragma once
/// @file spring_simulation.hpp
/// @brief Physics-based damped harmonic oscillator simulation for ENKI.
///
/// Features:
///   - Analytical, exact O(1) physics solution (Underdamped, Critically Damped, Overdamped).
///   - Glitch-free interruptibility preserving velocity on retargeting.
///   - Standard industry presets (bouncy, smooth, snappy, interactive, gentle).
///   - SpringCurve adapter enabling direct use with existing Curve APIs.
///
/// @copyright ENKI Framework — MIT License

#include "enki/animation/curves.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// SpringDescription — Physical parameters of a spring
// ════════════════════════════════════════════════════════════════

/// @brief Physical characteristics of a spring-mass-damper system.
struct SpringDescription {
    float mass      = 1.0f;    ///< Mass of the moving object (m > 0).
    float stiffness = 180.0f;  ///< Spring stiffness constant k (k > 0).
    float damping   = 12.0f;   ///< Damping drag coefficient c (c >= 0).

    constexpr SpringDescription() = default;
    constexpr SpringDescription(float m, float k, float d)
        : mass(m), stiffness(k), damping(d) {}

    /// Construct via damping ratio zeta and natural frequency omega0:
    /// k = m * omega0^2, c = 2 * zeta * sqrt(k * m)
    static SpringDescription fromRatioAndFrequency(float damping_ratio, float natural_frequency, float mass = 1.0f) {
        float stiffness = mass * natural_frequency * natural_frequency;
        float damping = 2.0f * damping_ratio * std::sqrt(stiffness * mass);
        return SpringDescription{mass, stiffness, damping};
    }

    /// @return Damping ratio zeta: c / (2 * sqrt(k * m))
    [[nodiscard]] float dampingRatio() const {
        if (mass <= 0.0f || stiffness <= 0.0f) return 1.0f;
        return damping / (2.0f * std::sqrt(stiffness * mass));
    }

    /// @return Undamped natural angular frequency omega0: sqrt(k / m)
    [[nodiscard]] float naturalFrequency() const {
        if (mass <= 0.0f || stiffness <= 0.0f) return 0.0f;
        return std::sqrt(stiffness / mass);
    }

    bool operator==(const SpringDescription&) const = default;
};

// ════════════════════════════════════════════════════════════════
// Springs — Standard Industry Presets
// ════════════════════════════════════════════════════════════════

/// @brief Curated, industry-standard spring configurations
struct Springs {
    /// Playful, bouncy spring with noticeable organic overshoot.
    static constexpr SpringDescription bouncy{1.0f, 180.0f, 12.0f};

    /// Buttery smooth critically damped spring, zero overshoot (SwiftUI default feel).
    static constexpr SpringDescription smooth{1.0f, 180.0f, 26.83f};

    /// Crisp, fast, highly responsive with subtle micro-bounce.
    static constexpr SpringDescription snappy{1.0f, 300.0f, 30.0f};

    /// Ultra responsive for direct gestures and drag tracking.
    static constexpr SpringDescription interactive{0.5f, 250.0f, 20.0f};

    /// Gentle, slow and graceful movement for large panels or modals.
    static constexpr SpringDescription gentle{1.0f, 100.0f, 15.0f};
};

// ════════════════════════════════════════════════════════════════
// SpringSimulation — Analytical Damped Harmonic Oscillator
// ════════════════════════════════════════════════════════════════

/// @brief Exact closed-form simulation of a spring moving towards an equilibrium target.
class SpringSimulation {
public:
    SpringSimulation() = default;

    /// @param desc Spring physical parameters (mass, stiffness, damping)
    /// @param start Initial position x(0)
    /// @param end Target equilibrium position
    /// @param velocity Initial velocity v(0)
    /// @param distance_tolerance Settling distance threshold
    /// @param velocity_tolerance Settling velocity threshold
    SpringSimulation(const SpringDescription& desc,
                     float start,
                     float end,
                     float velocity = 0.0f,
                     float distance_tolerance = 1e-3f,
                     float velocity_tolerance = 1e-2f);

    /// Compute position at elapsed time t (in seconds).
    [[nodiscard]] float x(float t) const;

    /// Compute velocity dx/dt at elapsed time t (in seconds).
    [[nodiscard]] float dx(float t) const;

    /// Check if the spring has settled within tolerance at time t (in seconds).
    [[nodiscard]] bool isDone(float t) const;

    /// Estimated total duration (in seconds) until spring settles.
    [[nodiscard]] float estimatedDuration() const { return estimated_duration_; }

    [[nodiscard]] float start() const { return start_; }
    [[nodiscard]] float end() const { return end_; }
    [[nodiscard]] const SpringDescription& description() const { return desc_; }

private:
    void solve();

    SpringDescription desc_{Springs::smooth};
    float start_              = 0.0f;
    float end_                = 1.0f;
    float initial_velocity_   = 0.0f;
    float distance_tolerance_ = 1e-3f;
    float velocity_tolerance_ = 1e-2f;

    enum class SolutionType {
        Underdamped,
        CriticallyDamped,
        Overdamped,
    } solution_type_ = SolutionType::CriticallyDamped;

    // Coefficients
    float c1_ = 0.0f;
    float c2_ = 0.0f;
    float omega0_ = 0.0f;
    float zeta_   = 1.0f;
    float omega_d_ = 0.0f; // For underdamped
    float r1_ = 0.0f;      // For overdamped
    float r2_ = 0.0f;      // For overdamped
    float estimated_duration_ = 0.4f;
};

// ════════════════════════════════════════════════════════════════
// SpringCurve — Adapter to use Spring physics as a standard Curve
// ════════════════════════════════════════════════════════════════

/// @brief Adapts a SpringDescription into an enki::Curve over a normalized [0, 1] duration.
class SpringCurve final : public Curve {
public:
    explicit SpringCurve(SpringDescription desc = Springs::bouncy, float settling_duration = 0.6f)
        : desc_(desc), duration_seconds_(std::max(0.01f, settling_duration)),
          sim_(desc, 0.0f, 1.0f, 0.0f, 1e-3f, 1e-2f) {}

    [[nodiscard]] double evaluate(double t) const override {
        if (t <= 0.0) return 0.0;
        if (t >= 1.0) return 1.0;
        float elapsed_s = static_cast<float>(t) * duration_seconds_;
        return static_cast<double>(sim_.x(elapsed_s));
    }

private:
    SpringDescription desc_;
    float duration_seconds_;
    SpringSimulation sim_;
};

} // namespace enki
