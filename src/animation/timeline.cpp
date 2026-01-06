/// @file timeline.cpp
/// @brief AnimationTimeline implementation with Ticker and multi-track orchestration.

#include "enki/animation/timeline.hpp"

namespace enki {

AnimationTimeline::AnimationTimeline() = default;

AnimationTimeline::AnimationTimeline(Duration duration_override)
    : total_duration_(duration_override), manual_duration_(true) {}

AnimationTimeline::~AnimationTimeline() {
    dispose();
}

AnimationTimeline& AnimationTimeline::add(Duration start_offset, Duration duration,
                                         std::function<void(float progress)> on_update,
                                         const Curve* curve) {
    Track track;
    track.start_ms = start_offset.count();
    track.duration_ms = duration.count();
    track.update_fn = std::move(on_update);
    track.curve = curve ? curve : &Curves::linear;

    tracks_.push_back(std::move(track));
    updateDuration();
    return *this;
}

void AnimationTimeline::updateDuration() {
    if (manual_duration_) return;

    int64_t max_end = 0;
    for (const auto& t : tracks_) {
        int64_t end_ms = t.start_ms + t.duration_ms;
        if (end_ms > max_end) max_end = end_ms;
    }
    total_duration_ = Duration{max_end};
}

void AnimationTimeline::applyProgress(float p) {
    progress_ = std::clamp(p, 0.0f, 1.0f);
    double total_ms = static_cast<double>(total_duration_.count());
    if (total_ms <= 0.0) {
        for (auto& track : tracks_) {
            if (track.update_fn) track.update_fn(1.0f);
        }
        return;
    }

    double current_ms = progress_ * total_ms;

    for (auto& track : tracks_) {
        if (!track.update_fn) continue;

        if (track.duration_ms <= 0) {
            float val = (current_ms >= track.start_ms) ? 1.0f : 0.0f;
            track.update_fn(val);
            continue;
        }

        constexpr double time_eps = 1e-4;
        double diff = current_ms - static_cast<double>(track.start_ms);
        if (diff <= time_eps) {
            track.update_fn(0.0f);
            continue;
        }

        double raw = diff / static_cast<double>(track.duration_ms);
        float clamped = static_cast<float>(std::clamp(raw, 0.0, 1.0));
        float curved = track.curve ? track.curve->evaluateF(clamped) : clamped;
        track.update_fn(curved);
    }
}

void AnimationTimeline::play() {
    forward();
}

void AnimationTimeline::forward() {
    is_forward_ = true;
    is_playing_ = true;
    status_ = AnimationStatus::Forward;
    last_tick_time_ = std::chrono::steady_clock::now();
    ensureTicker();
    notifyStatusListeners();
}

void AnimationTimeline::reverse() {
    is_forward_ = false;
    is_playing_ = true;
    status_ = AnimationStatus::Reverse;
    last_tick_time_ = std::chrono::steady_clock::now();
    ensureTicker();
    notifyStatusListeners();
}

void AnimationTimeline::pause() {
    is_playing_ = false;
    stopTicker();
    notifyStatusListeners();
}

void AnimationTimeline::stop() {
    is_playing_ = false;
    stopTicker();
    notifyStatusListeners();
}

void AnimationTimeline::reset() {
    stop();
    seek(0.0f);
    status_ = AnimationStatus::Dismissed;
    notifyStatusListeners();
}

void AnimationTimeline::seek(float progress) {
    applyProgress(progress);
    notifyListeners();
}

void AnimationTimeline::seekMs(int64_t ms) {
    double total_ms = static_cast<double>(total_duration_.count());
    if (total_ms <= 0.0) {
        seek(1.0f);
    } else {
        seek(static_cast<float>(static_cast<double>(ms) / total_ms));
    }
}

bool AnimationTimeline::tick() {
    if (!is_playing_) return false;

    auto now = std::chrono::steady_clock::now();
    double dt_sec = std::chrono::duration<double>(now - last_tick_time_).count();
    last_tick_time_ = now;

    double total_sec = static_cast<double>(total_duration_.count()) / 1000.0;
    if (total_sec <= 0.0) {
        applyProgress(1.0f);
        is_playing_ = false;
        status_ = AnimationStatus::Completed;
        stopTicker();
        notifyListeners();
        notifyStatusListeners();
        return false;
    }

    double delta_p = (dt_sec * speed_) / total_sec;

    if (is_forward_) {
        progress_ += static_cast<float>(delta_p);
        if (progress_ >= 1.0f) {
            if (repeat_) {
                if (pingpong_) {
                    progress_ = 1.0f;
                    is_forward_ = false;
                    status_ = AnimationStatus::Reverse;
                } else {
                    progress_ = 0.0f;
                }
            } else {
                progress_ = 1.0f;
                is_playing_ = false;
                status_ = AnimationStatus::Completed;
                stopTicker();
                applyProgress(progress_);
                notifyListeners();
                notifyStatusListeners();
                return false;
            }
        }
    } else {
        progress_ -= static_cast<float>(delta_p);
        if (progress_ <= 0.0f) {
            if (repeat_) {
                if (pingpong_) {
                    progress_ = 0.0f;
                    is_forward_ = true;
                    status_ = AnimationStatus::Forward;
                } else {
                    progress_ = 1.0f;
                }
            } else {
                progress_ = 0.0f;
                is_playing_ = false;
                status_ = AnimationStatus::Dismissed;
                stopTicker();
                applyProgress(progress_);
                notifyListeners();
                notifyStatusListeners();
                return false;
            }
        }
    }

    applyProgress(progress_);
    notifyListeners();
    return true;
}

void AnimationTimeline::ensureTicker() {
    if (ticker_id_ != 0) return;
    ticker_id_ = SchedulerBinding::instance().addFrameCallback([this] {
        tick();
    });
}

void AnimationTimeline::stopTicker() {
    if (ticker_id_ == 0) return;
    SchedulerBinding::instance().removeFrameCallback(ticker_id_);
    ticker_id_ = 0;
}

void AnimationTimeline::addListener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void AnimationTimeline::addStatusListener(StatusListener listener) {
    status_listeners_.push_back(std::move(listener));
}

void AnimationTimeline::clearListeners() {
    listeners_.clear();
    status_listeners_.clear();
}

void AnimationTimeline::dispose() {
    stopTicker();
    listeners_.clear();
    status_listeners_.clear();
    is_playing_ = false;
}

void AnimationTimeline::notifyListeners() {
    auto snapshot = listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn();
    }
}

void AnimationTimeline::notifyStatusListeners() {
    auto snapshot = status_listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn(status_);
    }
}

} // namespace enki
