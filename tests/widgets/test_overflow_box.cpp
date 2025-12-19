/// @file test_overflow_box.cpp
/// @brief Comprehensive Unit Tests for ENKI Section 11: OverflowBox Widget.

#include "enki/widgets/overflow_box.hpp"
#include "enki/widgets/container.hpp"
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

void test_overflow_box_basic_and_sizing() {
    std::cout << "Testing OverflowBox Widget: Basic Properties and Sizing Independence..." << std::endl;

    // A 100x100 container holding an OverflowBox with child forced to 180x180
    WidgetPtr w = container({
        .width = StyleValue::point(100.0f),
        .height = StyleValue::point(100.0f),
        .child = overflowBox({
            .alignment = Alignment::Center,
            .min_width = 180.0f,
            .max_width = 180.0f,
            .min_height = 180.0f,
            .max_height = 180.0f,
            .child = container({
                .width = StyleValue::point(180.0f),
                .height = StyleValue::point(180.0f),
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

    // Layout
    root_ro->layout(800.0f, 600.0f);

    // Root container must remain strictly 100x100
    std::cout << "  Root container size: " << root_ro->size().width << "x" << root_ro->size().height << std::endl;
    assert(approxEqual(root_ro->size().width, 100.0f));
    assert(approxEqual(root_ro->size().height, 100.0f));

    // OverflowBox render object
    assert(!root_ro->children().empty());
    auto* ofb_ro = root_ro->children()[0];
    assert(ofb_ro != nullptr);
    std::cout << "  OverflowBox size: " << ofb_ro->size().width << "x" << ofb_ro->size().height << std::endl;
    assert(approxEqual(ofb_ro->size().width, 100.0f));
    assert(approxEqual(ofb_ro->size().height, 100.0f));

    // Child container size must be 180x180
    assert(!ofb_ro->children().empty());
    auto* child_ro = ofb_ro->children()[0];
    assert(child_ro != nullptr);
    std::cout << "  Child container size: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;
    assert(approxEqual(child_ro->size().width, 180.0f));
    assert(approxEqual(child_ro->size().height, 180.0f));

    // For Center alignment, offset must be dx*0.5 = (100 - 180)*0.5 = -40
    std::cout << "  Child offset: (" << child_ro->offset().x << ", " << child_ro->offset().y << ")" << std::endl;
    assert(approxEqual(child_ro->offset().x, -40.0f));
    assert(approxEqual(child_ro->offset().y, -40.0f));

    el->unmount();
    std::cout << "  ✓ Basic and sizing independence test passed." << std::endl;
}

void test_overflow_box_alignment_offsets() {
    std::cout << "Testing OverflowBox Widget: All Alignment Offsets..." << std::endl;

    auto test_alignment = [](Alignment align, float expected_x, float expected_y) {
        WidgetPtr w = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
            .child = overflowBox({
                .alignment = align,
                .min_width = 160.0f,
                .max_width = 160.0f,
                .min_height = 160.0f,
                .max_height = 160.0f,
                .child = container({
                    .width = StyleValue::point(160.0f),
                    .height = StyleValue::point(160.0f),
                }),
            }),
        });

        auto el = w->createElement();
        el->mount(nullptr, 0);
        el->rebuild();

        auto* root_ro = el->findRenderObject();
        root_ro->layout(800.0f, 600.0f);

        auto* ofb_ro = root_ro->children()[0];
        auto* child_ro = ofb_ro->children()[0];

        // dx = 100 - 160 = -60, dy = 100 - 160 = -60
        assert(approxEqual(child_ro->offset().x, expected_x));
        assert(approxEqual(child_ro->offset().y, expected_y));

        el->unmount();
    };

    // dx = -60, dy = -60
    test_alignment(Alignment::TopLeft,      0.0f,   0.0f);
    test_alignment(Alignment::TopCenter,  -30.0f,   0.0f);
    test_alignment(Alignment::TopRight,   -60.0f,   0.0f);
    test_alignment(Alignment::CenterLeft,   0.0f, -30.0f);
    test_alignment(Alignment::Center,     -30.0f, -30.0f);
    test_alignment(Alignment::CenterRight,-60.0f, -30.0f);
    test_alignment(Alignment::BottomLeft,   0.0f, -60.0f);
    test_alignment(Alignment::BottomCenter,-30.0f, -60.0f);
    test_alignment(Alignment::BottomRight, -60.0f, -60.0f);

    std::cout << "  ✓ All 9 alignment directions verified with exact offsets." << std::endl;
}

void test_overflow_box_underflow_alignment() {
    std::cout << "Testing OverflowBox Widget: Underflow (Child Smaller than Box)..." << std::endl;

    // Box is 200x200, child is 80x80 -> dx = 120, dy = 120
    WidgetPtr w = container({
        .width = StyleValue::point(200.0f),
        .height = StyleValue::point(200.0f),
        .child = overflowBox({
            .alignment = Alignment::Center,
            .min_width = 80.0f,
            .max_width = 80.0f,
            .min_height = 80.0f,
            .max_height = 80.0f,
            .child = container({
                .width = StyleValue::point(80.0f),
                .height = StyleValue::point(80.0f),
            }),
        }),
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* root_ro = el->findRenderObject();
    root_ro->layout(800.0f, 600.0f);

    auto* ofb_ro = root_ro->children()[0];
    auto* child_ro = ofb_ro->children()[0];

    // dx = 120, dy = 120 -> Center offset = (60, 60)
    std::cout << "  Child underflow offset: (" << child_ro->offset().x << ", " << child_ro->offset().y << ")" << std::endl;
    assert(approxEqual(child_ro->offset().x, 60.0f));
    assert(approxEqual(child_ro->offset().y, 60.0f));

    el->unmount();
    std::cout << "  ✓ Underflow centering test passed." << std::endl;
}

void test_overflow_box_clipping_and_hit_testing() {
    std::cout << "Testing OverflowBox Widget: Clipping and Hit Testing..." << std::endl;

    // 1. Unclipped (Clip::None): Interactions outside 100x100 parent hit the child
    {
        WidgetPtr w = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
            .child = overflowBox({
                .alignment = Alignment::Center,
                .min_width = 160.0f,
                .max_width = 160.0f,
                .min_height = 160.0f,
                .max_height = 160.0f,
                .clip_behavior = Clip::None,
                .child = container({
                    .width = StyleValue::point(160.0f),
                    .height = StyleValue::point(160.0f),
                }),
            }),
        });

        auto el = w->createElement();
        el->mount(nullptr, 0);
        el->rebuild();

        auto* root_ro = el->findRenderObject();
        root_ro->layout(800.0f, 600.0f);

        auto* ofb_ro = root_ro->children()[0];

        // Child is at offset (-30, -30) and size 160x160 (spans x: -30..130, y: -30..130).
        // Point (-10, -10) is OUTSIDE the 100x100 parent, but INSIDE the overflowing child!
        HitTestResult result;
        bool hit = ofb_ro->hitTest(result, {-10.0f, -10.0f});
        assert(hit == true);
        assert(result.isHit());
        std::cout << "  Unclipped hit-test outside box bounds: " << (hit ? "PASSED" : "FAILED") << std::endl;

        // Point (-50, -50) is outside the child as well
        HitTestResult result_miss;
        bool hit_miss = ofb_ro->hitTest(result_miss, {-50.0f, -50.0f});
        assert(hit_miss == false);

        el->unmount();
    }

    // 2. Clipped (Clip::HardEdge): Interactions outside 100x100 parent are blocked
    {
        WidgetPtr w = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
            .child = overflowBox({
                .alignment = Alignment::Center,
                .min_width = 160.0f,
                .max_width = 160.0f,
                .min_height = 160.0f,
                .max_height = 160.0f,
                .clip_behavior = Clip::HardEdge,
                .child = container({
                    .width = StyleValue::point(160.0f),
                    .height = StyleValue::point(160.0f),
                }),
            }),
        });

        auto el = w->createElement();
        el->mount(nullptr, 0);
        el->rebuild();

        auto* root_ro = el->findRenderObject();
        root_ro->layout(800.0f, 600.0f);

        auto* ofb_ro = root_ro->children()[0];

        // Point (-10, -10) is outside the clipped 100x100 box -> MUST BE REJECTED
        HitTestResult result;
        bool hit = ofb_ro->hitTest(result, {-10.0f, -10.0f});
        assert(hit == false);
        std::cout << "  Clipped hit-test outside box bounds correctly rejected: PASSED" << std::endl;

        // Point (50, 50) is inside the clipped box -> MUST SUCCEED
        HitTestResult result_inside;
        bool hit_inside = ofb_ro->hitTest(result_inside, {50.0f, 50.0f});
        assert(hit_inside == true);
        assert(result_inside.isHit());
        std::cout << "  Clipped hit-test inside box bounds succeeded: PASSED" << std::endl;

        el->unmount();
    }

    std::cout << "  ✓ Clipping and hit testing tests passed." << std::endl;
}

void test_overflow_box_dynamic_update() {
    std::cout << "Testing OverflowBox Widget: Dynamic Property Updates..." << std::endl;

    auto ofb_widget = overflowBox({
        .alignment = Alignment::Center,
        .min_width = 140.0f,
        .max_width = 140.0f,
        .min_height = 140.0f,
        .max_height = 140.0f,
        .clip_behavior = Clip::None,
        .child = container({
            .width = StyleValue::point(140.0f),
            .height = StyleValue::point(140.0f),
        }),
    });

    WidgetPtr parent_w = container({
        .width = StyleValue::point(100.0f),
        .height = StyleValue::point(100.0f),
        .child = ofb_widget,
    });

    auto el = parent_w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* root_ro = el->findRenderObject();
    root_ro->layout(800.0f, 600.0f);

    auto* ofb_ro = root_ro->children()[0];
    auto* child_ro = ofb_ro->children()[0];

    // Initial Center offset: dx = -40 -> -20, dy = -40 -> -20
    assert(approxEqual(child_ro->offset().x, -20.0f));
    assert(approxEqual(child_ro->offset().y, -20.0f));

    // Dynamic update: change alignment to TopLeft
    ofb_widget->alignment = Alignment::TopLeft;
    BuildContext ctx{el.get()};
    ofb_widget->updateRenderObject(ctx, *ofb_ro);

    // Re-layout
    root_ro->layout(800.0f, 600.0f);

    // After switching to TopLeft, offset should be (0, 0)
    assert(approxEqual(child_ro->offset().x, 0.0f));
    assert(approxEqual(child_ro->offset().y, 0.0f));
    std::cout << "  Dynamic alignment switch to TopLeft: ("
              << child_ro->offset().x << ", " << child_ro->offset().y << ") - PASSED" << std::endl;

    // Dynamic update: change alignment to BottomRight
    ofb_widget->alignment = Alignment::BottomRight;
    ofb_widget->updateRenderObject(ctx, *ofb_ro);

    root_ro->layout(800.0f, 600.0f);

    // After switching to BottomRight, offset should be (-40, -40)
    assert(approxEqual(child_ro->offset().x, -40.0f));
    assert(approxEqual(child_ro->offset().y, -40.0f));
    std::cout << "  Dynamic alignment switch to BottomRight: ("
              << child_ro->offset().x << ", " << child_ro->offset().y << ") - PASSED" << std::endl;

    el->unmount();
    std::cout << "  ✓ Dynamic property updates test passed." << std::endl;
}

void test_overflow_box_declarative_syntax() {
    std::cout << "Testing OverflowBox Widget: Declarative C++20 Syntax..." << std::endl;

    // 1. Struct with designated initializers and operator WidgetPtr
    WidgetPtr w1 = OverflowBox {
        .alignment = Alignment::TopRight,
        .min_width = 150.0f,
        .max_width = 200.0f,
        .clip_behavior = Clip::AntiAlias,
        .child = container({ .width = StyleValue::point(50.0f) }),
    };
    assert(w1 != nullptr);
    assert(w1->typeName() == "OverflowBox");

    // 2. Factory overflowBox(OverflowBoxProps)
    WidgetPtr w2 = overflowBox({
        .alignment = Alignment::BottomCenter,
        .min_height = 80.0f,
        .child = container({ .height = StyleValue::point(40.0f) }),
    });
    assert(w2 != nullptr);
    assert(w2->typeName() == "OverflowBox");

    // 3. Simple factory overflowBox(child)
    WidgetPtr w3 = overflowBox(container({ .width = StyleValue::point(100.0f) }));
    assert(w3 != nullptr);
    assert(w3->typeName() == "OverflowBox");

    std::cout << "  ✓ Declarative C++20 syntax variants verified." << std::endl;
}

int main() {
    std::cout << "\n=== Starting ENKI OverflowBox Unit Tests ===" << std::endl;

    test_overflow_box_basic_and_sizing();
    test_overflow_box_alignment_offsets();
    test_overflow_box_underflow_alignment();
    test_overflow_box_clipping_and_hit_testing();
    test_overflow_box_dynamic_update();
    test_overflow_box_declarative_syntax();

    std::cout << "=== All OverflowBox Unit Tests PASSED Successfully! ===\n" << std::endl;
    return 0;
}
