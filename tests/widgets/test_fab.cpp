#include "enki/widgets/floating_action_button.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icons_material.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_fab_initialization() {
    std::cout << "Testing FAB Default Initialization..." << std::endl;
    bool clicked = false;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    
    FloatingActionButton btn_decl {
        .child = child,
        .on_pressed = [&](){ clicked = true; }
    };
    
    WidgetPtr ptr = btn_decl;
    auto btn = std::dynamic_pointer_cast<FloatingActionButtonWidget>(ptr);
    assert(btn != nullptr);
    assert(btn->child != nullptr);
    assert(btn->on_pressed != nullptr);
    assert(btn->options.size == 56.0f);
    assert(btn->options.border_radius == 28.0f);
    assert(btn->options.shadow_blur == 12.0f);
    
    // Simulate click
    btn->on_pressed();
    assert(clicked == true);
    std::cout << "FAB Default Initialization passed." << std::endl;
}

void test_fab_declarative_props() {
    std::cout << "Testing FAB Declarative Props..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    FloatingActionButton btn_decl {
        .child = child,
        .normal_color = 0xFF00FF00,
        .hover_color = 0xFF00DD00,
        .size = 80.0f,
        .border_radius = 16.0f,
        .shadow_blur = 20.0f,
        .shadow_offset_dy = 10.0f
    };
    
    WidgetPtr ptr = btn_decl;
    auto btn = std::dynamic_pointer_cast<FloatingActionButtonWidget>(ptr);
    assert(btn != nullptr);
    assert(btn->options.normal_color == 0xFF00FF00);
    assert(btn->options.hover_color == 0xFF00DD00);
    assert(btn->options.size == 80.0f);
    assert(btn->options.border_radius == 16.0f); // e.g. for extended FAB
    assert(btn->options.shadow_blur == 20.0f);
    assert(btn->options.shadow_offset_dy == 10.0f);
    
    std::cout << "FAB Declarative Props passed." << std::endl;
}

int main() {
    test_fab_initialization();
    test_fab_declarative_props();
    std::cout << "All FAB tests passed!" << std::endl;
    return 0;
}
