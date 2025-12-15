/// @file test_floating_panel.cpp
/// @brief Unit tests for FloatingPanel overlay widget (Section 19: Overlay & Popup Extended).

#include "enki/widgets/floating_panel.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"

#include <iostream>
#include <cassert>
#include <memory>

using namespace enki;

void test_declarative_instantiation() {
    std::cout << "Testing FloatingPanel declarative instantiation..." << std::endl;

    bool state_changed = false;
    bool closed = false;

    auto ctrl = std::make_shared<FloatingPanelController>();

    WidgetPtr w = floatingPanel({
        .content = text("Inspector Properties"),
        .body = text("Dashboard Main Screen"),
        .options = {
            .title = "Audio HUD",
            .icon = "🎛️",
            .initial_x = 180.0f,
            .initial_y = 90.0f,
            .initial_width = 400.0f,
            .initial_height = 280.0f,
            .on_state_changed = [&](FloatingPanelDisplayState) {
                state_changed = true;
            },
            .on_closed = [&]() {
                closed = true;
            }
        },
        .controller = ctrl,
        .initial_open = true
    });

    assert(w != nullptr);
    assert(w->typeName() == "FloatingPanel");

    auto elem = w->createElement();
    assert(elem != nullptr);
    elem->mount(nullptr, 0);
    elem->rebuild();

    // Verify initial controller state
    assert(ctrl->isOpen() == true);
    assert(ctrl->getPosition().x == 180.0f);
    assert(ctrl->getPosition().y == 90.0f);
    assert(ctrl->getSize().width == 400.0f);
    assert(ctrl->getSize().height == 280.0f);
    assert(ctrl->getState() == FloatingPanelDisplayState::Normal);

    // Minimize panel
    ctrl->minimize();
    assert(ctrl->getState() == FloatingPanelDisplayState::Minimized);
    assert(state_changed == true);

    // Maximize panel
    ctrl->maximize();
    assert(ctrl->getState() == FloatingPanelDisplayState::Maximized);

    // Restore panel
    ctrl->restore();
    assert(ctrl->getState() == FloatingPanelDisplayState::Normal);

    // Reposition panel
    ctrl->setPosition(300.0f, 200.0f);
    assert(ctrl->getPosition().x == 300.0f);
    assert(ctrl->getPosition().y == 200.0f);

    // Resize panel
    ctrl->setSize(520.0f, 360.0f);
    assert(ctrl->getSize().width == 520.0f);
    assert(ctrl->getSize().height == 360.0f);

    // Hide panel
    ctrl->hide();
    assert(ctrl->isOpen() == false);

    // Show panel
    ctrl->show();
    assert(ctrl->isOpen() == true);
}

void test_controller_operations() {
    std::cout << "Testing FloatingPanel controller operations..." << std::endl;

    auto ctrl = std::make_shared<FloatingPanelController>();

    WidgetPtr w = FloatingPanel {
        .content = container({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f)
        }),
        .body = text("App View"),
        .controller = ctrl,
        .initial_open = false
    };

    auto elem = w->createElement();
    elem->mount(nullptr, 0);
    elem->rebuild();

    assert(ctrl->isOpen() == false);

    ctrl->toggle();
    assert(ctrl->isOpen() == true);

    ctrl->bringToFront();

    ctrl->toggle();
    assert(ctrl->isOpen() == false);
}

void test_render_box_updates() {
    std::cout << "Testing FloatingPanel direct render box updates and callbacks..." << std::endl;

    bool drag_updated = false;
    bool resize_updated = false;
    bool moved_called = false;
    bool resized_called = false;

    auto ctrl = std::make_shared<FloatingPanelController>();

    WidgetPtr w = FloatingPanel {
        .content = text("Audio Content"),
        .body = text("Studio Suite"),
        .options = {
            .initial_x = 100.0f,
            .initial_y = 100.0f,
            .initial_width = 300.0f,
            .initial_height = 200.0f,
            .on_moved = [&](float x, float y) {
                moved_called = true;
                assert(x == 150.0f);
                assert(y == 160.0f);
            },
            .on_resized = [&](float w, float h) {
                resized_called = true;
                assert(w == 450.0f);
                assert(h == 350.0f);
            },
            .on_drag_update = [&](float, float) {
                drag_updated = true;
            },
            .on_resize_update = [&](float, float) {
                resize_updated = true;
            }
        },
        .controller = ctrl,
        .initial_open = true
    };

    auto elem = w->createElement();
    elem->mount(nullptr, 0);
    elem->rebuild();

    // Verify initial positions
    assert(ctrl->getPosition().x == 100.0f);
    assert(ctrl->getPosition().y == 100.0f);
    assert(ctrl->getSize().width == 300.0f);
    assert(ctrl->getSize().height == 200.0f);

    // Call setPosition and verify on_moved is invoked
    ctrl->setPosition(150.0f, 160.0f);
    assert(moved_called == true);
    assert(ctrl->getPosition().x == 150.0f);
    assert(ctrl->getPosition().y == 160.0f);

    // Call setSize and verify on_resized is invoked
    ctrl->setSize(450.0f, 350.0f);
    assert(resized_called == true);
    assert(ctrl->getSize().width == 450.0f);
    assert(ctrl->getSize().height == 350.0f);
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  RUNNING FLOATING PANEL UNIT TESTS       " << std::endl;
    std::cout << "==========================================" << std::endl;

    test_declarative_instantiation();
    test_controller_operations();
    test_render_box_updates();

    std::cout << "All FloatingPanel tests passed successfully!" << std::endl;
    return 0;
}
