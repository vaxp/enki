# 2D GPU Particle Physics System & Emitter

> High-performance 2D particle simulation engine and declarative emitter widget with zero runtime allocations and hardware-accelerated Skia batch rendering.

- **Header Files**:
  - `#include "enki/animation/particle_system.hpp"` (Core physics simulation, pre-allocated pool, and presets)
  - `#include "enki/widgets/particle_emitter.hpp"` (Declarative `ParticleEmitter` widget with automatic Ticker lifecycle)
- **Primary C++ Classes**:
  - `enki::ParticleSystem` (High-performance particle update, collision/drag physics, and Skia batch drawing)
  - `enki::ParticleEmitterProps` & `enki::ParticleEmitter` (Declarative widget struct supporting C++20 designated initializers)
  - `enki::ParticlePresets` (Industry presets: Confetti Burst, Neon Sparks, Ambient Dust, Snowfall)

---

## Overview & Architecture

Particle effects (such as celebration confetti, magical neon sparks, weather effects, and ambient dust) typically suffer from two architectural pitfalls: frequent heap allocations per particle (`malloc`/`new`), and continuous CPU load even after particles fade away.

Enki's **Particle Subsystem** is engineered from the ground up for 60+ FPS native desktop performance:

1. **Zero Runtime Allocations**: Particle memory is pre-allocated in a single contiguous `std::vector<Particle>` pool (default 150-300 particles). Inactive particles are recycled via an $O(1)$ free-list slot check.
2. **Integrated 2D Physics**:
   - Gravity acceleration vector ($g_x, g_y$).
   - Linear drag / atmospheric friction.
   - Tumbling ribbon rotation with 3D projection emulation ($\cos(\theta)$ flip).
   - Easing alpha fadeout along particle lifespan.
3. **Automatic Ticker Sleep (0% Idle CPU)**:
   - When active, the internal platform `Ticker` drives the simulation at screen refresh rate.
   - As soon as all active particles expire and continuous emission is off, the `Ticker` stops immediately, ensuring **0.0% CPU consumption** in idle state.
4. **Direct Skia Canvas Batching**: Particles are drawn directly to the Skia GPU surface (`SkCanvas`) using batched path and geometric primitives with minimal state changes.

---

## Presets & Configuration

### `ParticleShape` & `ParticlePreset`
```cpp
namespace enki {

enum class ParticleShape {
    Circle,
    Square,
    ConfettiRibbon,   ///< Rotating rectangle simulating falling 3D paper ribbons
    Star,             ///< 5-point star
};

enum class ParticlePreset {
    ConfettiBurst,    ///< Multi-color explosive celebration burst with gravity and tumbling
    NeonSparks,       ///< High-velocity glowing electrical sparks with heavy drag
    AmbientDust,      ///< Subtle, floating ambient background bokeh particles
    SnowFall,         ///< Softly drifting gravitational flakes
};

} // namespace enki
```

### `ParticleConfig` Struct
```cpp
namespace enki {

struct ParticleConfig {
    size_t             max_particles    = 150;
    float              emission_rate    = 60.0f;          ///< Particles/sec for continuous emitters
    Point              gravity          = {0.0f, 400.0f}; ///< Gravity vector (pixels/s^2)
    float              drag             = 0.02f;          ///< Air friction drag coefficient
    float              min_life         = 0.8f;           ///< Min lifespan in seconds
    float              max_life         = 1.8f;           ///< Max lifespan in seconds
    float              min_speed        = 120.0f;         ///< Min initial velocity
    float              max_speed        = 320.0f;         ///< Max initial velocity
    float              min_angle_rad    = 0.0f;           ///< Emission arc start (radians)
    float              max_angle_rad    = 6.2831853f;     ///< Emission arc end (2 * PI)
    float              min_size         = 4.0f;
    float              max_size         = 10.0f;
    std::vector<Color> color_palette    = {0xFFFF0055, 0xFF00FFCC, 0xFFFFCC00, 0xFF3B82F6, 0xFFA855F7};
    ParticleShape      shape            = ParticleShape::ConfettiRibbon;
    bool               burst_mode       = false;          ///< True for single explosion, false for continuous
};

} // namespace enki
```

---

## C++ API Reference

