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
        auto title = text("Slider Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold });
        
        // 1. Standard Slider
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(0) << vol_val;
        auto txt1 = text("Volume: " + ss1.str(), { .color = 0xFFCCCCCC });
        
        auto s1 = Slider {
            .value = vol_val,
            .on_change = [this](float v) {
                vol_val = v;
                setState([]{});
            },
            .min_value = 0.0f,
            .max_value = 100.0f
        };
        
        auto row1 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {txt1, expanded(s1)}
        });
        
        // 2. Custom Colored Slider
        std::stringstream ss2;
        ss2 << std::fixed << std::setprecision(2) << brightness_val;
        auto txt2 = text("Brightness: " + ss2.str(), { .color = 0xFFCCCCCC });
        
        auto s2 = Slider {
            .value = brightness_val,
            .on_change = [this](float v) {
                brightness_val = v;
                setState([]{});
            },
            .active_color = 0xFFF59E0B, // Amber
            .inactive_color = 0xFF78350F, // Dark Amber
            .thumb_color = 0xFFFEF3C7, // Light Amber
            .min_value = 0.0f,
            .max_value = 1.0f
        };
          
        auto row2 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {txt2, expanded(s2)}
        });

        // 3. Thick Track Slider
        std::stringstream ss3;
        ss3 << std::fixed << std::setprecision(1) << custom_val;
        auto txt3 = text("Custom: " + ss3.str(), { .color = 0xFFCCCCCC });
        
        auto s3 = Slider {
            .value = custom_val,
            .on_change = [this](float v) {
                custom_val = v;
                setState([]{});
            },
            .active_color = 0xFF10B981, // Emerald
            .track_height = 12.0f,
            .thumb_radius = 16.0f,
            .min_value = 0.0f,
            .max_value = 50.0f
        };
          
        auto row3 = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {txt3, expanded(s3)}
        });

        return container({
            .color = 0xFF1E293B, // Dark slate background
            .child = column({
                .gap = StyleValue::point(40.0f),
                .padding = StyleInsets::all(40.0f),
                .children = {title, row1, row2, row3}
            })
        });
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
    config.target_fps = 0;
    config.show_performance_overlay = true;
    
    return runApp(std::make_shared<SliderDemoWidget>(), config);
}
