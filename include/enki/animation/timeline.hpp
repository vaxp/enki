#pragma once
/// @file timeline.hpp
/// @brief Multi-track AnimationTimeline and Keyframe sequencer for ENKI.
///
/// Enables choreographing complex, multi-element parallel and staggered animations
/// over a unified timeline. Supports keyframes, scrubbing, looping, and reverse.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/animation/curves.hpp"
#include "enki/animation/tween.hpp"
#include "enki/animation/animation_controller.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Keyframe<T> — Single milestone in a Keyframe track
// ════════════════════════════════════════════════════════════════

template<typename T>
struct Keyframe {
    float        time_fraction = 0.0f; ///< Position in track timeline [0.0, 1.0].
    T            value{};              ///< Target value at this keyframe.
    const Curve* curve = &Curves::linear; ///< Easing curve transitioning towards this keyframe.

    Keyframe() = default;
    Keyframe(float fraction, T val, const Curve* c = &Curves::linear)
        : time_fraction(std::clamp(fraction, 0.0f, 1.0f)), value(std::move(val)), curve(c) {}
};

// ════════════════════════════════════════════════════════════════
// KeyframeSequence<T> — Multi-stop interpolated timeline
// ════════════════════════════════════════════════════════════════

template<typename T>
class KeyframeSequence {
public:
    KeyframeSequence() = default;
    explicit KeyframeSequence(std::vector<Keyframe<T>> keyframes)
        : keyframes_(std::move(keyframes)) {
        std::sort(keyframes_.begin(), keyframes_.end(), [](const auto& a, const auto& b) {
            return a.time_fraction < b.time_fraction;
        });
    }

    void addKeyframe(float fraction, T value, const Curve* curve = &Curves::linear) {
        keyframes_.emplace_back(fraction, std::move(value), curve);
        std::sort(keyframes_.begin(), keyframes_.end(), [](const auto& a, const auto& b) {
            return a.time_fraction < b.time_fraction;
        });
    }

    [[nodiscard]] T evaluate(float t) const {
        if (keyframes_.empty()) return T{};
        if (keyframes_.size() == 1 || t <= keyframes_.front().time_fraction) {
            return keyframes_.front().value;
        }
        if (t >= keyframes_.back().time_fraction) {
            return keyframes_.back().value;
        }

        // Find surrounding interval
        for (size_t i = 0; i < keyframes_.size() - 1; ++i) {
            const auto& k1 = keyframes_[i];
            const auto& k2 = keyframes_[i + 1];
            if (t >= k1.time_fraction && t <= k2.time_fraction) {
                float seg_dur = k2.time_fraction - k1.time_fraction;
                float local_t = (seg_dur > 0.0f) ? (t - k1.time_fraction) / seg_dur : 1.0f;
                local_t = std::clamp(local_t, 0.0f, 1.0f);
                if (k2.curve) {
                    local_t = k2.curve->evaluateF(local_t);
                }
                return lerp(k1.value, k2.value, local_t);
            }
        }
        return keyframes_.back().value;
    }

private:
    static T lerp(const T& a, const T& b, float t) {
        return static_cast<T>(a + (b - a) * t);
    }

    std::vector<Keyframe<T>> keyframes_;
};

