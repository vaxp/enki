/// @file main.cpp
/// @brief ENKI Advanced Spinner Showcase Application.
/// Demonstrates Spokes, OrbitDots, DualArc, and SkSL Shader Injection spinner variants.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/widgets/spinner.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class SpinnerDemoState : public State {
    float size_ = 48.0f;
    float speed_ = 1.0f;

public:
    WidgetPtr build(BuildContext& ctx) override {
        // Title Header
        auto title = text({
            .text = "Advanced Loading Spinners",
            .color = 0xFFFFFFFF,
            .font_size = 26.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle = text({
            .text = "Spokes, OrbitDots, DualArc, and SkSL Shader Injection in ENKI",
            .color = 0xFF94A3B8,
            .font_size = 14.0f,
        });

        auto header = column({
            .align_items = Align::Center,
            .margin = StyleInsets::only(0, 0, 30.0f, 0),
            .children = { title, subtitle }
        });

        // 1. Spokes Spinner (macOS / iOS Style)
        auto spin_spokes = Spinner {
            .style = SpinnerStyle::Spokes,
            .size = size_,
            .color = 0xFF60A5FA,
            .rotation_speed = speed_
        };

        // 2. OrbitDots Spinner (Material / Fluent Style)
        auto spin_dots = Spinner {
            .style = SpinnerStyle::OrbitDots,
            .size = size_,
            .color = 0xFF10B981,
            .dot_count = 6,
            .dot_size = 7.0f,
            .rotation_speed = speed_
        };

        // 3. DualArc Spinner (Futuristic Dual Arc with Glow)
        auto spin_dual = Spinner {
            .style = SpinnerStyle::DualArc,
            .size = size_,
            .color = 0xFFEC4899,
            .rotation_speed = speed_,
            .glow_color = 0x80EC4899,
            .glow_blur = 12.0f
        };

        // 4. Custom SkSL Shader Spinner (Vortex Particle Spiral)
        auto spin_shader = Spinner {
            .style = SpinnerStyle::CustomShader,
            .size = size_,
            .rotation_speed = speed_,
            .custom_shader = R"(
            uniform vec2 resolution;
            uniform float time;

            vec4 main(vec2 fragCoord) {
                vec2 st = (fragCoord - resolution * 0.5) / resolution.y;
                float r = length(st);
                float a = atan(st.y, st.x) + time * 4.0;
                
                float spiral = sin(a * 4.0 + r * 30.0);
                float ring = smoothstep(0.45, 0.35, r) * smoothstep(0.15, 0.25, r);
                
                vec3 col = 0.5 + 0.5 * cos(time + a + vec3(0.0, 2.0, 4.0));
                return vec4(col * ring * (0.5 + 0.5 * spiral), ring);
            }
        )"
        };

        // Row of Spinners
        auto col_s1 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = { spin_spokes, text({ .text = "Spokes (iOS)", .color = 0xFF94A3B8, .font_size = 12.0f }) }
        });

        auto col_s2 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = { spin_dots, text({ .text = "OrbitDots (Material)", .color = 0xFF94A3B8, .font_size = 12.0f }) }
        });

        auto col_s3 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = { spin_dual, text({ .text = "DualArc Glow", .color = 0xFF94A3B8, .font_size = 12.0f }) }
        });

        auto col_s4 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = { spin_shader, text({ .text = "SkSL Vortex Shader", .color = 0xFF94A3B8, .font_size = 12.0f }) }
        });

        auto row_spinners = row({
            .justify_content = Justify::SpaceAround,
            .width = StyleValue::percent(100.0f),
            .children = { col_s1, col_s2, col_s3, col_s4 }
        });

        auto card_spinners = container(row_spinners);
        card_spinners->color(0xFF1E293B).borderRadius(12.0f).paddingAll(24.0f).width(StyleValue::percent(100.0f));

        // Interactive Controls
        auto size_txt = text({
            .text = "Adjust Spinner Size: " + std::to_string(static_cast<int>(size_)) + "px",
            .color = 0xFFF1F5F9,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        auto sz_slider = Slider {
            .value = size_,
            .on_change = [this](float val) {
                setState([this, val] { size_ = val; });
            },
            .active_color = 0xFF3B82F6,
            .min_value = 24.0f,
            .max_value = 96.0f
        };

        auto speed_txt = text({
            .text = "Adjust Rotation Speed: " + std::to_string(speed_).substr(0, 4) + "x",
            .color = 0xFFF1F5F9,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        auto sp_slider = Slider {
            .value = speed_,
            .on_change = [this](float val) {
                setState([this, val] { speed_ = val; });
            },
            .active_color = 0xFF10B981,
            .min_value = 0.2f,
            .max_value = 3.0f
        };

        auto col_controls = column({
            .gap = StyleValue::point(10.0f),
            .width = StyleValue::percent(100.0f),
            .children = { size_txt, sz_slider, speed_txt, sp_slider }
        });

        auto card_ctrl = container(col_controls);
        card_ctrl->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(StyleValue::percent(100.0f));

        auto mainCol = column({
            .align_items = Align::Start,
            .gap = StyleValue::point(20.0f),
            .children = { header, card_spinners, card_ctrl }
        });

        auto root = container(mainCol);
        root->color(0xFF0F172A)
            .paddingAll(30.0f)
            .flexGrow(1.0f);

        return root;
    }
};

class SpinnerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SpinnerDemoState>();
    }
    std::string_view typeName() const override { return "SpinnerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Spinner Showcase  \n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Spinner Widget Demo";
    config.width       = 750;
    config.height      = 550;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<SpinnerDemoApp>(), config);
}
