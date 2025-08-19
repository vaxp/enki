#include "enki/widgets/card.hpp"
#include "enki/widgets/text.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_card_initialization() {
    std::cout << "Testing Card Default Initialization..." << std::endl;
    auto child = text("Card Content");
    Card c { .child = child };
    
    assert(c.child != nullptr);
    assert(c.elevation == 8.0f);
    assert(c.color == 0xFF1E293B);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_card_fluent_api() {
    std::cout << "Testing Card Declarative API..." << std::endl;
    Card c {
        .color = 0xFFFFFFFF,
        .elevation = 12.0f,
        .border_radius = BorderRadius::circular(16.0f),
        .padding = StyleInsets::all(20.0f),
        .child = text("Hello")
    };
                
    assert(c.color == 0xFFFFFFFF);
    assert(c.elevation == 12.0f);
    assert(c.padding.top.value == 20.0f);
    std::cout << "  ✓ Declarative API passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Card Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_card_initialization();
    test_card_fluent_api();

    std::cout << "========================================" << std::endl;
    std::cout << "All Card Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
