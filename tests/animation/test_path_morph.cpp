/// @file test_path_morph.cpp
/// @brief Tests for PathMorph SVG vector morphing engine and SvgMorph widget.

#include "enki/animation/path_morph.hpp"
#include "enki/widgets/svg_morph.hpp"
#include <cassert>
#include <cstdio>

using namespace enki;

void test_path_morph_hamburger_close() {
    PathMorph morph(SvgMorphPaths::hamburger, SvgMorphPaths::close, 60);

    assert(morph.isValid());
    assert(morph.sampleCount() == 60);

    // Evaluate at beginning, midpoint, and end
    auto p0 = morph.evaluate(0.0f);
    assert(p0 != nullptr);
    assert(!p0->getBounds().size().isEmpty());

    auto p_mid = morph.evaluate(0.5f);
    assert(p_mid != nullptr);
    assert(!p_mid->getBounds().size().isEmpty());

    auto p1 = morph.evaluate(1.0f);
    assert(p1 != nullptr);
    assert(!p1->getBounds().size().isEmpty());

    printf("  [PASS] path morph hamburger -> close sampling & evaluation\n");
}

void test_path_morph_play_pause() {
    PathMorph morph(SvgMorphPaths::play, SvgMorphPaths::pause, 80);

    assert(morph.isValid());
    assert(morph.sampleCount() == 80);

    auto p0 = morph.evaluate(0.0f);
    auto p1 = morph.evaluate(1.0f);
    assert(p0 != nullptr && p1 != nullptr);

    printf("  [PASS] path morph play -> pause\n");
}

void test_svg_morph_widget() {
    WidgetPtr w = svgMorph({
        .from_path = SvgMorphPaths::star,
        .to_path   = SvgMorphPaths::circle,
        .progress  = 0.42f,
        .color     = 0xFFFFCC00,
        .stroke_width = 3.0f,
    });

    assert(w != nullptr);
    printf("  [PASS] svgMorph declarative widget creation\n");
}

int main() {
    printf("Running Path Morph tests...\n");
    test_path_morph_hamburger_close();
    test_path_morph_play_pause();
    test_svg_morph_widget();
    printf("All Path Morph tests passed successfully!\n");
    return 0;
}
