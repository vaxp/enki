#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/switch.hpp"
#include "enki/state/state.hpp"

#include <iostream>

using namespace enki;

class SwitchDemoWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "SwitchDemoWidget"; }
    std::unique_ptr<State> createState() override;
};

class SwitchDemoState : public State {
    bool switch1_ = false;
    bool switch2_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("Switch Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f });
        
        auto row1 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {
                Switch {
                    .value = switch1_,
                    .on_changed = [this](bool val){
                        setState([this, val]{ switch1_ = val; });
                    }
                },
                text("Off by default", { .color = 0xFFCCCCCC })
            }
        });

        auto row2 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {
                Switch {
                    .value = switch2_,
                    .on_changed = [this](bool val){
                        setState([this, val]{ switch2_ = val; });
                    }
                },
                text("On by default", { .color = 0xFFCCCCCC })
            }
        });
        
        auto row3 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {
                Switch {
                    .value = false,
                    .on_changed = nullptr,
                    .disabled = true
                },
                text("Disabled Switch (Off)", { .color = 0xFF888888 })
            }
        });
        
        auto row4 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {
                Switch {
                    .value = true,
                    .on_changed = nullptr,
                    .disabled = true
                },
                text("Disabled Switch (On)", { .color = 0xFF888888 })
            }
        });

        return container({
            .color = 0xFF1E1E1E,
            .child = column({
                .gap = StyleValue::point(24.0f),
                .padding = StyleInsets::all(32.0f),
                .children = {title, row1, row2, row3, row4}
            })
        });
    }
};

std::unique_ptr<State> SwitchDemoWidget::createState() {
    return std::make_unique<SwitchDemoState>();
}

int main() {
    std::cout << "Starting Switch Demo...\n";
    AppConfig config;
    config.title = "ENKI Switch Demo";
    config.width = 600;
    config.height = 400;
    config.target_fps = 60;
    
    return runApp(std::make_shared<SwitchDemoWidget>(), config);
}
