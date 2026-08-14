#include "enki/widgets/divider.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Divider Default Initialization..." << std::endl;
    auto div = divider();
    auto* d = static_cast<Divider*>(div.get());
    
    assert(d->options.height == 16.0f);
    assert(d->options.thickness == 1.0f);
    assert(d->options.indent == 0.0f);
    assert(d->options.end_indent == 0.0f);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_custom_options() {
    std::cout << "Testing Divider Custom Options..." << std::endl;
    DividerOptions opt;
    opt.height = 32.0f;
    opt.thickness = 4.0f;
    opt.indent = 10.0f;
    opt.end_indent = 20.0f;
    
    auto div = divider(opt);
    auto* d = static_cast<Divider*>(div.get());
    
    assert(d->options.height == 32.0f);
    assert(d->options.thickness == 4.0f);
    assert(d->options.indent == 10.0f);
    assert(d->options.end_indent == 20.0f);
    std::cout << "  ✓ Custom Options passed." << std::endl;
}

void test_vertical_divider() {
    std::cout << "Testing Vertical Divider..." << std::endl;
    auto vdiv = verticalDivider();
    auto* vd = static_cast<VerticalDivider*>(vdiv.get());
    
    assert(vd->options.height == 16.0f);
    assert(vd->options.thickness == 1.0f);
    std::cout << "  ✓ Vertical Divider passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Divider Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_default_initialization();
    test_custom_options();
    test_vertical_divider();

    std::cout << "========================================" << std::endl;
    std::cout << "All Divider Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
