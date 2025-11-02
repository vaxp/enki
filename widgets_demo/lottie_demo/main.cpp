/// @file main.cpp
/// @brief ENKI Lottie Animation Interactive Showcase & Verification Suite.
///
/// Showcases:
///   1. Declarative C++20 Lottie Widget API with auto-play and loop.
///   2. Interactive LottieController (Play, Pause, Stop, Reverse, Speed, Scrubbing).
///   3. Named Marker & Segment Triggering ("circle", "checkmark").
///   4. Interactive Tap & Hover triggers.
///   5. Dynamic Layer Recoloring & Property Observers.
///   6. Multi-Instance High-Performance Grid.
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/lottie.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// LottieDemoState
// ════════════════════════════════════════════════════════════════

class LottieDemoState : public State {
private:
    int current_tab_ = 0;

    // Interactive Controller State
    std::shared_ptr<LottieController> main_controller_;
    std::shared_ptr<LottieComposition> spinner_comp_;
    std::shared_ptr<LottieComposition> success_comp_;
    std::shared_ptr<LottieComposition> heart_comp_;

    float scrub_progress_ = 0.0f;
    bool is_scrubbing_ = false;

public:
    void initState() override {
        State::initState();

        spinner_comp_ = LottieCache::getOrLoad("assets/animations/loading_spinner.json");
        success_comp_ = LottieCache::getOrLoad("assets/animations/success_check.json");
        heart_comp_   = LottieCache::getOrLoad("assets/animations/heart_pulse.json");

        main_controller_ = std::make_shared<LottieController>(spinner_comp_);
        main_controller_->addListener([this]() {
            if (mounted()) {
                if (!is_scrubbing_) {
                    scrub_progress_ = main_controller_->progress();
                }
                setState([] {});
            }
        });
        main_controller_->play();
    }

