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
        auto title = text("Advanced Progress Indicators");
        title->fontSize(26.0f).bold().color(0xFFFFFFFF);

        auto subtitle = text("Linear ProgressBar & Circular ProgressRing with SkSL Shader Injection");
        subtitle->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> header_items = {title, subtitle};
        auto header = column(header_items);
        header->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 30.0f, 0));

        // -------------------------------------------------------------
        // Section 1: Linear ProgressBars
        // -------------------------------------------------------------
        auto s1_title = text("1. Linear ProgressBars");
        s1_title->fontSize(18.0f).bold().color(0xFF60A5FA);

        // 1a. Determinate with Label
        ProgressBarOptions opt_det;
        opt_det.height = 16.0f;
        opt_det.border_radius = 8.0f;
        opt_det.progress_color = 0xFF3B82F6;
        opt_det.show_label = true;
        auto pb_determinate = progressBar(progress_val_, opt_det);

        // 1b. Gradient with Glow
        ProgressBarOptions opt_grad;
        opt_grad.height = 14.0f;
        opt_grad.border_radius = 7.0f;
        opt_grad.gradient_colors = {0xFFEC4899, 0xFF8B5CF6, 0xFF3B82F6};
        opt_grad.glow_color = 0x80EC4899;
        opt_grad.glow_blur = 10.0f;
        auto pb_gradient = progressBar(progress_val_, opt_grad);

        // 1c. Indeterminate (Shimmer Sweep)
        ProgressBarOptions opt_indet;
        opt_indet.height = 12.0f;
        opt_indet.border_radius = 6.0f;
        opt_indet.indeterminate = true;
        opt_indet.progress_color = 0xFF10B981;
        auto pb_indet = progressBar(0.0f, opt_indet);

        // 1d. Custom SkSL Shader Linear Bar (Neon Wave)
        ProgressBarOptions opt_shader_bar;
        opt_shader_bar.height = 18.0f;
        opt_shader_bar.border_radius = 9.0f;
        opt_shader_bar.custom_shader = R"(
            uniform float time;
            uniform vec2 resolution;
            uniform float progress;

            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float wave = sin(uv.x * 20.0 + time * 6.0) * 0.5 + 0.5;
                vec3 base = mix(vec3(0.1, 0.8, 0.9), vec3(0.9, 0.2, 0.8), uv.x + wave * 0.2);
                return vec4(base * (0.8 + 0.2 * wave), 1.0);
            }
        )";
        auto pb_shader = progressBar(progress_val_, opt_shader_bar);

        std::vector<WidgetPtr> bar_items = {
            text("Determinate (Interactive Slider Driven):", TextStyle{.color = 0xFFCBD5E1, .font_size = 13.0f}),
            pb_determinate,
            text("Multi-stop Gradient with Outer Glow:", TextStyle{.color = 0xFFCBD5E1, .font_size = 13.0f}),
            pb_gradient,
            text("Indeterminate Shimmer Sweep:", TextStyle{.color = 0xFFCBD5E1, .font_size = 13.0f}),
            pb_indet,
            text("SkSL Procedural Energy Shader:", TextStyle{.color = 0xFFCBD5E1, .font_size = 13.0f}),
            pb_shader
        };
        auto col_bars = column(bar_items);
        col_bars->gap(StyleValue::point(10.0f)).width(StyleValue::percent(100.0f));

        // -------------------------------------------------------------
        // Section 2: Circular ProgressRings
        // -------------------------------------------------------------
        auto s2_title = text("2. Circular ProgressRings");
        s2_title->fontSize(18.0f).bold().color(0xFF60A5FA);

        // 2a. Ring with Center Text Child
        ProgressRingOptions opt_r1;
        opt_r1.size = 72.0f;
        opt_r1.stroke_width = 8.0f;
        opt_r1.progress_color = 0xFF3B82F6;
        opt_r1.glow_color = 0x603B82F6;
        opt_r1.glow_blur = 8.0f;

        int pct = static_cast<int>(progress_val_ * 100);
        auto center_txt = text(std::to_string(pct) + "%", TextStyle{
            .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        auto ring_det = progressRing(progress_val_, center_txt, opt_r1);

        // 2b. Indeterminate Spinning Ring
        ProgressRingOptions opt_r2;
        opt_r2.size = 72.0f;
        opt_r2.stroke_width = 7.0f;
        opt_r2.indeterminate = true;
        opt_r2.progress_color = 0xFF10B981;
        auto ring_indet = progressRing(0.0f, nullptr, opt_r2);

        // 2c. Custom SkSL Shader Ring (Rainbow Aura)
        ProgressRingOptions opt_r3;
        opt_r3.size = 72.0f;
        opt_r3.stroke_width = 8.0f;
        opt_r3.custom_shader = R"(
            uniform float time;
            uniform vec2 resolution;
            uniform float progress;

            vec4 main(vec2 fragCoord) {
                vec2 st = (fragCoord - resolution * 0.5) / resolution.y;
                float angle = atan(st.y, st.x) + time * 3.0;
                vec3 col = 0.5 + 0.5 * cos(angle + vec3(0.0, 2.0, 4.0));
                return vec4(col, 1.0);
            }
        )";
        auto ring_shader = progressRing(progress_val_, nullptr, opt_r3);

        std::vector<WidgetPtr> r1_items = {ring_det, text("Determinate", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_r1 = column(r1_items);
        col_r1->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> r2_items = {ring_indet, text("Indeterminate", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_r2 = column(r2_items);
        col_r2->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> r3_items = {ring_shader, text("SkSL Rainbow", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_r3 = column(r3_items);
        col_r3->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> ring_row_items = {col_r1, col_r2, col_r3};
        auto row_rings = row(ring_row_items);
        row_rings->justifyContent(Justify::SpaceAround).width(StyleValue::percent(100.0f));

        // -------------------------------------------------------------
        // Section 3: Interactive Slider Controller
        // -------------------------------------------------------------
        auto ctrl_label = text("Adjust Progress Value: " + std::to_string(pct) + "%", TextStyle{
            .color = 0xFFF1F5F9, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        
        SliderOptions slider_opt;
        slider_opt.min_value = 0.0f;
        slider_opt.max_value = 1.0f;
        slider_opt.active_color = 0xFF3B82F6;

        auto ctrl_slider = std::make_shared<Slider>(progress_val_, [this](float val) {
            setState([this, val] {
                progress_val_ = val;
            });
        }, slider_opt);

        std::vector<WidgetPtr> slider_items = {ctrl_label, ctrl_slider};
        auto slider_box = column(slider_items);
        slider_box->gap(StyleValue::point(8.0f)).width(StyleValue::percent(100.0f));

        // Container Card Layout
        auto card_bars = container(col_bars);
        card_bars->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(StyleValue::percent(100.0f));

        auto card_rings = container(row_rings);
        card_rings->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(StyleValue::percent(100.0f));

        auto card_ctrl = container(slider_box);
        card_ctrl->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> main_items = {
            header,
            s1_title, card_bars,
            s2_title, card_rings,
            card_ctrl
        };
        auto mainCol = column(main_items);
        mainCol->gap(StyleValue::point(16.0f)).alignItems(Align::Start);

        auto root = container(mainCol);
        root->color(0xFF0F172A)
            .paddingAll(30.0f)
            .flexGrow(1.0f);

        return root;
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
    config.vsync       = true;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ProgressDemoApp>(), config);
}