### 1. `ParticleEmitter` Widget
```cpp
namespace enki {

struct ParticleEmitterProps {
    ParticlePreset                preset        = ParticlePreset::ConfettiBurst;
    std::optional<ParticleConfig> custom_config = std::nullopt; ///< Override preset
    bool                          active        = true;         ///< Trigger burst or keep emitting
    WidgetPtr                     child         = nullptr;      ///< Optional content behind particles
    Key                           key           = Key::none();

    operator WidgetPtr() const;
};

struct ParticleEmitter : public ParticleEmitterProps {
    using ParticleEmitterProps::ParticleEmitterProps;
};

inline WidgetPtr particleEmitter(const ParticleEmitterProps& props = {});

} // namespace enki
```

### 2. `ParticleSystem` Standalone Engine
```cpp
namespace enki {

class ParticleSystem {
public:
    explicit ParticleSystem(const ParticleConfig& config = ParticlePresets::confetti());

    void setConfig(const ParticleConfig& config);
    [[nodiscard]] const ParticleConfig& config() const;

    /// Trigger instantaneous explosion of particles from origin point
    void burst(Point origin);

    /// Control continuous stream emission
    void setEmitting(bool emitting);
    [[nodiscard]] bool isEmitting() const;
    void setEmitterPosition(Point origin);

    /// Advance physics by dt (seconds)
    void update(float dt_sec, Size boundary = {1000.0f, 1000.0f});

    /// Draw all active particles to Canvas
    void render(Canvas& canvas, const Rect& bounds) const;

    void clear();
    [[nodiscard]] size_t activeCount() const;
    [[nodiscard]] bool hasActiveParticles() const;
};

} // namespace enki
```

---

## Real Code Examples

### Example 1: Declarative Confetti Celebration Button (`particleEmitter`)

```cpp
#include "enki/widgets/particle_emitter.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class CelebrationState : public State {
    bool celebrate_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        return particleEmitter({
            .preset = ParticlePreset::ConfettiBurst,
            .active = celebrate_,
            .child = button(ButtonProps{
                .child = text("🎉 Claim Victory!", { .color = 0xFFFFFFFF, .font_size = 18.0f }),
                .on_pressed = [this] {
                    setState([this] { celebrate_ = true; });
                },
                .normal_color = 0xFFEC4899, // Vibrant Pink
            }),
        });
    }
};
```

### Example 2: Interactive Multi-Preset Studio (`widgets_demo/animation_suite_demo/main.cpp`)

```cpp
#include "enki/animation/particle_system.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/skia_canvas.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class ParticleStudioState : public State {
private:
    std::unique_ptr<ParticleSystem> system_;
    std::unique_ptr<Ticker> ticker_;

public:
    void initState() override {
        State::initState();

        system_ = std::make_unique<ParticleSystem>(ParticlePresets::confetti());
        system_->burst({560.0f, 120.0f});

        ticker_ = createTicker([this] {
            if (system_) {
                system_->update(1.0f / 60.0f);
            }
            setState([] {});
        });
        ticker_->start();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                // Preset Trigger Buttons
                row({
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        button(ButtonProps{
                            .child = text("🎉 Confetti", { .color = 0xFFFFFFFF }),
                            .on_pressed = [this] {
                                system_->setConfig(ParticlePresets::confetti());
                                system_->burst({560.0f, 120.0f});
                            },
                            .normal_color = 0xFFEC4899,
                        }),
                        button(ButtonProps{
                            .child = text("⚡ Neon Sparks", { .color = 0xFFFFFFFF }),
                            .on_pressed = [this] {
                                system_->setConfig(ParticlePresets::neonSparks());
                                system_->burst({560.0f, 120.0f});
                            },
                            .normal_color = 0xFF8B5CF6,
                        }),
                        button(ButtonProps{
                            .child = text("❄️ Snowfall", { .color = 0xFFFFFFFF }),
                            .on_pressed = [this] {
                                system_->setConfig(ParticlePresets::snowFall());
                                system_->setEmitterPosition({560.0f, 10.0f});
                                system_->setEmitting(true);
                            },
                            .normal_color = 0xFF06B6D4,
                        }),
                    }
                }),

                // Skia Canvas Particle Layer
                skiaCanvas(SkiaCanvasProps{
                    .painter = [this](Canvas& canvas, Size size) {
                        if (system_) {
                            system_->render(canvas, Rect{0, 0, size.width, size.height});
                        }
                    },
                    .width = StyleValue::point(1080.0f),
                    .height = StyleValue::point(140.0f),
                })
            }
        });
    }
};
```
