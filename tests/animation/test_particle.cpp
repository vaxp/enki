/// @file test_particle.cpp
/// @brief Tests for 2D Skia Particle Physics System and ParticleEmitter widget.

#include "enki/animation/particle_system.hpp"
#include "enki/widgets/particle_emitter.hpp"
#include "enki/widgets/container.hpp"
#include <cassert>
#include <cstdio>

using namespace enki;

void test_particle_presets() {
    auto confetti = ParticlePresets::confetti();
    assert(confetti.burst_mode);
    assert(confetti.max_particles > 0);

    auto sparks = ParticlePresets::neonSparks();
    assert(sparks.burst_mode);

    auto dust = ParticlePresets::ambientDust();
    assert(!dust.burst_mode);
    assert(dust.emission_rate > 0.0f);

    printf("  [PASS] particle presets initialization\n");
}

void test_particle_burst_and_update() {
    ParticleSystem system(ParticlePresets::confetti());
    assert(system.activeCount() == 0);

    system.burst({150.0f, 150.0f});
    assert(system.hasActiveParticles());
    assert(system.activeCount() > 0);

    // Update simulation by 100ms
    system.update(0.1f);
    assert(system.hasActiveParticles());

    // Run until all settled
    system.clear();
    assert(system.activeCount() == 0);
    assert(!system.hasActiveParticles());

    printf("  [PASS] particle burst, physics update, and clear\n");
}

void test_particle_emitter_widget() {
    WidgetPtr w = particleEmitter({
        .preset = ParticlePreset::NeonSparks,
        .child = container({
            .color = 0xFF1E293B,
            .width = 120.0f,
            .height = 48.0f,
        }),
    });

    assert(w != nullptr);
    printf("  [PASS] particleEmitter declarative widget creation\n");
}

int main() {
    printf("Running Particle System tests...\n");
    test_particle_presets();
    test_particle_burst_and_update();
    test_particle_emitter_widget();
    printf("All Particle System tests passed successfully!\n");
    return 0;
}
