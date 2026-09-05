/// @file main.cpp
/// @brief ENKI Next-Gen Animation Suite — Dedicated Showcase & Benchmark.
///
/// Demonstrates the 5 cutting-edge animation systems added to ENKI:
///   1. Physics-Based Spring Engine (Damped harmonic oscillator, interruptible momentum).
///   2. Timeline Choreography & Staggered Sequencer (Multi-track orchestration, interval curves).
///   3. Vector SVG Path Morphing (Continuous topology normalization via SkPathMeasure).
///   4. 2D GPU Skia Particle Physics System (Confetti, neon sparks, ambient dust, snowfall).
///   5. Hero & Shared Element Transitions (Coordinate tracking via RenderObject::globalBounds).
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/state/state.hpp"

// Animation Suite Subsystem Headers
#include "enki/animation/ticker.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/spring_simulation.hpp"
#include "enki/animation/spring_controller.hpp"
#include "enki/animation/timeline.hpp"
#include "enki/animation/stagger.hpp"
#include "enki/animation/path_morph.hpp"
#include "enki/animation/particle_system.hpp"
#include "enki/widgets/svg_morph.hpp"
#include "enki/widgets/hero.hpp"
#include "enki/widgets/particle_emitter.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <iomanip>
#include <sstream>

using namespace enki;

class AnimationSuiteDemoState : public State {
private:
    // ── 1. Spring Controller ─────────────────────────────────────
    SpringController spring_ctrl_{Springs::bouncy, 0.0f};
    std::string current_spring_name_ = "Bouncy";

    // ── 2. SVG Morph Controller ──────────────────────────────────
    AnimationController morph_ctrl_{std::chrono::milliseconds(1500)};

    // ── 3. Stagger Timeline Controller ───────────────────────────
    AnimationTimeline timeline_;
    float stagger_master_p_ = 0.0f;

    // ── 4. Particle System ───────────────────────────────────────
    std::unique_ptr<ParticleSystem> particle_sys_;
    std::string particle_mode_name_ = "Confetti Burst";

    // Ticker to drive the demo animations
    std::unique_ptr<Ticker> ticker_;

public:
    void initState() override {
        State::initState();

        // 1. Spring setup
        spring_ctrl_.addListener([this] {
            setState([] {});
        });

        // 2. SVG Morph setup (Ping-pong 60 FPS)
        morph_ctrl_.setPingPong(true);
        morph_ctrl_.setRepeats(true);
        morph_ctrl_.addListener([this] {
            setState([] {});
        });
        morph_ctrl_.forward();

        // 3. Stagger timeline setup
        setupTimeline();

        // 4. Particle System setup
        particle_sys_ = std::make_unique<ParticleSystem>(ParticlePresets::confetti());
        particle_sys_->burst({600.0f, 600.0f});

        // Master ticker
        ticker_ = createTicker([this] {
            morph_ctrl_.tick();
            if (particle_sys_) {
                particle_sys_->update(1.0f / 60.0f);
            }
            setState([] {});
        });
        ticker_->start();
    }

