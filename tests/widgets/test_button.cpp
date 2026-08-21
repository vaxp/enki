#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/state/state.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Default Initialization..." << std::endl;
    auto t = text("Click Me");
    auto btn = std::dynamic_pointer_cast<Button>(button(t));

    assert(btn != nullptr);
    assert(btn->typeName() == "Button");
    assert(btn->disabled == true); // No callback = disabled
    assert(btn->options.normal_color == 0xFF2563EB);
    assert(btn->options.enable_ripple == true);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_custom_options() {
    std::cout << "Testing Custom Options..." << std::endl;
    ButtonProps opt;
    opt.normal_color = 0xFFFF0000;
    opt.enable_ripple = false;
    opt.custom_shader = "void main() {}";

    bool clicked = false;
    auto btn = std::dynamic_pointer_cast<Button>(button(text("Submit"), [&]() { clicked = true; }, opt));

    assert(btn->disabled == false);
    assert(btn->options.normal_color == 0xFFFF0000);
    assert(btn->options.enable_ripple == false);
    assert(btn->options.custom_shader == "void main() {}");

    if (btn->on_pressed) {
        btn->on_pressed();
    }
    assert(clicked == true);
    std::cout << "  ✓ Custom Options passed." << std::endl;
}

void test_create_state() {
    std::cout << "Testing Create State..." << std::endl;
    auto btn = std::dynamic_pointer_cast<Button>(button(text("Test")));
    auto state = btn->createState();
    assert(state != nullptr);
    std::cout << "  ✓ Create State passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Button Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_default_initialization();
    test_custom_options();
    test_create_state();

    std::cout << "========================================" << std::endl;
    std::cout << "All Button Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
