#pragma once
/// @file animation_controller.hpp
/// @brief AnimationController — drives animations over time.
///
/// AnimationController manages the progress of an animation:
///   - Tracks elapsed time relative to a `duration`
///   - Computes a normalized value t ∈ [0, 1]
///   - Supports forward, reverse, repeat, and ping-pong modes
///   - Fires callbacks via Signal on each tick and on completion
///
/// ## Usage (inside a StatefulWidget's State):
/// @code
///   class MyState : public State {
///       AnimationController controller;
///
///       void initState() override {
///           State::initState();
///           controller.setDuration(500ms);
///           controller.addListener([this] {
///               setState([] {}); // trigger rebuild on each frame
///           });
///           controller.forward();
///       }
///
///       WidgetPtr build(BuildContext& ctx) override {
///           float t = controller.value();
///           return opacity_widget(t, child);
///       }
///
///       void dispose() override {
///           controller.dispose();
///           State::dispose();
///       }
///   };
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/core/signal.hpp"
#include "enki/animation/curves.hpp"
#include <chrono>
#include <functional>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// AnimationStatus
// ════════════════════════════════════════════════════════════════

enum class AnimationStatus {
    Dismissed,   ///< At beginning (value = 0, stopped)
    Forward,     ///< Running toward end (value increases)
    Reverse,     ///< Running toward beginning (value decreases)
    Completed,   ///< At end (value = 1, stopped)
};

// ════════════════════════════════════════════════════════════════
// AnimationController
// ════════════════════════════════════════════════════════════════

/// @brief Drives a time-based animation, producing values in [0, 1].
class AnimationController {
public:
    using Duration    = std::chrono::milliseconds;
    using Listener    = std::function<void()>;
    using TimePoint   = std::chrono::steady_clock::time_point;

    // ── Construction ────────────────────────────────────────────

    explicit AnimationController(Duration duration = Duration{300})
        : duration_(duration) {}

    ~AnimationController() { dispose(); }

    // Non-copyable
    AnimationController(const AnimationController&) = delete;
    AnimationController& operator=(const AnimationController&) = delete;

    // ── Configuration ────────────────────────────────────────────

    void setDuration(Duration d) { duration_ = d; }
    [[nodiscard]] Duration duration() const { return duration_; }

    void setRepeats(bool repeats)  { repeats_  = repeats; }
    void setPingPong(bool pingpong) { pingpong_ = pingpong; }

    // ── Control ─────────────────────────────────────────────────

    /// Start animating from current value toward end (value = 1).
    void forward() {
        status_ = AnimationStatus::Forward;
        start_time_ = std::chrono::steady_clock::now();
        start_value_ = value_;
        active_ = true;
    }

    /// Start animating from current value toward beginning (value = 0).
    void reverse() {
        status_ = AnimationStatus::Reverse;
        start_time_ = std::chrono::steady_clock::now();
        start_value_ = value_;
        active_ = true;
    }

    /// Stop the animation at its current value.
    void stop() {
        active_ = false;
        if (status_ == AnimationStatus::Forward ||
            status_ == AnimationStatus::Reverse) {
            status_ = value_ >= 1.0f
                ? AnimationStatus::Completed
                : AnimationStatus::Dismissed;
        }
    }

    /// Reset to 0 and stop.
    void reset() {
        active_ = false;
        value_  = 0.0f;
        status_ = AnimationStatus::Dismissed;
        notifyListeners();
    }

    /// Jump directly to a value.
    void setValue(float val) {
        value_ = std::clamp(val, 0.0f, 1.0f);
        if (value_ >= 1.0f) status_ = AnimationStatus::Completed;
        else if (value_ <= 0.0f) status_ = AnimationStatus::Dismissed;
        notifyListeners();
    }

    /// Animate forward if dismissed/reversed, backward if completed/forward.
    void toggle() {
        if (status_ == AnimationStatus::Forward ||
            status_ == AnimationStatus::Completed) {
            reverse();
        } else {
            forward();
        }
    }

    // ── Per-frame tick — called by App/Ticker ────────────────────

    /// Advance the animation. Returns true if still running.
    bool tick() {
        if (!active_) return false;

        auto now     = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());

        // Compute raw progress [0, 1] in the direction of travel
        double raw = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        raw = std::clamp(raw, 0.0, 1.0);

        if (status_ == AnimationStatus::Forward) {
            double new_val = start_value_ + raw * (1.0 - start_value_);
            value_ = static_cast<float>(std::clamp(new_val, 0.0, 1.0));
        } else {
            double new_val = start_value_ - raw * start_value_;
            value_ = static_cast<float>(std::clamp(new_val, 0.0, 1.0));
        }

