#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/checkbox.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <string>

using namespace enki;

class CheckboxDemoWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "CheckboxDemoWidget"; }
    std::unique_ptr<State> createState() override;
};

class CheckboxDemoState : public State {
    bool checked1_ = false;
    bool checked2_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("Checkbox Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f });
        
        auto row1 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                Checkbox {
                    .value = checked1_,
                    .on_changed = [this](bool val){
                        setState([this, val]{ checked1_ = val; });
                    }
                },
                text("Unchecked by default", { .color = 0xFFCCCCCC })
            }
        });

        auto row2 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                Checkbox {
                    .value = checked2_,
                    .on_changed = [this](bool val){
                        setState([this, val]{ checked2_ = val; });
                    }
                },
                text("Checked by default", { .color = 0xFFCCCCCC })
            }
        });
        
        auto row3 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                Checkbox {
                    .value = false,
                    .on_changed = nullptr,
                    .disabled = true
                },
                text("Disabled Checkbox", { .color = 0xFF888888 })
            }
        });

        return container({
            .color = 0xFF1E1E1E,
            .child = column({
                .gap = StyleValue::point(24.0f),
                .padding = StyleInsets::all(32.0f),
                .children = {title, row1, row2, row3}
            })
        });
    }
};

std::unique_ptr<State> CheckboxDemoWidget::createState() {
    return std::make_unique<CheckboxDemoState>();
}

int main() {
    std::cout << "Starting Checkbox Demo...\n";
    AppConfig config;
    config.title = "ENKI Checkbox Demo";
    config.width = 600;
    config.height = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0; // Uncapped max speed
    config.show_performance_overlay = true; // Display real-time FPS & Frame Time HUD
    config.clear_color = 0xFF0B0F19;
    
    return runApp(std::make_shared<CheckboxDemoWidget>(), config);
}
