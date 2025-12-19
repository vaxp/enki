/// @file test_custom_multi_child_layout.cpp
/// @brief Comprehensive Unit Tests for ENKI Section 11: CustomMultiChildLayout Widget & MultiChildLayoutDelegate.

#include "enki/widgets/custom_multi_child_layout.hpp"
#include "enki/widgets/container.hpp"
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

// ════════════════════════════════════════════════════════════════
// Custom Delegate Example for Testing
// ════════════════════════════════════════════════════════════════

class TestHeaderBodyDelegate : public MultiChildLayoutDelegate {
public:
    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 400.0f,
            300.0f
        };
    }

    void performLayout(Size size) override {
        assert(hasChild("header"));
        assert(hasChild("body"));
        assert(!hasChild("ghost_child"));

        Size headerSize = layoutChild("header", BoxConstraints::tightFor(size.width, 60.0f));
        positionChild("header", Point{0.0f, 0.0f});

        Size bodySize = layoutChild("body", BoxConstraints::tightFor(size.width, size.height - headerSize.height));
        (void)bodySize;
        positionChild("body", Point{0.0f, headerSize.height});
    }
};

void test_custom_multi_child_layout_basic() {
    std::cout << "Testing CustomMultiChildLayout: Basic Delegate (Header + Body)..." << std::endl;

    auto delegate = std::make_shared<TestHeaderBodyDelegate>();

    WidgetPtr w = customMultiChildLayout({
        .delegate = delegate,
        .children = {
            layoutId("header", container({
                .color = 0xFF1E293B,
                .child = text("Header Bar"),
            })),
            layoutId("body", container({
                .color = 0xFF0F172A,
                .child = text("Body Content"),
            })),
        },
    });

    assert(w != nullptr);

    auto el = w->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* layout_ro = el->findRenderObject();
    assert(layout_ro != nullptr);

    layout_ro->layout(500.0f, 400.0f);

    std::cout << "  CustomMultiChildLayout size: " << layout_ro->size().width << "x" << layout_ro->size().height << std::endl;
    assert(approxEqual(layout_ro->size().width, 500.0f));
    assert(approxEqual(layout_ro->size().height, 300.0f));

    assert(layout_ro->children().size() == 2);
    auto* header_ro = layout_ro->children()[0];
    auto* body_ro   = layout_ro->children()[1];

    std::cout << "  Header offset: (" << header_ro->offset().x << ", " << header_ro->offset().y << "), size: "
              << header_ro->size().width << "x" << header_ro->size().height << std::endl;
    assert(approxEqual(header_ro->offset().x, 0.0f));
    assert(approxEqual(header_ro->offset().y, 0.0f));
    assert(approxEqual(header_ro->size().width, 500.0f));
    assert(approxEqual(header_ro->size().height, 60.0f));

    std::cout << "  Body offset: (" << body_ro->offset().x << ", " << body_ro->offset().y << "), size: "
              << body_ro->size().width << "x" << body_ro->size().height << std::endl;
    assert(approxEqual(body_ro->offset().x, 0.0f));
    assert(approxEqual(body_ro->offset().y, 60.0f));
    assert(approxEqual(body_ro->size().width, 500.0f));
    assert(approxEqual(body_ro->size().height, 240.0f));

    el->unmount();
    std::cout << "  ✓ Basic delegate test passed." << std::endl;
}

