/// @file test_image.cpp
/// @brief Comprehensive unit tests for ImageWidget, RenderImage, BoxFit, and ImageCache.

#include "enki/widgets/image.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace enki;

static bool approxEqual(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

// ════════════════════════════════════════════════════════════════
// 1. ImageCache & File Loading Tests
// ════════════════════════════════════════════════════════════════

void test_image_cache() {
    std::cout << "[Test 1] ImageCache & File Decoding...\n";

    ImageCache::clear();
    assert(ImageCache::count() == 0);

    // Test loading real asset
    auto img1 = ImageCache::getOrLoad("assets/0.png");
    assert(img1 != nullptr);
    assert(img1->getWidth() > 0);
    assert(img1->getHeight() > 0);
    assert(ImageCache::count() == 1);

    // Test cache hit
    auto img2 = ImageCache::getOrLoad("assets/0.png");
    assert(img2 == img1); // Same pointer
    assert(ImageCache::count() == 1);

    // Test another asset
    auto img_vaxp = ImageCache::getOrLoad("assets/vaxp.png");
    assert(img_vaxp != nullptr);
    assert(ImageCache::count() == 2);

    // Test clear
    ImageCache::clear();
    assert(ImageCache::count() == 0);

    std::cout << "  ✓ ImageCache loading, caching and hit-rate passed.\n";
}

// ════════════════════════════════════════════════════════════════
// 2. BoxFit Math & Geometry Calculation Tests
// ════════════════════════════════════════════════════════════════

void test_box_fit_math() {
    std::cout << "[Test 2] BoxFit Geometry Calculations...\n";

    Size src_size{100.0f, 50.0f}; // 2:1 aspect ratio
    Size dst_size{200.0f, 200.0f}; // 1:1 destination box
    Rect src, dst;

    // 1. BoxFit::Fill
    RenderImage::calculateBoxFitGeometry(BoxFit::Fill, Alignment::Center, src_size, dst_size, src, dst);
    assert(approxEqual(src.width, 100.0f) && approxEqual(src.height, 50.0f));
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 0.0f));
    assert(approxEqual(dst.width, 200.0f) && approxEqual(dst.height, 200.0f));

    // 2. BoxFit::Contain (letterboxed top/bottom)
    RenderImage::calculateBoxFitGeometry(BoxFit::Contain, Alignment::Center, src_size, dst_size, src, dst);
    assert(approxEqual(src.width, 100.0f) && approxEqual(src.height, 50.0f));
    assert(approxEqual(dst.width, 200.0f) && approxEqual(dst.height, 100.0f));
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 50.0f)); // Centered vertically

    // 3. BoxFit::Cover (cropped left/right)
    RenderImage::calculateBoxFitGeometry(BoxFit::Cover, Alignment::Center, src_size, dst_size, src, dst);
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 0.0f));
    assert(approxEqual(dst.width, 200.0f) && approxEqual(dst.height, 200.0f));
    assert(approxEqual(src.width, 50.0f) && approxEqual(src.height, 50.0f)); // 1:1 cropped
    assert(approxEqual(src.x, 25.0f) && approxEqual(src.y, 0.0f)); // Centered horizontally in source

    // 4. BoxFit::FitWidth
    RenderImage::calculateBoxFitGeometry(BoxFit::FitWidth, Alignment::TopLeft, src_size, dst_size, src, dst);
    assert(approxEqual(dst.width, 200.0f) && approxEqual(dst.height, 100.0f));
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 0.0f));

    // 5. BoxFit::FitHeight
    RenderImage::calculateBoxFitGeometry(BoxFit::FitHeight, Alignment::Center, src_size, dst_size, src, dst);
    assert(approxEqual(dst.width, 200.0f) && approxEqual(dst.height, 200.0f));
    assert(approxEqual(src.width, 50.0f) && approxEqual(src.height, 50.0f));

    // 6. BoxFit::None (1:1 scale centered)
    RenderImage::calculateBoxFitGeometry(BoxFit::None, Alignment::Center, src_size, dst_size, src, dst);
    assert(approxEqual(dst.width, 100.0f) && approxEqual(dst.height, 50.0f));
    assert(approxEqual(dst.x, 50.0f) && approxEqual(dst.y, 75.0f));

    std::cout << "  ✓ All BoxFit transformation modes verified.\n";
}

