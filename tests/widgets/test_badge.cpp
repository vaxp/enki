#include "enki/widgets/badge.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_badge_initialization() {
    std::cout << "Testing Badge Default Initialization..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons")); // Some icon
    auto b = badge(child);
    
    assert(b->child != nullptr);
    assert(b->label == nullptr);
    assert(b->options.bg_color == 0xFFEF4444);
    assert(b->options.alignment == Alignment::TopRight);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_badge_fluent_api() {
    std::cout << "Testing Badge Fluent API..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    auto lbl = text("3");
    auto b = badge(child, lbl);
    b->bgColor(0xFF00FF00)
     .alignment(Alignment::TopLeft)
     .offset(5.0f, 5.0f)
     .size(20.0f);
                
    assert(b->options.bg_color == 0xFF00FF00);
    assert(b->options.alignment == Alignment::TopLeft);
    assert(b->options.offset.x == 5.0f);
    assert(b->options.offset.y == 5.0f);
    assert(b->options.size == 20.0f);
    assert(b->label != nullptr);
    std::cout << "  ✓ Fluent API passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Badge Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_badge_initialization();
    test_badge_fluent_api();

    std::cout << "========================================" << std::endl;
    std::cout << "All Badge Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
