/// @file particle_system.cpp
/// @brief 2D Skia Particle Physics Simulation implementation.

#include "enki/animation/particle_system.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ParticlePresets
// ════════════════════════════════════════════════════════════════

ParticleConfig ParticlePresets::confetti() {
    ParticleConfig cfg;
    cfg.max_particles = 120;
    cfg.emission_rate = 0.0f;
    cfg.burst_mode = true;
    cfg.gravity = {0.0f, 350.0f};
    cfg.drag = 0.015f;
    cfg.min_life = 1.2f;
    cfg.max_life = 2.4f;
    cfg.min_speed = 220.0f;
    cfg.max_speed = 520.0f;
    // Upward cone
    cfg.min_angle_rad = -3.14159f * 0.85f;
    cfg.max_angle_rad = -3.14159f * 0.15f;
    cfg.min_size = 5.0f;
    cfg.max_size = 10.0f;
    cfg.shape = ParticleShape::ConfettiRibbon;
    cfg.color_palette = {0xFFFF3366, 0xFFFFCC00, 0xFF00FFCC, 0xFF3B82F6, 0xFFA855F7, 0xFFFFFFFF};
    return cfg;
}

ParticleConfig ParticlePresets::neonSparks() {
    ParticleConfig cfg;
    cfg.max_particles = 80;
    cfg.emission_rate = 0.0f;
    cfg.burst_mode = true;
    cfg.gravity = {0.0f, 200.0f};
    cfg.drag = 0.04f;
    cfg.min_life = 0.4f;
    cfg.max_life = 1.0f;
    cfg.min_speed = 150.0f;
    cfg.max_speed = 400.0f;
    cfg.min_angle_rad = 0.0f;
    cfg.max_angle_rad = 6.2831853f; // Full radial
    cfg.min_size = 2.0f;
    cfg.max_size = 5.0f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {0xFF00FFFF, 0xFF7000FF, 0xFFFF0055, 0xFFFFFFFF};
    return cfg;
}

ParticleConfig ParticlePresets::ambientDust() {
    ParticleConfig cfg;
    cfg.max_particles = 60;
    cfg.emission_rate = 15.0f;
    cfg.burst_mode = false;
    cfg.gravity = {0.0f, -15.0f}; // Subtle upward float
    cfg.drag = 0.01f;
    cfg.min_life = 2.0f;
    cfg.max_life = 4.0f;
    cfg.min_speed = 10.0f;
    cfg.max_speed = 35.0f;
    cfg.min_angle_rad = 0.0f;
    cfg.max_angle_rad = 6.2831853f;
    cfg.min_size = 1.5f;
    cfg.max_size = 3.5f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {0x80FFFFFF, 0x6000FFFF, 0x50FFD700};
    return cfg;
}

ParticleConfig ParticlePresets::snowFall() {
    ParticleConfig cfg;
    cfg.max_particles = 100;
    cfg.emission_rate = 30.0f;
    cfg.burst_mode = false;
    cfg.gravity = {10.0f, 60.0f};
    cfg.drag = 0.005f;
    cfg.min_life = 3.0f;
    cfg.max_life = 5.0f;
    cfg.min_speed = 20.0f;
    cfg.max_speed = 50.0f;
    cfg.min_angle_rad = 0.5f;
    cfg.max_angle_rad = 1.2f;
    cfg.min_size = 2.0f;
    cfg.max_size = 5.0f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {0xEEFFFFFF, 0xCCDDFFFF, 0x99FFFFFF};
    return cfg;
}

ParticleConfig ParticlePresets::fromPreset(ParticlePreset preset) {
    switch (preset) {
        case ParticlePreset::ConfettiBurst: return confetti();
        case ParticlePreset::NeonSparks:    return neonSparks();
        case ParticlePreset::AmbientDust:   return ambientDust();
        case ParticlePreset::SnowFall:      return snowFall();
    }
    return confetti();
}

// ════════════════════════════════════════════════════════════════
// ParticleSystem Implementation
// ════════════════════════════════════════════════════════════════

ParticleSystem::ParticleSystem(const ParticleConfig& config) {
    setConfig(config);
}

void ParticleSystem::setConfig(const ParticleConfig& config) {
    config_ = config;
    pool_.clear();
    pool_.resize(config_.max_particles);
    is_emitting_ = !config_.burst_mode && (config_.emission_rate > 0.0f);
}

float ParticleSystem::randomFloat(float min_val, float max_val) {
    std::uniform_real_distribution<float> dist(min_val, max_val);
    return dist(rng_);
}

