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
        auto title = text("RangeSlider Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });
        
        // 1. Standard RangeSlider
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(0) << "$" << price_min << " - $" << price_max;
        auto txt1 = text("Price: " + ss1.str(), { .color = 0xFFCCCCCC });
        
        auto rs1 = RangeSlider {
            .start_value = price_min,
            .end_value = price_max,
            .on_change = [this](float s, float e) {
                price_min = s;
                price_max = e;
                setState([]{});
            },
            .min_value = 0.0f,
            .max_value = 100.0f
        };
        
        auto row1 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {txt1, expanded(rs1)}
        });
        
        // 2. Custom Colored RangeSlider
        std::stringstream ss2;
        ss2 << std::fixed << std::setprecision(1) << time_min << "h - " << time_max << "h";
        auto txt2 = text("Time: " + ss2.str(), { .color = 0xFFCCCCCC });
        
        auto rs2 = RangeSlider {
            .start_value = time_min,
            .end_value = time_max,
            .on_change = [this](float s, float e) {
                time_min = s;
                time_max = e;
                setState([]{});
            },
            .active_color = 0xFF8B5CF6,
            .inactive_color = 0xFF4C1D95,
            .thumb_color = 0xFFEDE9FE,
            .track_height = 8.0f,
            .thumb_radius = 12.0f,
            .min_value = 0.0f,
            .max_value = 24.0f
        };
          
        auto row2 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {txt2, expanded(rs2)}
        });

        auto col = column({
            .gap = StyleValue::point(40.0f),
            .children = {title, row1, row2}
        });

        return container({
            .color = 0xFF1E293B,
            .padding = StyleInsets::all(40.0f),
            .child = col
        });
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
