#include <enki/widgets/text.hpp>
#include <enki/tree/build_context.hpp>
#include <enki/tree/element.hpp>
#include <iostream>
#include <cassert>

using namespace enki;

void test_richtext_interactive_spans() {
    bool clicked = false;
    
    auto s = span({
        .text = "Click me!",
        .on_click = [&clicked]() { clicked = true; },
    });
    
    auto rt = richText(s);
    auto element = rt->createElement();
    
    assert(element != nullptr);
    assert(s->on_click != nullptr);
}

int main() {
    test_richtext_interactive_spans();
    std::cout << "All RichText tests passed!\n";
    return 0;
}
