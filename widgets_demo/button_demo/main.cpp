/// @file main.cpp
/// @brief ENKI Advanced Button Widget Interactive Showcase.
/// Demonstrates Hover, Press, Ripple effects, and SkSL Shader injection.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Root Button Demo App State & Application Widget
// ════════════════════════════════════════════════════════════════

class ButtonDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto title = text("Advanced Interactive Buttons");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);
        
        auto sub = text("Hover, Ripples, and SkSL Shaders in ENKI");
        sub->fontSize(14.0f).color(0xFF94A3B8);
        
        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. Default Button
        ButtonOptions opt_default;
        auto t_def = text("Default Primary Button");
        t_def->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto btn_default = button(t_def, [](){
            std::cout << "Default clicked\n";
        }, opt_default);

        // 2. Disabled Button
        ButtonOptions opt_disabled;
        auto t_dis = text("Disabled Button");
        t_dis->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto btn_disabled = button(t_dis, nullptr, opt_disabled);
        
        // 3. Custom Styled Button
        ButtonOptions opt_custom;
        opt_custom.normal_color = 0xFFEF4444;
        opt_custom.hover_color = 0xFFDC2626;
        opt_custom.pressed_color = 0xFFB91C1C;
        opt_custom.shadow_color = 0x80EF4444;
        opt_custom.shadow_blur = 12.0f;
        opt_custom.shadow_offset_dy = 6.0f;
        opt_custom.border_radius = 24.0f;
        opt_custom.padding = EdgeInsets::symmetric(14.0f, 32.0f);
        auto t_cus = text("Danger Action");
        t_cus->fontSize(15.0f).color(0xFFFFFFFF).bold();
        auto btn_custom = button(t_cus, [](){
            std::cout << "Danger clicked\n";
        }, opt_custom);

        // 4. Shader Injected Button (Animated Gradient SkSL)
        ButtonOptions opt_shader;
        opt_shader.border_radius = 12.0f;
        opt_shader.shadow_blur = 15.0f;
        opt_shader.shadow_color = 0x608B5CF6;
        opt_shader.enable_ripple = true;
        opt_shader.custom_shader = R"(
            uniform float time;
            uniform vec2 resolution;
            
            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float t = time * 0.5;
                vec3 color = 0.5 + 0.5 * cos(t + uv.xyx + vec3(0.0, 2.0, 4.0));
                return vec4(color * 0.8, 1.0);
            }
        )";
        auto t_sha = text("Shader Button (Live)");
        t_sha->fontSize(16.0f).color(0xFFFFFFFF).bold();
        auto btn_shader = button(t_sha, [](){
            std::cout << "Shader button clicked\n";
        }, opt_shader);

        std::vector<WidgetPtr> r1_children = {btn_default, btn_disabled};
        auto row1 = row(r1_children);
        row1->justifyContent(Justify::Center).alignItems(Align::Center).gap(30_px);
        
        std::vector<WidgetPtr> r2_children = {btn_custom, btn_shader};
        auto row2 = row(r2_children);
        row2->justifyContent(Justify::Center).alignItems(Align::Center).gap(30_px);

        std::vector<WidgetPtr> b_children = {row1, row2};
        auto buttonsCol = column(b_children);
        buttonsCol->alignItems(Align::Center).gap(40_px);

        std::vector<WidgetPtr> m_children = {titleCol, buttonsCol};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class ButtonDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ButtonDemoState>();
    }
    std::string_view typeName() const override { return "ButtonDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Button Widget Demo   \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Button Demo";
    config.width       = 800;
    config.height      = 300;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ButtonDemoApp>(), config);
}