// Keyframe lerp specializations
template<>
inline Point KeyframeSequence<Point>::lerp(const Point& a, const Point& b, float t) {
    return Point{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

template<>
inline Size KeyframeSequence<Size>::lerp(const Size& a, const Size& b, float t) {
    return Size{a.width + (b.width - a.width) * t, a.height + (b.height - a.height) * t};
}

template<>
inline Color KeyframeSequence<Color>::lerp(const Color& a, const Color& b, float t) {
    return Tween<Color>(a, b).evaluate(static_cast<double>(t));
}

// ════════════════════════════════════════════════════════════════
// AnimationTimeline — Orchestrator for multiple animation tracks
// ════════════════════════════════════════════════════════════════

class AnimationTimeline {
public:
    using Duration = std::chrono::milliseconds;
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    AnimationTimeline();
    explicit AnimationTimeline(Duration duration_override);
    ~AnimationTimeline();

    AnimationTimeline(const AnimationTimeline&) = delete;
    AnimationTimeline& operator=(const AnimationTimeline&) = delete;

    // ── Track Composition ────────────────────────────────────────

    /// Add an animation callback track with start offset and duration
    AnimationTimeline& add(Duration start_offset, Duration duration,
                           std::function<void(float progress)> on_update,
                           const Curve* curve = &Curves::linear);

    /// Add a Tween track interpolating between typed values
    template<typename T>
    AnimationTimeline& addTween(Duration start_offset, Duration duration,
                                Tween<T> tween,
                                std::function<void(const T& val)> on_update) {
        auto shared_tween = std::make_shared<Tween<T>>(std::move(tween));
        auto shared_cb = std::move(on_update);
        return add(start_offset, duration, [shared_tween, shared_cb](float p) {
            if (shared_cb) shared_cb(shared_tween->evaluateF(p));
        }, &Curves::linear);
    }

    /// Add a multi-stop Keyframe sequence track
    template<typename T>
    AnimationTimeline& addKeyframes(Duration start_offset, Duration duration,
                                    KeyframeSequence<T> sequence,
                                    std::function<void(const T& val)> on_update) {
        auto shared_seq = std::make_shared<KeyframeSequence<T>>(std::move(sequence));
        auto shared_cb = std::move(on_update);
        return add(start_offset, duration, [shared_seq, shared_cb](float p) {
            if (shared_cb) shared_cb(shared_seq->evaluate(p));
        }, &Curves::linear);
    }

    // ── Playback Controls ────────────────────────────────────────

    void play();
    void pause();
    void stop();
    void reset();
    void forward();
    void reverse();

    /// Jump directly to normalized timeline progress in [0.0, 1.0]
    void seek(float progress);

    /// Jump directly to specific millisecond timestamp in timeline
    void seekMs(int64_t ms);

    // ── Playback Configuration ───────────────────────────────────

    void setSpeed(float speed) { speed_ = std::max(0.01f, speed); }
    [[nodiscard]] float speed() const { return speed_; }

    void setRepeat(bool repeat) { repeat_ = repeat; }
    [[nodiscard]] bool isRepeating() const { return repeat_; }

    void setPingPong(bool pingpong) { pingpong_ = pingpong; }
    [[nodiscard]] bool isPingPong() const { return pingpong_; }

    // ── State Queries ────────────────────────────────────────────

    [[nodiscard]] Duration totalDuration() const { return total_duration_; }
    [[nodiscard]] float progress() const { return progress_; }
    [[nodiscard]] bool isPlaying() const { return is_playing_; }
    [[nodiscard]] AnimationStatus status() const { return status_; }

    // ── Per-Frame Update (driven by Ticker) ───────────────────────

    bool tick();

    // ── Listeners ────────────────────────────────────────────────

    void addListener(Listener listener);
    void addStatusListener(StatusListener listener);
    void clearListeners();

    void dispose();

private:
    struct Track {
        int64_t start_ms   = 0;
        int64_t duration_ms = 0;
        std::function<void(float)> update_fn;
        const Curve* curve = &Curves::linear;
    };

    void ensureTicker();
    void stopTicker();
    void applyProgress(float p);
    void updateDuration();
    void notifyListeners();
    void notifyStatusListeners();

    std::vector<Track> tracks_;
    Duration total_duration_{0};
    bool manual_duration_ = false;

    float progress_ = 0.0f;
    float speed_    = 1.0f;
    bool is_playing_ = false;
    bool is_forward_ = true;
    bool repeat_     = false;
    bool pingpong_   = false;

    AnimationStatus status_ = AnimationStatus::Dismissed;
    std::chrono::steady_clock::time_point last_tick_time_;
    size_t ticker_id_ = 0;

    std::vector<Listener>       listeners_;
    std::vector<StatusListener> status_listeners_;
};

} // namespace enki
