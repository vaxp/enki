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
        auto title = text("Advanced Loading Spinners");
        title->fontSize(26.0f).bold().color(0xFFFFFFFF);

        auto subtitle = text("Spokes, OrbitDots, DualArc, and SkSL Shader Injection in ENKI");
        subtitle->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> header_items = {title, subtitle};
        auto header = column(header_items);
        header->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 30.0f, 0));

        // 1. Spokes Spinner (macOS / iOS Style)
        SpinnerOptions opt_spokes;
        opt_spokes.style = SpinnerStyle::Spokes;
        opt_spokes.size = size_;
        opt_spokes.rotation_speed = speed_;
        opt_spokes.color = 0xFF60A5FA;
        auto spin_spokes = spinner(opt_spokes);

        // 2. OrbitDots Spinner (Material / Fluent Style)
        SpinnerOptions opt_dots;
        opt_dots.style = SpinnerStyle::OrbitDots;
        opt_dots.size = size_;
        opt_dots.rotation_speed = speed_;
        opt_dots.color = 0xFF10B981;
        opt_dots.dot_count = 6;
        opt_dots.dot_size = 7.0f;
        auto spin_dots = spinner(opt_dots);

        // 3. DualArc Spinner (Futuristic Dual Arc with Glow)
        SpinnerOptions opt_dual;
        opt_dual.style = SpinnerStyle::DualArc;
        opt_dual.size = size_;
        opt_dual.rotation_speed = speed_;
        opt_dual.color = 0xFFEC4899;
        opt_dual.glow_color = 0x80EC4899;
        opt_dual.glow_blur = 12.0f;
        auto spin_dual = spinner(opt_dual);

        // 4. Custom SkSL Shader Spinner (Vortex Particle Spiral)
        SpinnerOptions opt_shader;
        opt_shader.style = SpinnerStyle::CustomShader;
        opt_shader.size = size_;
        opt_shader.rotation_speed = speed_;
        opt_shader.custom_shader = R"(
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
        )";
        auto spin_shader = spinner(opt_shader);

        // Row of Spinners
        std::vector<WidgetPtr> s1 = {spin_spokes, text("Spokes (iOS)", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_s1 = column(s1);
        col_s1->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> s2 = {spin_dots, text("OrbitDots (Material)", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_s2 = column(s2);
        col_s2->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> s3 = {spin_dual, text("DualArc Glow", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_s3 = column(s3);
        col_s3->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> s4 = {spin_shader, text("SkSL Vortex Shader", TextStyle{.color = 0xFF94A3B8, .font_size = 12.0f})};
        auto col_s4 = column(s4);
        col_s4->alignItems(Align::Center).gap(StyleValue::point(8.0f));

        std::vector<WidgetPtr> spinner_row_items = {col_s1, col_s2, col_s3, col_s4};
        auto row_spinners = row(spinner_row_items);
        row_spinners->justifyContent(Justify::SpaceAround).width(StyleValue::percent(100.0f));

        auto card_spinners = container(row_spinners);
        card_spinners->color(0xFF1E293B).borderRadius(12.0f).paddingAll(24.0f).width(StyleValue::percent(100.0f));

        // Interactive Controls
        auto size_txt = text("Adjust Spinner Size: " + std::to_string(static_cast<int>(size_)) + "px", TextStyle{
            .color = 0xFFF1F5F9, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        SliderOptions sz_opt;
        sz_opt.min_value = 24.0f;
        sz_opt.max_value = 96.0f;
        sz_opt.active_color = 0xFF3B82F6;
        auto sz_slider = std::make_shared<Slider>(size_, [this](float val) {
            setState([this, val] { size_ = val; });
        }, sz_opt);

        auto speed_txt = text("Adjust Rotation Speed: " + std::to_string(speed_).substr(0, 4) + "x", TextStyle{
            .color = 0xFFF1F5F9, .font_size = 14.0f, .font_weight = FontWeight::Bold
        });
        SliderOptions sp_opt;
        sp_opt.min_value = 0.2f;
        sp_opt.max_value = 3.0f;
        sp_opt.active_color = 0xFF10B981;
        auto sp_slider = std::make_shared<Slider>(speed_, [this](float val) {
            setState([this, val] { speed_ = val; });
        }, sp_opt);

        std::vector<WidgetPtr> ctrl_items = {
            size_txt, sz_slider,
            speed_txt, sp_slider
        };
        auto ctrl_col = column(ctrl_items);
        ctrl_col->gap(StyleValue::point(10.0f)).width(StyleValue::percent(100.0f));

        auto card_ctrl = container(ctrl_col);
        card_ctrl->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> main_items = {
            header,
            card_spinners,
            card_ctrl
        };
        auto mainCol = column(main_items);
        mainCol->gap(StyleValue::point(20.0f)).alignItems(Align::Start);

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
    config.vsync       = true;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<SpinnerDemoApp>(), config);
}
