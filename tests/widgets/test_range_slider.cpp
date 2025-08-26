#include "enki/widgets/range_slider.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_range_slider_declarative() {
    float s = 0.2f, e = 0.8f;
    WidgetPtr child = RangeSlider {
        .start_value = s,
        .end_value = e,
        .on_change = [&s, &e](float sv, float ev) {
            s = sv;
            e = ev;
        },
        .active_color = 0xFF00FF00,
        .inactive_color = 0xFF333333,
        .thumb_color = 0xFFFF0000,
        .track_height = 10.0f,
        .thumb_radius = 20.0f,
        .min_value = 0.0f,
        .max_value = 100.0f,
    };

    assert(child != nullptr);
    auto w = std::dynamic_pointer_cast<RangeSliderWidget>(child);
    assert(w != nullptr);
    assert(w->start_value == 0.2f);
    assert(w->end_value == 0.8f);
    assert(w->options.active_color == 0xFF00FF00);
    assert(w->options.inactive_color == 0xFF333333);
    assert(w->options.thumb_color == 0xFFFF0000);
    assert(w->options.track_height == 10.0f);
    assert(w->options.thumb_radius == 20.0f);
    assert(w->options.min_value == 0.0f);
    assert(w->options.max_value == 100.0f);
}

int main() {
    test_range_slider_declarative();
    std::cout << "All RangeSlider tests passed!" << std::endl;
    return 0;
}
