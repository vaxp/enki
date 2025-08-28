#include "enki/widgets/divider.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Divider Default Initialization..." << std::endl;
    auto div = divider();
    auto* d = static_cast<DividerWidget*>(div.get());
    
    assert(d->options.height == 16.0f);
    assert(d->options.thickness == 1.0f);
    assert(d->options.indent == 0.0f);
    assert(d->options.end_indent == 0.0f);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_custom_options() {
    std::cout << "Testing Divider Custom Options..." << std::endl;
    DividerProps opt;
    opt.height = 32.0f;
    opt.thickness = 4.0f;
    opt.indent = 10.0f;
    opt.end_indent = 20.0f;
    
    auto div = divider(opt);
    auto* d = static_cast<DividerWidget*>(div.get());
    
    assert(d->options.height == 32.0f);
    assert(d->options.thickness == 4.0f);
    assert(d->options.indent == 10.0f);
    assert(d->options.end_indent == 20.0f);
    std::cout << "  ✓ Custom Options passed." << std::endl;
}

void test_declarative_syntax() {
    std::cout << "Testing Divider Declarative Syntax..." << std::endl;
    WidgetPtr div = Divider {
        .height = 24.0f,
        .thickness = 2.0f,
        .indent = 5.0f,
        .end_indent = 15.0f,
        .color = 0xFF10B981,
        .style = DividerStyle::Dashed,
        .label = "TEST",
    };
    auto* d = static_cast<DividerWidget*>(div.get());
    assert(d->options.height == 24.0f);
    assert(d->options.thickness == 2.0f);
    assert(d->options.indent == 5.0f);
    assert(d->options.end_indent == 15.0f);
    assert(d->options.color == 0xFF10B981);
    assert(d->options.style == DividerStyle::Dashed);
    assert(d->options.label == "TEST");

    WidgetPtr vdiv = VerticalDivider {
        .width = 20.0f,
        .thickness = 3.0f,
        .color = 0xFFEF4444,
        .style = DividerStyle::Gradient,
    };
    auto* vd = static_cast<VerticalDividerWidget*>(vdiv.get());
    assert(vd->options.height == 20.0f);
    assert(vd->options.thickness == 3.0f);
    assert(vd->options.color == 0xFFEF4444);
    assert(vd->options.style == DividerStyle::Gradient);
    std::cout << "  ✓ Declarative Syntax passed." << std::endl;
}

void test_vertical_divider() {
    std::cout << "Testing Vertical Divider..." << std::endl;
    auto vdiv = verticalDivider();
    auto* vd = static_cast<VerticalDividerWidget*>(vdiv.get());
    
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
    test_declarative_syntax();
    test_vertical_divider();

    std::cout << "========================================" << std::endl;
    std::cout << "All Divider Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
