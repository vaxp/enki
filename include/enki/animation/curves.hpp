#pragma once
/// @file curves.hpp
/// @brief Animation easing curves — maps [0,1] → [0,1].
///
/// Curves control the "feel" of an animation: does it start slow and
/// end fast? Bounce? Overshoot? Each Curve is a pure function
/// `double evaluate(double t)` where t ∈ [0, 1].
///
/// @copyright ENKI Framework — MIT License

#include <cmath>
#include <functional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Curve — abstract base
// ════════════════════════════════════════════════════════════════

/// @brief Abstract animation curve (easing function).
/// Transforms a linear animation progress t ∈ [0,1] into a curved value.
class Curve {
public:
    virtual ~Curve() = default;

    /// Evaluate the curve at progress t ∈ [0, 1].
    /// May return values outside [0, 1] for overshoot curves.
    [[nodiscard]] virtual double evaluate(double t) const = 0;

    /// Convenience: evaluate with float input/output.
    [[nodiscard]] float evaluateF(float t) const {
        return static_cast<float>(evaluate(static_cast<double>(t)));
    }
};

// ════════════════════════════════════════════════════════════════
// Built-in curve implementations
// ════════════════════════════════════════════════════════════════

/// Linear — no easing (constant speed).
class LinearCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override { return t; }
};

/// Ease-in — starts slow, ends fast (cubic).
class EaseInCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        return t * t * t;
    }
};

/// Ease-out — starts fast, ends slow (cubic).
class EaseOutCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        double u = 1.0 - t;
        return 1.0 - u * u * u;
    }
};

/// Ease-in-out — slow at both ends (cubic).
class EaseInOutCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        return t < 0.5
            ? 4.0 * t * t * t
            : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
    }
};

/// Elastic — spring-like overshoot at the end.
class ElasticOutCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        if (t == 0.0 || t == 1.0) return t;
        constexpr double c4 = (2.0 * M_PI) / 3.0;
        return std::pow(2.0, -10.0 * t)
             * std::sin((t * 10.0 - 0.75) * c4)
             + 1.0;
    }
};

/// Bounce — bounces at the end.
class BounceOutCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        constexpr double n1 = 7.5625;
        constexpr double d1 = 2.75;
        if (t < 1.0 / d1) {
            return n1 * t * t;
        } else if (t < 2.0 / d1) {
            t -= 1.5 / d1;
            return n1 * t * t + 0.75;
        } else if (t < 2.5 / d1) {
            t -= 2.25 / d1;
            return n1 * t * t + 0.9375;
        } else {
            t -= 2.625 / d1;
            return n1 * t * t + 0.984375;
        }
    }
};

/// Back — slight overshoot like pulling back before launching.
class BackOutCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        constexpr double c1 = 1.70158;
        constexpr double c3 = c1 + 1.0;
        double u = t - 1.0;
        return 1.0 + c3 * u * u * u + c1 * u * u;
    }
};

/// FastOutSlowIn — Material Design standard curve.
class FastOutSlowInCurve final : public Curve {
public:
    [[nodiscard]] double evaluate(double t) const override {
        // Cubic bezier: (0.4, 0.0, 0.2, 1.0)
        // Approximated with a smooth cubic
        return t < 0.5
            ? 2.0 * t * t
            : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
    }
};

// ════════════════════════════════════════════════════════════════
// Curves — static factory / named instances
// ════════════════════════════════════════════════════════════════

/// @brief Named curve instances — use like: `Curves::easeInOut`
struct Curves {
    static const LinearCurve       linear;
    static const EaseInCurve       easeIn;
    static const EaseOutCurve      easeOut;
    static const EaseInOutCurve    easeInOut;
    static const ElasticOutCurve   elasticOut;
    static const BounceOutCurve    bounceOut;
    static const BackOutCurve      backOut;
    static const FastOutSlowInCurve fastOutSlowIn;
};

}  // namespace enki
