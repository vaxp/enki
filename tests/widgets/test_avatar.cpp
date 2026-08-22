#include "enki/widgets/avatar.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/tree/build_context.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_default_initialization() {
    std::cout << "Testing Default Initialization..." << std::endl;
    WidgetPtr w = Avatar { .initials = "AB" };
    auto av = std::dynamic_pointer_cast<AvatarWidget>(w);
    assert(av != nullptr);
    assert(av->options.initials == "AB");
    assert(av->options.radius == 24.0f);
    assert(av->options.show_badge == false);
    std::cout << "  ✓ Default Initialization passed." << std::endl;
}

void test_initials_constructor() {
    std::cout << "Testing Avatar with Custom Options..." << std::endl;
    WidgetPtr w = Avatar { .radius = 30.0f, .initials = "CD", .show_badge = true };
    auto av = std::dynamic_pointer_cast<AvatarWidget>(w);
    assert(av != nullptr);
    assert(av->options.initials == "CD");
    assert(av->options.radius == 30.0f);
    assert(av->options.show_badge == true);
    std::cout << "  ✓ Custom Options passed." << std::endl;
}

void test_avatar_group_initialization() {
    std::cout << "Testing Avatar Group Initialization..." << std::endl;
    WidgetPtr av1 = Avatar { .initials = "A" };
    WidgetPtr av2 = Avatar { .initials = "B" };
    
    WidgetPtr w = AvatarGroup {
        .avatars = {av1, av2},
        .spacing = -10.0f,
        .max_avatars = 3
    };
    auto group = std::dynamic_pointer_cast<AvatarGroupWidget>(w);
    assert(group != nullptr);
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
