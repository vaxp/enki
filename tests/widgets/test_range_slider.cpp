#include "enki/widgets/range_slider.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_range_slider_initialization() {
    float s = 0.2f, e = 0.8f;
    auto child = rangeSlider(s, e, [&s, &e](float sv, float ev) {
        s = sv;
        e = ev;
    });

    assert(child != nullptr);
    assert(child->start_value == 0.2f);
    assert(child->end_value == 0.8f);
}

void test_range_slider_fluent_api() {
    auto child = rangeSlider(10.0f, 90.0f, [](float, float){});
    
    child->activeColor(0xFF00FF00)
         ->inactiveColor(0xFF333333)
         ->thumbColor(0xFFFF0000)
         ->trackHeight(10.0f)
         ->thumbRadius(20.0f)
         ->min(0.0f)
         ->max(100.0f);
                
    assert(child->options.active_color == 0xFF00FF00);
    assert(child->options.inactive_color == 0xFF333333);
    assert(child->options.thumb_color == 0xFFFF0000);
    assert(child->options.track_height == 10.0f);
    assert(child->options.thumb_radius == 20.0f);
    assert(child->options.min_value == 0.0f);
    assert(child->options.max_value == 100.0f);
}

int main() {
    test_range_slider_initialization();
    test_range_slider_fluent_api();
    std::cout << "All RangeSlider tests passed!" << std::endl;
    return 0;
}
