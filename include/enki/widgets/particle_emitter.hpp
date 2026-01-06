#pragma once
/// @file particle_emitter.hpp
/// @brief Declarative 2D Particle Emitter widget for ENKI.
///
/// Automatically manages Ticker lifecycle — runs at 60+ FPS while particles
/// are alive, and sleeps immediately when settled for 0% idle CPU.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/animation/particle_system.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/widgets/skia_canvas.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include <memory>
#include <optional>

namespace enki {

struct ParticleEmitterProps {
    ParticlePreset                preset        = ParticlePreset::ConfettiBurst;
    std::optional<ParticleConfig> custom_config = std::nullopt;
    bool                          active        = true;   ///< Trigger or keep emitting
    WidgetPtr                     child         = nullptr;///< Optional child widget behind particles
    Key                           key           = Key::none();

    operator WidgetPtr() const;
};

struct ParticleEmitter : public ParticleEmitterProps {
    using ParticleEmitterProps::ParticleEmitterProps;
};

inline WidgetPtr particleEmitter(const ParticleEmitterProps& props = {}) {
    return static_cast<WidgetPtr>(props);
}

class ParticleEmitterWidget : public StatefulWidget {
public:
    ParticleConfig config;
    bool           active = true;
    WidgetPtr      child  = nullptr;

    ParticleEmitterWidget(Key key, ParticleConfig cfg, bool act, WidgetPtr ch)
        : StatefulWidget(std::move(key)), config(std::move(cfg)), active(act), child(std::move(ch)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ParticleEmitter"; }
};

class ParticleEmitterState : public State {
public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const ParticleEmitterWidget*>(widget());
        system_ = std::make_unique<ParticleSystem>(w->config);

        ticker_ = createTicker([this] {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - last_tick_).count();
            last_tick_ = now;
            dt = std::clamp(dt, 0.001f, 0.05f);

            system_->update(dt);
            setState([] {});

            if (!system_->hasActiveParticles() && !system_->isEmitting()) {
                ticker_->stop();
            }
        });

        if (w->active) {
            triggerEmitter();
        }
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        auto& old_w = static_cast<const ParticleEmitterWidget&>(oldWidget);
        auto* new_w = static_cast<const ParticleEmitterWidget*>(widget());

        if (!old_w.active && new_w->active) {
            triggerEmitter();
        }
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ParticleEmitterWidget*>(widget());

        auto canvas_layer = skiaCanvas(SkiaCanvasProps{
            .painter = [this](Canvas& canvas, Size size) {
                if (system_) {
                    system_->render(canvas, Rect{0, 0, size.width, size.height});
                }
            },
        });

        if (w->child) {
            return stack({
                .children = {
                    w->child,
                    Positioned::fill(canvas_layer),
                },
            });
        }

        return canvas_layer;
    }

private:
    void triggerEmitter() {
        auto* w = static_cast<const ParticleEmitterWidget*>(widget());
        if (w->config.burst_mode) {
            system_->burst({200.0f, 200.0f});
        } else {
            system_->setEmitting(true);
        }

        last_tick_ = std::chrono::steady_clock::now();
        if (ticker_ && !ticker_->isActive()) {
            ticker_->start();
        }
    }

    std::unique_ptr<ParticleSystem> system_;
    std::unique_ptr<Ticker>         ticker_;
    std::chrono::steady_clock::time_point last_tick_;
};

inline std::unique_ptr<State> ParticleEmitterWidget::createState() {
    return std::make_unique<ParticleEmitterState>();
}

inline ParticleEmitterProps::operator WidgetPtr() const {
    ParticleConfig cfg = custom_config.value_or(ParticlePresets::fromPreset(preset));
    return std::make_shared<ParticleEmitterWidget>(key, std::move(cfg), active, child);
}

} // namespace enki
