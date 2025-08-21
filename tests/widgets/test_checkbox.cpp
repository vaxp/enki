#include "enki/widgets/checkbox.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Checkbox Default Initialization..." << std::endl;
    Checkbox c { .value = true };
    
    assert(c.value == true);
    assert(c.size == 18.0f);
    assert(c.disabled == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_callback() {
    std::cout << "Testing Checkbox Callback..." << std::endl;
    bool clicked = false;
    bool new_val = false;
    Checkbox c {
        .value = false,
        .on_changed = [&](bool v) {
            clicked = true;
            new_val = v;
        }
    };
    
    assert(c.value == false);
    
    if (c.on_changed) {
        c.on_changed(true);
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
