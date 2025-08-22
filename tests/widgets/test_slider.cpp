#include "enki/widgets/slider.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_slider_initialization() {
    float current_val = 0.5f;
    std::shared_ptr<SliderWidget> child = std::dynamic_pointer_cast<SliderWidget>(static_cast<WidgetPtr>(Slider {
        .value = current_val,
        .on_change = [&current_val](float v) {
            current_val = v;
        }
    }));

    assert(child != nullptr);
    assert(child->value == 0.5f);
}

void test_slider_fluent_api() {
    float current_val = 0.2f;
    std::shared_ptr<SliderWidget> child = std::dynamic_pointer_cast<SliderWidget>(static_cast<WidgetPtr>(Slider {
        .value = current_val,
        .on_change = [](float){},
        .active_color = 0xFF00FF00,
        .inactive_color = 0xFF333333,
        .thumb_color = 0xFFFF0000,
        .track_height = 10.0f,
        .thumb_radius = 20.0f,
        .min_value = -1.0f,
        .max_value = 1.0f
    }));
                
    assert(child->active_color == 0xFF00FF00);
    assert(child->inactive_color == 0xFF333333);
    assert(child->thumb_color == 0xFFFF0000);
    assert(child->track_height == 10.0f);
    assert(child->thumb_radius == 20.0f);
    assert(child->min_value == -1.0f);
    assert(child->max_value == 1.0f);
}

int main() {
    test_slider_initialization();
    test_slider_fluent_api();
    std::cout << "All Slider tests passed!" << std::endl;
    return 0;
}