Color ParticleSystem::randomColor() {
    if (config_.color_palette.empty()) return 0xFFFFFFFF;
    std::uniform_int_distribution<size_t> dist(0, config_.color_palette.size() - 1);
    return config_.color_palette[dist(rng_)];
}

void ParticleSystem::spawnParticle(Point origin) {
    // Find first dead particle in pool
    for (auto& p : pool_) {
        if (!p.alive) {
            p.alive = true;
            p.position = origin;

            float speed = randomFloat(config_.min_speed, config_.max_speed);
            float angle = randomFloat(config_.min_angle_rad, config_.max_angle_rad);
            p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};

            p.max_life = randomFloat(config_.min_life, config_.max_life);
            p.life = p.max_life;
            p.size = randomFloat(config_.min_size, config_.max_size);
            p.rotation = randomFloat(0.0f, 6.2831853f);
            p.rotation_speed = randomFloat(-5.0f, 5.0f);
            p.color = randomColor();
            return;
        }
    }
}

void ParticleSystem::burst(Point origin) {
    for (size_t i = 0; i < config_.max_particles; ++i) {
        spawnParticle(origin);
    }
}

void ParticleSystem::clear() {
    for (auto& p : pool_) {
        p.alive = false;
    }
}

size_t ParticleSystem::activeCount() const {
    size_t count = 0;
    for (const auto& p : pool_) {
        if (p.alive) ++count;
    }
    return count;
}

bool ParticleSystem::hasActiveParticles() const {
    for (const auto& p : pool_) {
        if (p.alive) return true;
    }
    return false;
}

void ParticleSystem::update(float dt_sec, Size boundary) {
    if (is_emitting_) {
        emission_accumulator_ += config_.emission_rate * dt_sec;
        while (emission_accumulator_ >= 1.0f) {
            spawnParticle(emitter_origin_);
            emission_accumulator_ -= 1.0f;
        }
    }

    for (auto& p : pool_) {
        if (!p.alive) continue;

        p.life -= dt_sec;
        if (p.life <= 0.0f) {
            p.alive = false;
            continue;
        }

        // Apply forces
        p.velocity.x += config_.gravity.x * dt_sec;
        p.velocity.y += config_.gravity.y * dt_sec;

        p.velocity.x *= (1.0f - config_.drag);
        p.velocity.y *= (1.0f - config_.drag);

        p.position.x += p.velocity.x * dt_sec;
        p.position.y += p.velocity.y * dt_sec;

        p.rotation += p.rotation_speed * dt_sec;
    }
}

void ParticleSystem::render(Canvas& canvas, const Rect&) const {
    auto* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
    if (!sk_canvas) return;

    for (const auto& p : pool_) {
        if (!p.alive) continue;

        float norm_life = std::clamp(p.life / p.max_life, 0.0f, 1.0f);
        uint8_t base_alpha = (p.color >> 24) & 0xFF;
        uint8_t cur_alpha  = static_cast<uint8_t>(base_alpha * norm_life);

        Color active_color = (static_cast<Color>(cur_alpha) << 24) | (p.color & 0x00FFFFFF);

        SkPaint sk_paint;
        sk_paint.setAntiAlias(true);
        sk_paint.setColor(active_color);

        switch (config_.shape) {
            case ParticleShape::Circle: {
                sk_canvas->drawCircle(p.position.x, p.position.y, p.size, sk_paint);
                break;
            }
            case ParticleShape::Square: {
                SkRect r = SkRect::MakeXYWH(p.position.x - p.size, p.position.y - p.size,
                                           p.size * 2.0f, p.size * 2.0f);
                sk_canvas->drawRect(r, sk_paint);
                break;
            }
            case ParticleShape::ConfettiRibbon: {
                float flip = std::cos(p.rotation);
                sk_canvas->save();
                sk_canvas->translate(p.position.x, p.position.y);
                sk_canvas->rotate(p.rotation * 57.29578f);
                SkRect r = SkRect::MakeXYWH(-p.size, -p.size * flip * 0.5f,
                                           p.size * 2.0f, p.size * std::abs(flip));
                sk_canvas->drawRect(r, sk_paint);
                sk_canvas->restore();
                break;
            }
            case ParticleShape::Star: {
                // Approximate star with cross/circles for speed
                sk_canvas->drawCircle(p.position.x, p.position.y, p.size, sk_paint);
                break;
            }
        }
    }
}

} // namespace enki
