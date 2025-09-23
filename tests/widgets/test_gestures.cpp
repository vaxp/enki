/// @file test_gestures.cpp
/// @brief Comprehensive unit tests for GestureDetector and gesture recognizers.

#include "enki/gestures/recognizer.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/build_context.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

using namespace enki;

void testTapRecognizer() {
    std::cout << "[TEST] Running testTapRecognizer...\n";

    TapGestureRecognizer tap;
    bool tapped = false;
    bool tap_down = false;
    bool tap_up = false;
    bool tap_canceled = false;

    tap.on_tap      = [&] { tapped = true; };
    tap.on_tap_down = [&](const TapDownDetails& d) { tap_down = true; assert(d.button == MouseButton::Left); };
    tap.on_tap_up   = [&](const TapUpDetails& d) { tap_up = true; assert(d.button == MouseButton::Left); };
    tap.on_tap_cancel = [&] { tap_canceled = true; };

    // 1. Valid Tap Sequence
    PointerEvent down{PointerEvent::Down, {100, 100}, {10, 10}, MouseButton::Left, 0, 1.0};
    tap.handlePointerDown(down);
    assert(tap_down);
    assert(!tapped);

    PointerEvent up{PointerEvent::Up, {102, 101}, {12, 11}, MouseButton::Left, 0, 1.1};
    tap.handlePointerUp(up);
    assert(tap_up);
    assert(tapped);
    assert(!tap_canceled);

    // 2. Tap Canceled by exceeding slop distance
    tapped = false; tap_down = false; tap_up = false; tap_canceled = false;
    tap.handlePointerDown(down);
    assert(tap_down);

    PointerEvent far_move{PointerEvent::Move, {200, 200}, {110, 110}, MouseButton::Left, 0, 1.2};
    tap.handlePointerMove(far_move);
    assert(tap_canceled);
    assert(!tapped);

    tap.handlePointerUp(up);
    assert(!tapped);

    std::cout << "  ✓ testTapRecognizer passed!\n";
}

void testDoubleTapRecognizer() {
    std::cout << "[TEST] Running testDoubleTapRecognizer...\n";

    TapGestureRecognizer tap;
    int single_tap_count = 0;
    int double_tap_count = 0;

    tap.on_tap        = [&] { single_tap_count++; };
    tap.on_double_tap = [&] { double_tap_count++; };

    // First tap at t = 1.0
    tap.handlePointerDown({PointerEvent::Down, {50, 50}, {5, 5}, MouseButton::Left, 0, 1.0});
    tap.handlePointerUp({PointerEvent::Up, {50, 50}, {5, 5}, MouseButton::Left, 0, 1.05});
    assert(single_tap_count == 1);
    assert(double_tap_count == 0);

    // Second tap within 200ms at t = 1.20
    tap.handlePointerDown({PointerEvent::Down, {52, 51}, {7, 6}, MouseButton::Left, 0, 1.20});
    tap.handlePointerUp({PointerEvent::Up, {52, 51}, {7, 6}, MouseButton::Left, 0, 1.25});
    assert(double_tap_count == 1);

    std::cout << "  ✓ testDoubleTapRecognizer passed!\n";
}

void testSecondaryTapRecognizer() {
    std::cout << "[TEST] Running testSecondaryTapRecognizer...\n";

    TapGestureRecognizer tap;
    bool sec_tap = false;
    bool primary_tap = false;

    tap.on_tap           = [&] { primary_tap = true; };
    tap.on_secondary_tap = [&] { sec_tap = true; };

    // Right Click at t = 2.0
    tap.handlePointerDown({PointerEvent::Down, {80, 80}, {10, 10}, MouseButton::Right, 0, 2.0});
    tap.handlePointerUp({PointerEvent::Up, {80, 80}, {10, 10}, MouseButton::Right, 0, 2.1});

    assert(sec_tap);
    assert(!primary_tap);

    std::cout << "  ✓ testSecondaryTapRecognizer passed!\n";
}

void testLongPressRecognizer() {
    std::cout << "[TEST] Running testLongPressRecognizer...\n";

    LongPressGestureRecognizer lp;
    lp.duration_threshold = 0.300; // 300ms for fast testing
    bool lp_started = false;
    bool lp_ended   = false;

    lp.on_long_press_start = [&](const LongPressStartDetails&) { lp_started = true; };
    lp.on_long_press_end   = [&](const LongPressEndDetails&) { lp_ended = true; };

    // Pointer down at t = 10.0
    lp.handlePointerDown({PointerEvent::Down, {100, 100}, {10, 10}, MouseButton::Left, 0, 10.0});

    // Move before threshold (at t = 10.15) — should not trigger yet
    lp.handlePointerMove({PointerEvent::Move, {101, 101}, {11, 11}, MouseButton::Left, 0, 10.15});
    assert(!lp_started);

    // Move after threshold (at t = 10.35) — should trigger
    lp.handlePointerMove({PointerEvent::Move, {102, 101}, {12, 11}, MouseButton::Left, 0, 10.35});
    assert(lp_started);

    // Release pointer
    lp.handlePointerUp({PointerEvent::Up, {102, 101}, {12, 11}, MouseButton::Left, 0, 10.40});
    assert(lp_ended);

    std::cout << "  ✓ testLongPressRecognizer passed!\n";
}

