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
    auto btn = std::dynamic_pointer_cast<ButtonWidget>(button(t));

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
    auto btn = std::dynamic_pointer_cast<ButtonWidget>(button(text("Submit"), [&]() { clicked = true; }, opt));

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

void test_declarative_syntax() {
    std::cout << "Testing Declarative Syntax..." << std::endl;
    bool clicked = false;
    WidgetPtr w = Button {
        .child = text("Declarative Button"),
        .on_pressed = [&]() { clicked = true; },
        .normal_color = 0xFF10B981,
        .border_radius = 12.0f,
    };

    auto btn = std::dynamic_pointer_cast<ButtonWidget>(w);
    assert(btn != nullptr);
    assert(btn->disabled == false);
    assert(btn->options.normal_color == 0xFF10B981);
    assert(btn->options.border_radius == 12.0f);

    if (btn->on_pressed) {
        btn->on_pressed();
    }
    assert(clicked == true);
    std::cout << "  ✓ Declarative Syntax passed." << std::endl;
}

void test_create_state() {
    std::cout << "Testing Create State..." << std::endl;
    auto btn = std::dynamic_pointer_cast<ButtonWidget>(button(text("Test")));
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
    test_declarative_syntax();
    test_create_state();

    std::cout << "========================================" << std::endl;
    std::cout << "All Button Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
