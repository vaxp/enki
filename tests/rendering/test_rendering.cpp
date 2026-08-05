/// @file test_rendering.cpp
/// @brief Unit tests for Skia Rendering wrappers.

#include "enki/rendering/canvas.hpp"
#include "enki/rendering/path.hpp"
#include "enki/rendering/color.hpp"
#include <cassert>
#include <cstdio>
#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>

using namespace enki;

// Extern function to get the wrapper from Skia backend
namespace enki {
extern std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas);
}

void test_colors() {
    assert(Colors::White == 0xFFFFFFFF);
    assert(Colors::Black == 0xFF000000);
    assert(makeColorRGB(255, 0, 0) == Colors::Red);
    assert(makeColorARGB(128, 0, 255, 0) == 0x8000FF00);
    printf("  [PASS] Colors\n");
}

void test_path() {
    Path p;
    p.moveTo(0, 0);
    p.lineTo(100, 0);
    p.lineTo(100, 100);
    p.close();

    Rect bounds = p.getBounds();
    assert(bounds.width == 100 && bounds.height == 100);
    assert(bounds.x == 0 && bounds.y == 0);

    printf("  [PASS] Path\n");
}

void test_canvas() {
    // Create an offscreen Skia surface/bitmap
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas sk_canvas(bitmap);

    // Create wrapper
    auto canvas = createCanvasWrapper(&sk_canvas);
    assert(canvas != nullptr);

    // Test basic drawing commands (no crash expected)
    canvas->clear(Colors::White);

    Paint paint;
    paint.setColor(Colors::Red);
    canvas->drawRect(Rect{10, 10, 50, 50}, paint);

    Path p;
    p.addCircle(50, 50, 20);
    canvas->drawPath(p, paint);

    // Verify a pixel
    SkColor c = bitmap.getColor(15, 15);
    // SkColor is ARGB. Colors::Red is 0xFFFF0000
    assert(c == 0xFFFF0000);

    printf("  [PASS] Canvas drawing\n");
}

int main() {
    printf("=== Rendering Tests ===\n");
    test_colors();
    test_path();
    test_canvas();
    printf("=== All Rendering tests passed! ===\n");
    return 0;
}
