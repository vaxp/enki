/// @file test_text.cpp
/// @brief Comprehensive Unit & Integration Tests for Enki Text & RichText Widgets (SkParagraph + Anu).

#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

static bool approxEqual(float a, float b, float epsilon = 1.0f) {
    return std::fabs(a - b) <= epsilon;
}

// ════════════════════════════════════════════════════════════════
// Test 1: Basic Text Intrinsic Measurement with Anu Layout
// ════════════════════════════════════════════════════════════════
void test_text_basic_measurement() {
    std::cout << "Testing Text Intrinsic Measurement..." << std::endl;

    auto t = text({
        .text = "Hello ENKI Shell Typography",
        .color = 0xFFFFFFFF,
        .font_size = 16.0f,
    });

    auto c = container(t);
    auto el = c->createElement();
    el->mount(nullptr, 0);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);

    // Layout unconstrained
    rdb->layout(800.0f, 600.0f);

    auto* rp = dynamic_cast<RenderParagraph*>(rdb->children()[0]);
    assert(rp != nullptr);

    std::cout << "  Measured size: " << rp->size().width << " x " << rp->size().height << std::endl;
    assert(rp->size().width > 100.0f); // Text should have a positive measured width
    assert(rp->size().height >= 14.0f); // Height should reflect line height of ~16px font

    el->unmount();
    std::cout << "✓ Text Intrinsic Measurement Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Multiline Text Wrapping under Constraints
