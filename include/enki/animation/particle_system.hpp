#pragma once
/// @file particle_system.hpp
/// @brief Ultra high-performance 2D Skia Particle Simulation Engine for ENKI.
///
/// Features:
///   - Zero runtime allocations via pre-allocated contiguous particle pools.
///   - Physics simulation: Gravity, velocity, drag, turbulence, tumbling rotation.
///   - Built-in Presets: ConfettiBurst, NeonSparks, AmbientDust, SnowFall.
///   - Hardware-accelerated direct Skia batch rendering.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/animation/ticker.hpp"
#include <vector>
#include <random>
#include <memory>
#include <chrono>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Particle & ParticleShape
// ════════════════════════════════════════════════════════════════

enum class ParticleShape {
    Circle,
    Square,
    ConfettiRibbon,
    Star,
};

enum class ParticlePreset {
    ConfettiBurst,
    NeonSparks,
    AmbientDust,
    SnowFall,
};

struct Particle {
    Point position{0.0f, 0.0f};
    Point velocity{0.0f, 0.0f};
    Color color = 0xFFFFFFFF;
    float size  = 4.0f;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    float life  = 1.0f;
    float max_life = 1.0f;
    bool  alive = false;
};

// ════════════════════════════════════════════════════════════════
// ParticleConfig
// ════════════════════════════════════════════════════════════════

struct ParticleConfig {
    size_t             max_particles    = 150;
    float              emission_rate    = 60.0f;  ///< Particles per second (for continuous emitters)
    Point              gravity          = {0.0f, 400.0f}; ///< Gravity acceleration (pixels/s^2)
    float              drag             = 0.02f;  ///< Air resistance drag factor
    float              min_life         = 0.8f;
    float              max_life         = 1.8f;
    float              min_speed        = 120.0f;
    float              max_speed        = 320.0f;
    float              min_angle_rad    = 0.0f;
    float              max_angle_rad    = 6.2831853f; // 2 * PI
    float              min_size         = 4.0f;
    float              max_size         = 10.0f;
    std::vector<Color> color_palette    = {0xFFFF0055, 0xFF00FFCC, 0xFFFFCC00, 0xFF3B82F6, 0xFFA855F7};
    ParticleShape      shape            = ParticleShape::ConfettiRibbon;
    bool               burst_mode       = false;  ///< If true, emits all particles in one explosive burst
};

// ════════════════════════════════════════════════════════════════
// ParticlePresets
// ════════════════════════════════════════════════════════════════

struct ParticlePresets {
    static ParticleConfig confetti();
    static ParticleConfig neonSparks();
    static ParticleConfig ambientDust();
    static ParticleConfig snowFall();
    static ParticleConfig fromPreset(ParticlePreset preset);
};

// ════════════════════════════════════════════════════════════════
// ParticleSystem
// ════════════════════════════════════════════════════════════════

class ParticleSystem {
public:
    explicit ParticleSystem(const ParticleConfig& config = ParticlePresets::confetti());

    void setConfig(const ParticleConfig& config);
    [[nodiscard]] const ParticleConfig& config() const { return config_; }

    /// Trigger a burst from origin point (or center of bounds)
    void burst(Point origin);

    /// Set continuous emitter state
    void setEmitting(bool emitting) { is_emitting_ = emitting; }
    [[nodiscard]] bool isEmitting() const { return is_emitting_; }

    /// Set continuous emission spawn origin
    void setEmitterPosition(Point origin) { emitter_origin_ = origin; }

    /// Advance particle physics by delta time (in seconds)
    void update(float dt_sec, Size boundary = {1000.0f, 1000.0f});

    /// Render all active particles on Canvas
    void render(Canvas& canvas, const Rect& bounds) const;

    /// Clear all active particles
    void clear();

    [[nodiscard]] size_t activeCount() const;
    [[nodiscard]] bool hasActiveParticles() const;

private:
    void spawnParticle(Point origin);
    float randomFloat(float min_val, float max_val);
    Color randomColor();

    ParticleConfig config_;
    std::vector<Particle> pool_;
    Point emitter_origin_{100.0f, 100.0f};
    bool is_emitting_ = false;
    float emission_accumulator_ = 0.0f;

    mutable std::mt19937 rng_{42};
};

} // namespace enki
