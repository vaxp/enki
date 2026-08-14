#include "enki/widgets/icon.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_font_data() {
    std::cout << "Testing Font IconData..." << std::endl;
    auto data = Icons::Material::favorite();
    assert(data.codepoint == 0xe87d);
    assert(data.font_family == "Material Icons");
    assert(!data.isSvg());
}

void test_svg_data() {
    std::cout << "Testing SVG IconData..." << std::endl;
    auto data = Icons::SVG::play();
    assert(!data.svg_path.empty());
    assert(data.isSvg());
}

void test_widget_properties() {
    std::cout << "Testing Icon Widget properties..." << std::endl;
    auto ic = icon(Icons::Material::home())->size(48.0f)->color(0xFF00FF00);
    assert(ic->size_val == 48.0f);
    assert(ic->color_val == Color(0xFF00FF00));
}

int main() {
    std::cout << "Running Enki Icon Test Suite" << std::endl;
    test_font_data();
    test_svg_data();
    test_widget_properties();
    std::cout << "All Icon Tests Passed Successfully!" << std::endl;
    return 0;
}
