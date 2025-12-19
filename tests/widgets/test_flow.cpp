/// @file test_flow.cpp
/// @brief Comprehensive Unit Tests for ENKI Section 11: Flow Widget & FlowDelegate.

#include "enki/widgets/flow.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>

#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

static bool approxEqual(float a, float b, float epsilon = 0.5f) {
    return std::abs(a - b) <= epsilon;
}

// ════════════════════════════════════════════════════════════════
// Matrix4 Unit Tests
// ════════════════════════════════════════════════════════════════

void test_matrix4_operations() {
    std::cout << "Testing Matrix4: Math & Transformations..." << std::endl;

    // Translation
    auto t = Matrix4::translation(50.0f, 80.0f);
    Point p1{10.0f, 20.0f};
    Point mapped_t = t.mapPoint(p1);
    assert(approxEqual(mapped_t.x, 60.0f));
    assert(approxEqual(mapped_t.y, 100.0f));

    Point inv_t = t.mapPointInverse(mapped_t);
    assert(approxEqual(inv_t.x, 10.0f));
    assert(approxEqual(inv_t.y, 20.0f));

    // Scale
    auto s = Matrix4::scale(2.0f, 3.0f);
    Point mapped_s = s.mapPoint(p1);
    assert(approxEqual(mapped_s.x, 20.0f));
    assert(approxEqual(mapped_s.y, 60.0f));

    Point inv_s = s.mapPointInverse(mapped_s);
    assert(approxEqual(inv_s.x, 10.0f));
    assert(approxEqual(inv_s.y, 20.0f));

    // SkMatrix row-major conversion
    float sk9[9];
    t.toSkMatrix9(sk9);
    assert(approxEqual(sk9[0], 1.0f));  // scaleX
    assert(approxEqual(sk9[2], 50.0f)); // transX
    assert(approxEqual(sk9[4], 1.0f));  // scaleY
    assert(approxEqual(sk9[5], 80.0f)); // transY

    std::cout << "  ✓ Matrix4 tests passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Custom Delegate for Flow Testing
// ════════════════════════════════════════════════════════════════

class TestMenuFlowDelegate : public FlowDelegate {
public:
    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 300.0f,
            250.0f
        };
    }

    BoxConstraints getConstraintsForChild(size_t /*index*/, const BoxConstraints& /*constraints*/) override {
        return BoxConstraints::tight(Size{60.0f, 60.0f});
    }

    void paintChildren(FlowPaintingContext& context) override {
        assert(context.childCount() == 3);
        assert(approxEqual(context.size().height, 250.0f));

        for (size_t i = 0; i < context.childCount(); ++i) {
            Size sz = context.getChildSize(i);
            assert(approxEqual(sz.width, 60.0f));
            assert(approxEqual(sz.height, 60.0f));

            float x = static_cast<float>(i) * 70.0f + 10.0f;
            float y = 20.0f;
            context.paintChild(i, Point{x, y}, 1.0f - (static_cast<float>(i) * 0.2f));
        }
    }
};

void test_flow_basic_sizing() {
    std::cout << "Testing Flow: Basic Sizing & Constraints..." << std::endl;

    auto delegate = std::make_shared<TestMenuFlowDelegate>();

    WidgetPtr w = flow({
        .delegate = delegate,
        .children = {
            container({ .color = 0xFFEF4444 }),
            container({ .color = 0xFF3B82F6 }),
            container({ .color = 0xFF10B981 }),
        },
    });

    assert(w != nullptr);

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);
    ro->layout(400.0f, 300.0f);

    std::cout << "  Flow size: " << ro->size().width << "x" << ro->size().height << std::endl;
    assert(approxEqual(ro->size().width, 400.0f));
    assert(approxEqual(ro->size().height, 250.0f));

    assert(ro->children().size() == 3);
    for (size_t i = 0; i < 3; ++i) {
        assert(approxEqual(ro->children()[i]->size().width, 60.0f));
        assert(approxEqual(ro->children()[i]->size().height, 60.0f));
    }

    el->unmount();
    std::cout << "  ✓ Basic sizing test passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Functional Lambda Delegate Test
// ════════════════════════════════════════════════════════════════

