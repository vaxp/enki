/// @file test_flexbox.cpp
/// @brief Exhaustive test suite for Enki Flexbox widget and Anu layout engine integration.

#include "enki/widgets/flexbox.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/render_object.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace enki;

// Helper to check floating-point approximate equality
static bool approxEqual(float a, float b, float epsilon = 0.01f) {
    return std::abs(a - b) <= epsilon;
}

// ════════════════════════════════════════════════════════════════
// Mock Leaf Widget with fixed size for layout testing
// ════════════════════════════════════════════════════════════════

class RenderFixedBox : public RenderBox {
public:
    void paint(PaintContext&) override {}
};

class FixedBox : public SingleChildRenderObjectWidget {
public:
    float w;
    float h;

    FixedBox(float w, float h) : w(w), h(h) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderFixedBox>();
        ANUNodeStyleSetWidth(ro->anuNode(), w);
        ANUNodeStyleSetHeight(ro->anuNode(), h);
        return ro;
    }

    std::string_view typeName() const override { return "FixedBox"; }
};

inline WidgetPtr fixedBox(float w, float h) {
    return std::make_shared<FixedBox>(w, h);
}

// ════════════════════════════════════════════════════════════════
// Test 1: Basic Row and Column Layout
// ════════════════════════════════════════════════════════════════

void test_row_basic() {
    std::cout << "Testing Basic Row Layout..." << std::endl;

    auto rowWidget = row({
        .width = 400_px,
        .height = 100_px,
        .children = {
            fixedBox(100.0f, 50.0f),
            fixedBox(150.0f, 50.0f),
            fixedBox(50.0f, 50.0f)
        }
    });

    auto element = rowWidget->createElement();
    element->mount(nullptr, 0);

    auto* renderFlex = dynamic_cast<RenderFlex*>(element->findRenderObject());
    assert(renderFlex != nullptr);

    renderFlex->layout(400.0f, 100.0f);

    assert(approxEqual(renderFlex->size().width, 400.0f));
    assert(approxEqual(renderFlex->size().height, 100.0f));

    const auto& children = renderFlex->children();
    assert(children.size() == 3);

    // Child 0: x=0, y=0, w=100, h=50
    assert(approxEqual(children[0]->offset().x, 0.0f));
    assert(approxEqual(children[0]->offset().y, 0.0f));
    assert(approxEqual(children[0]->size().width, 100.0f));
    assert(approxEqual(children[0]->size().height, 50.0f));

    // Child 1: x=100, y=0, w=150, h=50
    assert(approxEqual(children[1]->offset().x, 100.0f));
    assert(approxEqual(children[1]->offset().y, 0.0f));
    assert(approxEqual(children[1]->size().width, 150.0f));

    // Child 2: x=250, y=0, w=50, h=50
    assert(approxEqual(children[2]->offset().x, 250.0f));
    assert(approxEqual(children[2]->offset().y, 0.0f));
    assert(approxEqual(children[2]->size().width, 50.0f));

    std::cout << "  ✓ Basic Row passed." << std::endl;
}

