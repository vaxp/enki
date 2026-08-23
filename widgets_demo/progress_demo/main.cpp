/// @file main.cpp
/// @brief ENKI Advanced ProgressBar & ProgressRing Showcase.
/// Demonstrates Determinate, Indeterminate, Gradient, and SkSL Shader Injection features.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/progress_ring.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ProgressDemoState : public State {
    float progress_val_ = 0.65f;

public:
    WidgetPtr build(BuildContext& ctx) override {
        // Title Header
        auto title = text("Advanced Progress Indicators", { .color = 0xFFFFFFFF, .font_size = 26.0f, .font_weight = FontWeight::Bold });
        auto subtitle = text("Linear ProgressBar & Circular ProgressRing with SkSL Shader Injection", { .color = 0xFF94A3B8, .font_size = 14.0f });

        auto header = column({
            .align_items = Align::Center,
            .margin = StyleInsets::only(0, 0, 30.0f, 0),
            .children = {title, subtitle}
        });

        // -------------------------------------------------------------
        // Section 1: Linear ProgressBars
        // -------------------------------------------------------------
        auto s1_title = text("1. Linear ProgressBars", { .color = 0xFF60A5FA, .font_size = 18.0f, .font_weight = FontWeight::Bold });

        // 1a. Determinate with Label
        auto pb_determinate = ProgressBar {
            .value = progress_val_,
            .height = 16.0f,
            .border_radius = 8.0f,
            .progress_color = 0xFF3B82F6,
            .show_label = true
        };

        // 1b. Gradient with Glow
        auto pb_gradient = ProgressBar {
            .value = progress_val_,
            .height = 14.0f,
            .border_radius = 7.0f,
            .gradient_colors = {0xFFEC4899, 0xFF8B5CF6, 0xFF3B82F6},
            .glow_color = 0x80EC4899,
            .glow_blur = 10.0f
        };

        // 1c. Indeterminate (Shimmer Sweep)
        auto pb_indet = ProgressBar {
            .value = 0.0f,
            .height = 12.0f,
            .border_radius = 6.0f,
            .progress_color = 0xFF10B981,
            .indeterminate = true
        };

        // 1d. Custom SkSL Shader Linear Bar (Neon Wave)
        auto pb_shader = ProgressBar {
            .value = progress_val_,
            .height = 18.0f,
            .border_radius = 9.0f,
            .custom_shader = R"(
                uniform float time;
                uniform vec2 resolution;
                uniform float progress;

                vec4 main(vec2 fragCoord) {
                    vec2 uv = fragCoord / resolution;
                    float wave = sin(uv.x * 20.0 + time * 6.0) * 0.5 + 0.5;
                    vec3 base = mix(vec3(0.1, 0.8, 0.9), vec3(0.9, 0.2, 0.8), uv.x + wave * 0.2);
                    return vec4(base * (0.8 + 0.2 * wave), 1.0);
                }
            )"
        };

        auto col_bars = column({
            .justify_content = std::nullopt,
            .align_items = std::nullopt,
            .gap = StyleValue::point(10.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                text("Determinate Bar", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                pb_determinate,
                text("Determinate Gradient", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                pb_gradient,
                text("Indeterminate Bar", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                pb_indet,
                text("SkSL Shader Integration", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                pb_shader
            }
        });

        // -------------------------------------------------------------
        // Section 2: Circular ProgressRings
        // -------------------------------------------------------------
        auto s2_title = text("2. Circular ProgressRings", { .color = 0xFF60A5FA, .font_size = 18.0f, .font_weight = FontWeight::Bold });

        // 2a. Ring with Center Text Child
        int pct = static_cast<int>(progress_val_ * 100);
        auto center_txt = text(std::to_string(pct) + "%", {
            .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        
        auto ring_det = ProgressRing {
            .value = progress_val_,
            .size = 72.0f,
            .stroke_width = 8.0f,
            .progress_color = 0xFF3B82F6,
            .glow_color = 0x603B82F6,
            .glow_blur = 8.0f,
            .child = center_txt
        };

        // 2b. Indeterminate Spinning Ring
        auto ring_indet = ProgressRing {
            .value = 0.0f,
            .size = 72.0f,
            .stroke_width = 7.0f,
            .progress_color = 0xFF10B981,
            .indeterminate = true
        };

        // 2c. Custom SkSL Shader Ring (Rainbow Aura)
        auto ring_shader = ProgressRing {
            .value = progress_val_,
            .size = 72.0f,
            .stroke_width = 8.0f,
            .custom_shader = R"(
                uniform float time;
                uniform vec2 resolution;
                uniform float progress;

                vec4 main(vec2 fragCoord) {
                    vec2 st = (fragCoord - resolution * 0.5) / resolution.y;
                    float angle = atan(st.y, st.x) + time * 3.0;
                    vec3 col = 0.5 + 0.5 * cos(angle + vec3(0.0, 2.0, 4.0));
                    return vec4(col, 1.0);
                }
            )"
        };

        auto col_r1 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {ring_det, text("Determinate", { .color = 0xFF94A3B8, .font_size = 12.0f })}
        });

        auto col_r2 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {ring_indet, text("Indeterminate", { .color = 0xFF94A3B8, .font_size = 12.0f })}
        });

        auto col_r3 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {ring_shader, text("SkSL Rainbow", { .color = 0xFF94A3B8, .font_size = 12.0f })}
        });

        auto row_rings = row({
            .justify_content = Justify::SpaceAround,
            .width = StyleValue::percent(100.0f),
            .children = {col_r1, col_r2, col_r3}
        });

        // -------------------------------------------------------------
        // Section 3: Interactive Slider Controller
        // -------------------------------------------------------------
        auto ctrl_label = text("Adjust Progress Value: " + std::to_string(pct) + "%", {
            .color = 0xFFF1F5F9, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        
        auto ctrl_slider = Slider {
            .value = progress_val_,
            .on_change = [this](float val) {
                setState([this, val] {
                    progress_val_ = val;
                });
            },
            .active_color = 0xFF3B82F6,
            .min_value = 0.0f,
            .max_value = 1.0f
        };

        auto slider_box = column({
            .gap = StyleValue::point(8.0f),
            .width = StyleValue::percent(100.0f),
            .children = {ctrl_label, ctrl_slider}
        });

        // Container Card Layout
        auto card_bars = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = col_bars
        });

        auto card_rings = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = row_rings
        });

        auto card_ctrl = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = slider_box
        });

        auto mainCol = column({
            .align_items = Align::Start,
            .gap = StyleValue::point(16.0f),
            .children = {
                header,
                s1_title, card_bars,
                s2_title, card_rings,
                card_ctrl
            }
        });

        return container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::all(30.0f),
            .flex_grow = 1.0f,
            .child = mainCol
        });
    }
};

class ProgressDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ProgressDemoState>();
    }
    std::string_view typeName() const override { return "ProgressDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — ProgressBar & ProgressRing Showcase  \n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Progress Widgets Demo";
    config.width       = 700;
    config.height      = 850;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ProgressDemoApp>(), config);
}