void test_flow_functional_lambda() {
    std::cout << "Testing Flow: Functional Lambdas (paint_callback & size_callback)..." << std::endl;

    bool paint_called = false;
    size_t painted_count = 0;

    WidgetPtr w = flow({
        .paint_callback = [&](FlowPaintingContext& ctx) {
            paint_called = true;
            painted_count = ctx.childCount();
            for (size_t i = 0; i < ctx.childCount(); ++i) {
                ctx.paintChild(i, Matrix4::translation(static_cast<float>(i) * 50.0f, 0.0f));
            }
        },
        .size_callback = [](const BoxConstraints& c) {
            return Size{350.0f, 150.0f};
        },
        .children = {
            container({ .width = StyleValue::point(40.0f), .height = StyleValue::point(40.0f) }),
            container({ .width = StyleValue::point(40.0f), .height = StyleValue::point(40.0f) }),
        },
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(500.0f, 500.0f);

    assert(approxEqual(ro->size().width, 350.0f));
    assert(approxEqual(ro->size().height, 150.0f));

    // Paint using Skia Surface
    auto surface = SkSurface::MakeRasterN32Premul(500, 500);
    assert(surface != nullptr);
    auto canvas = createCanvasWrapper(surface->getCanvas());
    PaintContext pctx{*canvas, {0.0f, 0.0f}, Rect{0, 0, 500, 500}, 1.0f};

    ro->paint(pctx);

    assert(paint_called);
    assert(painted_count == 2);

    el->unmount();
    std::cout << "  ✓ Functional lambda test passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Hit Testing with Matrix4 Transforms
// ════════════════════════════════════════════════════════════════

void test_flow_hit_testing_with_transforms() {
    std::cout << "Testing Flow: Hit Testing with Transformed Children..." << std::endl;

    WidgetPtr w = flow({
        .paint_callback = [](FlowPaintingContext& ctx) {
            // Child 0 is placed at (100, 100)
            ctx.paintChild(0, Matrix4::translation(100.0f, 100.0f));
            // Child 1 is placed at (250, 50)
            ctx.paintChild(1, Matrix4::translation(250.0f, 50.0f));
        },
        .constraints_callback = [](size_t /*i*/, const BoxConstraints& /*c*/) {
            return BoxConstraints::tight(Size{80.0f, 80.0f});
        },
        .children = {
            container({
                .color = 0xFF10B981,
                .width = StyleValue::point(80.0f),
                .height = StyleValue::point(80.0f),
            }),
            container({
                .color = 0xFF6366F1,
                .width = StyleValue::point(80.0f),
                .height = StyleValue::point(80.0f),
            }),
        },
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(400.0f, 300.0f);

    // Must paint once to record transforms
    auto surface = SkSurface::MakeRasterN32Premul(400, 300);
    assert(surface != nullptr);
    auto canvas = createCanvasWrapper(surface->getCanvas());
    PaintContext pctx{*canvas, {0.0f, 0.0f}, Rect{0, 0, 400, 300}, 1.0f};
    ro->paint(pctx);

    auto* child0_ro = ro->children()[0];
    auto* child1_ro = ro->children()[1];

    // 1. Hit test point (50, 50): Should miss both children
    HitTestResult res1;
    bool hit1 = ro->hitTest(res1, Point{50.0f, 50.0f});
    (void)hit1;
    bool child0_in_res1 = false;
    bool child1_in_res1 = false;
    for (const auto& entry : res1.path()) {
        if (entry.target == child0_ro) child0_in_res1 = true;
        if (entry.target == child1_ro) child1_in_res1 = true;
    }
    assert(!child0_in_res1);
    assert(!child1_in_res1);

    // 2. Hit test point (120, 120): Should hit child 0 (bounds: 100..180, 100..180)
    HitTestResult res2;
    bool hit2 = ro->hitTest(res2, Point{120.0f, 120.0f});
    assert(hit2);
    bool child0_in_res2 = false;
    for (const auto& entry : res2.path()) {
        if (entry.target == child0_ro) child0_in_res2 = true;
    }
    assert(child0_in_res2);

    // 3. Hit test point (270, 70): Should hit child 1 (bounds: 250..330, 50..130)
    HitTestResult res3;
    bool hit3 = ro->hitTest(res3, Point{270.0f, 70.0f});
    assert(hit3);
    bool child1_in_res3 = false;
    for (const auto& entry : res3.path()) {
        if (entry.target == child1_ro) child1_in_res3 = true;
    }
    assert(child1_in_res3);

    el->unmount();
    std::cout << "  ✓ Hit testing with transforms passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Declarative Props & Keys
// ════════════════════════════════════════════════════════════════

void test_flow_declarative_props() {
    std::cout << "Testing Flow: Declarative Props & Keys..." << std::endl;

    Key flow_key = Key::string("my_flow_widget");

    FlowProps props{
        .key = flow_key,
        .children = {
            text("First Item"),
            text("Second Item"),
        },
    };

    WidgetPtr w = props;
    assert(w != nullptr);
    assert(w->key == flow_key);
    assert(w->typeName() == "Flow");

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    ro->layout(200.0f, 100.0f);
    assert(ro->children().size() == 2);

    el->unmount();
    std::cout << "  ✓ Declarative props test passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Flow Widget Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_matrix4_operations();
    test_flow_basic_sizing();
    test_flow_functional_lambda();
    test_flow_hit_testing_with_transforms();
    test_flow_declarative_props();

    std::cout << "========================================" << std::endl;
    std::cout << "All Flow Unit Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
