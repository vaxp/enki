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
        std::vector<WidgetPtr> r1_items;
        r1_items.push_back(sw1);
        r1_items.push_back(text1);
        auto row1 = row(r1_items);
        row1->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto sw2 = toggleSwitch(switch2_, [this](bool val){
            setState([this, val]{ switch2_ = val; });
        });
        auto text2 = std::make_shared<Text>("On by default", TextStyle{.color = 0xFFCCCCCC});
        std::vector<WidgetPtr> r2_items;
        r2_items.push_back(sw2);
        r2_items.push_back(text2);
        auto row2 = row(r2_items);
        row2->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        SwitchProps disabled_opt;
        disabled_opt.disabled = true;
        auto sw3 = toggleSwitch(false, nullptr, disabled_opt);
        auto text3 = std::make_shared<Text>("Disabled Switch (Off)", TextStyle{.color = 0xFF888888});
        std::vector<WidgetPtr> r3_items;
        r3_items.push_back(sw3);
        r3_items.push_back(text3);
        auto row3 = row(r3_items);
        row3->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        SwitchProps disabled_opt_on;
        disabled_opt_on.disabled = true;
        auto sw4 = toggleSwitch(true, nullptr, disabled_opt_on);
        auto text4 = std::make_shared<Text>("Disabled Switch (On)", TextStyle{.color = 0xFF888888});
        std::vector<WidgetPtr> r4_items;
        r4_items.push_back(sw4);
        r4_items.push_back(text4);
        auto row4 = row(r4_items);
        row4->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> col_items;
        col_items.push_back(title);
        col_items.push_back(row1);
        col_items.push_back(row2);
        col_items.push_back(row3);
        col_items.push_back(row4);
        auto col = column(col_items);
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