    void dispose() override {
        if (main_controller_) {
            main_controller_->dispose();
            main_controller_ = nullptr;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header ───────────────────────────────────────────────
        auto header = container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::symmetric(24.0f, 16.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            row({
                                .align_items = Align::Center,
                                .gap = StyleValue::point(12.0f),
                                .children = {
                                    lottie({
                                        .asset = "assets/animations/loading_spinner.json",
                                        .width = 36,
                                        .height = 36,
                                        .repeat = true,
                                    }),
                                    text("ENKI Lottie Animation Engine", {
                                        .color = 0xFF38BDF8,
                                        .font_size = 22.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                },
                            }),
                            container({
                                .color = 0xFF1E293B,
                                .border_radius = BorderRadius::circular(20.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .padding = StyleInsets::symmetric(12.0f, 6.0f),
                                .child = text("GPU Accelerated · Skottie", {
                                    .color = 0xFF10B981,
                                    .font_size = 12.0f,
                                    .font_weight = FontWeight::Medium,
                                }),
                            }),
                        },
                    }),
                    text("High-performance vector animations with C++20 Declarative API, Markers, and Dynamic Recoloring", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ── Navigation Tabs ───────────────────────────────────────
        auto tabs = container({
            .color = 0xFF1E293B,
            .padding = StyleInsets::symmetric(16.0f, 8.0f),
            .child = row({
                .gap = StyleValue::point(10.0f),
                .children = {
                    buildTabButton(0, "Presets & Showcase"),
                    buildTabButton(1, "Timeline & Controller"),
                    buildTabButton(2, "Dynamic Recoloring"),
                    buildTabButton(3, "Performance Grid"),
                },
            }),
        });

        // ── Content Area ──────────────────────────────────────────
        WidgetPtr content;
        switch (current_tab_) {
            case 0: content = buildPresetsTab(); break;
            case 1: content = buildControllerTab(); break;
            case 2: content = buildThemingTab(); break;
            case 3: content = buildGridTab(); break;
            default: content = buildPresetsTab(); break;
        }

        return column({
            .children = {
                header,
                tabs,
                expanded(scrollView(container({
                    .padding = StyleInsets::all(20.0f),
                    .child = content,
                }))),
            },
        });
    }

private:
    WidgetPtr buildTabButton(int index, const std::string& label) {
        bool selected = (current_tab_ == index);
        return button(ButtonProps{
            .child = text(label, {
                .color = selected ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 14.0f,
                .font_weight = selected ? FontWeight::Bold : FontWeight::Medium,
            }),
            .on_pressed = [this, index]() {
                setState([this, index]() { current_tab_ = index; });
            },
            .normal_color = selected ? 0xFF2563EB : 0xFF0F172A,
            .hover_color  = selected ? 0xFF1D4ED8 : 0xFF334155,
            .border_radius = 8.0f,
            .padding = EdgeInsets::symmetric(8.0f, 16.0f),
        });
    }

    // ── Tab 1: Presets & Showcase ─────────────────────────────────
    WidgetPtr buildPresetsTab() {
        return column({
            .gap = StyleValue::point(20.0f),
            .children = {
                text("Atomic Lottie Widgets & Visual Presets", {
                    .color = 0xFFF1F5F9,
                    .font_size = 18.0f,
                    .font_weight = FontWeight::Bold,
                }),
                row({
                    .gap = StyleValue::point(16.0f),
                    .children = {
                        // Card 1: Spinner
                        expanded(buildAnimationCard(
                            "Loading Spinner",
                            "Continuous smooth looping ring",
                            lottie({
                                .asset = "assets/animations/loading_spinner.json",
                                .width = 130,
                                .height = 130,
                                .repeat = true,
                            })
                        )),
                        // Card 2: Success Check
                        expanded(buildAnimationCard(
                            "Success Checkmark",
                            "Marker-driven completion states",
                            lottie({
                                .asset = "assets/animations/success_check.json",
                                .width = 130,
                                .height = 130,
                                .repeat = true,
                                .speed = 1.2f,
                            })
                        )),
                        // Card 3: Heart Pulse
                        expanded(buildAnimationCard(
                            "Interactive Pulse",
                            "Hover / Tap triggered like effect",
                            lottie({
                                .asset = "assets/animations/heart_pulse.json",
                                .width = 130,
                                .height = 130,
                                .animate_on_hover = true,
                            })
                        )),
                    },
                }),
            },
        });
    }

    WidgetPtr buildAnimationCard(const std::string& title, const std::string& desc, WidgetPtr anim) {
        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(10.0f),
                        .padding = StyleInsets::all(16.0f),
                        .child = anim,
                    }),
                    text(title, {
                        .color = 0xFFFFFFFF,
                        .font_size = 16.0f,
                        .font_weight = FontWeight::SemiBold,
                    }),
                    text(desc, {
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f,
                    }),
                },
            }),
        });
    }

    // ── Tab 2: Timeline & Controller ──────────────────────────────
    WidgetPtr buildControllerTab() {
        std::ostringstream ss;
        ss << "Progress: " << std::fixed << std::setprecision(1) << (main_controller_->progress() * 100.0f) << "%"
           << "  |  Frame: " << static_cast<int>(main_controller_->currentFrame())
           << "  |  Loop: #" << main_controller_->loopCount();

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .gap = StyleValue::point(18.0f),
                .children = {
                    text("Master Playback Controller & Timeline Scrubbing", {
                        .color = 0xFFFFFFFF,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .children = {
                            lottie({
                                .controller = main_controller_,
                                .width = 180,
                                .height = 180,
                            }),
                        },
                    }),
                    // Timeline Scrubber Slider
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            row({
                                .justify_content = Justify::SpaceBetween,
                                .children = {
                                    text("Timeline Scrubber", {.color = 0xFF94A3B8, .font_size = 13.0f}),
                                    text(ss.str(), {.color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Medium}),
                                },
                            }),
                            Slider{
                                .value = scrub_progress_,
                                .on_change = [this](float val) {
                                    is_scrubbing_ = true;
                                    scrub_progress_ = val;
                                    main_controller_->seek(val);
                                    setState([] {});
                                },
                                .active_color = 0xFF38BDF8,
                                .inactive_color = 0xFF334155,
                                .min_value = 0.0f,
                                .max_value = 1.0f,
                            },
                        },
                    }),
                    // Playback Controls Row
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            button(ButtonProps{
                                .child = text("Play", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    is_scrubbing_ = false;
                                    main_controller_->play();
                                },
                                .normal_color = 0xFF10B981,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Pause", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    main_controller_->pause();
                                },
                                .normal_color = 0xFFF59E0B,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Reverse", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    is_scrubbing_ = false;
                                    main_controller_->reverse();
                                },
                                .normal_color = 0xFF8B5CF6,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Reset", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    main_controller_->reset();
                                },
                                .normal_color = 0xFFEF4444,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text(main_controller_->playbackMode() == LottiePlaybackMode::PingPong ? "Mode: PingPong" : "Mode: Loop", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (main_controller_->playbackMode() == LottiePlaybackMode::Loop) {
                                        main_controller_->setPlaybackMode(LottiePlaybackMode::PingPong);
                                    } else {
                                        main_controller_->setPlaybackMode(LottiePlaybackMode::Loop);
                                    }
                                    setState([] {});
                                },
                                .normal_color = 0xFF3B82F6,
                                .border_radius = 6.0f,
                            }),
                        },
                    }),
                    // Markers Trigger Row
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text("Trigger Marker Segment:", {.color = 0xFF94A3B8, .font_size = 13.0f}),
                            button(ButtonProps{
                                .child = text("Marker: 'start'", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    main_controller_->playMarker("start", true);
                                },
                                .normal_color = 0xFF334155,
                                .border_radius = 6.0f,
                            }),
                            button(ButtonProps{
                                .child = text("Marker: 'pulse'", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    main_controller_->playMarker("pulse", true);
                                },
                                .normal_color = 0xFF334155,
                                .border_radius = 6.0f,
                            }),
                        },
                    }),
                },
            }),
        });
    }

    // ── Tab 3: Dynamic Recoloring ─────────────────────────────────
    WidgetPtr buildThemingTab() {
        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .gap = StyleValue::point(20.0f),
                .children = {
                    text("Dynamic Layer Recoloring via Property Observers", {
                        .color = 0xFFFFFFFF,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Change vector path colors and fills dynamically in real-time without reloading the animation JSON.", {
                        .color = 0xFF94A3B8,
                        .font_size = 14.0f,
                    }),
                    row({
                        .gap = StyleValue::point(24.0f),
                        .children = {
                            // Section A: Spinner Recoloring
                            expanded(container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .padding = StyleInsets::all(16.0f),
                                .child = column({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(12.0f),
                                    .children = {
                                        lottie({
                                            .composition = spinner_comp_,
                                            .width = 140,
                                            .height = 140,
                                        }),
                                        text("Select Spinner Ring Color:", {.color = 0xFFCBD5E1, .font_size = 13.0f}),
                                        row({
                                            .gap = StyleValue::point(8.0f),
                                            .children = {
                                                buildColorPickerButton(0xFF3B82F6, "Blue", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFF3B82F6);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFF10B981, "Green", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFF10B981);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFF59E0B, "Amber", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFFF59E0B);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFA855F7, "Purple", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFFA855F7);
                                                    setState([] {});
                                                }),
                                            },
                                        }),
                                    },
                                }),
                            })),
                            // Section B: Heart Recoloring
                            expanded(container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .padding = StyleInsets::all(16.0f),
                                .child = column({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(12.0f),
                                    .children = {
                                        lottie({
                                            .composition = heart_comp_,
                                            .width = 140,
                                            .height = 140,
                                            .repeat = true,
                                        }),
                                        text("Select Heart Fill Color:", {.color = 0xFFCBD5E1, .font_size = 13.0f}),
                                        row({
                                            .gap = StyleValue::point(8.0f),
                                            .children = {
                                                buildColorPickerButton(0xFFF43F5E, "Rose", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFFF43F5E);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFEC4899, "Pink", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFFEC4899);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFF06B6D4, "Cyan", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFF06B6D4);
                                                    setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFEAB308, "Gold", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFFEAB308);
                                                    setState([] {});
                                                }),
                                            },
                                        }),
                                    },
                                }),
                            })),
                        },
                    }),
                },
            }),
        });
    }

    WidgetPtr buildColorPickerButton(Color c, const std::string& name, std::function<void()> onClick) {
        return button(ButtonProps{
            .child = text(name, {.color = 0xFFFFFFFF, .font_size = 12.0f}),
            .on_pressed = std::move(onClick),
            .normal_color = c,
            .border_radius = 6.0f,
            .padding = EdgeInsets::symmetric(6.0f, 10.0f),
        });
    }

    // ── Tab 4: Performance Grid ───────────────────────────────────
    WidgetPtr buildGridTab() {
        std::vector<WidgetPtr> rows;
        for (int r = 0; r < 4; ++r) {
            std::vector<WidgetPtr> cols;
            for (int c = 0; c < 4; ++c) {
                const char* asset = ((r + c) % 2 == 0)
                    ? "assets/animations/loading_spinner.json"
                    : "assets/animations/heart_pulse.json";

                cols.push_back(expanded(container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(8.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::all(8.0f),
                    .child = lottie({
                        .asset = asset,
                        .width = 64,
                        .height = 64,
                        .repeat = true,
                        .speed = 0.8f + (c * 0.2f),
                    }),
                })));
            }
            rows.push_back(row({
                .gap = StyleValue::point(10.0f),
                .children = std::move(cols),
            }));
        }

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("16x Simultaneous Lottie Grid (Stress Test)", {
                                .color = 0xFFFFFFFF,
                                .font_size = 18.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Shared LottieCache · 60 FPS", {
                                .color = 0xFF10B981,
                                .font_size = 13.0f,
                                .font_weight = FontWeight::SemiBold,
                            }),
                        },
                    }),
                    column({
                        .gap = StyleValue::point(10.0f),
                        .children = std::move(rows),
                    }),
                },
            }),
        });
    }
};

class LottieDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<LottieDemoState>();
    }
    std::string_view typeName() const override { return "LottieDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "ENKI Engine — Lottie Animation Showcase";
    config.width       = 980;
    config.height      = 800;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;
    return runApp(std::make_shared<LottieDemoApp>(), config);
}
