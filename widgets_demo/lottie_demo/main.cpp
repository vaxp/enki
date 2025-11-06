/// @file main.cpp
/// @brief ENKI Lottie Animation Interactive Showcase & Verification Suite.
///
/// Features isolated component architecture:
///   - Every tab is an independent StatefulWidget with dedicated lifecycle.
///   - Each Lottie instance has its own unique Key and isolated Controller.
///   - Zero unnecessary tree re-renders or cross-tab layout recalculations.
///   - Clean unmounting and disposal when switching tabs.
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
// 1. PresetsTab (Isolated Tab 1)
// ════════════════════════════════════════════════════════════════

class PresetsTabState : public State {
public:
    WidgetPtr build(BuildContext&) override {
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
                                .key = Key::string("preset_lottie_spinner"),
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
                                .key = Key::string("preset_lottie_success"),
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
                                .key = Key::string("preset_lottie_heart"),
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

private:
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
};

class PresetsTabWidget : public StatefulWidget {
public:
    explicit PresetsTabWidget(Key key = Key::string("presets_tab")) : StatefulWidget(std::move(key)) {}
    std::unique_ptr<State> createState() override { return std::make_unique<PresetsTabState>(); }
    std::string_view typeName() const override { return "PresetsTabWidget"; }
};

// ════════════════════════════════════════════════════════════════
// 2. TimelineControllerTab (Isolated Tab 2)
// ════════════════════════════════════════════════════════════════

class TimelineControllerTabState : public State {
private:
    std::shared_ptr<LottieComposition> comp_;
    std::shared_ptr<LottieController>  controller_;
    float                              scrub_progress_ = 0.0f;
    bool                               is_scrubbing_ = false;

public:
    void initState() override {
        State::initState();
        comp_ = LottieCache::getOrLoad("assets/animations/loading_spinner.json");
        controller_ = std::make_shared<LottieController>(comp_);
        controller_->addListener([this]() {
            if (mounted()) {
                if (!is_scrubbing_) {
                    scrub_progress_ = controller_->progress();
                }
                setState([] {});
            }
        });
        controller_->play();
    }

