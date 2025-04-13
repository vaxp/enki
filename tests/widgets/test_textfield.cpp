#include "enki/widgets/text_field.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_controller_state() {
    std::cout << "Testing TextFieldController state..." << std::endl;
    auto ctrl = std::make_shared<TextFieldController>("Hello Enki");
    
    assert(ctrl->text == "Hello Enki");
    assert(ctrl->selection_start == 0);
    assert(ctrl->selection_end == 0);
    
    ctrl->selectAll();
    assert(ctrl->selection_start == 0);
    assert(ctrl->selection_end == 10);
    assert(ctrl->hasSelection() == true);
    
    ctrl->clearSelection();
    assert(ctrl->hasSelection() == false);
    assert(ctrl->selection_end == 0);
    
    std::cout << "TextFieldController tests passed!" << std::endl;
}

int main() {
    test_controller_state();
    std::cout << "All TextField tests passed!" << std::endl;
    return 0;
}
