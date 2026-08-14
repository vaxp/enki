#include "enki/widgets/radio.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_radio_initialization() {
    std::cout << "Testing Radio Default Initialization..." << std::endl;
    // value = 1, group_value = 1 (selected)
    auto rb = radio(1, 1, nullptr);
    auto* r = static_cast<Radio*>(rb.get());
    
    assert(r->value == 1);
    assert(r->group_value == 1);
    assert(r->options.size == 20.0f);
    assert(r->options.disabled == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_radio_callback() {
    std::cout << "Testing Radio Callback..." << std::endl;
    bool clicked = false;
    int new_val = 0;
    // value = 2, group_value = 1 (unselected)
    auto rb = radio(2, 1, [&](int v) {
        clicked = true;
        new_val = v;
    });
    
    auto* r = static_cast<Radio*>(rb.get());
    
    if (r->on_changed) {
        r->on_changed(r->value);
    }
    
    assert(clicked == true);
    assert(new_val == 2);
    std::cout << "  ✓ Callback passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Radio Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_radio_initialization();
    test_radio_callback();

    std::cout << "========================================" << std::endl;
    std::cout << "All Radio Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