void test_custom_multi_child_layout_dependent_layout() {
    std::cout << "Testing CustomMultiChildLayout: Dependent Layout (Badge Overlapping Avatar)..." << std::endl;

    class AvatarBadgeDelegate : public MultiChildLayoutDelegate {
    public:
        void performLayout(Size size) override {
            Size avatarSize = layoutChild("avatar", BoxConstraints::tight(Size{80.0f, 80.0f}));
            positionChild("avatar", Point{50.0f, 50.0f});

            Size badgeSize = layoutChild("badge", BoxConstraints::tight(Size{24.0f, 24.0f}));
            // Overlap badge at top right corner of avatar
            Point badgeOffset = {
                50.0f + avatarSize.width - (badgeSize.width / 2.0f),
                50.0f - (badgeSize.height / 2.0f)
            };
            positionChild("badge", badgeOffset);
        }
    };

    WidgetPtr w = container({
        .width = StyleValue::point(300.0f),
        .height = StyleValue::point(200.0f),
        .child = customMultiChildLayout({
            .delegate = std::make_shared<AvatarBadgeDelegate>(),
            .children = {
                layoutId("avatar", container({ .width = StyleValue::point(80.0f), .height = StyleValue::point(80.0f) })),
                layoutId("badge", container({ .width = StyleValue::point(24.0f), .height = StyleValue::point(24.0f) })),
            },
        }),
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* root_ro = el->findRenderObject();
    root_ro->layout(300.0f, 200.0f);

    auto* layout_ro = root_ro->children()[0];
    auto* avatar_ro = layout_ro->children()[0];
    auto* badge_ro  = layout_ro->children()[1];

    std::cout << "  Avatar offset: (" << avatar_ro->offset().x << ", " << avatar_ro->offset().y << ")" << std::endl;
    assert(approxEqual(avatar_ro->offset().x, 50.0f));
    assert(approxEqual(avatar_ro->offset().y, 50.0f));
    assert(approxEqual(avatar_ro->size().width, 80.0f));
    assert(approxEqual(avatar_ro->size().height, 80.0f));

    std::cout << "  Badge offset: (" << badge_ro->offset().x << ", " << badge_ro->offset().y << ")" << std::endl;
    assert(approxEqual(badge_ro->offset().x, 50.0f + 80.0f - 12.0f)); // 118.0f
    assert(approxEqual(badge_ro->offset().y, 50.0f - 12.0f));          // 38.0f
    assert(approxEqual(badge_ro->size().width, 24.0f));
    assert(approxEqual(badge_ro->size().height, 24.0f));

    el->unmount();
    std::cout << "  ✓ Dependent layout test passed." << std::endl;
}

void test_custom_multi_child_layout_functional_lambda() {
    std::cout << "Testing CustomMultiChildLayout: Functional C++20 Lambda Delegate..." << std::endl;

    bool layout_invoked = false;

    WidgetPtr w = customMultiChildLayout({
        .layout_callback = [&layout_invoked](MultiChildLayoutDelegate& del, Size size) {
            layout_invoked = true;
            assert(del.hasChild("boxA"));
            Size s = del.layoutChild("boxA", BoxConstraints::tight(Size{120.0f, 80.0f}));
            assert(approxEqual(s.width, 120.0f));
            assert(approxEqual(s.height, 80.0f));
            del.positionChild("boxA", Point{45.0f, 65.0f});
        },
        .size_callback = [](const BoxConstraints& constraints) {
            return Size{350.0f, 250.0f};
        },
        .children = {
            layoutId("boxA", container({
                .color = 0xFF6366F1,
            })),
        },
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* layout_ro = el->findRenderObject();
    layout_ro->layout(500.0f, 500.0f);

    assert(layout_invoked);
    assert(approxEqual(layout_ro->size().width, 350.0f));
    assert(approxEqual(layout_ro->size().height, 250.0f));

    auto* boxA_ro = layout_ro->children()[0];
    assert(approxEqual(boxA_ro->offset().x, 45.0f));
    assert(approxEqual(boxA_ro->offset().y, 65.0f));
    assert(approxEqual(boxA_ro->size().width, 120.0f));
    assert(approxEqual(boxA_ro->size().height, 80.0f));

    el->unmount();
    std::cout << "  ✓ Functional lambda delegate test passed." << std::endl;
}

void test_custom_multi_child_layout_hit_test() {
    std::cout << "Testing CustomMultiChildLayout: Hit Testing with Child Offsets..." << std::endl;

    WidgetPtr w = customMultiChildLayout({
        .layout_callback = [](MultiChildLayoutDelegate& del, Size size) {
            del.layoutChild("btn1", BoxConstraints::tight(Size{100.0f, 50.0f}));
            del.positionChild("btn1", Point{100.0f, 100.0f});
        },
        .children = {
            layoutId("btn1", container({
                .color = 0xFF10B981,
                .width = StyleValue::point(100.0f),
                .height = StyleValue::point(50.0f),
            })),
        },
    });

    auto el = w->createElement();
    el->mount(nullptr, 0);
    el->rebuild();

    auto* layout_ro = el->findRenderObject();
    layout_ro->layout(400.0f, 400.0f);

    // Hit test outside btn1 bounds: Point{50, 50}
    HitTestResult res1;
    bool hit1 = layout_ro->hitTest(res1, Point{50.0f, 50.0f});
    (void)hit1;
    // btn1 is at (100, 100) to (200, 150), so Point{50, 50} does not hit btn1
    auto* btn1_ro = layout_ro->children()[0];
    bool btn1_in_res1 = false;
    for (const auto& entry : res1.path()) {
        if (entry.target == btn1_ro || entry.target == btn1_ro->children()[0]) {
            btn1_in_res1 = true;
            break;
        }
    }
    assert(!btn1_in_res1);

    // Hit test inside btn1 bounds: Point{120, 110}
    HitTestResult res2;
    bool hit2 = layout_ro->hitTest(res2, Point{120.0f, 110.0f});
    assert(hit2);
    bool btn1_in_res2 = false;
    for (const auto& entry : res2.path()) {
        if (entry.target == btn1_ro || entry.target == btn1_ro->children()[0]) {
            btn1_in_res2 = true;
            break;
        }
    }
    assert(btn1_in_res2);

    el->unmount();
    std::cout << "  ✓ Hit testing with child offsets test passed." << std::endl;
}

void test_custom_multi_child_layout_declarative_props() {
    std::cout << "Testing CustomMultiChildLayout: Declarative C++20 Syntax & Types..." << std::endl;

    LayoutIdProps id_props = {
        .key = Key::string("lid_key"),
        .id = "custom_id",
        .child = container({}),
    };
    WidgetPtr id_w = id_props;
    assert(id_w != nullptr);
    assert(id_w->typeName() == "LayoutId");

    auto lid_widget = std::dynamic_pointer_cast<LayoutIdWidget>(id_w);
    assert(lid_widget != nullptr);
    assert(lid_widget->id == "custom_id");

    CustomMultiChildLayoutProps cmcl_props = {
        .key = Key::string("cmcl_key"),
        .children = { id_w },
    };
    WidgetPtr cmcl_w = cmcl_props;
    assert(cmcl_w != nullptr);
    assert(cmcl_w->typeName() == "CustomMultiChildLayout");

    std::cout << "  ✓ Declarative syntax test passed." << std::endl;
}

int main() {
    std::cout << "================================================================" << std::endl;
    std::cout << "  ENKI Test Suite — CustomMultiChildLayout & Delegate Layout    " << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended                 " << std::endl;
    std::cout << "================================================================" << std::endl;

    test_custom_multi_child_layout_basic();
    test_custom_multi_child_layout_dependent_layout();
    test_custom_multi_child_layout_functional_lambda();
    test_custom_multi_child_layout_hit_test();
    test_custom_multi_child_layout_declarative_props();

    std::cout << "\n>>> ALL CUSTOM_MULTI_CHILD_LAYOUT UNIT TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