// ════════════════════════════════════════════════════════════════
void test_text_multiline_wrapping() {
    std::cout << "Testing Multiline Text Wrapping..." << std::endl;

    std::string longText = "This is a very long text paragraph designed to test automatic line wrapping across multiple lines when constrained by narrow parent width.";
    auto t1 = text({
        .text = longText,
        .font_size = 14.0f,
    });
    auto c1 = container(t1);

    auto el1 = c1->createElement();
    el1->mount(nullptr, 0);

    auto* rdb1 = dynamic_cast<RenderDecoratedBox*>(el1->findRenderObject());
    assert(rdb1 != nullptr);

    // Measure unconstrained
    rdb1->layout(1000.0f, 1000.0f);
    auto* rp1 = dynamic_cast<RenderParagraph*>(rdb1->children()[0]);
    float singleLineHeight = rp1->size().height;
    float singleLineWidth = rp1->size().width;

    // Now constrain parent width to 150px
    auto t2 = text({
        .text = longText,
        .font_size = 14.0f,
    });
    auto c2 = container(t2);
    c2->width(150.0f);

    auto el2 = c2->createElement();
    el2->mount(nullptr, 0);

    auto* rdb2 = dynamic_cast<RenderDecoratedBox*>(el2->findRenderObject());
    rdb2->layout(150.0f, 1000.0f);
    auto* rp2 = dynamic_cast<RenderParagraph*>(rdb2->children()[0]);

    std::cout << "  Unconstrained: " << singleLineWidth << "x" << singleLineHeight 
              << ", Wrapped in 150px: " << rp2->size().width << "x" << rp2->size().height << std::endl;

    assert(rp2->size().width <= 150.5f);
    assert(rp2->size().height > singleLineHeight * 1.5f); // Should wrap into multiple lines!

    el1->unmount();
    el2->unmount();
    std::cout << "✓ Multiline Text Wrapping Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: Text Overflow & Max Lines
// ════════════════════════════════════════════════════════════════
void test_text_overflow_max_lines() {
    std::cout << "Testing Text Overflow & MaxLines..." << std::endl;

    std::string longText = "First line of text that continues to second and third line when wrapped inside small container";
    auto t = text({
        .text = longText,
        .font_size = 14.0f,
        .overflow = TextOverflow::Ellipsis,
        .max_lines = 1,
    });

    auto c = container(t);
    c->width(100.0f);

    auto el = c->createElement();
    el->mount(nullptr, 0);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);

    rdb->layout(100.0f, 1000.0f);
    auto* rp = dynamic_cast<RenderParagraph*>(rdb->children()[0]);

    // Height should be clamped to 1 line
    std::cout << "  Single line clamped height: " << rp->size().height << std::endl;
    assert(rp->size().height < 30.0f);

    el->unmount();
    std::cout << "✓ Text Overflow & MaxLines Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: RichText with Mixed Spans and Styles
// ════════════════════════════════════════════════════════════════
void test_richtext_spans() {
    std::cout << "Testing RichText Spans..." << std::endl;

    auto rootSpan = span("Welcome to ", TextStyle{ .color = 0xFF888888, .font_size = 14.0f }, {
        span("ENKI ", TextStyle{ .color = 0xFF3B82F6, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
        span("Desktop Shell", TextStyle{ .color = 0xFF10B981, .font_size = 14.0f, .font_style = FontStyle::Italic })
    });

    auto rt = richText(rootSpan);
    auto c = container(rt);
    auto el = c->createElement();
    el->mount(nullptr, 0);

    auto* rdb = dynamic_cast<RenderDecoratedBox*>(el->findRenderObject());
    assert(rdb != nullptr);

    rdb->layout(600.0f, 400.0f);
    auto* rp = dynamic_cast<RenderParagraph*>(rdb->children()[0]);

    std::cout << "  RichText measured size: " << rp->size().width << " x " << rp->size().height << std::endl;
    assert(rp->size().width > 120.0f);
    assert(rp->size().height >= 14.0f);

    el->unmount();
    std::cout << "✓ RichText Spans Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: Text Inside Flexbox Tree (Row & Column)
// ════════════════════════════════════════════════════════════════
void test_text_flexbox_integration() {
    std::cout << "Testing Text Inside Flexbox Tree..." << std::endl;

    auto title = text({
        .text = "Title Text",
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold,
    });

    auto subtitle = text({
        .text = "Subtitle Description",
        .color = 0xFFAAAAAA,
        .font_size = 12.0f,
    });

    std::vector<WidgetPtr> colChildren = {title, subtitle};
    auto card = container(column({
        .children = std::move(colChildren)
    }));
    card->padding(EdgeInsets::all(16.0f));

    auto el = card->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    ro->layout(400.0f, 300.0f);

    std::cout << "  Flexbox Text Card size: " << ro->size().width << " x " << ro->size().height << std::endl;
    assert(ro->size().height > 40.0f); // 16 top + 16 bottom + title height + subtitle height

    el->unmount();
    std::cout << "✓ Text Inside Flexbox Tree Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: Paint Rendering to Skia Canvas Surface
// ════════════════════════════════════════════════════════════════
void test_text_paint_rendering() {
    std::cout << "Testing Text Paint on Skia Canvas..." << std::endl;

    auto skSurface = SkSurface::MakeRasterN32Premul(400, 200);
    assert(skSurface != nullptr);

    auto wrapper = createCanvasWrapper(skSurface->getCanvas());
    PaintContext ctx(*wrapper);

    auto t = text({
        .text = "Renderable Text",
        .color = 0xFF00FF00,
        .font_size = 20.0f,
        .shadows = { BoxShadow(0x80000000, {0, 2}, 4.0f) },
    });

    auto el = t->createElement();
    el->mount(nullptr, 0);

    auto* rp = dynamic_cast<RenderParagraph*>(el->findRenderObject());
    assert(rp != nullptr);

    rp->layout(400.0f, 200.0f);
    rp->paint(ctx);

    el->unmount();
    std::cout << "✓ Text Paint on Skia Canvas Passed!" << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 7: Text Reconciler Update
// ════════════════════════════════════════════════════════════════
void test_text_reconcile_update() {
    std::cout << "Testing Text Reconcile Update..." << std::endl;

    auto t1 = text("Initial Short");
    auto r1 = row({
        .children = { t1 }
    });
    auto el1 = r1->createElement();
    el1->mount(nullptr, 0);

    auto* ro1 = el1->findRenderObject();
    ro1->layout(800.0f, 600.0f);
    auto* rp1 = dynamic_cast<RenderParagraph*>(ro1->children()[0]);
    float w1 = rp1->size().width;

    auto t2 = text("A Much Much Longer String That Replaces Initial");
    BuildContext ctx(el1.get());
    t2->updateRenderObject(ctx, *rp1);
    ro1->layout(800.0f, 600.0f);
    float w2 = rp1->size().width;

    std::cout << "  Before update width: " << w1 << ", After update width: " << w2 << std::endl;
    assert(w2 > w1 * 1.5f);

    el1->unmount();
    std::cout << "✓ Text Reconcile Update Passed!" << std::endl;
}

#include <include/core/SkImage.h>
#include <include/core/SkStream.h>
#include <include/core/SkData.h>
#include <include/encode/SkPngEncoder.h>

// ════════════════════════════════════════════════════════════════
// Test 8: Render Full Showcase Snapshot to PNG
// ════════════════════════════════════════════════════════════════
void test_text_render_snapshot_png() {
    std::cout << "Testing Text Snapshot to PNG..." << std::endl;

    int width = 1080;
    int height = 900;
    auto skSurface = SkSurface::MakeRasterN32Premul(width, height);
    assert(skSurface != nullptr);

    auto* canvas = skSurface->getCanvas();
    canvas->clear(0xFF0B0F19); // Dark navy background

    auto wrapper = createCanvasWrapper(canvas);
    PaintContext ctx(*wrapper);

    // Build rich hierarchy showcase
    auto title = text({
        .text = "⚡ ENKI ENGINE — TYPOGRAPHY & SKPARAGRAPH DEMO",
        .color = 0xFFFFFFFF,
        .font_size = 22.0f,
        .font_weight = FontWeight::Bold,
        .shadows = { BoxShadow(0x8038BDF8, {0, 0}, 8.0f) },
    });

    auto sub = text({
        .text = "SkParagraph Text Layout + HarfBuzz Font Shaping + Anu Flexbox Integration",
        .color = 0xFF818CF8,
        .font_size = 12.0f,
    });

    auto dispL = text({
        .text = "Display Large — 28px Bold Glow Headline",
        .color = 0xFF38BDF8,
        .font_size = 28.0f,
        .font_weight = FontWeight::Bold,
        .shadows = { BoxShadow(0x8038BDF8, {0, 0}, 12.0f) },
    });

    auto richDemoSpan = span("ENKI Framework delivers ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 15.0f }, {
        span("blazing-fast ", TextStyle{ .color = 0xFF38BDF8, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
        span("desktop shell typography. Combining ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 15.0f }),
        span("Skia GPU Rasterization ", TextStyle{ .color = 0xFF34D399, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
        span("with ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 15.0f }),
        span("Anu Flexbox Layout ", TextStyle{ .color = 0xFFF59E0B, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
        span("and ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 15.0f }),
        span("HarfBuzz text shaping!", TextStyle{ .color = 0xFFEC4899, .font_size = 15.0f, .font_weight = FontWeight::Bold, .font_style = FontStyle::Italic })
    });
    auto richDemo = richText(richDemoSpan);

    auto arabicDemo = text({
        .text = "محرك رسم فائق السرعة يدعم اللغة العربية والخطوط المخصصة بالكامل",
        .color = 0xFF34D399,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto codeSpan = span("", TextStyle{ .color = 0xFFE2E8F0, .font_size = 13.0f }, {
        span("// Syntax Highlighting with SkParagraph Spans\n", TextStyle{ .color = 0xFF64748B, .font_style = FontStyle::Italic }),
        span("auto ", TextStyle{ .color = 0xFF818CF8, .font_weight = FontWeight::Bold }),
        span("label ", TextStyle{ .color = 0xFFFBBF24, .font_weight = FontWeight::Bold }),
        span("= ", TextStyle{ .color = 0xFF94A3B8 }),
        span("text", TextStyle{ .color = 0xFF38BDF8 }),
        span("({", TextStyle{ .color = 0xFFE2E8F0 }),
        span(".text = \"Hello World!\", ", TextStyle{ .color = 0xFF34D399 }),
        span(".font_size = 18.0f", TextStyle{ .color = 0xFFA78BFA }),
        span("});", TextStyle{ .color = 0xFFE2E8F0 })
    });
    auto codeDemo = richText(codeSpan);

    auto codeBox = container(codeDemo);
    codeBox->color(0xFF0F172A).borderRadius(10.0f).border(0xFF1E293B, 1.0f).paddingAll(12.0f);

    auto mainCol = column({
        .children = {
            title,
            sub,
            dispL,
            richDemo,
            arabicDemo,
            codeBox
        }
    });

    auto root = container(mainCol);
    root->color(0xFF0B0F19)
        .paddingAll(24.0f);

    auto el = root->createElement();
    el->mount(nullptr, 0);

    auto* ro = el->findRenderObject();
    ro->layout(width, height);
    ro->paint(ctx);

    // Save PNG snapshot
    SkPixmap pixmap;
    if (skSurface->peekPixels(&pixmap)) {
        std::string path = "/tmp/text_demo_snapshot.png";
        SkFILEWStream stream(path.c_str());
        SkPngEncoder::Options opts;
        bool ok = SkPngEncoder::Encode(&stream, pixmap, opts);
        std::cout << "  Snapshot written to " << path << " (Success: " << (ok ? "YES" : "NO") << ")" << std::endl;
        assert(ok);
    }

    el->unmount();
    std::cout << "✓ Text Snapshot to PNG Passed!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ENKI Text & RichText Widgets Test Suite " << std::endl;
    std::cout << "========================================" << std::endl;

    test_text_basic_measurement();
    test_text_multiline_wrapping();
    test_text_overflow_max_lines();
    test_richtext_spans();
    test_text_flexbox_integration();
    test_text_paint_rendering();
    test_text_reconcile_update();
    test_text_render_snapshot_png();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL 8 TEXT WIDGET TESTS PASSED!       " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
