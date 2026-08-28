/// @file test_utility.cpp
/// @brief Unit Tests for ENKI Section 20 Utility / Behavioral widgets.
///
/// Tests:
///   1. Visibility
///   2. IgnorePointer

#include "enki/widgets/utility.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cassert>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: Visibility
// ════════════════════════════════════════════════════════════════
void test_visibility() {
    std::cout << "Testing Visibility Widget..." << std::endl;

    // 1. Visible = true
    WidgetPtr w1 = visibility({
        .child = text("Visible Text"),
        .replacement = text("Placeholder"),
        .visible = true,
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "Visibility");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    // 2. Visible = false (replaces with replacement)
    WidgetPtr w2 = visibility({
        .child = text("Visible Text"),
        .replacement = text("Placeholder"),
        .visible = false,
    });
    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    // 3. Visible = false with maintain_size = true
    WidgetPtr w3 = visibility({
        .child = container({
            .width = StyleValue::point(120.0f),
            .height = StyleValue::point(40.0f),
        }),
        .visible = false,
        .maintain_size = true,
    });
    assert(w3->canUpdate(*w1));
    el->update(w3);
    el->rebuild();

    // 4. Visible = false with maintain_state = true
    WidgetPtr w4 = visibility({
        .child = text("Stateful Child"),
        .visible = false,
        .maintain_state = true,
    });
    assert(w4->canUpdate(*w1));
    el->update(w4);
    el->rebuild();

    el->unmount();
    std::cout << "  ✓ Visibility passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: IgnorePointer
// ════════════════════════════════════════════════════════════════
void test_ignore_pointer() {
    std::cout << "Testing IgnorePointer Widget..." << std::endl;

    // 1. ignoring = true
    WidgetPtr w1 = ignorePointer({
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(50.0f),
        }),
        .ignoring = true,
    });

    assert(w1 != nullptr);
    assert(w1->typeName() == "IgnorePointer");

    auto el = w1->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Hit test must fail when ignoring == true
    HitTestResult hit_res1;
    bool hit1 = ro->hitTest(hit_res1, {50.0f, 25.0f});
    assert(!hit1);

    // 2. ignoring = false
    WidgetPtr w2 = ignorePointer({
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(50.0f),
        }),
        .ignoring = false,
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    // Re-test hit test when ignoring == false
    ro->layout(100.0f, 50.0f);
    HitTestResult hit_res2;
    bool hit2 = ro->hitTest(hit_res2, {50.0f, 25.0f});
    assert(hit2);

    el->unmount();
    std::cout << "  ✓ IgnorePointer passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Runner
// ════════════════════════════════════════════════════════════════
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Section 20 Utility Widgets Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_visibility();
    test_ignore_pointer();

    std::cout << "========================================" << std::endl;
    std::cout << "All Section 20 Utility Tests PASSED! ✓" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
