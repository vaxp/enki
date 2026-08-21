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
        auto title = std::make_shared<Text>("Checkbox Widget Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f});
        
        auto cb1 = checkbox(checked1_, [this](bool val){
            setState([this, val]{ checked1_ = val; });
        });
        auto text1 = std::make_shared<Text>("Unchecked by default", TextStyle{.color = 0xFFCCCCCC});
        auto row1 = row({cb1, text1});
        row1->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto cb2 = checkbox(checked2_, [this](bool val){
            setState([this, val]{ checked2_ = val; });
        });
        auto text2 = std::make_shared<Text>("Checked by default", TextStyle{.color = 0xFFCCCCCC});
        auto row2 = row({cb2, text2});
        row2->gap(StyleValue::point(12.0f)).alignItems(Align::Center);
        
        CheckboxProps disabled_opt;
        disabled_opt.disabled = true;
        auto cb3 = checkbox(false, nullptr, disabled_opt);
        auto text3 = std::make_shared<Text>("Disabled Checkbox", TextStyle{.color = 0xFF888888});
        std::vector<WidgetPtr> r3_items;
        r3_items.push_back(cb3);
        r3_items.push_back(text3);
        auto row3 = row(r3_items);
        row3->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> col_items;
        col_items.push_back(title);
        col_items.push_back(row1);
        col_items.push_back(row2);
        col_items.push_back(row3);
        auto col = column(col_items);
        col->gap(StyleValue::point(24.0f));
        col->padding(StyleInsets::all(32.0f));

        auto bg = container(col);
        bg->color(0xFF1E1E1E);

        return bg;
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
