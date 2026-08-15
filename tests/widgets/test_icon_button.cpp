#include "enki/widgets/icon_button.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icons_material.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_icon_button_initialization() {
    std::cout << "Testing IconButton Default Initialization..." << std::endl;
    bool clicked = false;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    
    auto btn = iconButton(child, [&](){ clicked = true; });
    
    assert(btn->icon != nullptr);
    assert(btn->on_pressed != nullptr);
    assert(btn->options.size == 48.0f);
    assert(btn->options.normal_color == 0x00000000); // transparent
    assert(btn->options.enable_ripple == true);
    
    // Simulate click
    btn->on_pressed();
    assert(clicked == true);
    std::cout << "IconButton Default Initialization passed." << std::endl;
}

void test_icon_button_fluent_api() {
    std::cout << "Testing IconButton Fluent API..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    auto btn = iconButton(child);
    
    btn->bgColor(0xFF112233)
       ->hoverColor(0xFF445566)
       ->size(64.0f)
       ->paddingAll(12.0f);
                
    assert(btn->options.normal_color == 0xFF112233);
    assert(btn->options.hover_color == 0xFF445566);
    assert(btn->options.size == 64.0f);
    assert(btn->options.padding.top == 12.0f);
    
    std::cout << "IconButton Fluent API passed." << std::endl;
}

int main() {
    test_icon_button_initialization();
    test_icon_button_fluent_api();
    std::cout << "All IconButton tests passed!" << std::endl;
    return 0;
}
