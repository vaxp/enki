#include "enki/widgets/slider.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_slider_initialization() {
    float current_val = 0.5f;
    auto child = slider(current_val, [&current_val](float v) {
        current_val = v;
    });

    assert(child != nullptr);
    assert(child->value == 0.5f);
}

void test_slider_fluent_api() {
    float current_val = 0.2f;
    auto child = slider(current_val, [](float){});
    
    child->activeColor(0xFF00FF00)
         ->inactiveColor(0xFF333333)
         ->thumbColor(0xFFFF0000)
         ->trackHeight(10.0f)
         ->thumbRadius(20.0f)
         ->min(-1.0f)
         ->max(1.0f);
                
    assert(child->options.active_color == 0xFF00FF00);
    assert(child->options.inactive_color == 0xFF333333);
    assert(child->options.thumb_color == 0xFFFF0000);
    assert(child->options.track_height == 10.0f);
    assert(child->options.thumb_radius == 20.0f);
    assert(child->options.min_value == -1.0f);
    assert(child->options.max_value == 1.0f);
}

int main() {
    test_slider_initialization();
    test_slider_fluent_api();
    std::cout << "All Slider tests passed!" << std::endl;
    return 0;
}