void test_column_basic() {
    std::cout << "Testing Basic Column Layout..." << std::endl;

    auto colWidget = column({
        .width = 200_px,
        .height = 300_px,
        .children = {
            fixedBox(100.0f, 40.0f),
            fixedBox(100.0f, 60.0f),
            fixedBox(100.0f, 100.0f)
        }
    });

    auto element = colWidget->createElement();
    element->mount(nullptr, 0);

    auto* renderFlex = dynamic_cast<RenderFlex*>(element->findRenderObject());
    assert(renderFlex != nullptr);

    renderFlex->layout(200.0f, 300.0f);

    const auto& children = renderFlex->children();
    assert(children.size() == 3);

    assert(approxEqual(children[0]->offset().y, 0.0f));
    assert(approxEqual(children[0]->size().height, 40.0f));

    assert(approxEqual(children[1]->offset().y, 40.0f));
    assert(approxEqual(children[1]->size().height, 60.0f));

    assert(approxEqual(children[2]->offset().y, 100.0f));
    assert(approxEqual(children[2]->size().height, 100.0f));

    std::cout << "  ✓ Basic Column passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Flex Factors (FlexGrow, FlexShrink, FlexBasis)
// ════════════════════════════════════════════════════════════════

void test_flex_grow() {
    std::cout << "Testing FlexGrow Distribution..." << std::endl;

    // Total width 300px.
    // Child 0: flexGrow=1
    // Child 1: flexGrow=2
    // Child 2: fixed 60px (flexGrow=0)
    // Remaining space = 300 - 60 = 240px.
    // Child 0 gets 1/3 * 240 = 80px.
    // Child 1 gets 2/3 * 240 = 160px.
    auto rowWidget = row({
        .width = 300_px,
        .height = 50_px,
        .children = {
            expanded(fixedBox(0.0f, 50.0f), 1.0f),
            expanded(fixedBox(0.0f, 50.0f), 2.0f),
            fixedBox(60.0f, 50.0f)
        }
    });

    auto element = rowWidget->createElement();
    element->mount(nullptr, 0);

    auto* renderFlex = dynamic_cast<RenderFlex*>(element->findRenderObject());
    assert(renderFlex != nullptr);

    renderFlex->layout(300.0f, 50.0f);

    const auto& children = renderFlex->children();
    assert(children.size() == 3);

    assert(approxEqual(children[0]->size().width, 80.0f));
    assert(approxEqual(children[1]->size().width, 160.0f));
    assert(approxEqual(children[2]->size().width, 60.0f));

    assert(approxEqual(children[0]->offset().x, 0.0f));
    assert(approxEqual(children[1]->offset().x, 80.0f));
    assert(approxEqual(children[2]->offset().x, 240.0f));

    std::cout << "  ✓ FlexGrow passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: JustifyContent (SpaceBetween, Center, SpaceEvenly)
// ════════════════════════════════════════════════════════════════

void test_justify_content() {
    std::cout << "Testing JustifyContent Modes..." << std::endl;

    // Row width 400px, 2 children of 100px each. Remaining = 200px.
    // SpaceBetween -> child0 at x=0, child1 at x=300
    {
        auto r = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Start,
            .width = 400_px,
            .height = 50_px,
            .children = {
                fixedBox(100.0f, 50.0f),
                fixedBox(100.0f, 50.0f)
            }
        });

        auto el = r->createElement();
        el->mount(nullptr, 0);
        auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
        rf->layout(400.0f, 50.0f);

        assert(approxEqual(rf->children()[0]->offset().x, 0.0f));
        assert(approxEqual(rf->children()[1]->offset().x, 300.0f));
    }

    // Center -> (400 - 200)/2 = 100px offset. child0 at x=100, child1 at x=200
    {
        auto r = row({
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .width = 400_px,
            .height = 50_px,
            .children = {
                fixedBox(100.0f, 50.0f),
                fixedBox(100.0f, 50.0f)
            }
        });

        auto el = r->createElement();
        el->mount(nullptr, 0);
        auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
        rf->layout(400.0f, 50.0f);

        assert(approxEqual(rf->children()[0]->offset().x, 100.0f));
        assert(approxEqual(rf->children()[1]->offset().x, 200.0f));
    }

    // SpaceEvenly -> 3 gaps of 200/3 = 66.67px. child0 at 66.67, child1 at 66.67+100+66.67 = 233.33
    {
        auto r = row({
            .justify_content = Justify::SpaceEvenly,
            .align_items = Align::Start,
            .width = 400_px,
            .height = 50_px,
            .children = {
                fixedBox(100.0f, 50.0f),
                fixedBox(100.0f, 50.0f)
            }
        });

        auto el = r->createElement();
        el->mount(nullptr, 0);
        auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
        rf->layout(400.0f, 50.0f);

        assert(approxEqual(rf->children()[0]->offset().x, 67.0f));
        assert(approxEqual(rf->children()[1]->offset().x, 233.0f));
    }

    std::cout << "  ✓ JustifyContent passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: AlignItems and AlignSelf
// ════════════════════════════════════════════════════════════════

void test_align_items_and_self() {
    std::cout << "Testing AlignItems and AlignSelf..." << std::endl;

    // Container height 200px. AlignItems: Center (y = (200-50)/2 = 75px).
    // Child 1 overrides with alignSelf: FlexEnd (y = 200-50 = 150px).
    auto r = row({
        .justify_content = Justify::Start,
        .align_items = Align::Center,
        .width = 300_px,
        .height = 200_px,
        .children = {
            fixedBox(100.0f, 50.0f),
            flexItem({
                .align_self = Align::End,
                .child = fixedBox(100.0f, 50.0f),
            })
        }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(300.0f, 200.0f);

    assert(approxEqual(rf->children()[0]->offset().y, 75.0f));
    assert(approxEqual(rf->children()[1]->offset().y, 150.0f));

    std::cout << "  ✓ AlignItems & AlignSelf passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: Gaps / Gutters
// ════════════════════════════════════════════════════════════════

void test_gaps() {
    std::cout << "Testing Gap / Gutters..." << std::endl;

    // Row with 20px gap
    auto r = row({
        .gap = 20_px,
        .width = 300_px,
        .height = 100_px,
        .children = {
            fixedBox(50.0f, 50.0f),
            fixedBox(50.0f, 50.0f),
            fixedBox(50.0f, 50.0f)
        }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(300.0f, 100.0f);

    assert(approxEqual(rf->children()[0]->offset().x, 0.0f));
    assert(approxEqual(rf->children()[1]->offset().x, 70.0f));  // 50 + 20
    assert(approxEqual(rf->children()[2]->offset().x, 140.0f)); // 70 + 50 + 20

    std::cout << "  ✓ Gaps passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: Insets (Padding & Margin)
// ════════════════════════════════════════════════════════════════

void test_insets() {
    std::cout << "Testing Padding and Margin..." << std::endl;

    auto r = row({
        .width = 200_px,
        .height = 200_px,
        .padding = StyleInsets::all(15_px),
        .children = { fixedBox(50.0f, 50.0f) }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(200.0f, 200.0f);

    assert(approxEqual(rf->children()[0]->offset().x, 15.0f));
    assert(approxEqual(rf->children()[0]->offset().y, 15.0f));

    std::cout << "  ✓ Padding passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 7: Percentage Dimensions & AspectRatio
// ════════════════════════════════════════════════════════════════

void test_percentage_and_aspect_ratio() {
    std::cout << "Testing Percentage Dimensions & AspectRatio..." << std::endl;

    // Child width = 50% of 400 = 200px
    // AspectRatio = 2.0 (width/height = 2) -> height = 100px
    auto r = row({
        .width = 400_px,
        .height = 300_px,
        .children = {
            flexItem({
                .width = 50_pct,
                .aspect_ratio = 2.0f,
                .child = fixedBox(0, 0),
            })
        }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(400.0f, 300.0f);

    assert(approxEqual(rf->children()[0]->size().width, 200.0f));
    assert(approxEqual(rf->children()[0]->size().height, 100.0f));

    std::cout << "  ✓ Percentage Dimensions & AspectRatio passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 8: FlexWrap
// ════════════════════════════════════════════════════════════════

void test_flex_wrap() {
    std::cout << "Testing FlexWrap..." << std::endl;

    // Container width 250px. 3 children of 100px width.
    // Child 0 & 1 take 200px on row 0.
    // Child 2 wraps to row 1 (y=50px).
    auto r = row({
        .flex_wrap = FlexWrap::Wrap,
        .width = 250_px,
        .height = 200_px,
        .children = {
            fixedBox(100.0f, 50.0f),
            fixedBox(100.0f, 50.0f),
            fixedBox(100.0f, 50.0f)
        }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(250.0f, 200.0f);

    assert(approxEqual(rf->children()[0]->offset().x, 0.0f));
    assert(approxEqual(rf->children()[0]->offset().y, 0.0f));

    assert(approxEqual(rf->children()[1]->offset().x, 100.0f));
    assert(approxEqual(rf->children()[1]->offset().y, 0.0f));

    assert(approxEqual(rf->children()[2]->offset().x, 0.0f));
    assert(approxEqual(rf->children()[2]->offset().y, 50.0f));

    std::cout << "  ✓ FlexWrap passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 9: RTL Direction
// ════════════════════════════════════════════════════════════════

void test_rtl_direction() {
    std::cout << "Testing RTL Direction..." << std::endl;

    // In RTL, items start from the right edge.
    // Container width 300px. Child 0 (100px) starts at x = 200px. Child 1 (50px) at x = 150px.
    auto r = row({
        .direction = Direction::RTL,
        .width = 300_px,
        .height = 100_px,
        .children = {
            fixedBox(100.0f, 50.0f),
            fixedBox(50.0f, 50.0f)
        }
    });

    auto el = r->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(300.0f, 100.0f);

    assert(approxEqual(rf->children()[0]->offset().x, 200.0f));
    assert(approxEqual(rf->children()[1]->offset().x, 150.0f));

    std::cout << "  ✓ RTL Direction passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 10: Nested Flexbox Trees
// ════════════════════════════════════════════════════════════════

void test_nested_flexbox() {
    std::cout << "Testing Nested Flexbox Trees..." << std::endl;

    // Outer column of 400x400
    // Row 1: height 100, containing 2 equal expanded items (200px each)
    // Row 2: height 300, containing 1 fixed item (100px) + 1 expanded (300px)
    auto r1 = row({
        .height = 100_px,
        .children = {
            expanded(fixedBox(0.0f, 100.0f), 1.0f),
            expanded(fixedBox(0.0f, 100.0f), 1.0f)
        }
    });

    auto r2 = row({
        .height = 300_px,
        .children = {
            fixedBox(100.0f, 300.0f),
            expanded(fixedBox(0.0f, 300.0f), 1.0f)
        }
    });

    auto outer = column({
        .width = 400_px,
        .height = 400_px,
        .children = { r1, r2 }
    });

    auto el = outer->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(400.0f, 400.0f);

    assert(approxEqual(rf->size().width, 400.0f));
    assert(approxEqual(rf->size().height, 400.0f));

    auto* row1 = dynamic_cast<RenderFlex*>(rf->children()[0]);
    auto* row2 = dynamic_cast<RenderFlex*>(rf->children()[1]);

    assert(row1 != nullptr);
    assert(row2 != nullptr);

    assert(approxEqual(row1->size().height, 100.0f));
    assert(approxEqual(row2->size().height, 300.0f));

    assert(approxEqual(row1->children()[0]->size().width, 200.0f));
    assert(approxEqual(row1->children()[1]->size().width, 200.0f));

    assert(approxEqual(row2->children()[0]->size().width, 100.0f));
    assert(approxEqual(row2->children()[1]->size().width, 300.0f));

    std::cout << "  ✓ Nested Flexbox passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Flexbox Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_row_basic();
    test_column_basic();
    test_flex_grow();
    test_justify_content();
    test_align_items_and_self();
    test_gaps();
    test_insets();
    test_percentage_and_aspect_ratio();
    test_flex_wrap();
    test_rtl_direction();
    test_nested_flexbox();

    std::cout << "========================================" << std::endl;
    std::cout << "All Flexbox Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
