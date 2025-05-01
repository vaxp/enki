#include "enki/widgets/checkbox.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Checkbox Default Initialization..." << std::endl;
    auto cb = checkbox(true, nullptr);
    auto* c = static_cast<Checkbox*>(cb.get());
    
    assert(c->value == true);
    assert(c->options.size == 18.0f);
    assert(c->options.disabled == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_callback() {
    std::cout << "Testing Checkbox Callback..." << std::endl;
    bool clicked = false;
    bool new_val = false;
    auto cb = checkbox(false, [&](bool v) {
        clicked = true;
        new_val = v;
    });
    
    auto* c = static_cast<Checkbox*>(cb.get());
    assert(c->value == false);
    
    if (c->on_changed) {
        c->on_changed(true);
    }
    
    assert(clicked == true);
    assert(new_val == true);
    std::cout << "  ✓ Callback passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Checkbox Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_default_initialization();
    test_callback();

    std::cout << "========================================" << std::endl;
    std::cout << "All Checkbox Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