    void setupTimeline() {
        timeline_.reset();
        timeline_.setRepeat(true);
        timeline_.setPingPong(true);
        timeline_.add(std::chrono::milliseconds(0), std::chrono::milliseconds(2000), [this](float p) {
            stagger_master_p_ = p;
        }, &Curves::easeInOut);
        timeline_.play();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        spring_ctrl_.dispose();
        morph_ctrl_.dispose();
        timeline_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header ───────────────────────────────────────────────
        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .margin = StyleInsets::only(0, 0, 24.0f, 0),
            .children = {
                text("ENKI Next-Gen Animation Suite", {
                    .color = 0xFFFFFFFF,
                    .font_size = 28.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Interactive Physics, Vector Morphing, Staggered Timelines & GPU Particles", {
                    .color = 0xFF94A3B8,
                    .font_size = 14.0f,
                })
            }
        });

        // ═════════════════════════════════════════════════════════
        // SECTION 1: PHYSICS SPRINGS (Damped Harmonic Oscillator)
        // ═════════════════════════════════════════════════════════
        float spring_val = spring_ctrl_.value();
        float spring_vel = spring_ctrl_.velocity();

        std::ostringstream ss_spring;
        ss_spring << "Spring: " << current_spring_name_
                  << " | Target: " << std::fixed << std::setprecision(1) << spring_ctrl_.target()
                  << " | Value: " << spring_val
                  << " | Velocity: " << spring_vel
                  << (spring_ctrl_.isAnimating() ? " [Moving 60 FPS]" : " [Settled: 0% CPU]");

        auto springControls = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                button(ButtonProps{
                    .child = text("Bouncy", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        current_spring_name_ = "Bouncy";
                        spring_ctrl_.setSpring(Springs::bouncy);
                        spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                    },
                    .normal_color = 0xFF3B82F6,
                }),
                button(ButtonProps{
                    .child = text("Smooth", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        current_spring_name_ = "Smooth";
                        spring_ctrl_.setSpring(Springs::smooth);
                        spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                    },
                    .normal_color = 0xFF10B981,
                }),
                button(ButtonProps{
                    .child = text("Snappy", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        current_spring_name_ = "Snappy";
                        spring_ctrl_.setSpring(Springs::snappy);
                        spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                    },
                    .normal_color = 0xFFF59E0B,
                }),
                button(ButtonProps{
                    .child = text("Gentle", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        current_spring_name_ = "Gentle";
                        spring_ctrl_.setSpring(Springs::gentle);
                        spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                    },
                    .normal_color = 0xFF8B5CF6,
                }),
            }
        });

        // Visual spring puck
        float puck_x = 40.0f + spring_val * 480.0f;
        auto springCanvas = skiaCanvas(SkiaCanvasProps{
            .painter = [puck_x](Canvas& canvas, Size size) {
                // Draw track
                Paint track_paint;
                track_paint.setColor(0xFF334155);
                track_paint.setStrokeWidth(6.0f);
                canvas.drawLine({40.0f, size.height * 0.5f}, {size.width - 40.0f, size.height * 0.5f}, track_paint);

                // Draw puck
                Paint puck_paint;
                puck_paint.setColor(0xFF38BDF8);
                canvas.drawCircle({puck_x, size.height * 0.5f}, 18.0f, puck_paint);

                // Glow ring
                Paint glow_paint;
                glow_paint.setColor(0x8038BDF8);
                glow_paint.setStrokeWidth(3.0f);
                canvas.drawCircle({puck_x, size.height * 0.5f}, 24.0f, glow_paint);
            },
            .width = StyleValue::point(580.0f),
            .height = StyleValue::point(70.0f),
        });

        auto sectionSpring = container({
            .color = 0xEB1E293B,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF38BDF8, 1.5f),
            .box_shadow = { BoxShadow::glow(0x4038BDF8, 15.0f) },
            .width = StyleValue::point(1120.0f),
            .padding = StyleInsets::all(20.0f),
            .margin = StyleInsets::only(0, 0, 20.0f, 0),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("1. Physics Spring Engine (Damped Harmonic Oscillator)", {
                        .color = 0xFF38BDF8,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text(ss_spring.str(), {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                    springControls,
                    springCanvas,
                }
            })
        });

        // ═════════════════════════════════════════════════════════
        // SECTION 2: VECTOR SVG PATH MORPHING
        // ═════════════════════════════════════════════════════════
        float morph_t = morph_ctrl_.value();

        auto morph1 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                svgMorph({
                    .from_path = SvgMorphPaths::hamburger,
                    .to_path = SvgMorphPaths::close,
                    .progress = morph_t,
                    .color = 0xFF38BDF8,
                    .stroke_width = 3.0f,
                    .width = 48.0f,
                    .height = 48.0f,
                }),
                text("Menu ⟷ Close", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
            }
        });

        auto morph2 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                svgMorph({
                    .from_path = SvgMorphPaths::play,
                    .to_path = SvgMorphPaths::pause,
                    .progress = morph_t,
                    .color = 0xFF34D399,
                    .stroke_width = 3.0f,
                    .width = 48.0f,
                    .height = 48.0f,
                }),
                text("Play ⟷ Pause", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
            }
        });

        auto morph3 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                svgMorph({
                    .from_path = SvgMorphPaths::star,
                    .to_path = SvgMorphPaths::circle,
                    .progress = morph_t,
                    .color = 0xFFFBBF24,
                    .stroke_width = 3.0f,
                    .width = 48.0f,
                    .height = 48.0f,
                }),
                text("Star ⟷ Circle", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
            }
        });

        auto morph4 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                svgMorph({
                    .from_path = SvgMorphPaths::arrow_right,
                    .to_path = SvgMorphPaths::checkmark,
                    .progress = morph_t,
                    .color = 0xFFA855F7,
                    .stroke_width = 3.0f,
                    .width = 48.0f,
                    .height = 48.0f,
                }),
                text("Arrow ⟷ Check", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
            }
        });

        auto morphRow = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(40.0f),
            .children = { morph1, morph2, morph3, morph4 }
        });

        auto sectionMorph = container({
            .color = 0xEB0F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF34D399, 1.5f),
            .box_shadow = { BoxShadow::glow(0x4034D399, 15.0f) },
            .width = StyleValue::point(1120.0f),
            .padding = StyleInsets::all(20.0f),
            .margin = StyleInsets::only(0, 0, 20.0f, 0),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("2. SVG Vector Path Morphing (Topology Normalization via SkPathMeasure)", {
                        .color = 0xFF34D399,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Continuous real-time interpolation between arbitrary vector shapes (60 FPS Native Skia)", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                    morphRow,
                }
            })
        });

        // ═════════════════════════════════════════════════════════
        // SECTION 3: TIMELINE & STAGGERED SEQUENCER
        // ═════════════════════════════════════════════════════════
        StaggerConfig stg_cfg{
            .item_duration = std::chrono::milliseconds(300),
            .delay_between_items = std::chrono::milliseconds(80),
            .curve = &Curves::easeOut,
        };

        std::vector<WidgetPtr> stagger_cards;
        const char* card_titles[4] = {"Dashboard", "Analytics", "Reports", "Security"};
        Color card_colors[4] = {0xFF3B82F6, 0xFF10B981, 0xFFF59E0B, 0xFFEC4899};

        for (size_t i = 0; i < 4; ++i) {
            float p = StaggerHelper::itemProgress(i, 4, stagger_master_p_, stg_cfg);
            float card_scale = 0.85f + 0.15f * p;

            auto c = container({
                .color = 0xEB1E293B,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(card_colors[i], 1.5f),
                .box_shadow = { BoxShadow::standard(0x40000000, 10.0f, 3.0f) },
                .align = Alignment::Center,
                .width = StyleValue::point(240.0f * card_scale),
                .height = StyleValue::point(100.0f),
                .padding = StyleInsets::all(14.0f),
                .child = column({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .gap = StyleValue::point(4.0f),
                    .children = {
                        text(card_titles[i], { .color = card_colors[i], .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                        text("Cascading Stagger Entry", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                    }
                })
            });
            stagger_cards.push_back(c);
        }

        auto staggerRow = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = stagger_cards
        });

        auto sectionStagger = container({
            .color = 0xEB180D2B,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFFA855F7, 1.5f),
            .box_shadow = { BoxShadow::glow(0x40A855F7, 15.0f) },
            .width = StyleValue::point(1120.0f),
            .padding = StyleInsets::all(20.0f),
            .margin = StyleInsets::only(0, 0, 20.0f, 0),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("3. Timeline & Staggered Sequencer (Interval Curves)", {
                        .color = 0xFFA855F7,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Multi-track orchestration with auto-calculated staggered delays and keyframes", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                    staggerRow,
                }
            })
        });

        // ═════════════════════════════════════════════════════════
        // SECTION 4: 2D GPU SKIA PARTICLE SYSTEM & HERO
        // ═════════════════════════════════════════════════════════
        size_t active_particles = particle_sys_ ? particle_sys_->activeCount() : 0;
        std::ostringstream ss_part;
        ss_part << "Active Particles: " << active_particles
                << " | Preset: " << particle_mode_name_
                << " | Batch Renderer: SkCanvas GPU";

        auto particleButtons = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                button(ButtonProps{
                    .child = text("🎉 Confetti Burst", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        particle_mode_name_ = "Confetti Burst";
                        particle_sys_->setConfig(ParticlePresets::confetti());
                        particle_sys_->burst({560.0f, 120.0f});
                    },
                    .normal_color = 0xFFEC4899,
                }),
                button(ButtonProps{
                    .child = text("⚡ Neon Sparks", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        particle_mode_name_ = "Neon Sparks";
                        particle_sys_->setConfig(ParticlePresets::neonSparks());
                        particle_sys_->burst({560.0f, 120.0f});
                    },
                    .normal_color = 0xFF8B5CF6,
                }),
                button(ButtonProps{
                    .child = text("✨ Ambient Dust", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        particle_mode_name_ = "Ambient Dust";
                        particle_sys_->setConfig(ParticlePresets::ambientDust());
                        particle_sys_->setEmitterPosition({560.0f, 120.0f});
                        particle_sys_->setEmitting(true);
                    },
                    .normal_color = 0xFF3B82F6,
                }),
                button(ButtonProps{
                    .child = text("❄️ Snowfall", { .color = 0xFFFFFFFF }),
                    .on_pressed = [this] {
                        particle_mode_name_ = "Snowfall";
                        particle_sys_->setConfig(ParticlePresets::snowFall());
                        particle_sys_->setEmitterPosition({560.0f, 10.0f});
                        particle_sys_->setEmitting(true);
                    },
                    .normal_color = 0xFF06B6D4,
                }),
            }
        });

        auto particleCanvas = skiaCanvas(SkiaCanvasProps{
            .painter = [this](Canvas& canvas, Size size) {
                if (particle_sys_) {
                    particle_sys_->render(canvas, Rect{0, 0, size.width, size.height});
                }
            },
            .width = StyleValue::point(1080.0f),
            .height = StyleValue::point(140.0f),
        });

        auto sectionParticles = container({
            .color = 0xEB0B132B,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFFEC4899, 1.5f),
            .box_shadow = { BoxShadow::glow(0x40EC4899, 15.0f) },
            .width = StyleValue::point(1120.0f),
            .padding = StyleInsets::all(20.0f),
            .margin = StyleInsets::only(0, 0, 20.0f, 0),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("4. 2D GPU Particle Physics System & Hero Transitions", {
                        .color = 0xFFEC4899,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text(ss_part.str(), {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                    particleButtons,
                    hero({
                        .tag = "particle_hero_shuttle",
                        .child = particleCanvas,
                    }),
                }
            })
        });

        // ── Main Layout ───────────────────────────────────────────
        auto contentCol = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .children = {
                header,
                sectionSpring,
                sectionMorph,
                sectionStagger,
                sectionParticles,
            }
        });

        auto app_body = container({
            .color = 0x4D000000,
            .align = Alignment::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(24.0f, 20.0f),
            .child = scrollView(contentCol),
        });

        // CSD WindowFrame
        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "ENKI — Next-Gen Animation Suite Showcase",
            .border_radius = 12.0f,
            .border_width = 2.0f,
            .background_color = 0x4D000000,
            .titlebar_background_color = 0x4D000000,
            .titlebar_inactive_background_color = 0x4D000000,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }
};

class AnimationSuiteApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<AnimationSuiteDemoState>();
    }
    std::string_view typeName() const override { return "AnimationSuiteApp"; }
};

int main() {
    std::cout << "=======================================================\n";
    std::cout << "  ENKI Engine — Next-Gen Animation Suite Showcase\n";
    std::cout << "=======================================================\n";

    AppConfig config;
    config.title       = "ENKI — Next-Gen Animation Suite Showcase";
    config.width       = 1240;
    config.height      = 860;
    config.resizable   = true;
    config.enable_csd  = true;
    config.app_id      = "org.enki.animation_suite_demo";
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0x0000004D;

    return runApp(std::make_shared<AnimationSuiteApp>(), config);
}
