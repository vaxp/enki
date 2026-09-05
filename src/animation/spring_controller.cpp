/// @file spring_controller.cpp
/// @brief SpringController implementation with Ticker binding.

#include "enki/animation/spring_controller.hpp"

namespace enki {

SpringController::SpringController(SpringDescription desc, float initial_value)
    : desc_(desc),
      current_value_(initial_value),
      target_value_(initial_value),
      sim_(desc, initial_value, initial_value, 0.0f) {}

SpringController::~SpringController() {
    dispose();
}

void SpringController::setSpring(const SpringDescription& desc) {
    desc_ = desc;
    if (is_animating_) {
        // Retarget with new spring parameters without resetting progress
        animateTo(target_value_);
    }
}

void SpringController::setTolerance(float distance_tol, float velocity_tol) {
    distance_tol_ = distance_tol;
    velocity_tol_ = velocity_tol;
}

void SpringController::animateTo(float target, std::optional<float> initial_velocity) {
    float start_val = current_value_;
    float start_vel = initial_velocity.value_or(current_velocity_);

    target_value_ = target;

    // Build fresh simulation from current dynamic state
    sim_ = SpringSimulation(desc_, start_val, target_value_, start_vel, distance_tol_, velocity_tol_);
    start_time_ = std::chrono::steady_clock::now();
    is_animating_ = true;

    status_ = (target_value_ >= start_val) ? AnimationStatus::Forward : AnimationStatus::Reverse;

    ensureTicker();
    notifyStatusListeners();
}

void SpringController::snapTo(float value) {
    stopTicker();
    current_value_ = value;
    current_velocity_ = 0.0f;
    target_value_ = value;
    is_animating_ = false;
    status_ = (value >= 1.0f) ? AnimationStatus::Completed : AnimationStatus::Dismissed;
    notifyListeners();
    notifyStatusListeners();
}

void SpringController::stop() {
    if (!is_animating_) return;
    stopTicker();
    current_velocity_ = 0.0f;
    is_animating_ = false;
    notifyStatusListeners();
}

void SpringController::reset() {
    snapTo(0.0f);
}

bool SpringController::tick() {
    if (!is_animating_) return false;

    auto now = std::chrono::steady_clock::now();
    double elapsed_s = std::chrono::duration<double>(now - start_time_).count();
    float t = static_cast<float>(elapsed_s);

    current_value_ = sim_.x(t);
    current_velocity_ = sim_.dx(t);

    notifyListeners();

    if (sim_.isDone(t)) {
        current_value_ = target_value_;
        current_velocity_ = 0.0f;
        is_animating_ = false;
        status_ = (target_value_ >= 1.0f) ? AnimationStatus::Completed : AnimationStatus::Dismissed;
        stopTicker();
        notifyStatusListeners();
        return false;
    }

    return true;
}

void SpringController::ensureTicker() {
    if (ticker_id_ != 0) return;
    ticker_id_ = SchedulerBinding::instance().addFrameCallback([this] {
        tick();
    });
}

void SpringController::stopTicker() {
    if (ticker_id_ == 0) return;
    SchedulerBinding::instance().removeFrameCallback(ticker_id_);
    ticker_id_ = 0;
}

void SpringController::addListener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void SpringController::addStatusListener(StatusListener listener) {
    status_listeners_.push_back(std::move(listener));
}

void SpringController::clearListeners() {
    listeners_.clear();
    status_listeners_.clear();
}

void SpringController::dispose() {
    stopTicker();
    listeners_.clear();
    status_listeners_.clear();
    is_animating_ = false;
}

void SpringController::notifyListeners() {
    auto snapshot = listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn();
    }
}

void SpringController::notifyStatusListeners() {
    auto snapshot = status_listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn(status_);
    }
}

} // namespace enki
