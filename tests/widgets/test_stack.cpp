/// @file test_stack.cpp
/// @brief Comprehensive Unit & Integration Tests for Stack and Positioned Layout Widgets.

#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/tree/element.hpp"
#include "enki/rendering/canvas.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

static bool approxEqual(float a, float b, float epsilon = 0.5f) {
    return std::fabs(a - b) <= epsilon;
}

class TestBox : public SingleChildRenderObjectWidget {
public:
    float w;
    float h;

    TestBox(float w, float h) : w(w), h(h) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderDecoratedBox>();
        ANUNodeStyleSetWidth(ro->anuNode(), w);
        ANUNodeStyleSetHeight(ro->anuNode(), h);
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        ANUNodeStyleSetWidth(ro.anuNode(), w);
        ANUNodeStyleSetHeight(ro.anuNode(), h);
        ro.markNeedsLayout();
    }

    std::string_view typeName() const override { return "TestBox"; }
};

inline std::shared_ptr<TestBox> testBox(float w, float h) {
    return std::make_shared<TestBox>(w, h);
}

// ════════════════════════════════════════════════════════════════
// Test 1: Stack Basic Sizing & Children Mounting
// ════════════════════════════════════════════════════════════════
void test_stack_basic_layout() {
    std::cout << "Testing Stack Basic Layout..." << std::endl;

    auto s = stack({
        testBox(100.0f, 100.0f),
        testBox(60.0f, 60.0f),
    });
    s->width(300.0f).height(200.0f);

    auto el = s->createElement();
    el->mount(nullptr, 0);

    auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
    assert(rs != nullptr);

    rs->layout(500.0f, 500.0f);

    assert(approxEqual(rs->size().width, 300.0f));
    assert(approxEqual(rs->size().height, 200.0f));
    assert(rs->children().size() == 2);

    std::cout << "  ✓ Stack basic layout passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Positioned Explicit Coordinates (top, right, bottom, left)
// ════════════════════════════════════════════════════════════════
void test_positioned_coordinates() {
    std::cout << "Testing Positioned Coordinates..." << std::endl;

    auto p1 = positioned(testBox(50.0f, 50.0f));
    p1->top(15.0f).left(25.0f);

    auto p2 = positioned(testBox(40.0f, 40.0f));
    p2->bottom(10.0f).right(20.0f);

    auto s = stack({ p1, p2 });
    s->width(400.0f).height(300.0f);

    auto el = s->createElement();
    el->mount(nullptr, 0);

    auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
    assert(rs != nullptr);
    rs->layout(500.0f, 500.0f);

    auto* ro1 = rs->children()[0];
    auto* ro2 = rs->children()[1];

    // p1: left=25, top=15
    assert(approxEqual(ro1->offset().x, 25.0f));
    assert(approxEqual(ro1->offset().y, 15.0f));

    // p2: right=20 -> x = 400 - 20 - 40 = 340, bottom=10 -> y = 300 - 10 - 40 = 250
    assert(approxEqual(ro2->offset().x, 340.0f));
    assert(approxEqual(ro2->offset().y, 250.0f));

    std::cout << "  ✓ Positioned coordinates passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: Positioned::fill Spanning
// ════════════════════════════════════════════════════════════════
void test_positioned_fill() {
    std::cout << "Testing Positioned::fill..." << std::endl;

    auto p_fill = Positioned::fill(container(), 10.0f, 15.0f, 20.0f, 25.0f); // left=10, top=15, right=20, bottom=25

    auto s = stack({ p_fill });
    s->width(500.0f).height(400.0f);

    auto el = s->createElement();
    el->mount(nullptr, 0);

    auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
    assert(rs != nullptr);
    rs->layout(600.0f, 600.0f);

    auto* ro_fill = rs->children()[0];

    // left=10, top=15
    assert(approxEqual(ro_fill->offset().x, 10.0f));
    assert(approxEqual(ro_fill->offset().y, 15.0f));

    // width = 500 - 10 - 20 = 470, height = 400 - 15 - 25 = 360
    assert(approxEqual(ro_fill->size().width, 470.0f));
    assert(approxEqual(ro_fill->size().height, 360.0f));

    std::cout << "  ✓ Positioned::fill passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: Stack Non-Positioned Alignment
// ════════════════════════════════════════════════════════════════
void test_stack_alignment() {
    std::cout << "Testing Stack Non-Positioned Alignment..." << std::endl;

    // Center alignment
    {
        auto s = stack(Alignment::Center, {
            testBox(100.0f, 60.0f)
        });
        s->width(300.0f).height(200.0f);

        auto el = s->createElement();
        el->mount(nullptr, 0);
        auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
        rs->layout(500.0f, 500.0f);

        // x = (300 - 100) / 2 = 100, y = (200 - 60) / 2 = 70
        assert(approxEqual(rs->children()[0]->offset().x, 100.0f));
        assert(approxEqual(rs->children()[0]->offset().y, 70.0f));
    }

    // BottomRight alignment
    {
        auto s = stack(Alignment::BottomRight, {
            testBox(100.0f, 60.0f)
        });
        s->width(300.0f).height(200.0f);

        auto el = s->createElement();
        el->mount(nullptr, 0);
        auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
        rs->layout(500.0f, 500.0f);

        // x = 300 - 100 = 200, y = 200 - 60 = 140
        assert(approxEqual(rs->children()[0]->offset().x, 200.0f));
        assert(approxEqual(rs->children()[0]->offset().y, 140.0f));
    }

    std::cout << "  ✓ Stack alignment passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: Hit-Testing Reverse Stacking Order
// ════════════════════════════════════════════════════════════════
void test_stack_hit_testing_order() {
    std::cout << "Testing Stack Hit-Testing Order (Topmost First)..." << std::endl;

    // Layer 1 (Bottom): 0,0 to 200,200
    auto layer1 = container();
    layer1->size(200.0f, 200.0f);

    // Layer 2 (Top): Positioned at 50,50 with size 100,100
    auto layer2 = positioned(50.0f, 50.0f, 50.0f, 50.0f, container());
    layer2->width(100.0f).height(100.0f);

    auto s = stack({ layer1, layer2 });
    s->width(200.0f).height(200.0f);

    auto el = s->createElement();
    el->mount(nullptr, 0);
    auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
    rs->layout(300.0f, 300.0f);

    // Point at (80, 80) is inside BOTH Layer 1 and Layer 2
    HitTestResult result;
    bool hit = rs->hitTest(result, {80.0f, 80.0f});
    assert(hit);
    assert(!result.path().empty());

    // Topmost child (Layer 2) MUST be the primary target
    auto* topmost_target = result.path().front().target;
    auto* layer2_ro = rs->children()[1];
    assert(topmost_target == layer2_ro || topmost_target == layer2_ro->children()[0]);

    std::cout << "  ✓ Hit-testing stacking order passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: Paint Pipeline Execution with Clipping
// ════════════════════════════════════════════════════════════════
void test_stack_painting() {
    std::cout << "Testing Stack Painting Pipeline with Clipping..." << std::endl;

    auto surface = SkSurface::MakeRasterN32Premul(400, 400);
    assert(surface != nullptr);
    SkCanvas* sk_canvas = surface->getCanvas();
    auto canvas = createCanvasWrapper(sk_canvas);
    PaintContext ctx{*canvas, {0.0f, 0.0f}, Rect{0, 0, 400, 400}, 1.0f};

    auto s = stack({
        Positioned::fill(container()),
        positioned(10.0f, 10.0f, 0, 0, container())
    });
    s->width(300.0f).height(300.0f).clip(Clip::HardEdge);

    auto el = s->createElement();
    el->mount(nullptr, 0);
    auto* rs = dynamic_cast<RenderStack*>(el->findRenderObject());
    rs->layout(400.0f, 400.0f);

    // Call paint
    rs->paint(ctx);

    std::cout << "  ✓ Stack painting pipeline passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Stack & Positioned Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_stack_basic_layout();
    test_positioned_coordinates();
    test_positioned_fill();
    test_stack_alignment();
    test_stack_hit_testing_order();
    test_stack_painting();

    std::cout << "========================================" << std::endl;
    std::cout << "All Stack & Positioned Tests Passed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
