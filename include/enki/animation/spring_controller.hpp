#pragma once
/// @file spring_controller.hpp
/// @brief Physics-based SpringController for fluid, interruptible UI animations.
///
/// Unlike fixed-duration AnimationController, SpringController runs on natural
/// physics. Retargeting mid-animation preserves momentum without visual jarring.
/// Automatically sleeps when kinetic energy settles (0% idle CPU).
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/spring_simulation.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// SpringController — Ticker-driven Physics Controller
// ════════════════════════════════════════════════════════════════

class SpringController {
public:
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    explicit SpringController(SpringDescription desc = Springs::smooth,
                              float initial_value = 0.0f);
    ~SpringController();

    // Non-copyable
    SpringController(const SpringController&) = delete;
    SpringController& operator=(const SpringController&) = delete;

    // ── Configuration ────────────────────────────────────────────

    void setSpring(const SpringDescription& desc);
    [[nodiscard]] const SpringDescription& spring() const { return desc_; }

    void setTolerance(float distance_tol, float velocity_tol = 1e-2f);

    // ── Interactive Control ──────────────────────────────────────

    /// Smoothly animates toward target value. If already moving, preserves
    /// current velocity so movement seamlessly redirects without jerking.
    void animateTo(float target, std::optional<float> initial_velocity = std::nullopt);

    /// Instantly snaps to value and clears velocity.
    void snapTo(float value);

    /// Stops animation at current position, resetting velocity to 0.
    void stop();

    /// Reset to 0 and stop.
    void reset();

    // ── Per-Frame Update (called by Ticker) ───────────────────────

    /// Advance physics simulation by elapsed time.
    bool tick();

    // ── State Queries ────────────────────────────────────────────

    [[nodiscard]] float value() const { return current_value_; }
    [[nodiscard]] float velocity() const { return current_velocity_; }
    [[nodiscard]] float target() const { return target_value_; }
    [[nodiscard]] bool isAnimating() const { return is_animating_; }
    [[nodiscard]] AnimationStatus status() const { return status_; }

    // ── Listeners ────────────────────────────────────────────────

    void addListener(Listener listener);
    void addStatusListener(StatusListener listener);
    void clearListeners();

    // ── Cleanup ──────────────────────────────────────────────────

    void dispose();

private:
    void ensureTicker();
    void stopTicker();
    void notifyListeners();
    void notifyStatusListeners();

    SpringDescription desc_{Springs::smooth};
    float current_value_    = 0.0f;
    float current_velocity_ = 0.0f;
    float target_value_     = 0.0f;
    SpringSimulation  sim_;
    float distance_tol_     = 1e-3f;
    float velocity_tol_     = 1e-2f;

    bool is_animating_ = false;
    AnimationStatus status_ = AnimationStatus::Dismissed;

    std::chrono::steady_clock::time_point start_time_;
    size_t ticker_id_ = 0;

    std::vector<Listener>       listeners_;
    std::vector<StatusListener> status_listeners_;
};

// ════════════════════════════════════════════════════════════════
// SpringValue<T> — Typed Physics Animated Value
// ════════════════════════════════════════════════════════════════

/// @brief Syntactic sugar wrapping SpringController for typed properties (float, Point, Size, Color).
template<typename T>
class SpringValue {
public:
    explicit SpringValue(T initial, SpringDescription desc = Springs::smooth)
        : current_(initial), target_(initial), controller_(desc, 0.0f) {
        controller_.addListener([this] {
            float p = controller_.value();
            current_ = interpolate(start_, target_, p);
        });
    }

    void animateTo(T target, std::optional<float> velocity = std::nullopt) {
        start_ = current_;
        target_ = target;
        controller_.snapTo(0.0f);
        controller_.animateTo(1.0f, velocity);
    }

    void snapTo(T val) {
        start_ = val;
        current_ = val;
        target_ = val;
        controller_.snapTo(1.0f);
    }

    [[nodiscard]] T get() const { return current_; }
    [[nodiscard]] bool isAnimating() const { return controller_.isAnimating(); }
    void addListener(SpringController::Listener l) { controller_.addListener(std::move(l)); }
    SpringController& controller() { return controller_; }

private:
    static T interpolate(const T& a, const T& b, float t) {
        return static_cast<T>(a + (b - a) * t);
    }

    T current_;
    T start_;
    T target_;
    SpringController controller_;
};

// Specialization for Point
template<>
inline Point SpringValue<Point>::interpolate(const Point& a, const Point& b, float t) {
    return Point{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

// Specialization for Size
template<>
inline Size SpringValue<Size>::interpolate(const Size& a, const Size& b, float t) {
    return Size{a.width + (b.width - a.width) * t, a.height + (b.height - a.height) * t};
}

} // namespace enki
