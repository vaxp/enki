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
    
    auto btn = floatingActionButton(child, [&](){ clicked = true; });
    
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

void test_fab_fluent_api() {
    std::cout << "Testing FAB Fluent API..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    auto btn = floatingActionButton(child);
    
    btn->bgColor(0xFF00FF00)
       ->hoverColor(0xFF00DD00)
       ->size(80.0f)
       ->borderRadius(16.0f)
       ->elevation(20.0f, 10.0f);
                
    assert(btn->options.normal_color == 0xFF00FF00);
    assert(btn->options.hover_color == 0xFF00DD00);
    assert(btn->options.size == 80.0f);
    assert(btn->options.border_radius == 16.0f); // e.g. for extended FAB
    assert(btn->options.shadow_blur == 20.0f);
    assert(btn->options.shadow_offset_dy == 10.0f);
    
    std::cout << "FAB Fluent API passed." << std::endl;
}

int main() {
    test_fab_initialization();
    test_fab_fluent_api();
    std::cout << "All FAB tests passed!" << std::endl;
    return 0;
}
