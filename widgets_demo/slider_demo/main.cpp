#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace enki;

class SliderDemoWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "SliderDemoWidget"; }
    std::unique_ptr<State> createState() override;
};

class SliderDemoState : public State {
    float vol_val = 50.0f;
    float brightness_val = 0.8f;
    float custom_val = 20.0f;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = std::make_shared<Text>("Slider Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold});
        
        // 1. Standard Slider
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(0) << vol_val;
        auto txt1 = std::make_shared<Text>("Volume: " + ss1.str(), TextStyle{.color = 0xFFCCCCCC});
        
        auto s1 = slider(vol_val, [this](float v) {
            vol_val = v;
            setState([]{});
        });
        s1->min(0.0f)->max(100.0f);
        
        auto row1 = row(std::vector<WidgetPtr>{txt1, expanded(s1)});
        row1->gap(StyleValue::point(24.0f)).alignItems(Align::Center);
        
        // 2. Custom Colored Slider
        std::stringstream ss2;
        ss2 << std::fixed << std::setprecision(2) << brightness_val;
        auto txt2 = std::make_shared<Text>("Brightness: " + ss2.str(), TextStyle{.color = 0xFFCCCCCC});
        
        auto s2 = slider(brightness_val, [this](float v) {
            brightness_val = v;
            setState([]{});
        });
        s2->min(0.0f)->max(1.0f)
          ->activeColor(0xFFF59E0B) // Amber
          ->inactiveColor(0xFF78350F) // Dark Amber
          ->thumbColor(0xFFFEF3C7); // Light Amber
          
        auto row2 = row(std::vector<WidgetPtr>{txt2, expanded(s2)});
        row2->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        // 3. Thick Track Slider
        std::stringstream ss3;
        ss3 << std::fixed << std::setprecision(1) << custom_val;
        auto txt3 = std::make_shared<Text>("Custom: " + ss3.str(), TextStyle{.color = 0xFFCCCCCC});
        
        auto s3 = slider(custom_val, [this](float v) {
            custom_val = v;
            setState([]{});
        });
        s3->min(0.0f)->max(50.0f)
          ->activeColor(0xFF10B981) // Emerald
          ->trackHeight(12.0f)
          ->thumbRadius(16.0f);
          
        auto row3 = row(std::vector<WidgetPtr>{txt3, expanded(s3)});
        row3->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto col = column(std::vector<WidgetPtr>{title, row1, row2, row3});
        col->gap(StyleValue::point(40.0f)).padding(StyleInsets::all(40.0f));

        auto bg = container(col);
        bg->color(0xFF1E293B); // Dark slate background

        return bg;
    }
};

std::unique_ptr<State> SliderDemoWidget::createState() {
    return std::make_unique<SliderDemoState>();
}

int main() {
    std::cout << "Starting Slider Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI Slider Demo";
    config.width = 600;
    config.height = 400;
    config.target_fps = 60;
    
    return runApp(std::make_shared<SliderDemoWidget>(), config);
}