void testPanRecognizer() {
    std::cout << "[TEST] Running testPanRecognizer...\n";

    PanGestureRecognizer pan;
    pan.touch_slop = 5.0f;

    bool pan_started = false;
    float accumulated_dx = 0.0f;
    float accumulated_dy = 0.0f;
    bool pan_ended   = false;

    pan.on_pan_start  = [&](const DragStartDetails&) { pan_started = true; };
    pan.on_pan_update = [&](const DragUpdateDetails& d) {
        accumulated_dx += d.delta.x;
        accumulated_dy += d.delta.y;
    };
    pan.on_pan_end    = [&](const DragEndDetails&) { pan_ended = true; };

    // Down at {100, 100}
    pan.handlePointerDown({PointerEvent::Down, {100, 100}, {10, 10}, MouseButton::Left, 0, 5.0});
    assert(!pan_started);

    // Small move within slop (2px)
    pan.handlePointerMove({PointerEvent::Move, {102, 100}, {12, 10}, MouseButton::Left, 0, 5.05});
    assert(!pan_started);

    // Move past slop (10px total)
    pan.handlePointerMove({PointerEvent::Move, {110, 100}, {20, 10}, MouseButton::Left, 0, 5.10});
    assert(pan_started);
    assert(accumulated_dx == 10.0f); // 20 - 10

    // Further move by +15px x, +20px y
    pan.handlePointerMove({PointerEvent::Move, {125, 120}, {35, 30}, MouseButton::Left, 0, 5.15});
    assert(accumulated_dx == 25.0f); // 10 + 15
    assert(accumulated_dy == 20.0f);

    // Pointer up
    pan.handlePointerUp({PointerEvent::Up, {125, 120}, {35, 30}, MouseButton::Left, 0, 5.20});
    assert(pan_ended);

    std::cout << "  ✓ testPanRecognizer passed!\n";
}

void testGestureDetectorWidgetAndHitTest() {
    std::cout << "[TEST] Running testGestureDetectorWidgetAndHitTest...\n";

    bool clicked = false;
    auto childBox = container();
    auto gd = gestureDetector({
        .child = childBox,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .cursor_type = SystemCursor::Pointer,
        .on_tap = [&] { clicked = true; },
    });

    auto element = gd->createElement();
    assert(element != nullptr);
    element->mount(nullptr, 0);

    auto* ro = element->findRenderObject();
    assert(ro != nullptr);
    assert(ro->cursor() == SystemCursor::Pointer);

    auto* rgd = dynamic_cast<RenderGestureDetector*>(ro);
    assert(rgd != nullptr);
    assert(rgd->hit_test_behavior == HitTestBehavior::Opaque);

    // Set layout dimensions on render object
    ANUNodeStyleSetWidth(rgd->anuNode(), 200.0f);
    ANUNodeStyleSetHeight(rgd->anuNode(), 100.0f);
    rgd->layout(200.0f, 100.0f);
    rgd->syncLayout();

    // Hit test inside bounds
    HitTestResult result;
    bool hit = rgd->hitTest(result, {50.0f, 50.0f});
    assert(hit);
    assert(result.isHit());

    // Dispatch tap sequence
    PointerEvent down{PointerEvent::Down, {50, 50}, {50, 50}, MouseButton::Left, 0, 1.0};
    rgd->handlePointerDown(down);
    PointerEvent up{PointerEvent::Up, {50, 50}, {50, 50}, MouseButton::Left, 0, 1.05};
    rgd->handlePointerUp(up);

    assert(clicked);

    std::cout << "  ✓ testGestureDetectorWidgetAndHitTest passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  ENKI Engine — Gestures Test Suite     \n";
    std::cout << "========================================\n";

    testTapRecognizer();
    testDoubleTapRecognizer();
    testSecondaryTapRecognizer();
    testLongPressRecognizer();
    testPanRecognizer();
    testGestureDetectorWidgetAndHitTest();

    std::cout << "========================================\n";
    std::cout << "  ALL GESTURE TESTS PASSED (6/6)!      \n";
    std::cout << "========================================\n";

    return 0;
}
