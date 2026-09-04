/// @file test_intrinsic_height.cpp
/// @brief Unit Tests for ENKI Section 11: IntrinsicHeight Widget.

#include "enki/widgets/intrinsic_height.hpp"
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

void test_intrinsic_height_basic() {
    std::cout << "Testing IntrinsicHeight Widget: Basic Properties..." << std::endl;

    WidgetPtr w = intrinsicHeight({
        .step_height = 40.0f,
        .child = container({
            .width = StyleValue::point(120.0f),
            .height = StyleValue::point(55.0f),
        }),
    });

    assert(w != nullptr);
    assert(w->typeName() == "IntrinsicHeight");

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Calculate layout with large unconstrained parent
    ro->layout(800.0f, 600.0f);

    // Child is 55px tall; with step_height 40px, height must snap to 80px
    std::cout << "  Computed height: " << ro->size().height << " (expected 80)" << std::endl;
    assert(ro->size().height == 80.0f);
    assert(ro->size().width == 120.0f);

    el->unmount();
    std::cout << "  ✓ Basic test passed." << std::endl;
}

void test_intrinsic_height_unstepped() {
    std::cout << "Testing IntrinsicHeight Widget: Natural Unstepped Sizing..." << std::endl;

    WidgetPtr w = intrinsicHeight(container({
        .width = StyleValue::point(150.0f),
        .height = StyleValue::point(73.0f),
    }));

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(1000.0f, 800.0f);

    std::cout << "  Computed height: " << ro->size().height << " (expected 73)" << std::endl;
    assert(ro->size().height == 73.0f);
    assert(ro->size().width == 150.0f);

    el->unmount();
    std::cout << "  ✓ Unstepped sizing passed." << std::endl;
}

void test_intrinsic_height_update() {
    std::cout << "Testing IntrinsicHeight Widget: Dynamic Property Update..." << std::endl;

    WidgetPtr w1 = intrinsicHeight({
        .step_height = 25.0f,
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(40.0f),
        }),
    });

    auto el = w1->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(500.0f, 500.0f);
    // 40 snapped to step 25 -> 50
    assert(ro->size().height == 50.0f);

    // Update with new step_height 50.0f -> 40 snapped to 50 is 50
    // If step_height = 60.0f -> 40 snapped to 60 is 60
    WidgetPtr w2 = intrinsicHeight({
        .step_height = 60.0f,
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(40.0f),
        }),
    });

    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    ro->layout(500.0f, 500.0f);
    assert(ro->size().height == 60.0f);

    el->unmount();
    std::cout << "  ✓ Dynamic property update passed." << std::endl;
}

void test_intrinsic_height_row_tallest_child() {
    std::cout << "Testing IntrinsicHeight Widget: Sizing Row to Tallest Child..." << std::endl;

    // Inside a tall parent, a row wrapped in IntrinsicHeight should size to
    // exactly its tallest child (140px).
    auto r = row({
        .align_items = Align::Stretch,
        .children = {
            container({ .width = StyleValue::point(60.0f), .height = StyleValue::point(60.0f) }),
            container({ .width = StyleValue::point(80.0f), .height = StyleValue::point(140.0f) }),
            container({ .width = StyleValue::point(70.0f), .height = StyleValue::point(90.0f) }),
        },
    });

    WidgetPtr w = intrinsicHeight(r);

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(800.0f, 600.0f);

    std::cout << "  Row in IntrinsicHeight height: " << ro->size().height << " (expected 140)" << std::endl;
    assert(ro->size().height == 140.0f);

    el->unmount();
    std::cout << "  ✓ Tallest child row sizing passed." << std::endl;
}

void test_intrinsic_height_row_with_expanded() {
    std::cout << "Testing IntrinsicHeight Widget: Row with Expanded Children..." << std::endl;

    auto r = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Stretch,
        .children = {
            expanded({.flex = 1.0f, .child = container({.height = StyleValue::percent(100.0f), .child = text("Short")})}),
            expanded({.flex = 1.0f, .child = container({.height = StyleValue::point(120.0f)})}),
        },
    });

    WidgetPtr w = column({
        .width = StyleValue::point(600.0f),
        .children = {
            intrinsicHeight(r),
        },
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(600.0f, 600.0f);

    auto* ih_ro = ro->children()[0];
    assert(ih_ro != nullptr);
    std::cout << "  IntrinsicHeight size: " << ih_ro->size().width << " x " << ih_ro->size().height << std::endl;
    auto* row_ro = ih_ro->children()[0];
    assert(row_ro != nullptr);
    std::cout << "  Row child size: " << row_ro->size().width << " x " << row_ro->size().height << std::endl;

    for (size_t i = 0; i < row_ro->children().size(); ++i) {
        auto* ch = row_ro->children()[i];
        std::cout << "    Child " << i << " (FlexItem) size: " << ch->size().width << " x " << ch->size().height << std::endl;
        if (!ch->children().empty()) {
            auto* inner = ch->children()[0];
            std::cout << "      Inner child size: " << inner->size().width << " x " << inner->size().height << std::endl;
        }
    }

    assert(ih_ro->size().height == 120.0f);
    assert(ih_ro->size().width == 600.0f);
    assert(row_ro->size().width == 600.0f);
    assert(row_ro->children()[0]->children()[0]->size().height == 120.0f);
    assert(row_ro->children()[1]->children()[0]->size().height == 120.0f);

    el->unmount();
    std::cout << "  ✓ Row with expanded children test passed." << std::endl;
}

void test_intrinsic_height_dynamic_content_expansion() {
    std::cout << "Testing IntrinsicHeight Widget: Dynamic Content Expansion Across Frames..." << std::endl;

    auto make_widget = [](float tallest_h) {
        auto r = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Stretch,
            .children = {
                expanded({.flex = 1.0f, .child = container({.height = StyleValue::percent(100.0f)})}),
                expanded({.flex = 1.0f, .child = container({.height = StyleValue::point(tallest_h)})}),
            },
        });
        return column({
            .width = StyleValue::point(600.0f),
            .children = {
                intrinsicHeight(r),
            },
        });
    };

    WidgetPtr w1 = make_widget(100.0f);
    auto el = w1->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(600.0f, 600.0f);

    auto* ih_ro = ro->children()[0];
    auto* row_ro = ih_ro->children()[0];

    std::cout << "  Frame 1 IntrinsicHeight height: " << ih_ro->size().height << " (expected 100)" << std::endl;
    assert(ih_ro->size().height == 100.0f);
    assert(row_ro->children()[0]->children()[0]->size().height == 100.0f);

    // Frame 2: Dynamic addition of content expands the tallest card from 100px to 250px!
    WidgetPtr w2 = make_widget(250.0f);
    assert(w2->canUpdate(*w1));
    el->update(w2);
    el->rebuild();

    ro->layout(600.0f, 600.0f);

    std::cout << "  Frame 2 IntrinsicHeight height after expansion: " << ih_ro->size().height << " (expected 250)" << std::endl;
    assert(ih_ro->size().height == 250.0f);
    assert(row_ro->children()[0]->children()[0]->size().height == 250.0f);

    el->unmount();
    std::cout << "  ✓ Dynamic content expansion test passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ENKI Section 11: IntrinsicHeight Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_intrinsic_height_basic();
    test_intrinsic_height_unstepped();
    test_intrinsic_height_update();
    test_intrinsic_height_row_tallest_child();
    test_intrinsic_height_row_with_expanded();
    test_intrinsic_height_dynamic_content_expansion();

    std::cout << "All IntrinsicHeight tests PASSED!" << std::endl;
    return 0;
}
