#pragma once
/// @file ticker.hpp
/// @brief Ticker — per-frame callback system for animations.
///
/// A Ticker fires a callback on every frame. AnimationControllers
/// register with the app's global Ticker so they can advance their
/// progress each frame without needing direct App access.
///
/// The App owns one SchedulerBinding (like  's SchedulerBinding)
/// that drives all registered Tickers on each frame.
///
/// ## Usage
/// @code
///   // In State::initState():
///   ticker_ = createTicker([this](Duration elapsed) {
///       controller_.tick();
///   });
///   ticker_->start();
///
///   // In State::dispose():
///   ticker_->stop();
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include <functional>
#include <chrono>
#include <vector>
#include <memory>

namespace enki {

using FrameCallback = std::function<void()>;

// ════════════════════════════════════════════════════════════════
// SchedulerBinding — drives all tickers every frame
// ════════════════════════════════════════════════════════════════

/// @brief Global animation scheduler — owned by the App.
/// Holds a list of per-frame callbacks and fires them each frame.
class SchedulerBinding {
public:
    /// Register a callback to be called every frame.
    /// Returns an ID that can be used to cancel it.
    size_t addFrameCallback(FrameCallback cb) {
        size_t id = next_id_++;
        callbacks_.push_back({id, std::move(cb)});
        return id;
    }

    /// Remove a previously registered callback.
    void removeFrameCallback(size_t id) {
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [id](const Entry& e) { return e.id == id; }),
            callbacks_.end());
    }

    /// Called once per frame by App — fires all registered callbacks.
    void tick() {
        // Copy to avoid issues if callbacks modify the list
        auto snapshot = callbacks_;
        for (auto& entry : snapshot) {
            entry.callback();
        }
    }

    /// @return true if there are any active callbacks.
    [[nodiscard]] bool hasCallbacks() const { return !callbacks_.empty(); }

    /// @return The number of currently active frame callbacks (Tickers / AnimationControllers).
    [[nodiscard]] size_t tickerCount() const { return callbacks_.size(); }

    /// @return Global singleton instance (set by App on startup).
    static SchedulerBinding& instance() {
        static SchedulerBinding singleton;
        return singleton;
    }

private:
    struct Entry {
        size_t id;
        FrameCallback callback;
    };

    std::vector<Entry> callbacks_;
    size_t next_id_ = 1;
};

// ════════════════════════════════════════════════════════════════
// Ticker — a single per-frame listener
// ════════════════════════════════════════════════════════════════

/// @brief A single animation tick listener.
/// Created via createTicker() inside State objects.
class Ticker {
public:
    explicit Ticker(FrameCallback callback)
        : callback_(std::move(callback)) {}

    ~Ticker() { stop(); }

    Ticker(const Ticker&) = delete;
    Ticker& operator=(const Ticker&) = delete;

    /// Start firing the callback every frame.
    void start() {
        if (active_) return;
        active_ = true;
        id_ = SchedulerBinding::instance().addFrameCallback(callback_);
    }

    /// Stop firing the callback.
    void stop() {
        if (!active_) return;
        active_ = false;
        SchedulerBinding::instance().removeFrameCallback(id_);
    }

    [[nodiscard]] bool isActive() const { return active_; }

private:
    FrameCallback callback_;
    size_t id_ = 0;
    bool active_ = false;
};

/// @brief Create a Ticker with the given per-frame callback.
inline std::unique_ptr<Ticker> createTicker(FrameCallback callback) {
    return std::make_unique<Ticker>(std::move(callback));
}

}  // namespace enki
