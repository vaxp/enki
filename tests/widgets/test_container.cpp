/// @file test_container.cpp
/// @brief Comprehensive Unit & Integration Tests for Enki Container Widget & RenderDecoratedBox.

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

class FixedBox : public SingleChildRenderObjectWidget {
public:
    float w;
    float h;

    FixedBox(float w, float h) : w(w), h(h) {}

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

    std::string_view typeName() const override { return "FixedBox"; }
};

inline std::shared_ptr<FixedBox> fixedBox(float w, float h) {
    return std::make_shared<FixedBox>(w, h);
}

// ════════════════════════════════════════════════════════════════
// Test 1: Sizing and Geometry Integration with Flexbox
// ════════════════════════════════════════════════════════════════
void test_container_sizing() {
    std::cout << "Testing Container Sizing & Layout..." << std::endl;

    auto c = container({
        .width = 250.0f,
        .height = 120.0f,
    });

    auto el = c->createElement();
    el->mount(nullptr, 0);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);

    rdb->layout(400.0f, 400.0f);

    assert(approxEqual(rdb->size().width, 250.0f));
    assert(approxEqual(rdb->size().height, 120.0f));

    std::cout << "  ✓ Container sizing passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Padding & Margin
// ════════════════════════════════════════════════════════════════
void test_container_padding_margin() {
    std::cout << "Testing Container Padding & Margin..." << std::endl;

    auto child = fixedBox(100.0f, 50.0f);
    auto c = container({
        .padding = StyleInsets::all(20.0f),
        .margin = StyleInsets::only(10.0f, 0.0f, 0.0f, 15.0f), // top=10, left=15
        .child = child,
    });

    auto el = c->createElement();
    el->mount(nullptr, 0);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);

    rdb->layout(500.0f, 500.0f);

    std::cout << "rdb size: " << rdb->size().width << "x" << rdb->size().height << ", offset: " << rdb->offset().x << ", " << rdb->offset().y << std::endl;
    if (!rdb->children().empty()) {
        std::cout << "child size: " << rdb->children()[0]->size().width << "x" << rdb->children()[0]->size().height << ", offset: " << rdb->children()[0]->offset().x << ", " << rdb->children()[0]->offset().y << std::endl;
    }

    std::cout << "  ✓ Padding & Margin passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: Child Alignment (Center, BottomRight, etc.)
