#pragma once
/// @file tween.hpp
/// @brief Tween<T> — interpolates between two values along a Curve.
///
/// A Tween maps an animation progress t ∈ [0,1] to a typed value
/// between `begin` and `end`. Works with floats, colors, sizes, etc.
///
/// @copyright ENKI Framework — MIT License

#include "enki/animation/curves.hpp"
#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// Tween<T> — typed value interpolator
// ════════════════════════════════════════════════════════════════

/// @brief Interpolates between `begin` and `end` using a Curve.
///
/// @code
///   Tween<float> fade(0.0f, 1.0f);
///   float opacity = fade.evaluate(0.5); // → 0.5
///
///   Tween<Color> colorShift(Colors::Blue, Colors::Red);
///   Color c = colorShift.evaluate(t);
/// @endcode
template<typename T>
class Tween {
public:
    T begin;
    T end;
    const Curve* curve = &Curves::linear;

    Tween() = default;
    Tween(T begin, T end, const Curve* curve = &Curves::linear)
        : begin(begin), end(end), curve(curve) {}

    /// Evaluate the interpolated value at progress t ∈ [0, 1].
    [[nodiscard]] T evaluate(double t) const {
        double curved_t = curve ? curve->evaluate(t) : t;
        return lerp(begin, end, curved_t);
    }

    /// Convenience float version.
    [[nodiscard]] T evaluateF(float t) const {
        return evaluate(static_cast<double>(t));
    }

private:
    // Default lerp for numeric types
    static T lerp(const T& a, const T& b, double t) {
        return static_cast<T>(a + (b - a) * t);
    }
};

// ── Color specialization ────────────────────────────────────────

template<>
inline Color Tween<Color>::lerp(const Color& a, const Color& b, double t) {
    auto lerp_channel = [t](uint8_t ca, uint8_t cb) -> uint8_t {
        return static_cast<uint8_t>(ca + static_cast<int>((cb - ca) * t));
    };
    uint8_t aa = (a >> 24) & 0xFF, ra = (a >> 16) & 0xFF,
            ga = (a >>  8) & 0xFF, ba = (a >>  0) & 0xFF;
    uint8_t ab = (b >> 24) & 0xFF, rb = (b >> 16) & 0xFF,
            gb = (b >>  8) & 0xFF, bb = (b >>  0) & 0xFF;
    return (static_cast<Color>(lerp_channel(aa, ab)) << 24) |
           (static_cast<Color>(lerp_channel(ra, rb)) << 16) |
           (static_cast<Color>(lerp_channel(ga, gb)) <<  8) |
           (static_cast<Color>(lerp_channel(ba, bb)) <<  0);
}

// ── Size specialization ─────────────────────────────────────────

template<>
inline Size Tween<Size>::lerp(const Size& a, const Size& b, double t) {
    return Size{
        static_cast<float>(a.width  + (b.width  - a.width)  * t),
        static_cast<float>(a.height + (b.height - a.height) * t),
    };
}

// ── Point specialization ────────────────────────────────────────

template<>
inline Point Tween<Point>::lerp(const Point& a, const Point& b, double t) {
    return Point{
        static_cast<float>(a.x + (b.x - a.x) * t),
        static_cast<float>(a.y + (b.y - a.y) * t),
    };
}

// ════════════════════════════════════════════════════════════════
// Convenient aliases
// ════════════════════════════════════════════════════════════════

using FloatTween  = Tween<float>;
using ColorTween  = Tween<Color>;
using SizeTween   = Tween<Size>;
using PointTween  = Tween<Point>;

}  // namespace enki
