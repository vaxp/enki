/// @file test_intrinsic_width.cpp
/// @brief Unit Tests for ENKI Section 11: IntrinsicWidth Widget.

#include "enki/widgets/intrinsic_width.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/render_object.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

void test_intrinsic_width_basic() {
    std::cout << "Testing IntrinsicWidth Widget: Basic Properties..." << std::endl;

    WidgetPtr w = intrinsicWidth({
        .step_width = 50.0f,
        .child = container({
            .width = StyleValue::point(112.0f),
            .height = StyleValue::point(40.0f),
        }),
    });

    assert(w != nullptr);
    assert(w->typeName() == "IntrinsicWidth");

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Calculate layout with large unconstrained parent
    ro->layout(800.0f, 600.0f);

    // Child is 112px wide; with step_width 50px, width must snap to 150px
    std::cout << "  Computed width: " << ro->size().width << " (expected 150)" << std::endl;
    assert(ro->size().width == 150.0f);
    assert(ro->size().height == 40.0f);

    el->unmount();
    std::cout << "  ✓ Basic test passed." << std::endl;
}

void test_intrinsic_width_unstepped() {
    std::cout << "Testing IntrinsicWidth Widget: Natural Unstepped Sizing..." << std::endl;

    WidgetPtr w = intrinsicWidth(container({
        .width = StyleValue::point(137.0f),
        .height = StyleValue::point(55.0f),
    }));

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(1000.0f, 800.0f);

    std::cout << "  Computed width: " << ro->size().width << " (expected 137)" << std::endl;
    assert(ro->size().width == 137.0f);
    assert(ro->size().height == 55.0f);

    el->unmount();
    std::cout << "  ✓ Unstepped sizing passed." << std::endl;
}

void test_intrinsic_width_update() {
    std::cout << "Testing IntrinsicWidth Widget: Dynamic Property Update..." << std::endl;

    WidgetPtr w1 = intrinsicWidth({
        .step_width = 20.0f,
        .child = container({
            .width = StyleValue::point(65.0f),
            .height = StyleValue::point(30.0f),
        }),
    });

    auto el = w1->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(500.0f, 500.0f);
    // 65 snapped to step 20 -> 80
    assert(ro->size().width == 80.0f);

    // Update with new step_width 50.0f -> 65 snapped to 50 is 100
    WidgetPtr w2 = intrinsicWidth({
        .step_width = 50.0f,
        .child = container({
            .width = StyleValue::point(65.0f),
            .height = StyleValue::point(30.0f),
        }),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    ro->layout(500.0f, 500.0f);
    assert(ro->size().width == 100.0f);

    el->unmount();
    std::cout << "  ✓ Dynamic property update passed." << std::endl;
}

void test_intrinsic_width_column_widest_child() {
    std::cout << "Testing IntrinsicWidth Widget: Sizing Column to Widest Child..." << std::endl;

    // Inside a wide container (800px), a column wrapped in IntrinsicWidth
    // should NOT stretch to 800px, but size to exactly its widest child (220px).
    auto col = column({
        .align_items = Align::Stretch,
        .children = {
            container({ .width = StyleValue::point(120.0f), .height = StyleValue::point(30.0f) }),
            container({ .width = StyleValue::point(220.0f), .height = StyleValue::point(30.0f) }),
            container({ .width = StyleValue::point(90.0f),  .height = StyleValue::point(30.0f) }),
        },
    });

    WidgetPtr w = intrinsicWidth(col);

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(800.0f, 600.0f);

    std::cout << "  Column in IntrinsicWidth width: " << ro->size().width << " (expected 220)" << std::endl;
    assert(ro->size().width == 220.0f);

    el->unmount();
    std::cout << "  ✓ Widest child column sizing passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ENKI Section 11: IntrinsicWidth Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_intrinsic_width_basic();
    test_intrinsic_width_unstepped();
    test_intrinsic_width_update();
    test_intrinsic_width_column_widest_child();

    std::cout << "All IntrinsicWidth tests PASSED!" << std::endl;
    return 0;
}
