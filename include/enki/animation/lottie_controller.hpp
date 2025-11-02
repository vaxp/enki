#pragma once
/// @file lottie_controller.hpp
/// @brief Advanced playback controller for Lottie animations.
///
/// Features:
///   - Multiple playback modes: Loop, Once, PingPong, Segment.
///   - Frame & Marker-driven playback (e.g. play specific animation segments).
///   - Real-time speed regulation & reverse playback.
///   - Per-frame Ticker integration with smooth sub-frame interpolation.
///   - Listeners for loop completion, end of animation, and marker entry.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/lottie_composition.hpp"
#include "enki/animation/ticker.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace enki {

enum class LottiePlaybackMode {
    Loop,      ///< Play forward, repeat endlessly
    Once,      ///< Play forward once and stop at end
    PingPong,  ///< Alternate between forward and reverse
    Segment,   ///< Play between specific progress/frame bounds
};

class LottieController {
public:
    using Duration  = std::chrono::milliseconds;
    using Listener  = std::function<void()>;

    // ── Construction & Destruction ───────────────────────────────
    explicit LottieController(std::shared_ptr<LottieComposition> composition = nullptr,
                              Duration duration_override = Duration{0});
    ~LottieController();

    // Non-copyable
    LottieController(const LottieController&) = delete;
    LottieController& operator=(const LottieController&) = delete;

    // ── Composition Binding ──────────────────────────────────────
    void setComposition(std::shared_ptr<LottieComposition> comp);
    [[nodiscard]] std::shared_ptr<LottieComposition> composition() const { return composition_; }

    // ── Playback Configuration ───────────────────────────────────
    void setPlaybackMode(LottiePlaybackMode mode) { mode_ = mode; }
    [[nodiscard]] LottiePlaybackMode playbackMode() const { return mode_; }

    void setSpeed(float speed) { speed_ = speed; }
    [[nodiscard]] float speed() const { return speed_; }

    void setRepeat(bool repeat) {
        mode_ = repeat ? LottiePlaybackMode::Loop : LottiePlaybackMode::Once;
    }
    [[nodiscard]] bool isRepeating() const {
        return mode_ == LottiePlaybackMode::Loop || mode_ == LottiePlaybackMode::PingPong;
    }

    // ── Playback Control ─────────────────────────────────────────
    /// Start or resume animation playback.
    void play();

    /// Pause animation playback at current progress.
    void pause();

    /// Stop animation and reset to beginning.
    void stop();

    /// Reset animation to start position.
    void reset();

    /// Play forward from current position.
    void forward();

    /// Play in reverse from current position.
    void reverse();

    /// Jump directly to normalized progress in [0.0, 1.0].
    void seek(float progress);

    /// Jump directly to specific frame index.
    void seekFrame(double frame_index);

    /// Play a specific timeline segment between start and end progress.
    void playSegment(float start_progress, float end_progress, bool repeat = false);

    /// Play a named marker segment (e.g. "intro", "success", "hover").
    bool playMarker(std::string_view marker_name, bool repeat = false);

    // ── Per-Frame Update ─────────────────────────────────────────
    /// Advance animation by delta time (called automatically by Ticker).
    void tick(double delta_seconds);

    // ── State Queries ────────────────────────────────────────────
    [[nodiscard]] bool isPlaying() const { return is_playing_; }
    [[nodiscard]] bool isPaused() const { return !is_playing_ && progress_ > segment_start_ && progress_ < segment_end_; }
    [[nodiscard]] bool isCompleted() const { return !is_playing_ && (progress_ >= segment_end_ || progress_ <= segment_start_); }

    [[nodiscard]] float progress() const { return progress_; }
    [[nodiscard]] double currentFrame() const;
    [[nodiscard]] double currentTime() const;
    [[nodiscard]] int loopCount() const { return loop_count_; }

    // ── Event Callbacks / Listeners ──────────────────────────────
    size_t addListener(Listener listener);
    void removeListener(size_t id);
    void clearListeners();

    void onStart(Listener cb) { on_start_ = std::move(cb); }
    void onEnd(Listener cb) { on_end_ = std::move(cb); }
    void onLoop(std::function<void(int)> cb) { on_loop_ = std::move(cb); }
    void onMarker(std::function<void(std::string_view)> cb) { on_marker_ = std::move(cb); }

    void dispose();

private:
    struct ListenerEntry {
        size_t   id;
        Listener fn;
    };

    void ensureTicker();
    void notifyListeners();

    std::shared_ptr<LottieComposition> composition_;
    std::shared_ptr<bool>              alive_ = std::make_shared<bool>(true);
    LottiePlaybackMode                 mode_ = LottiePlaybackMode::Loop;

    float  speed_          = 1.0f;
    float  progress_       = 0.0f;
    float  segment_start_  = 0.0f;
    float  segment_end_    = 1.0f;
    bool   is_playing_     = false;
    bool   is_forward_     = true;
    int    loop_count_     = 0;
    size_t ticker_id_      = 0;
    size_t next_listener_id_ = 1;

    std::chrono::steady_clock::time_point last_tick_time_;

    std::vector<ListenerEntry>              listeners_;
    std::function<void()>                   on_start_;
    std::function<void()>                   on_end_;
    std::function<void(int)>                on_loop_;
    std::function<void(std::string_view)>   on_marker_;
};

} // namespace enki
