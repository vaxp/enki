/// @file test_limited_box.cpp
/// @brief Comprehensive Unit Tests for ENKI Section 11: LimitedBox Widget.

#include "enki/widgets/limited_box.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/render_object.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

static bool approxEqual(float a, float b, float epsilon = 0.5f) {
    return std::abs(a - b) <= epsilon;
}

void test_limited_box_bounded_parent_no_effect() {
    std::cout << "Testing LimitedBox Widget: Bounded Parent (No Limiting Effect)..." << std::endl;

    // When the parent imposes bounded constraints (e.g. 400x300),
    // LimitedBox must NOT limit the child even if maxWidth/maxHeight are smaller!
    WidgetPtr w = container({
        .width = StyleValue::point(400.0f),
        .height = StyleValue::point(300.0f),
        .child = limitedBox({
            .max_width = 150.0f,
            .max_height = 100.0f,
            .child = container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
            }),
        }),
    });

    assert(w != nullptr);

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* root_ro = el->findRenderObject();
    assert(root_ro != nullptr);

    root_ro->layout(800.0f, 600.0f);

    std::cout << "  Root container size: " << root_ro->size().width << "x" << root_ro->size().height << std::endl;
    assert(approxEqual(root_ro->size().width, 400.0f));
    assert(approxEqual(root_ro->size().height, 300.0f));

    // LimitedBox render object
    assert(!root_ro->children().empty());
    auto* lbox_ro = root_ro->children()[0];
    assert(lbox_ro != nullptr);
    std::cout << "  LimitedBox size (bounded): " << lbox_ro->size().width << "x" << lbox_ro->size().height << std::endl;

    // Child container size
    assert(!lbox_ro->children().empty());
    auto* child_ro = lbox_ro->children()[0];
    assert(child_ro != nullptr);
    std::cout << "  Child container size: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;

    // Since parent is bounded (400x300), LimitedBox does not limit. Child gets full 400x300.
    assert(approxEqual(child_ro->size().width, 400.0f));
    assert(approxEqual(child_ro->size().height, 300.0f));

    el->unmount();
    std::cout << "  ✓ Bounded parent test passed." << std::endl;
}

void test_limited_box_unconstrained_height() {
    std::cout << "Testing LimitedBox Widget: Unconstrained Height (Clamped to maxHeight)..." << std::endl;

    // Inside a vertical ScrollView, height is unconstrained (unbounded).
    // Child container tries to be 350px high, but LimitedBox has max_height = 120px.
    WidgetPtr w = scrollView(
        ScrollOptions{ .direction = Axis::Vertical },
        limitedBox({
            .max_height = 120.0f,
            .child = container({
                .width = StyleValue::point(200.0f),
                .height = StyleValue::point(350.0f),
            }),
        })
    );

    assert(w != nullptr);

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* scroll_ro = el->findRenderObject();
    assert(scroll_ro != nullptr);

    scroll_ro->layout(800.0f, 600.0f);

    assert(!scroll_ro->children().empty());
    auto* lbox_ro = scroll_ro->children()[0];
    assert(lbox_ro != nullptr);

    std::cout << "  LimitedBox size in vertical scroll: " << lbox_ro->size().width << "x" << lbox_ro->size().height << std::endl;
    assert(approxEqual(lbox_ro->size().width, 200.0f));
    assert(approxEqual(lbox_ro->size().height, 120.0f));

    assert(!lbox_ro->children().empty());
    auto* child_ro = lbox_ro->children()[0];
    assert(child_ro != nullptr);
    std::cout << "  Child size inside LimitedBox: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;
    assert(approxEqual(child_ro->size().width, 200.0f));
    assert(approxEqual(child_ro->size().height, 120.0f));

    el->unmount();
    std::cout << "  ✓ Unconstrained height test passed." << std::endl;
}

