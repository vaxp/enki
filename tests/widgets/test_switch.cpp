#include "enki/widgets/switch.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_switch_initialization() {
    std::cout << "Testing Switch Default Initialization..." << std::endl;
    Switch s { .value = true };
    
    assert(s.value == true);
    assert(s.width == 44.0f);
    assert(s.height == 24.0f);
    assert(s.disabled == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_switch_callback() {
    std::cout << "Testing Switch Callback..." << std::endl;
    bool clicked = false;
    bool new_val = false;
    Switch s {
        .value = false,
        .on_changed = [&](bool v) {
            clicked = true;
            new_val = v;
        }
    };
    
    assert(s.value == false);
    
    if (s.on_changed) {
        s.on_changed(true);
    }
    
    assert(clicked == true);
    assert(new_val == true);
    std::cout << "  ✓ Callback passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Switch Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_switch_initialization();
    test_switch_callback();

    std::cout << "========================================" << std::endl;
    std::cout << "All Switch Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
