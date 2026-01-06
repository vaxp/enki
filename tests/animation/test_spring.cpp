/// @file test_spring.cpp
/// @brief Comprehensive tests for Physics-based Spring Simulation and SpringController.

#include "enki/animation/spring_simulation.hpp"
#include "enki/animation/spring_controller.hpp"
#include <cassert>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace enki;

void test_underdamped_spring_bouncy() {
    // Springs::bouncy: mass 1.0, stiffness 180, damping 12 (zeta ~ 0.447 < 1)
    SpringSimulation sim(Springs::bouncy, 0.0f, 1.0f, 0.0f);

    assert(std::abs(sim.x(0.0f) - 0.0f) < 1e-4f);
    assert(std::abs(sim.dx(0.0f) - 0.0f) < 1e-4f);

    bool observed_overshoot = false;
    float max_x = 0.0f;

    // Simulate over 2 seconds in 10ms steps
    for (float t = 0.01f; t <= 2.0f; t += 0.01f) {
        float val = sim.x(t);
        if (val > max_x) max_x = val;
        if (val > 1.05f) {
            observed_overshoot = true;
        }
    }

    // Bouncy spring MUST overshoot 1.0
    assert(observed_overshoot);
    assert(max_x > 1.10f); // typically reaches ~1.15-1.20

    // After 2.0s, it must be completely settled at 1.0
    assert(sim.isDone(2.0f));
    assert(std::abs(sim.x(2.0f) - 1.0f) < 1e-3f);

    printf("  [PASS] underdamped spring bouncy (max overshoot = %.3f)\n", max_x);
}

void test_critically_damped_smooth() {
    // Springs::smooth: critically damped, zeta ~ 1.0
    SpringSimulation sim(Springs::smooth, 0.0f, 1.0f, 0.0f);

    assert(std::abs(sim.x(0.0f) - 0.0f) < 1e-4f);

    float max_x = 0.0f;
    for (float t = 0.01f; t <= 2.0f; t += 0.01f) {
        float val = sim.x(t);
        if (val > max_x) max_x = val;
    }

    // Critically damped must NOT significantly overshoot 1.0
    assert(max_x <= 1.01f);
    assert(sim.isDone(1.5f));
    assert(std::abs(sim.x(1.5f) - 1.0f) < 1e-3f);

    printf("  [PASS] critically damped smooth spring (max overshoot = %.3f)\n", max_x);
}

void test_overdamped_spring() {
    // Heavy damping: mass 1.0, stiffness 100, damping 50 (zeta = 50 / (2 * 10) = 2.5 > 1)
    SpringDescription overdamped_desc{1.0f, 100.0f, 50.0f};
    assert(overdamped_desc.dampingRatio() > 1.0f);

    SpringSimulation sim(overdamped_desc, 0.0f, 1.0f, 0.0f);

    float prev_x = 0.0f;
    for (float t = 0.05f; t <= 2.0f; t += 0.05f) {
        float val = sim.x(t);
        assert(val >= prev_x); // Strictly monotonic increasing
        assert(val <= 1.0f);   // Absolutely zero overshoot
        prev_x = val;
    }

    printf("  [PASS] overdamped spring monotonic settling\n");
}

void test_spring_momentum_initial_velocity() {
    // Moving from 0 to 1, but with negative initial velocity (e.g. user flung in opposite direction)
    SpringSimulation sim(Springs::bouncy, 0.0f, 1.0f, -5.0f);

    assert(sim.dx(0.0f) == -5.0f);

    // Initial position dips negative because of momentum!
    bool dipped_negative = false;
    for (float t = 0.005f; t <= 0.2f; t += 0.005f) {
        if (sim.x(t) < -0.05f) {
            dipped_negative = true;
            break;
        }
    }
    assert(dipped_negative);

    // Eventually overcomes momentum and settles at 1.0
    assert(std::abs(sim.x(2.0f) - 1.0f) < 1e-3f);

    printf("  [PASS] spring initial velocity momentum fling\n");
}

void test_spring_curve_adapter() {
    SpringCurve curve(Springs::bouncy, 0.8f);

    assert(curve.evaluate(0.0) == 0.0);
    assert(curve.evaluate(1.0) == 1.0);

    // Midpoint should be non-zero and reasonable
    double mid = curve.evaluate(0.5);
    assert(mid > 0.5 && mid < 1.5);

    printf("  [PASS] spring curve adapter\n");
}

void test_spring_controller_retargeting() {
    SpringController ctrl(Springs::snappy, 0.0f);
    assert(ctrl.value() == 0.0f);

    ctrl.animateTo(100.0f);
    assert(ctrl.isAnimating());
    assert(ctrl.target() == 100.0f);

    // Let it tick once
    ctrl.tick();

    // Now retarget to 50.0f mid-motion
    ctrl.animateTo(50.0f);
    assert(ctrl.target() == 50.0f);
    assert(ctrl.isAnimating());

    ctrl.snapTo(42.0f);
    assert(ctrl.value() == 42.0f);
    assert(!ctrl.isAnimating());

    printf("  [PASS] spring controller retargeting\n");
}

void test_spring_value_point() {
    SpringValue<Point> pt({0.0f, 0.0f});
    assert(pt.get().x == 0.0f && pt.get().y == 0.0f);

    pt.snapTo({100.0f, 200.0f});
    assert(pt.get().x == 100.0f && pt.get().y == 200.0f);

    printf("  [PASS] spring value point interpolation\n");
}

int main() {
    printf("Running Spring Simulation & Controller tests...\n");
    test_underdamped_spring_bouncy();
    test_critically_damped_smooth();
    test_overdamped_spring();
    test_spring_momentum_initial_velocity();
    test_spring_curve_adapter();
    test_spring_controller_retargeting();
    test_spring_value_point();
    printf("All Spring tests passed successfully!\n");
    return 0;
}