        notifyListeners();

        // Check completion
        bool done = (raw >= 1.0);
        if (done) {
            if (repeats_) {
                if (pingpong_) {
                    // Flip direction
                    if (status_ == AnimationStatus::Forward) {
                        reverse();
                    } else {
                        forward();
                    }
                } else {
                    // Restart from beginning
                    reset();
                    forward();
                }
            } else {
                active_ = false;
                status_ = (status_ == AnimationStatus::Forward)
                    ? AnimationStatus::Completed
                    : AnimationStatus::Dismissed;
                notifyStatusListeners();
            }
            return repeats_;
        }

        return true;
    }

    // ── Listeners ────────────────────────────────────────────────

    /// Add a listener called on every value change (every frame).
    void addListener(Listener listener) {
        listeners_.push_back(std::move(listener));
    }

    /// Add a listener called when status changes (start/stop/complete).
    void addStatusListener(std::function<void(AnimationStatus)> listener) {
        status_listeners_.push_back(std::move(listener));
    }

    void clearListeners() { listeners_.clear(); }

    // ── Accessors ────────────────────────────────────────────────

    /// Current animation value in [0, 1].
    [[nodiscard]] float value() const { return value_; }

    /// Current animation status.
    [[nodiscard]] AnimationStatus status() const { return status_; }

    /// Whether the animation is currently running.
    [[nodiscard]] bool isAnimating() const { return active_; }

    /// Whether the animation has completed (reached end).
    [[nodiscard]] bool isCompleted() const {
        return status_ == AnimationStatus::Completed;
    }

    /// Whether the animation is dismissed (at start).
    [[nodiscard]] bool isDismissed() const {
        return status_ == AnimationStatus::Dismissed;
    }

    // ── Lifecycle ────────────────────────────────────────────────

    /// Call in State::dispose() to clean up listeners.
    void dispose() {
        active_ = false;
        listeners_.clear();
        status_listeners_.clear();
    }

private:
    void notifyListeners() {
        for (auto& fn : listeners_) fn();
    }

    void notifyStatusListeners() {
        for (auto& fn : status_listeners_) fn(status_);
    }

    // State
    float           value_      = 0.0f;
    double          start_value_ = 0.0;
    AnimationStatus status_     = AnimationStatus::Dismissed;
    bool            active_     = false;
    bool            repeats_    = false;
    bool            pingpong_   = false;

    // Timing
    Duration  duration_;
    TimePoint start_time_;

    // Callbacks
    std::vector<Listener>                             listeners_;
    std::vector<std::function<void(AnimationStatus)>> status_listeners_;
};

// ════════════════════════════════════════════════════════════════
// AnimatedValue<T> — syntactic sugar combining Controller + Tween
// ════════════════════════════════════════════════════════════════

/// @brief Combines an AnimationController with a Tween for convenience.
///
/// @code
///   AnimatedValue<float> opacity(0.0f, 1.0f, 300ms);
///   opacity.addListener([this] { setState([] {}); });
///   opacity.forward();
///   // In build:
///   float current = opacity.get(); // → interpolated value
/// @endcode
template<typename T>
class AnimatedValue {
public:
    AnimatedValue(T begin, T end,
                  AnimationController::Duration duration = std::chrono::milliseconds{300},
                  const Curve* curve = nullptr);

    void forward()  { controller_.forward(); }
    void reverse()  { controller_.reverse(); }
    void toggle()   { controller_.toggle(); }
    void reset()    { controller_.reset(); }
    bool isAnimating() const { return controller_.isAnimating(); }

    void addListener(AnimationController::Listener fn) {
        controller_.addListener(std::move(fn));
    }

    void dispose() { controller_.dispose(); }

    /// Advance one frame. Returns true if still animating.
    bool tick() { return controller_.tick(); }

    /// Get the current interpolated value.
    [[nodiscard]] T get() const;

    [[nodiscard]] AnimationController& controller() { return controller_; }

private:
    AnimationController controller_;
    T begin_;
    T end_;
    const Curve* curve_ = nullptr;
};

template<typename T>
AnimatedValue<T>::AnimatedValue(T begin, T end, AnimationController::Duration duration, const Curve* curve)
    : begin_(begin), end_(end), curve_(curve) {
    controller_.setDuration(duration);
}

template<typename T>
T AnimatedValue<T>::get() const {
    float t = controller_.value();
    if (curve_) {
        t = curve_->evaluateF(t);
    }
    return begin_ + static_cast<T>((end_ - begin_) * t);
}

}  // namespace enki
