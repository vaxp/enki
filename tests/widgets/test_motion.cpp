/// @file test_motion.cpp
/// @brief Comprehensive Unit & Integration Tests for Section 13 Animation & Motion widgets.
///
/// Tests:
///   1. AnimatedOpacity
///   2. AnimatedContainer
///   3. AnimatedScale
///   4. AnimatedRotation
///   5. AnimatedSlide
///   6. AnimatedSwitcher
///   7. SlideTransition

#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/animation/animation_controller.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: AnimatedOpacity
// ════════════════════════════════════════════════════════════════
void test_animated_opacity() {
    std::cout << "Testing AnimatedOpacity Widget..." << std::endl;

    bool end_called = false;
    WidgetPtr w1 = animatedOpacity({
        .opacity = 0.5f,
        .duration = std::chrono::milliseconds(200),
        .curve = &Curves::easeInOut,
        .on_end = [&end_called]() { end_called = true; },
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedOpacity");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);
    assert(ro->childCount() == 1);

    // Update with new opacity
    WidgetPtr w2 = animatedOpacity({
        .opacity = 1.0f,
        .duration = std::chrono::milliseconds(200),
        .curve = &Curves::easeOut,
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
        }),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedOpacity passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: AnimatedContainer
// ════════════════════════════════════════════════════════════════
void test_animated_container() {
    std::cout << "Testing AnimatedContainer Widget..." << std::endl;

    WidgetPtr w1 = animatedContainer({
        .color = 0xFF3B82F6,
        .border_radius = BorderRadius::circular(8.0f),
        .width = StyleValue::point(150.0f),
        .height = StyleValue::point(80.0f),
        .padding = StyleInsets::all(StyleValue::point(12.0f)),
        .duration = std::chrono::milliseconds(300),
        .curve = &Curves::fastOutSlowIn,
        .child = container({
            .width = StyleValue::point(40.0f),
            .height = StyleValue::point(40.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedContainer");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update with new dimensions & color
    WidgetPtr w2 = animatedContainer({
        .color = 0xFFEC4899,
        .border_radius = BorderRadius::circular(24.0f),
        .width = StyleValue::point(200.0f),
        .height = StyleValue::point(120.0f),
        .padding = StyleInsets::all(StyleValue::point(20.0f)),
        .duration = std::chrono::milliseconds(300),
        .curve = &Curves::easeInOut,
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedContainer passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: AnimatedScale
// ════════════════════════════════════════════════════════════════
void test_animated_scale() {
    std::cout << "Testing AnimatedScale Widget..." << std::endl;

    WidgetPtr w1 = animatedScale({
        .scale = 1.5f,
        .alignment = Alignment::Center,
        .duration = std::chrono::milliseconds(250),
        .curve = &Curves::elasticOut,
        .child = container({
            .width = StyleValue::point(60.0f),
            .height = StyleValue::point(60.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedScale");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Test hit test transformation
    HitTestResult result;
    ro->setSize({60.0f, 60.0f});
    bool hit = ro->hitTest(result, {30.0f, 30.0f});
    assert(hit == true);

    // Update with target scale 0.8
    WidgetPtr w2 = animatedScale({
        .scale = 0.8f,
        .alignment = Alignment::TopLeft,
        .duration = std::chrono::milliseconds(250),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedScale passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: AnimatedRotation
// ════════════════════════════════════════════════════════════════
void test_animated_rotation() {
    std::cout << "Testing AnimatedRotation Widget..." << std::endl;

    WidgetPtr w1 = animatedRotation({
        .turns = 0.25f, // 90 degrees
        .alignment = Alignment::Center,
        .duration = std::chrono::milliseconds(400),
        .curve = &Curves::bounceOut,
        .child = container({
            .width = StyleValue::point(80.0f),
            .height = StyleValue::point(80.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedRotation");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Test hit testing with rotated coords
    HitTestResult result;
    ro->setSize({80.0f, 80.0f});
    bool hit = ro->hitTest(result, {40.0f, 40.0f});
    assert(hit == true);

    // Update with turns = 1.0 (360 deg)
    WidgetPtr w2 = animatedRotation({
        .turns = 1.0f,
        .alignment = Alignment::Center,
        .duration = std::chrono::milliseconds(400),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedRotation passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: AnimatedSlide
// ════════════════════════════════════════════════════════════════
void test_animated_slide() {
    std::cout << "Testing AnimatedSlide Widget..." << std::endl;

    WidgetPtr w1 = animatedSlide({
        .offset = {0.0f, 1.0f}, // Slide down by 100%
        .duration = std::chrono::milliseconds(300),
        .curve = &Curves::easeOut,
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(50.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedSlide");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update with offset {0, 0}
    WidgetPtr w2 = animatedSlide({
        .offset = {0.0f, 0.0f},
        .duration = std::chrono::milliseconds(300),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedSlide passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: AnimatedSwitcher
// ════════════════════════════════════════════════════════════════
void test_animated_switcher() {
    std::cout << "Testing AnimatedSwitcher Widget..." << std::endl;

    WidgetPtr childA = container({
        .color = 0xFF3B82F6,
        .width = StyleValue::point(100.0f),
        .height = StyleValue::point(100.0f),
        .key = Key::string("child_A"),
    });

    WidgetPtr w1 = animatedSwitcher({
        .child = childA,
        .duration = std::chrono::milliseconds(250),
        .switch_in_curve = &Curves::easeIn,
        .switch_out_curve = &Curves::easeOut,
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "AnimatedSwitcher");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Switch to Child B
    WidgetPtr childB = container({
        .color = 0xFF10B981,
        .width = StyleValue::point(100.0f),
        .height = StyleValue::point(100.0f),
        .key = Key::string("child_B"),
    });

    WidgetPtr w2 = animatedSwitcher({
        .child = childB,
        .duration = std::chrono::milliseconds(250),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ AnimatedSwitcher passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 7: SlideTransition
// ════════════════════════════════════════════════════════════════
void test_slide_transition() {
    std::cout << "Testing SlideTransition Widget..." << std::endl;

    auto controller = std::make_shared<AnimationController>(std::chrono::milliseconds(500));
    controller->setValue(0.5f);

    WidgetPtr w1 = slideTransition({
        .position = controller,
        .begin = {0.0f, -1.0f},
        .end = {0.0f, 0.0f},
        .curve = &Curves::easeOut,
        .child = container({
            .width = StyleValue::point(120.0f),
            .height = StyleValue::point(60.0f),
        }),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "SlideTransition");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update with controller at 1.0f
    controller->setValue(1.0f);
    WidgetPtr w2 = slideTransition({
        .position = controller,
        .begin = {0.0f, -1.0f},
        .end = {0.0f, 0.0f},
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);

    el->unmount();
    std::cout << "  ✓ SlideTransition passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Section 13 Motion & Animation Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_animated_opacity();
    test_animated_container();
    test_animated_scale();
    test_animated_rotation();
    test_animated_slide();
    test_animated_switcher();
    test_slide_transition();

    std::cout << "========================================" << std::endl;
    std::cout << "All 7 Section 13 Widgets Tests PASSED! ✓" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
