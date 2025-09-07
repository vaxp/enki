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
    WidgetPtr build(BuildContext&) override {
        // 1. Header Section
        auto titleCol = column({
            .align_items = Align::Center,
            .margin = StyleInsets::only(0, 0, 30.0f, 0),
            .children = {
                text("Advanced Interactive Buttons", {
                    .color = 0xFFFFFFFF,
                    .font_size = 24.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Hover, Ripples, and SkSL Shaders in ENKI", {
                    .color = 0xFF94A3B8,
                    .font_size = 14.0f,
                })
            }
        });

        // 2. Button 1: Default Primary Button
        auto btn_default = Button {
            .child = text("Default Primary Button", {
                .color = 0xFFFFFFFF,
                .font_size = 14.0f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = []() {
                std::cout << "Default clicked\n";
            }
        };

        // 3. Button 2: Disabled Button
        auto btn_disabled = Button {
            .child = text("Disabled Button", {
                .color = 0xFFFFFFFF,
                .font_size = 14.0f,
                .font_weight = FontWeight::Bold,
            }),
            .disabled = true
        };

        // 4. Button 3: Custom Danger Button
        auto btn_custom = Button {
            .child = text("Danger Action", {
                .color = 0xFFFFFFFF,
                .font_size = 15.0f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = []() {
                std::cout << "Danger clicked\n";
            },
            .normal_color = 0xFFEF4444,
            .hover_color = 0xFFDC2626,
            .pressed_color = 0xFFB91C1C,
            .border_radius = 24.0f,
            .padding = EdgeInsets::symmetric(14.0f, 32.0f),
            .shadow_color = 0x80EF4444,
            .shadow_blur = 12.0f,
            .shadow_offset_dy = 6.0f,
        };

        // 5. Button 4: Live Shader Injected Button (SkSL)
        auto btn_shader = Button {
            .child = text("Shader Button (Live)", {
                .color = 0xFFFFFFFF,
                .font_size = 16.0f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = []() {
                std::cout << "Shader button clicked\n";
            },
            .border_radius = 12.0f,
            .shadow_color = 0x608B5CF6,
            .shadow_blur = 15.0f,
            .enable_ripple = true,
            .custom_shader = R"(
                uniform float time;
                uniform vec2 resolution;
                
                vec4 main(vec2 fragCoord) {
                    vec2 uv = fragCoord / resolution;
                    float t = time * 0.5;
                    vec3 color = 0.5 + 0.5 * cos(t + uv.xyx + vec3(0.0, 2.0, 4.0));
                    return vec4(color * 0.8, 1.0);
                }
            )"
        };

        // Row 1
        auto row1 = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(30.0f),
            .children = {btn_default, btn_disabled}
        });

        // Row 2
        auto row2 = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(30.0f),
            .children = {btn_custom, btn_shader}
        });

        auto buttonsCol = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(30.0f),
            .children = {row1, row2}
        });

        auto mainCol = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .children = {titleCol, buttonsCol}
        });

        return container({
            .color = 0xFF0F172A,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(40.0f),
            .child = mainCol
        });
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
    config.height      = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ButtonDemoApp>(), config);
}
