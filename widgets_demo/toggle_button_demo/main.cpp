#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/toggle_button.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class ToggleButtonDemoState : public State {
    bool t1_ = true;
    bool t2_ = false;
    bool t3_ = true;
    bool t4_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("ToggleButton Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Filled, Outlined, Ghost, and Glow ToggleButton Styles (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        auto tb1 = toggleButton({
            .is_toggled = t1_,
            .on_toggle = [this](bool val) {
                t1_ = val;
                setState([]{});
            },
            .label = "Filled Style",
            .icon = "●",
            .style = ToggleButtonStyle::Filled,
            .active_color = 0xFF00E5FF,
        });

        auto tb2 = toggleButton({
            .is_toggled = t2_,
            .on_toggle = [this](bool val) {
                t2_ = val;
                setState([]{});
            },
            .label = "Outlined Style",
            .icon = "◆",
            .style = ToggleButtonStyle::Outlined,
            .active_color = 0xFFF59E0B,
        });

        auto tb3 = toggleButton({
            .is_toggled = t3_,
            .on_toggle = [this](bool val) {
                t3_ = val;
                setState([]{});
            },
            .label = "Glow Style",
            .icon = "⚡",
            .style = ToggleButtonStyle::Glow,
            .active_color = 0xFF10B981,
        });

        auto tb4 = toggleButton({
            .is_toggled = t4_,
            .on_toggle = [this](bool val) {
                t4_ = val;
                setState([]{});
            },
            .label = "Ghost Style",
            .icon = "★",
            .style = ToggleButtonStyle::Ghost,
            .active_color = 0xFFA855F7,
        });

        auto buttons_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {tb1, tb2, tb3, tb4}
        });

        std::string summary = "Status: [Filled: " + std::string(t1_ ? "ON" : "OFF") +
                              "] [Outlined: " + std::string(t2_ ? "ON" : "OFF") +
                              "] [Glow: " + std::string(t3_ ? "ON" : "OFF") +
                              "] [Ghost: " + std::string(t4_ ? "ON" : "OFF") + "]";

        auto status = text(summary, {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(28.0f),
            .children = {title, subtitle, buttons_row, status}
        });

        return container(ContainerProps{
            .color = 0xFF0B1320,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(40.0f),
            .child = main_col
        });
    }
};

class ToggleButtonDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "ToggleButtonDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<ToggleButtonDemoState>(); }
};

int main() {
    std::cout << "=== ENKI ToggleButton Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — ToggleButton Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<ToggleButtonDemoApp>(), config);
}
