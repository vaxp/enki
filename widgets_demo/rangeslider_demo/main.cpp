#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/range_slider.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace enki;

class RangeSliderDemoWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "RangeSliderDemoWidget"; }
    std::unique_ptr<State> createState() override;
};

class RangeSliderDemoState : public State {
    float price_min = 20.0f;
    float price_max = 80.0f;
    float time_min = 8.0f;
    float time_max = 17.0f;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = std::make_shared<Text>("RangeSlider Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold});
        
        // 1. Standard RangeSlider
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(0) << "$" << price_min << " - $" << price_max;
        auto txt1 = std::make_shared<Text>("Price: " + ss1.str(), TextStyle{.color = 0xFFCCCCCC});
        
        auto rs1 = rangeSlider(price_min, price_max, [this](float s, float e) {
            price_min = s;
            price_max = e;
            setState([]{});
        });
        rs1->min(0.0f)->max(100.0f);
        
        auto row1 = row(std::vector<WidgetPtr>{txt1, expanded(rs1)});
        row1->gap(StyleValue::point(24.0f)).alignItems(Align::Center);
        
        // 2. Custom Colored RangeSlider
        std::stringstream ss2;
        ss2 << std::fixed << std::setprecision(1) << time_min << "h - " << time_max << "h";
        auto txt2 = std::make_shared<Text>("Time: " + ss2.str(), TextStyle{.color = 0xFFCCCCCC});
        
        auto rs2 = rangeSlider(time_min, time_max, [this](float s, float e) {
            time_min = s;
            time_max = e;
            setState([]{});
        });
        rs2->min(0.0f)->max(24.0f)
           ->activeColor(0xFF8B5CF6) // Violet
           ->inactiveColor(0xFF4C1D95) // Dark Violet
           ->thumbColor(0xFFEDE9FE) // Light Violet
           ->trackHeight(8.0f)
           ->thumbRadius(12.0f);
          
        auto row2 = row(std::vector<WidgetPtr>{txt2, expanded(rs2)});
        row2->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto col = column(std::vector<WidgetPtr>{title, row1, row2});
        col->gap(StyleValue::point(40.0f)).padding(StyleInsets::all(40.0f));

        auto bg = container(col);
        bg->color(0xFF1E293B); // Dark slate background

        return bg;
    }
};

std::unique_ptr<State> RangeSliderDemoWidget::createState() {
    return std::make_unique<RangeSliderDemoState>();
}

int main() {
    std::cout << "Starting RangeSlider Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI RangeSlider Demo";
    config.width = 600;
    config.height = 400;
    config.target_fps = 60;
    
    return runApp(std::make_shared<RangeSliderDemoWidget>(), config);
}
