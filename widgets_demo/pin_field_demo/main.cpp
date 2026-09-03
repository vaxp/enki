#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/pin_field.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class PinFieldDemoState : public State {
    std::string current_pin_ = "";
    std::string confirmed_pin_ = "None";

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("PinField Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Security Masked PIN Entry with Timed Obscure Reveal (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        auto status = text("Entered Digits: " + std::to_string(current_pin_.length()) + " / 4", {
            .color = 0xFF94A3B8,
            .font_size = 14.0f,
        });

        auto confirmed = text("Confirmed Master PIN: " + confirmed_pin_, {
            .color = 0xFF00E5FF,
            .font_size = 16.0f,
            .font_weight = FontWeight::Bold
        });

        auto pin = pinField({
            .length = 4,
            .box_size = 56.0f,
            .gap = 14.0f,
            .obscure_delay_ms = 400,
            .auto_focus = true,
            .on_changed = [this](const std::string& p) {
                current_pin_ = p;
                setState([]{});
            },
            .on_completed = [this](const std::string& p) {
                confirmed_pin_ = p;
                std::cout << ">>> Standalone PIN Entered: " << p << std::endl;
                setState([]{});
            },
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(28.0f),
            .children = {title, subtitle, pin, status, confirmed}
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

class PinFieldDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "PinFieldDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<PinFieldDemoState>(); }
};

int main() {
    std::cout << "=== ENKI PinField Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — PinField Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<PinFieldDemoApp>(), config);
}
