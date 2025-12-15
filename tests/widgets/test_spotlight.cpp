/// @file test_spotlight.cpp
/// @brief Unit tests for Spotlight tour overlay widget (Section 19: Overlay & Popup Extended).

#include "enki/widgets/spotlight.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"

#include <iostream>
#include <cassert>
#include <memory>

using namespace enki;

void test_declarative_instantiation() {
    std::cout << "Testing Spotlight declarative instantiation..." << std::endl;

    size_t changed_step = 0;
    bool tour_finished = false;
    bool tour_skipped = false;

    std::vector<SpotlightStep> steps;
    {
        SpotlightStep s1;
        s1.id = "step_nav";
        s1.title = "Explore Navigation";
        s1.description = "Use this sidebar to navigate files";
        s1.target_bounds = Rect{20.0f, 60.0f, 180.0f, 400.0f};
        s1.shape = SpotlightShape::RoundedRectangle;
        s1.placement = SpotlightPlacement::Right;
        steps.push_back(s1);

        SpotlightStep s2;
        s2.id = "step_search";
        s2.title = "Global Command Search";
        s2.description = "Press Ctrl+K anytime to launch commands";
        s2.target_bounds = Rect{240.0f, 20.0f, 300.0f, 40.0f};
        s2.shape = SpotlightShape::RoundedRectangle;
        s2.placement = SpotlightPlacement::Bottom;
        steps.push_back(s2);

        SpotlightStep s3;
        s3.id = "step_fab";
        s3.title = "Quick Create Action";
        s3.description = "Click to create a new resource";
        s3.target_bounds = Rect{700.0f, 500.0f, 56.0f, 56.0f};
        s3.shape = SpotlightShape::Circle;
        s3.placement = SpotlightPlacement::Top;
        steps.push_back(s3);
    }

    auto ctrl = std::make_shared<SpotlightTourController>();

    WidgetPtr w = spotlight({
        .body = text("Dashboard Main Screen"),
        .steps = steps,
        .options = {
            .card_width = 320.0f,
            .on_step_change = [&](size_t idx, const SpotlightStep&) {
                changed_step = idx;
            },
            .on_finish = [&]() {
                tour_finished = true;
            },
            .on_skip = [&]() {
                tour_skipped = true;
            }
        },
        .controller = ctrl,
        .initial_active = true
    });

    assert(w != nullptr);
    assert(w->typeName() == "Spotlight");

    auto elem = w->createElement();
    assert(elem != nullptr);
    elem->mount(nullptr, 0);
    elem->rebuild();

    // Verify controller initial state
    assert(ctrl->isActive() == true);
    assert(ctrl->getCurrentStepIndex() == 0);
    assert(ctrl->getTotalSteps() == 3);

    // Advance to Step 2
    ctrl->next();
    assert(ctrl->getCurrentStepIndex() == 1);
    assert(changed_step == 1);

    // Advance to Step 3 (Final step)
    ctrl->next();
    assert(ctrl->getCurrentStepIndex() == 2);
    assert(changed_step == 2);

    // Finish tour
    ctrl->next();
    assert(tour_finished == true);
    assert(ctrl->isActive() == false);
}

void test_controller_navigation() {
    std::cout << "Testing Spotlight controller navigation & skip..." << std::endl;

    auto ctrl = std::make_shared<SpotlightTourController>();
    bool skipped = false;

    std::vector<SpotlightStep> steps;
    {
        SpotlightStep s1;
        s1.title = "Step A";
        s1.target_bounds = Rect{100, 100, 200, 100};
        steps.push_back(s1);

        SpotlightStep s2;
        s2.title = "Step B";
        s2.target_bounds = Rect{350, 100, 200, 100};
        steps.push_back(s2);
    }

    WidgetPtr w = Spotlight {
        .body = container({
            .width = StyleValue::point(1000.0f),
            .height = StyleValue::point(700.0f)
        }),
        .steps = steps,
        .options = {
            .on_skip = [&]() { skipped = true; }
        },
        .controller = ctrl,
        .initial_active = false
    };

    auto elem = w->createElement();
    elem->mount(nullptr, 0);
    elem->rebuild();

    assert(ctrl->isActive() == false);

    // Start tour
    ctrl->start();
    assert(ctrl->isActive() == true);
    assert(ctrl->getCurrentStepIndex() == 0);

    // Test dynamic target rect update
    ctrl->updateTargetRect(Rect{120, 120, 250, 80});

    // Test go to step
    ctrl->goToStep(1);
    assert(ctrl->getCurrentStepIndex() == 1);

    // Test previous
    ctrl->previous();
    assert(ctrl->getCurrentStepIndex() == 0);

    // Test skip
    ctrl->skip();
    assert(skipped == true);
    assert(ctrl->isActive() == false);
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  RUNNING SPOTLIGHT TOUR UNIT TESTS       " << std::endl;
    std::cout << "==========================================" << std::endl;

    test_declarative_instantiation();
    test_controller_navigation();

    std::cout << "All Spotlight tests passed successfully!" << std::endl;
    return 0;
}
