/// @file lottie_controller.cpp
/// @brief Implementation of LottieController playback and timing.
/// @copyright ENKI Framework — MIT License

#include "enki/animation/lottie_controller.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

LottieController::LottieController(std::shared_ptr<LottieComposition> composition,
                                   Duration duration_override)
    : composition_(std::move(composition)) {
    if (composition_) {
        composition_->seek(0.0f);
    }
}

LottieController::~LottieController() {
    dispose();
}

void LottieController::dispose() {
    if (alive_) {
        *alive_ = false;
    }
    if (ticker_id_ != 0) {
        SchedulerBinding::instance().removeFrameCallback(ticker_id_);
        ticker_id_ = 0;
    }
    is_playing_ = false;
    listeners_.clear();
}

size_t LottieController::addListener(Listener listener) {
    size_t id = next_listener_id_++;
    listeners_.push_back({id, std::move(listener)});
    return id;
}

void LottieController::removeListener(size_t id) {
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(),
            [id](const ListenerEntry& e) { return e.id == id; }),
        listeners_.end());
}

void LottieController::clearListeners() {
    listeners_.clear();
}

void LottieController::setComposition(std::shared_ptr<LottieComposition> comp) {
    composition_ = std::move(comp);
    if (composition_) {
        composition_->seek(progress_);
    }
}

void LottieController::ensureTicker() {
    if (ticker_id_ == 0) {
        std::weak_ptr<bool> alive_weak = alive_;
        ticker_id_ = SchedulerBinding::instance().addFrameCallback([this, alive_weak]() {
            auto alive = alive_weak.lock();
            if (!alive || !*alive) return;
            if (!is_playing_) return;
            auto now = std::chrono::steady_clock::now();
            double delta = std::chrono::duration<double>(now - last_tick_time_).count();
            last_tick_time_ = now;
            tick(delta);
        });
    }
}

void LottieController::notifyListeners() {
    if (!alive_ || !*alive_) return;
    auto snapshot = listeners_;
    for (auto& entry : snapshot) {
        if (!alive_ || !*alive_) return;
        if (entry.fn) entry.fn();
    }
}

void LottieController::play() {
    if (is_playing_) return;
    is_playing_ = true;
    last_tick_time_ = std::chrono::steady_clock::now();
    ensureTicker();
    if (on_start_) on_start_();
}

void LottieController::pause() {
    is_playing_ = false;
}

void LottieController::stop() {
    is_playing_ = false;
    reset();
}

void LottieController::reset() {
    progress_ = segment_start_;
    is_forward_ = true;
    loop_count_ = 0;
    if (composition_) {
        composition_->seek(progress_);
    }
    notifyListeners();
}

void LottieController::forward() {
    is_forward_ = true;
    play();
}

void LottieController::reverse() {
    is_forward_ = false;
    play();
}

void LottieController::seek(float progress) {
    progress_ = std::clamp(progress, 0.0f, 1.0f);
    if (composition_) {
        composition_->seek(progress_);
    }
    notifyListeners();
}

void LottieController::seekFrame(double frame_index) {
    if (composition_ && composition_->frameCount() > 0) {
        float p = static_cast<float>(frame_index / composition_->frameCount());
        seek(p);
    }
}

void LottieController::playSegment(float start_progress, float end_progress, bool repeat) {
    segment_start_ = std::clamp(start_progress, 0.0f, 1.0f);
    segment_end_   = std::clamp(end_progress, 0.0f, 1.0f);
    if (segment_end_ < segment_start_) {
        std::swap(segment_start_, segment_end_);
    }
    if (std::abs(segment_end_ - segment_start_) < 0.001f) {
        segment_end_ = std::min(1.0f, segment_start_ + 0.01f);
    }
    mode_ = repeat ? LottiePlaybackMode::Loop : LottiePlaybackMode::Segment;
    progress_ = segment_start_;
    forward();
}

bool LottieController::playMarker(std::string_view marker_name, bool repeat) {
    if (!composition_) return false;
    auto marker = composition_->getMarker(marker_name);
    if (!marker.has_value()) return false;

    double dur = composition_->duration();
    if (dur <= 0.0001) return false;

    float start_p = static_cast<float>(marker->start_time / dur);
    float end_p   = static_cast<float>(marker->end_time / dur);

    playSegment(start_p, end_p, repeat);
    if (on_marker_) on_marker_(marker_name);
    return true;
}

void LottieController::tick(double delta_seconds) {
    if (!is_playing_) return;

    double comp_duration = composition_ ? composition_->duration() : 1.0;
    if (comp_duration <= 0.0001) comp_duration = 1.0;

    float progress_delta = static_cast<float>((delta_seconds / comp_duration) * std::abs(speed_));
    float span = segment_end_ - segment_start_;
    if (span <= 0.0001f) span = 1.0f;

    if (is_forward_) {
        progress_ += progress_delta;
        if (progress_ >= segment_end_) {
            if (mode_ == LottiePlaybackMode::Loop) {
                float overshoot = progress_ - segment_end_;
                progress_ = segment_start_ + std::fmod(overshoot, span);
                loop_count_++;
                if (on_loop_) on_loop_(loop_count_);
            } else if (mode_ == LottiePlaybackMode::PingPong) {
                progress_ = segment_end_;
                is_forward_ = false;
                loop_count_++;
                if (on_loop_) on_loop_(loop_count_);
            } else {
                progress_ = segment_end_;
                is_playing_ = false;
                if (on_end_) on_end_();
            }
        }
    } else {
        progress_ -= progress_delta;
        if (progress_ <= segment_start_) {
            if (mode_ == LottiePlaybackMode::Loop) {
                float undershoot = segment_start_ - progress_;
                progress_ = segment_end_ - std::fmod(undershoot, span);
                loop_count_++;
                if (on_loop_) on_loop_(loop_count_);
            } else if (mode_ == LottiePlaybackMode::PingPong) {
                progress_ = segment_start_;
                is_forward_ = true;
                loop_count_++;
                if (on_loop_) on_loop_(loop_count_);
            } else {
                progress_ = segment_start_;
                is_playing_ = false;
                if (on_end_) on_end_();
            }
        }
    }

    if (composition_) {
        composition_->seek(progress_);
    }

    notifyListeners();
}

double LottieController::currentFrame() const {
    if (!composition_) return 0.0;
    return progress_ * composition_->frameCount();
}

double LottieController::currentTime() const {
    if (!composition_) return 0.0;
    return progress_ * composition_->duration();
}

} // namespace enki