void test_limited_box_unconstrained_width() {
    std::cout << "Testing LimitedBox Widget: Unconstrained Width (Clamped to maxWidth)..." << std::endl;

    // Inside a horizontal ScrollView, width is unconstrained (unbounded).
    // Child container tries to be 400px wide, but LimitedBox has max_width = 160px.
    WidgetPtr w = scrollView(
        ScrollOptions{ .direction = Axis::Horizontal },
        limitedBox({
            .max_width = 160.0f,
            .child = container({
                .width = StyleValue::point(400.0f),
                .height = StyleValue::point(80.0f),
            }),
        })
    );

    assert(w != nullptr);

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* scroll_ro = el->findRenderObject();
    assert(scroll_ro != nullptr);

    scroll_ro->layout(800.0f, 600.0f);

    assert(!scroll_ro->children().empty());
    auto* lbox_ro = scroll_ro->children()[0];
    assert(lbox_ro != nullptr);

    std::cout << "  LimitedBox size in horizontal scroll: " << lbox_ro->size().width << "x" << lbox_ro->size().height << std::endl;
    assert(approxEqual(lbox_ro->size().width, 160.0f));
    assert(approxEqual(lbox_ro->size().height, 80.0f));

    assert(!lbox_ro->children().empty());
    auto* child_ro = lbox_ro->children()[0];
    assert(child_ro != nullptr);
    std::cout << "  Child size inside LimitedBox: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;
    assert(approxEqual(child_ro->size().width, 160.0f));
    assert(approxEqual(child_ro->size().height, 80.0f));

    el->unmount();
    std::cout << "  ✓ Unconstrained width test passed." << std::endl;
}

void test_limited_box_dynamic_update() {
    std::cout << "Testing LimitedBox Widget: Dynamic Constraint Updates..." << std::endl;

    auto lbox_widget = limitedBox({
        .max_height = 100.0f,
        .child = container({
            .width = StyleValue::point(150.0f),
            .height = StyleValue::point(300.0f),
        }),
    });

    WidgetPtr w = scrollView(
        ScrollOptions{ .direction = Axis::Vertical },
        lbox_widget
    );

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* scroll_ro = el->findRenderObject();
    scroll_ro->layout(800.0f, 600.0f);

    auto* lbox_ro = scroll_ro->children()[0];
    std::cout << "  Initial height clamped to: " << lbox_ro->size().height << " (expected 100)" << std::endl;
    assert(approxEqual(lbox_ro->size().height, 100.0f));

    // Update max_height to 220.0f
    lbox_widget->max_height = 220.0f;
    BuildContext ctx(el.get());
    lbox_widget->updateRenderObject(ctx, *lbox_ro);

    scroll_ro->layout(800.0f, 600.0f);

    std::cout << "  Updated height clamped to: " << lbox_ro->size().height << " (expected 220)" << std::endl;
    assert(approxEqual(lbox_ro->size().height, 220.0f));

    el->unmount();
    std::cout << "  ✓ Dynamic update test passed." << std::endl;
}

void test_limited_box_declarative_syntax() {
    std::cout << "Testing LimitedBox Widget: Declarative C++20 Syntax and Types..." << std::endl;

    LimitedBoxProps props;
    assert(!props.max_width.has_value());
    assert(!props.max_height.has_value());
    assert(props.child == nullptr);

    auto w = limitedBox({
        .key = Key::string("my_limited_box"),
        .max_width = 320.0f,
        .max_height = 240.0f,
        .child = container({ .color = 0xFF123456 }),
    });

    assert(w->key == Key::string("my_limited_box"));
    assert(w->max_width.has_value() && *w->max_width == 320.0f);
    assert(w->max_height.has_value() && *w->max_height == 240.0f);
    assert(w->typeName() == "LimitedBox");

    std::cout << "  ✓ Declarative syntax test passed." << std::endl;
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Test Suite — LimitedBox Layout Widget" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    test_limited_box_bounded_parent_no_effect();
    test_limited_box_unconstrained_height();
    test_limited_box_unconstrained_width();
    test_limited_box_dynamic_update();
    test_limited_box_declarative_syntax();

    std::cout << "\n>>> ALL LIMITED_BOX UNIT TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
