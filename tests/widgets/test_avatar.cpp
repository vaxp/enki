#include "enki/widgets/avatar.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/tree/build_context.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Default Initialization..." << std::endl;
    AvatarOptions opt;
    opt.initials = "AB";
    auto av = avatar(opt);
    
    assert(av->options.initials == "AB");
    assert(av->options.radius == 24.0f);
    assert(av->options.show_badge == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_initials_constructor() {
    std::cout << "Testing Initials Constructor..." << std::endl;
    auto av = avatar("CD");
    assert(av->options.initials == "CD");
    assert(av->options.radius == 24.0f);
    std::cout << "  ✓ Initials Constructor passed." << std::endl;
}

void test_avatar_group_initialization() {
    std::cout << "Testing Avatar Group Initialization..." << std::endl;
    auto av1 = avatar("A");
    auto av2 = avatar("B");
    
    auto group = avatarGroup({av1, av2}, -10.0f, 3);
    assert(group->avatars.size() == 2);
    assert(group->spacing == -10.0f);
    assert(group->max_avatars == 3);
    std::cout << "  ✓ Avatar Group Initialization passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Avatar Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_default_initialization();
    test_initials_constructor();
    test_avatar_group_initialization();

    std::cout << "========================================" << std::endl;
    std::cout << "All Avatar Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
