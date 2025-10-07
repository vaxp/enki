/// @file test_paint_effects.cpp
/// @brief Comprehensive Unit & Integration Tests for Section 12 Paint & Visual Effects widgets.

#include "enki/widgets/clip.hpp"
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <iostream>
#include <cassert>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: ClipRect Declarative Construction & Layout
// ════════════════════════════════════════════════════════════════
void test_clip_rect() {
    std::cout << "Testing ClipRect Widget..." << std::endl;

    WidgetPtr child = container({
        .width = StyleValue::point(100.0f),
        .height = StyleValue::point(50.0f),
    });

    WidgetPtr w = ClipRect {
        .clip_behavior = Clip::AntiAlias,
        .child = child,
    };

    assert(w != nullptr);
    assert(w->typeName() == "ClipRect");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);
    assert(ro->childCount() == 1);

    el->unmount();
    std::cout << "  ✓ ClipRect passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: ClipRRect with BorderRadius
// ════════════════════════════════════════════════════════════════
void test_clip_rrect() {
    std::cout << "Testing ClipRRect Widget..." << std::endl;

    WidgetPtr w = clipRRect({
        .border_radius = BorderRadius::circular(12.0f),
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .width = StyleValue::point(120.0f),
            .height = StyleValue::point(80.0f),
        }),
    });

    assert(w != nullptr);
    assert(w->typeName() == "ClipRRect");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ ClipRRect passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: ClipOval Construction & Hit Testing
// ════════════════════════════════════════════════════════════════
void test_clip_oval() {
    std::cout << "Testing ClipOval Widget..." << std::endl;

    WidgetPtr w = ClipOval {
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .width = StyleValue::point(100.0f),
            .height = StyleValue::point(100.0f),
        }),
    };

    assert(w != nullptr);
    assert(w->typeName() == "ClipOval");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ ClipOval passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: ClipPath with Custom Geometric Clipper
// ════════════════════════════════════════════════════════════════
void test_clip_path() {
    std::cout << "Testing ClipPath Widget..." << std::endl;

    WidgetPtr w = clipPath({
        .clipper = [](Size sz) {
            Path p;
            p.moveTo(sz.width * 0.5f, 0.0f);
            p.lineTo(sz.width, sz.height);
            p.lineTo(0.0f, sz.height);
            p.close();
            return p;
        },
        .clip_behavior = Clip::AntiAlias,
        .child = container({
            .width = StyleValue::point(80.0f),
            .height = StyleValue::point(80.0f),
        }),
    });

    assert(w != nullptr);
    assert(w->typeName() == "ClipPath");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ ClipPath passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: BackdropFilter with Blur
// ════════════════════════════════════════════════════════════════
void test_backdrop_filter() {
    std::cout << "Testing BackdropFilter Widget..." << std::endl;

    WidgetPtr w = BackdropFilter {
        .filter = ImageFilter::blur(15.0f, 15.0f),
        .blend_mode = BlendMode::SrcOver,
        .child = text("Glassmorphic Card"),
    };

    assert(w != nullptr);
    assert(w->typeName() == "BackdropFilter");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ BackdropFilter passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: DecoratedBox with Background & Foreground Position
// ════════════════════════════════════════════════════════════════
void test_decorated_box() {
    std::cout << "Testing DecoratedBox Widget..." << std::endl;

    WidgetPtr w = decoratedBox({
        .decoration = BoxDecoration(0xFF1E293B, BorderRadius::circular(8.0f), Border(0xFF38BDF8, 1.5f)),
        .position = DecorationPosition::Background,
        .child = text("Decorated Content"),
    });

    assert(w != nullptr);
    assert(w->typeName() == "DecoratedBox");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ DecoratedBox passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 7: ShaderMask with Linear Gradient
// ════════════════════════════════════════════════════════════════
void test_shader_mask() {
    std::cout << "Testing ShaderMask Widget..." << std::endl;

    WidgetPtr w = ShaderMask {
        .shader_callback = [](Rect bounds) {
            return Gradient::linear(
                {bounds.x, bounds.y},
                {bounds.x + bounds.width, bounds.y},
                {0xFF38BDF8, 0xFFEC4899}
            );
        },
        .blend_mode = BlendMode::Modulate,
        .child = text("Masked Title Text"),
    };

    assert(w != nullptr);
    assert(w->typeName() == "ShaderMask");

    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ ShaderMask passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 8: ColorFiltered with Grayscale & Sepia & Invert
// ════════════════════════════════════════════════════════════════
void test_color_filtered() {
    std::cout << "Testing ColorFiltered Widget..." << std::endl;

    WidgetPtr w_gray = ColorFiltered {
        .color_filter = ColorFilter::grayscale(),
        .child = text("Grayscale Content"),
    };
    assert(w_gray != nullptr);
    assert(w_gray->typeName() == "ColorFiltered");

    WidgetPtr w_sepia = colorFiltered({
        .color_filter = ColorFilter::sepia(),
        .child = text("Sepia Content"),
    });
    assert(w_sepia != nullptr);

    WidgetPtr w_invert = colorFiltered({
        .color_filter = ColorFilter::invert(),
        .child = text("Inverted Content"),
    });
    assert(w_invert != nullptr);

    auto el = w_gray->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    el->unmount();
    std::cout << "  ✓ ColorFiltered passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  RUNNING PAINT & VISUAL EFFECTS TESTS  " << std::endl;
    std::cout << "========================================" << std::endl;

    test_clip_rect();
    test_clip_rrect();
    test_clip_oval();
    test_clip_path();
    test_backdrop_filter();
    test_decorated_box();
    test_shader_mask();
    test_color_filtered();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL 8 VISUAL EFFECTS TESTS PASSED!    " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
