#include "enki/widgets/badge.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_badge_initialization() {
    std::cout << "Testing Badge Default Initialization..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons")); // Some icon
    Badge b { .child = child };
    
    assert(b.child != nullptr);
    assert(b.label == nullptr);
    assert(b.bg_color == 0xFFEF4444);
    assert(b.alignment == Alignment::TopRight);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_badge_fluent_api() {
    std::cout << "Testing Badge Declarative API..." << std::endl;
    auto child = icon(IconData::font(0xe7f4, "Material Icons"));
    auto lbl = text("3");
    Badge b {
        .child = child,
        .label = lbl,
        .bg_color = 0xFF00FF00,
        .alignment = Alignment::TopLeft,
        .offset = {5.0f, 5.0f},
        .size = 20.0f
    };
                
    assert(b.bg_color == 0xFF00FF00);
    assert(b.alignment == Alignment::TopLeft);
    assert(b.offset.x == 5.0f);
    assert(b.offset.y == 5.0f);
    assert(b.size == 20.0f);
    assert(b.label != nullptr);
    std::cout << "  ✓ Declarative API passed." << std::endl;
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