// ════════════════════════════════════════════════════════════════
// 3. Alignment Offsets Test
// ════════════════════════════════════════════════════════════════

void test_box_fit_alignment() {
    std::cout << "[Test 3] Image Alignment Anchoring...\n";

    Size src_size{100.0f, 50.0f};
    Size dst_size{200.0f, 200.0f};
    Rect src, dst;

    // TopLeft Contain
    RenderImage::calculateBoxFitGeometry(BoxFit::Contain, Alignment::TopLeft, src_size, dst_size, src, dst);
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 0.0f));

    // BottomRight Contain
    RenderImage::calculateBoxFitGeometry(BoxFit::Contain, Alignment::BottomRight, src_size, dst_size, src, dst);
    assert(approxEqual(dst.x, 0.0f) && approxEqual(dst.y, 100.0f));

    // TopRight Cover
    RenderImage::calculateBoxFitGeometry(BoxFit::Cover, Alignment::TopRight, src_size, dst_size, src, dst);
    assert(approxEqual(src.x, 50.0f) && approxEqual(src.y, 0.0f));

    std::cout << "  ✓ Alignment anchors (TopLeft, BottomRight, Center) verified.\n";
}

// ════════════════════════════════════════════════════════════════
// 4. RenderImage & Anu Layout Resolution Test
// ════════════════════════════════════════════════════════════════

void test_render_image_layout() {
    std::cout << "[Test 4] RenderImage Layout & Intrinsic Measurement...\n";

    auto img = ImageCache::getOrLoad("assets/0.png");
    assert(img != nullptr);

    ImageStyle style;
    style.image = img;
    style.width = StyleValue::point(300.0f);
    style.height = StyleValue::point(150.0f);

    RenderImage render_img(style);
    auto* node = render_img.getAnuNode();
    assert(node != nullptr);

    ANUNodeCalculateLayout(node, 300.0f, 150.0f, ANUDirectionLTR);

    assert(approxEqual(ANUNodeLayoutGetWidth(node), 300.0f));
    assert(approxEqual(ANUNodeLayoutGetHeight(node), 150.0f));

    std::cout << "  ✓ RenderImage flexbox layout measurement verified.\n";
}

// ════════════════════════════════════════════════════════════════
// 5. ImageWidget Declarative API Test
// ════════════════════════════════════════════════════════════════

void test_image_widget_builder() {
    std::cout << "[Test 5] ImageWidget Fluent Builder & Element Lifecycle...\n";

    auto w = imageAsset("assets/vaxp.png");
    w->fit(BoxFit::Cover)
     .alignment(Alignment::Center)
     .size(120.0f, 120.0f)
     .borderRadius(16.0f)
     .circle()
     .opacity(0.85f)
     .tint(0xFF00E5FF, BlendMode::SrcIn);

    assert(w->style.fit == BoxFit::Cover);
    assert(w->style.alignment == Alignment::Center);
    assert(w->style.shape == BoxShape::Circle);
    assert(approxEqual(w->style.opacity, 0.85f));
    assert(w->style.tint_color.has_value());
    assert(w->style.tint_color.value() == 0xFF00E5FF);

    // Element and render object creation
    auto el = w->createElement();
    el->mount(nullptr, 0);

    auto* rimg = dynamic_cast<RenderImage*>(el->findRenderObject());
    assert(rimg != nullptr);
    assert(rimg->style().shape == BoxShape::Circle);

    std::cout << "  ✓ ImageWidget fluent builder & render object creation verified.\n";
}

// ════════════════════════════════════════════════════════════════
// Main Test Runner
// ════════════════════════════════════════════════════════════════

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Image Widget Unit Test Suite    \n";
    std::cout << "================================================\n";

    test_image_cache();
    test_box_fit_math();
    test_box_fit_alignment();
    test_render_image_layout();
    test_image_widget_builder();

    std::cout << "================================================\n";
    std::cout << "  All 5 Image Widget Test Suites PASSED!        \n";
    std::cout << "================================================\n";
    return 0;
}
