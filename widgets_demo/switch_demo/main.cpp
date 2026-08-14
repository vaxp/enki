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
        auto title = std::make_shared<Text>("Switch Widget Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f});
        
        auto sw1 = toggleSwitch(switch1_, [this](bool val){
            setState([this, val]{ switch1_ = val; });
        });
        auto text1 = std::make_shared<Text>("Off by default", TextStyle{.color = 0xFFCCCCCC});
        auto row1 = row({sw1, text1});
        row1->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto sw2 = toggleSwitch(switch2_, [this](bool val){
            setState([this, val]{ switch2_ = val; });
        });
        auto text2 = std::make_shared<Text>("On by default", TextStyle{.color = 0xFFCCCCCC});
        auto row2 = row({sw2, text2});
        row2->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        SwitchOptions disabled_opt;
        disabled_opt.disabled = true;
        auto sw3 = toggleSwitch(false, nullptr, disabled_opt);
        auto text3 = std::make_shared<Text>("Disabled Switch (Off)", TextStyle{.color = 0xFF888888});
        auto row3 = row({sw3, text3});
        row3->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        SwitchOptions disabled_opt_on;
        disabled_opt_on.disabled = true;
        auto sw4 = toggleSwitch(true, nullptr, disabled_opt_on);
        auto text4 = std::make_shared<Text>("Disabled Switch (On)", TextStyle{.color = 0xFF888888});
        auto row4 = row({sw4, text4});
        row4->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto col = column({title, row1, row2, row3, row4});
        col->gap(StyleValue::point(24.0f));
        col->padding(StyleInsets::all(32.0f));

        auto bg = container(col);
        bg->color(0xFF1E1E1E);

        return bg;
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