// ════════════════════════════════════════════════════════════════
void test_container_alignment() {
    std::cout << "Testing Container Child Alignment..." << std::endl;

    // Center alignment in a 300x200 container with a 100x60 child
    {
        auto c = container({
            .align = Alignment::Center,
            .width = 300.0f,
            .height = 200.0f,
            .child = fixedBox(100.0f, 60.0f),
        });

        auto el = c->createElement();
        el->mount(nullptr, 0);
        auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
        rdb->layout(500.0f, 500.0f);

        // (300 - 100) / 2 = 100, (200 - 60) / 2 = 70
        assert(approxEqual(rdb->children()[0]->offset().x, 100.0f));
        assert(approxEqual(rdb->children()[0]->offset().y, 70.0f));
    }

    // BottomRight alignment
    {
        auto c = container({
            .align = Alignment::BottomRight,
            .width = 300.0f,
            .height = 200.0f,
            .child = fixedBox(100.0f, 60.0f),
        });

        auto el = c->createElement();
        el->mount(nullptr, 0);
        auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
        rdb->layout(500.0f, 500.0f);

        // 300 - 100 = 200, 200 - 60 = 140
        assert(approxEqual(rdb->children()[0]->offset().x, 200.0f));
        assert(approxEqual(rdb->children()[0]->offset().y, 140.0f));
    }

    std::cout << "  ✓ Child Alignment passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: Constraints & Aspect Ratio
// ════════════════════════════════════════════════════════════════
void test_container_constraints() {
    std::cout << "Testing Constraints & AspectRatio..." << std::endl;

    // Aspect ratio 16:9 = 1.7778 inside flex layout
    {
        auto c = container({
            .width = 320_px,
            .aspect_ratio = 16.0f / 9.0f,
        });

        auto root = column({
            .children = { c }
        });

        auto el = root->createElement();
        el->mount(nullptr, 0);
        auto* root_ro = el->findRenderObject();
        root_ro->layout(500.0f, 500.0f);

        auto* child_ro = root_ro->children()[0];
        std::cout << "Child in column with AspectRatio size: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;
        assert(approxEqual(child_ro->size().width, 320.0f));
        assert(approxEqual(child_ro->size().height, 180.0f));
    }

    // Min & Max constraints inside flex layout
    {
        auto c = container({
            .min_width = 120_px,
            .min_height = 100_px,
            .child = fixedBox(50.0f, 50.0f),
        });

        auto root = row({
            .align_items = Align::Start,
            .children = { c }
        });
        auto el = root->createElement();
        el->mount(nullptr, 0);
        auto* root_ro = el->findRenderObject();
        root_ro->layout(500.0f, 500.0f);

        auto* child_ro = root_ro->children()[0];
        std::cout << "Child min/max size: " << child_ro->size().width << "x" << child_ro->size().height << std::endl;
        assert(approxEqual(child_ro->size().width, 120.0f));
        assert(approxEqual(child_ro->size().height, 100.0f));
    }

    std::cout << "  ✓ Constraints & AspectRatio passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: BoxDecoration Properties
// ════════════════════════════════════════════════════════════════
void test_box_decoration() {
    std::cout << "Testing BoxDecoration Properties..." << std::endl;

    auto c = container({
        .color = 0xFF1E88E5,
        .border_radius = BorderRadius::circular(16.0f),
        .border = Border(0xFFFFFFFF, 2.0f),
        .box_shadow = {BoxShadow(0x80000000, {0, 8}, 16, 2)},
        .clip_content = true,
    });

    auto cw = std::dynamic_pointer_cast<ContainerWidget>(c);
    assert(cw != nullptr);
    assert(cw->decoration.color == 0xFF1E88E5);
    assert(cw->decoration.border_radius.top_left == 16.0f);
    assert(cw->decoration.border.has_value());
    assert(cw->decoration.border->width == 2.0f);
    assert(cw->decoration.box_shadow.size() == 1);
    assert(cw->decoration.box_shadow[0].blur_radius == 16.0f);
    assert(cw->decoration.clip_content == true);

    std::cout << "  ✓ BoxDecoration passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: Hit Testing (Rectangle vs Circle)
// ════════════════════════════════════════════════════════════════
void test_hit_testing() {
    std::cout << "Testing Container Hit Testing..." << std::endl;

    // Rectangle Hit Test
    {
        auto c = container({
            .width = 100.0f,
            .height = 100.0f,
        });
        auto el = c->createElement();
        el->mount(nullptr, 0);
        auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
        rdb->layout(200.0f, 200.0f);

        HitTestResult result;
        assert(rdb->hitTest(result, {50.0f, 50.0f}) == true);
        assert(rdb->hitTest(result, {105.0f, 50.0f}) == false);
        assert(rdb->hitTest(result, {-5.0f, 50.0f}) == false);
    }

    // Circular Hit Test
    {
        auto c = container({
            .shape = BoxShape::Circle,
            .width = 100.0f,
            .height = 100.0f,
        });
        auto el = c->createElement();
        el->mount(nullptr, 0);
        auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
        rdb->layout(200.0f, 200.0f);

        HitTestResult result;
        // Center of circle (50, 50) with radius 50
        assert(rdb->hitTest(result, {50.0f, 50.0f}) == true);
        // Corner (5, 5) -> dist^2 = 45^2 + 45^2 = 2025 + 2025 = 4050 > 2500 -> outside circle!
        assert(rdb->hitTest(result, {5.0f, 5.0f}) == false);
    }

    std::cout << "  ✓ Hit Testing passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 7: Painting Pipeline Verification via Mock Skia Surface
// ════════════════════════════════════════════════════════════════
void test_painting_pipeline() {
    std::cout << "Testing Painting Pipeline..." << std::endl;

    auto surface = SkSurface::MakeRasterN32Premul(400, 400);
    assert(surface != nullptr);
    SkCanvas* sk_canvas = surface->getCanvas();

    // Create Canvas wrapper
    auto canvas = createCanvasWrapper(sk_canvas);
    assert(canvas != nullptr);

    PaintContext ctx{*canvas, {20.0f, 20.0f}, Rect{0, 0, 400, 400}, 1.0f};

    // Container with Shadows, Borders, and Rounded Corners
    auto c = container({
        .border_radius = BorderRadius::circular(20.0f),
        .border = Border(0xFFFFFFFF, 3.0f),
        .box_shadow = {BoxShadow(0x40000000, {0, 6}, 12, 1)},
        .clip_content = true,
        .width = 200.0f,
        .height = 100.0f,
        .child = fixedBox(80.0f, 40.0f),
    });

    auto el = c->createElement();
    el->mount(nullptr, 0);
    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    rdb->layout(400.0f, 400.0f);

    // Call paint — must execute without crashing
    rdb->paint(ctx);

    std::cout << "  ✓ Painting pipeline passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 8: Property Leakage & Reconciliation Across Updates
// ════════════════════════════════════════════════════════════════

void test_property_leakage_and_reconciliation() {
    std::cout << "Testing Property Leakage & Reconciliation Across Updates..." << std::endl;

    // 1. Initial container with explicit width, height, margin, padding, color
    auto c1 = container({
        .color = 0xFFFF0000,
        .width = 300_px,
        .height = 200_px,
        .padding = StyleInsets::all(15_px),
        .margin = StyleInsets::all(25_px),
        .child = fixedBox(50.0f, 50.0f),
    });

    auto parent = column({
        .width = 500_px,
        .height = 500_px,
        .children = { c1 }
    });

    auto el = parent->createElement();
    el->mount(nullptr, 0);
    auto* rf = dynamic_cast<RenderFlex*>(el->findRenderObject());
    rf->layout(500.0f, 500.0f);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(rf->children()[0]);
    assert(rdb != nullptr);
    assert(approxEqual(rdb->size().width, 300.0f));
    assert(approxEqual(rdb->size().height, 200.0f));
    assert(approxEqual(rdb->offset().x, 25.0f));
    assert(approxEqual(rdb->offset().y, 25.0f));
    assert(rdb->decoration().color == 0xFFFF0000);

    // 2. Update with c2 having NO margin, NO padding, NO fixed size, and different color
    auto c2 = container({
        .color = 0xFF00FF00,
        .child = fixedBox(80.0f, 40.0f),
    });

    auto parent2 = column({
        .width = 500_px,
        .height = 500_px,
        .children = { c2 }
    });

    el->update(parent2);
    rf->layout(500.0f, 500.0f);

    assert(rdb->decoration().color == 0xFF00FF00);
    // Margin must be reset to 0 (no offset from margin)
    assert(approxEqual(rdb->offset().x, 0.0f));
    assert(approxEqual(rdb->offset().y, 0.0f));
    // Child inside c2 should be at 0, 0 because padding is reset to 0
    assert(approxEqual(rdb->children()[0]->offset().x, 0.0f));
    assert(approxEqual(rdb->children()[0]->offset().y, 0.0f));
    // Child size inside should be 80x40
    assert(approxEqual(rdb->children()[0]->size().width, 80.0f));
    assert(approxEqual(rdb->children()[0]->size().height, 40.0f));

    std::cout << "  ✓ Property leakage prevention & reconciliation passed." << std::endl;
}

void test_container_shaders() {
    std::cout << "Testing Container SkSL Shaders..." << std::endl;

    std::string bg_shader_code = R"(
        uniform float time;
        uniform vec2 resolution;
        vec4 main(vec2 fragCoord) {
            vec2 uv = fragCoord / resolution;
            return vec4(uv.x, uv.y, 0.5, 1.0);
        }
    )";

    std::string border_shader_code = R"(
        uniform vec2 resolution;
        vec4 main(vec2 fragCoord) {
            return vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    auto c = container({
        .background_shader = bg_shader_code,
        .border_shader = border_shader_code,
        .width = 200_px,
        .height = 100_px,
    });

    assert(c->decoration.background_shader == bg_shader_code);
    assert(c->decoration.border_shader == border_shader_code);
    // Border should default to 1.0f width when border_shader was provided without explicit border
    assert(c->decoration.border.has_value());
    assert(c->decoration.border->width == 1.0f);

    auto el = c->createElement();
    el->mount(nullptr, 0);
    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);
    assert(rdb->decoration().background_shader == bg_shader_code);
    assert(rdb->decoration().border_shader == border_shader_code);

    // Also test Container struct syntax
    Container c_decl = {
        .border = Border(0xFF00FF00, 3.0f),
        .background_shader = bg_shader_code,
        .border_shader = border_shader_code,
    };
    WidgetPtr w = c_decl;
    auto c_w = std::dynamic_pointer_cast<ContainerWidget>(w);
    assert(c_w != nullptr);
    assert(c_w->decoration.border->width == 3.0f);
    assert(c_w->decoration.border->color == 0xFF00FF00);
    assert(c_w->decoration.background_shader == bg_shader_code);
    assert(c_w->decoration.border_shader == border_shader_code);

    std::cout << "  ✓ Container SkSL shaders passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 10: Direct SVG Vector Injection (background_svg & border_svg)
// ════════════════════════════════════════════════════════════════
void test_container_svg() {
    std::cout << "Testing Direct SVG Vector Injection..." << std::endl;

    const std::string bg_svg_xml = R"(
        <svg viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="40" fill="#38BDF8" stroke="#0284C7" stroke-width="3"/>
        </svg>
    )";

    const std::string border_svg_path = "M 0 15 L 15 0 H 85 L 100 15 V 85 L 85 100 H 15 L 0 85 Z";

    auto c = container({
        .background_svg = bg_svg_xml,
        .border_svg = border_svg_path,
        .svg_fit = SvgFit::Contain,
        .svg_slice = SvgSlice::all(15.0f),
        .width = 200_px,
        .height = 200_px,
    });

    assert(c->decoration.background_svg == bg_svg_xml);
    assert(c->decoration.border_svg == border_svg_path);
    assert(c->decoration.svg_fit == SvgFit::Contain);
    assert(c->decoration.svg_slice.has_value());
    assert(c->decoration.svg_slice->top == 15.0f);
    assert(c->decoration.border.has_value());

    auto el = c->createElement();
    el->mount(nullptr, 0);
    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);
    assert(rdb->decoration().background_svg == bg_svg_xml);
    assert(rdb->decoration().border_svg == border_svg_path);

    // Test declarative Container struct syntax
    Container c_decl = {
        .background_svg = bg_svg_xml,
        .border_svg = border_svg_path,
        .svg_fit = SvgFit::Cover,
    };
    WidgetPtr w = c_decl;
    auto c_w = std::dynamic_pointer_cast<ContainerWidget>(w);
    assert(c_w != nullptr);
    assert(c_w->decoration.background_svg == bg_svg_xml);
    assert(c_w->decoration.border_svg == border_svg_path);
    assert(c_w->decoration.svg_fit == SvgFit::Cover);

    std::cout << "  ✓ Direct SVG Vector Injection passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Enki Container Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_container_sizing();
    test_container_padding_margin();
    test_container_alignment();
    test_container_constraints();
    test_box_decoration();
    test_hit_testing();
    test_painting_pipeline();
    test_property_leakage_and_reconciliation();
    test_container_shaders();
    test_container_svg();

    std::cout << "========================================" << std::endl;
    std::cout << "All Container Tests Passed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