    void dispose() override {
        if (controller_) {
            controller_->dispose();
            controller_ = nullptr;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        if (!controller_) return nullptr;

        std::ostringstream ss;
        ss << "Progress: " << std::fixed << std::setprecision(1) << (controller_->progress() * 100.0f) << "%"
           << "  |  Frame: " << static_cast<int>(controller_->currentFrame())
           << "  |  Loop: #" << controller_->loopCount();

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
                                .key = Key::string("controller_tab_lottie_instance"),
                                .controller = controller_,
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
                                    if (mounted()) {
                                        is_scrubbing_ = true;
                                        scrub_progress_ = val;
                                        if (controller_) controller_->seek(val);
                                        setState([] {});
                                    }
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
                                    if (mounted() && controller_) {
                                        is_scrubbing_ = false;
                                        controller_->play();
                                    }
                                },
                                .normal_color = 0xFF10B981,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Pause", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (mounted() && controller_) {
                                        controller_->pause();
                                    }
                                },
                                .normal_color = 0xFFF59E0B,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Reverse", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (mounted() && controller_) {
                                        is_scrubbing_ = false;
                                        controller_->reverse();
                                    }
                                },
                                .normal_color = 0xFF8B5CF6,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text("Reset", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (mounted() && controller_) {
                                        controller_->reset();
                                    }
                                },
                                .normal_color = 0xFFEF4444,
                                .border_radius = 6.0f,
                            }),

                            button(ButtonProps{
                                .child = text(controller_->playbackMode() == LottiePlaybackMode::PingPong ? "Mode: PingPong" : "Mode: Loop", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (mounted() && controller_) {
                                        if (controller_->playbackMode() == LottiePlaybackMode::Loop) {
                                            controller_->setPlaybackMode(LottiePlaybackMode::PingPong);
                                        } else {
                                            controller_->setPlaybackMode(LottiePlaybackMode::Loop);
                                        }
                                        setState([] {});
                                    }
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
                                    if (mounted() && controller_) {
                                        controller_->playMarker("start", true);
                                    }
                                },
                                .normal_color = 0xFF334155,
                                .border_radius = 6.0f,
                            }),
                            button(ButtonProps{
                                .child = text("Marker: 'pulse'", {.color = 0xFFFFFFFF}),
                                .on_pressed = [this]() {
                                    if (mounted() && controller_) {
                                        controller_->playMarker("pulse", true);
                                    }
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
};

class TimelineControllerTabWidget : public StatefulWidget {
public:
    explicit TimelineControllerTabWidget(Key key = Key::string("timeline_controller_tab")) : StatefulWidget(std::move(key)) {}
    std::unique_ptr<State> createState() override { return std::make_unique<TimelineControllerTabState>(); }
    std::string_view typeName() const override { return "TimelineControllerTabWidget"; }
};

// ════════════════════════════════════════════════════════════════
// 3. DynamicRecoloringTab (Isolated Tab 3)
// ════════════════════════════════════════════════════════════════

class DynamicRecoloringTabState : public State {
private:
    std::shared_ptr<LottieComposition> spinner_comp_;
    std::shared_ptr<LottieComposition> heart_comp_;

public:
    void initState() override {
        State::initState();
        spinner_comp_ = LottieComposition::loadFromFile("assets/animations/loading_spinner.json").valueOr(nullptr);
        heart_comp_   = LottieComposition::loadFromFile("assets/animations/heart_pulse.json").valueOr(nullptr);
    }

    WidgetPtr build(BuildContext&) override {
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
                                            .key = Key::string("theme_lottie_spinner"),
                                            .composition = spinner_comp_,
                                            .width = 140,
                                            .height = 140,
                                            .repeat = true,
                                        }),
                                        text("Select Spinner Ring Color:", {.color = 0xFFCBD5E1, .font_size = 13.0f}),
                                        row({
                                            .gap = StyleValue::point(8.0f),
                                            .children = {
                                                buildColorPickerButton(0xFF3B82F6, "Blue", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFF3B82F6);
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFF10B981, "Green", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFF10B981);
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFF59E0B, "Amber", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFFF59E0B);
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFA855F7, "Purple", [this]() {
                                                    if (spinner_comp_) spinner_comp_->setColor("RingStroke", 0xFFA855F7);
                                                    if (mounted()) setState([] {});
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
                                            .key = Key::string("theme_lottie_heart"),
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
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFEC4899, "Pink", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFFEC4899);
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFF06B6D4, "Cyan", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFF06B6D4);
                                                    if (mounted()) setState([] {});
                                                }),
                                                buildColorPickerButton(0xFFEAB308, "Gold", [this]() {
                                                    if (heart_comp_) heart_comp_->setColor("HeartFill", 0xFFEAB308);
                                                    if (mounted()) setState([] {});
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

private:
    WidgetPtr buildColorPickerButton(Color c, const std::string& name, std::function<void()> onClick) {
        return button(ButtonProps{
            .child = text(name, {.color = 0xFFFFFFFF, .font_size = 12.0f}),
            .on_pressed = std::move(onClick),
            .normal_color = c,
            .border_radius = 6.0f,
            .padding = EdgeInsets::symmetric(6.0f, 10.0f),
        });
    }
};

class DynamicRecoloringTabWidget : public StatefulWidget {
public:
    explicit DynamicRecoloringTabWidget(Key key = Key::string("dynamic_recoloring_tab")) : StatefulWidget(std::move(key)) {}
    std::unique_ptr<State> createState() override { return std::make_unique<DynamicRecoloringTabState>(); }
    std::string_view typeName() const override { return "DynamicRecoloringTabWidget"; }
};

// ════════════════════════════════════════════════════════════════
// 4. PerformanceGridTab (Isolated Tab 4)
// ════════════════════════════════════════════════════════════════

class PerformanceGridTabWidget : public StatelessWidget {
public:
    explicit PerformanceGridTabWidget(Key key = Key::string("perf_grid_tab")) : StatelessWidget(std::move(key)) {}

    WidgetPtr build(BuildContext&) override {
        std::vector<WidgetPtr> rows;
        for (int r = 0; r < 4; ++r) {
            std::vector<WidgetPtr> cols;
            for (int c = 0; c < 4; ++c) {
                const char* asset = ((r + c) % 2 == 0)
                    ? "assets/animations/loading_spinner.json"
                    : "assets/animations/heart_pulse.json";

                std::string cell_key = "grid_cell_" + std::to_string(r) + "_" + std::to_string(c);

                cols.push_back(expanded(container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(8.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::all(8.0f),
                    .child = lottie({
                        .key = Key::string(cell_key),
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

    std::string_view typeName() const override { return "PerformanceGridTabWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Root Demo State & Application Shell
// ════════════════════════════════════════════════════════════════

class LottieDemoState : public State {
private:
    int current_tab_ = 0;

public:
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
                                        .key = Key::string("header_logo_lottie"),
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

        // ── Isolated Content Area ─────────────────────────────────
        WidgetPtr content;
        switch (current_tab_) {
            case 0: content = std::make_shared<PresetsTabWidget>(Key::string("tab_presets")); break;
            case 1: content = std::make_shared<TimelineControllerTabWidget>(Key::string("tab_controller")); break;
            case 2: content = std::make_shared<DynamicRecoloringTabWidget>(Key::string("tab_recoloring")); break;
            case 3: content = std::make_shared<PerformanceGridTabWidget>(Key::string("tab_grid")); break;
            default: content = std::make_shared<PresetsTabWidget>(Key::string("tab_presets")); break;
        }

        return column(ColumnProps{
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
                if (current_tab_ != index) {
                    setState([this, index]() { current_tab_ = index; });
                }
            },
            .normal_color = selected ? 0xFF2563EB : 0xFF0F172A,
            .hover_color  = selected ? 0xFF1D4ED8 : 0xFF334155,
            .border_radius = 8.0f,
            .padding = EdgeInsets::symmetric(8.0f, 16.0f),
        });
    }
};

class LottieDemoApp : public StatefulWidget {
public:
    LottieDemoApp() : StatefulWidget(Key::string("lottie_demo_root")) {}
    explicit LottieDemoApp(Key key) : StatefulWidget(std::move(key)) {}
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
