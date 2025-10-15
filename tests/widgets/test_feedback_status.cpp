/// @file test_feedback_status.cpp
/// @brief Comprehensive Unit Tests for Section 18 Feedback & Status Extended widgets.
///
/// Tests:
///   1. Skeleton
///   2. Ripple
///   3. Pulse
///   4. CountBadge

#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cassert>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: Skeleton
// ════════════════════════════════════════════════════════════════
void test_skeleton() {
    std::cout << "Testing Skeleton Widget..." << std::endl;

    WidgetPtr w1 = skeleton({
        .enabled = true,
        .base_color = 0xFF1E293B,
        .highlight_color = 0xFF334155,
        .duration = std::chrono::milliseconds(1000),
        .width = StyleValue::point(140.0f),
        .height = StyleValue::point(24.0f),
        .border_radius = BorderRadius::circular(6.0f),
        .shape = SkeletonShape::Rectangle,
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "Skeleton");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update to circle skeleton
    WidgetPtr w2 = skeletonCircle(48.0f);
    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    // Disable skeleton
    WidgetPtr w3 = skeleton({
        .child = text("Loaded Profile"),
        .enabled = false,
    });
    assert(w3->canUpdate(*w1));
    el->update(w3);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ Skeleton passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Ripple
// ════════════════════════════════════════════════════════════════
void test_ripple() {
    std::cout << "Testing Ripple Widget..." << std::endl;

    bool tapped = false;
    WidgetPtr w1 = ripple({
        .child = container({
            .color = 0xFF3B82F6,
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(50.0f),
        }),
        .color = 0x40FFFFFF,
        .border_radius = BorderRadius::circular(8.0f),
        .clip_ripple = true,
        .on_tap = [&tapped]() { tapped = true; },
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "Ripple");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ Ripple passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: Pulse
// ════════════════════════════════════════════════════════════════
void test_pulse() {
    std::cout << "Testing Pulse Widget..." << std::endl;

    WidgetPtr w1 = pulse({
        .color = 0xFF10B981,
        .ring_count = 3,
        .max_radius = 28.0f,
        .dot_radius = 7.0f,
        .center_dot = true,
        .duration = std::chrono::milliseconds(1200),
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "Pulse");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update with recording red pulse
    WidgetPtr w2 = pulse({
        .color = 0xFFEF4444,
        .ring_count = 2,
        .max_radius = 20.0f,
    });
    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ Pulse passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: CountBadge
// ════════════════════════════════════════════════════════════════
void test_count_badge() {
    std::cout << "Testing CountBadge Widget..." << std::endl;

    WidgetPtr w1 = countBadge({
        .child = container({
            .width = StyleValue::point(40.0f),
            .height = StyleValue::point(40.0f),
        }),
        .count = 5,
        .max_count = 99,
        .show_zero = false,
        .bg_color = 0xFFEF4444,
        .alignment = Alignment::TopRight,
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "CountBadge");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Update with overflow counter (> 99)
    WidgetPtr w2 = countBadge({
        .child = container({
            .width = StyleValue::point(40.0f),
            .height = StyleValue::point(40.0f),
        }),
        .count = 150,
        .max_count = 99,
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    // Update with count = 0 (hidden when show_zero = false)
    WidgetPtr w3 = countBadge({
        .child = container({
            .width = StyleValue::point(40.0f),
            .height = StyleValue::point(40.0f),
        }),
        .count = 0,
        .show_zero = false,
    });

    assert(w3->canUpdate(*w1));
    el->update(w3);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ CountBadge passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Runner
// ════════════════════════════════════════════════════════════════
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Section 18 Feedback & Status Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_skeleton();
    test_ripple();
    test_pulse();
    test_count_badge();

    std::cout << "========================================" << std::endl;
    std::cout << "All 4 Section 18 Widgets Tests PASSED! ✓" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
