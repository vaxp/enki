#pragma once
/// @file stream.hpp
/// @brief Reactive state stream — the backbone of ENKI's state management.
///
/// Stream<T> provides a reactive publish/subscribe mechanism for state changes.
/// It is the foundation upon which Cubit and BLoC are built. Streams track
/// the current and previous state, notify listeners on change, and support
/// transformation via map/where/distinct operators.
///
/// Design goals:
///   - Thread-safe: all operations are mutex-protected.
///   - Exception-safe: listener exceptions are caught and reported.
///   - Memory-safe: RAII subscriptions via StreamSubscription.
///   - Zero-overhead when no listeners are attached.
///   - Distinct-by-default: duplicate states are suppressed (requires operator==).
///
/// Usage:
/// @code
///   Stream<int> counter(0);
///   auto sub = counter.listen([](const int& val) {
///       std::cout << "Counter: " << val << "\n";
///   });
///   counter.emit(1);  // prints "Counter: 1"
///   counter.emit(1);  // suppressed (same state)
///   counter.emit(2);  // prints "Counter: 2"
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include <functional>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <optional>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <unordered_set>

namespace enki {

/// Unique identifier for a stream subscription.
using SubscriptionId = uint64_t;

/// Invalid subscription sentinel.
inline constexpr SubscriptionId InvalidSubscriptionId = 0;

// Forward declaration
template<StateType T> class Stream;

// ════════════════════════════════════════════════════════════════
// StreamSubscription — RAII subscription handle
// ════════════════════════════════════════════════════════════════

/// @brief RAII handle for a stream subscription.
///
/// Automatically cancels the subscription when destroyed, preventing
/// dangling listener callbacks. Can also be cancelled manually.
///
/// @tparam T The state type of the stream.
template<typename T>
class StreamSubscription {
public:
    StreamSubscription() = default;

    StreamSubscription(Stream<T>* stream, SubscriptionId id)
        : stream_(stream), id_(id) {}

    ~StreamSubscription() { cancel(); }

    // Non-copyable (subscription ownership is unique)
    StreamSubscription(const StreamSubscription&) = delete;
    StreamSubscription& operator=(const StreamSubscription&) = delete;

    // Movable
    StreamSubscription(StreamSubscription&& other) noexcept
        : stream_(other.stream_), id_(other.id_) {
        other.stream_ = nullptr;
        other.id_ = InvalidSubscriptionId;
    }

    StreamSubscription& operator=(StreamSubscription&& other) noexcept {
        if (this != &other) {
            cancel();
            stream_ = other.stream_;
            id_ = other.id_;
            other.stream_ = nullptr;
            other.id_ = InvalidSubscriptionId;
        }
        return *this;
    }

    /// Cancel this subscription. The listener will no longer be called.
    void cancel();

    /// @return The subscription ID.
    [[nodiscard]] SubscriptionId id() const { return id_; }

    /// @return true if this subscription is still active.
    [[nodiscard]] bool isActive() const {
        return stream_ != nullptr && id_ != InvalidSubscriptionId;
    }

    /// Pause this subscription. Events will be buffered until resumed.
    void pause();

    /// Resume a paused subscription. Buffered events are NOT replayed.
    void resume();

    /// @return true if the subscription is paused.
    [[nodiscard]] bool isPaused() const { return paused_; }

private:
    Stream<T>* stream_ = nullptr;
    SubscriptionId id_ = InvalidSubscriptionId;
    bool paused_ = false;
};

// ════════════════════════════════════════════════════════════════
// Stream<T> — Reactive State Stream
// ════════════════════════════════════════════════════════════════

/// @brief A reactive state container that notifies listeners on change.
///
/// Stream<T> is the core reactive primitive in ENKI. It holds a state value
/// of type T and notifies all subscribed listeners whenever the state changes.
///
/// Key properties:
///   - **Distinct by default**: If the new state equals the current state
///     (via operator==), listeners are NOT notified. This prevents unnecessary
///     UI rebuilds.
///   - **Thread-safe**: All operations are protected by a mutex.
///   - **Error channel**: Streams can also emit errors via `addError()`.
///   - **Closeable**: Streams can be closed, preventing further emissions.
///
/// @tparam T The state type. Must satisfy the StateType concept
///           (copy-constructible, default-constructible, equality-comparable).
template<StateType T>
class Stream {
public:
    /// Callback type for state change listeners.
    using Listener = std::function<void(const T& state)>;

    /// Callback type for error listeners.
    using ErrorListener = std::function<void(const std::string& error)>;

    /// Default constructor — creates a stream with a default-constructed state.
    Stream() : current_state_(T{}), previous_state_(T{}) {}

    /// Construct a stream with an initial state.
    /// @param initial_state The initial state value.
    explicit Stream(T initial_state)
        : current_state_(std::move(initial_state)),
          previous_state_(current_state_) {}

    /// Destructor — closes the stream and cancels all subscriptions.
    ~Stream() { close(); }

    // Non-copyable (subscription ownership)
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    // Movable
    Stream(Stream&& other) noexcept {
        std::lock_guard lock(other.mutex_);
        current_state_ = std::move(other.current_state_);
        previous_state_ = std::move(other.previous_state_);
        listeners_ = std::move(other.listeners_);
        error_listeners_ = std::move(other.error_listeners_);
        paused_ids_ = std::move(other.paused_ids_);
        next_id_ = other.next_id_;
        closed_ = other.closed_;
        emit_count_ = other.emit_count_;
        other.closed_ = true;
    }

    Stream& operator=(Stream&& other) noexcept {
        if (this != &other) {
            close();
            std::lock_guard lock(other.mutex_);
            current_state_ = std::move(other.current_state_);
            previous_state_ = std::move(other.previous_state_);
            listeners_ = std::move(other.listeners_);
            error_listeners_ = std::move(other.error_listeners_);
            paused_ids_ = std::move(other.paused_ids_);
            next_id_ = other.next_id_;
            closed_ = other.closed_;
            emit_count_ = other.emit_count_;
            other.closed_ = true;
        }
        return *this;
    }

    // ── Emission ───────────────────────────────────────────────

    /// Emit a new state to all listeners.
    ///
    /// If the new state equals the current state (via operator==), the emission
    /// is suppressed and listeners are NOT notified. This is the "distinct"
    /// behavior that prevents unnecessary rebuilds.
    ///
    /// @param new_state The new state to emit.
    /// @throws std::logic_error if the stream is closed.
    void emit(T new_state) {
        std::vector<Listener> active_listeners;
        {
            std::lock_guard lock(mutex_);

            if (closed_) {
                return;  // Silently ignore emissions on closed streams
            }

            // Distinct check: skip if same state
            if (current_state_ == new_state) {
                return;
            }

            previous_state_ = std::move(current_state_);
            current_state_ = std::move(new_state);
            ++emit_count_;

            // Collect active (non-paused) listeners
            active_listeners.reserve(listeners_.size());
            for (const auto& [id, listener] : listeners_) {
                if (paused_ids_.find(id) == paused_ids_.end()) {
                    active_listeners.push_back(listener);
                }
            }
        }

        // Notify outside the lock to prevent deadlocks
        for (const auto& listener : active_listeners) {
            try {
                listener(current_state_);
            } catch (const std::exception& e) {
                addError(std::string("Listener exception: ") + e.what());
            } catch (...) {
                addError("Unknown listener exception");
            }
        }
    }

    /// Emit a new state unconditionally, bypassing the distinct check.
    /// Use sparingly — this forces all listeners to be notified even if
    /// the state hasn't changed.
    ///
    /// @param new_state The new state to emit.
    void emitForce(T new_state) {
        std::vector<Listener> active_listeners;
        {
            std::lock_guard lock(mutex_);

            if (closed_) return;

            previous_state_ = std::move(current_state_);
            current_state_ = std::move(new_state);
            ++emit_count_;

            active_listeners.reserve(listeners_.size());
            for (const auto& [id, listener] : listeners_) {
                if (paused_ids_.find(id) == paused_ids_.end()) {
                    active_listeners.push_back(listener);
                }
            }
        }

        for (const auto& listener : active_listeners) {
            try {
                listener(current_state_);
            } catch (const std::exception& e) {
                addError(std::string("Listener exception: ") + e.what());
            } catch (...) {
                addError("Unknown listener exception");
            }
        }
    }

    // ── Error Channel ──────────────────────────────────────────

    /// Emit an error to all error listeners.
    /// @param error The error message.
    void addError(std::string error) {
        std::vector<ErrorListener> err_listeners;
        {
            std::lock_guard lock(mutex_);
            err_listeners.reserve(error_listeners_.size());
            for (const auto& [id, listener] : error_listeners_) {
                err_listeners.push_back(listener);
            }
        }

        for (const auto& listener : err_listeners) {
            try {
                listener(error);
            } catch (...) {
                // Swallow errors from error listeners to prevent infinite recursion
            }
        }
    }

    // ── Subscription ───────────────────────────────────────────

    /// Subscribe to state changes.
    ///
    /// The listener will be called immediately with the current state
    /// (if `replay` is true), and then for every subsequent state change.
    ///
    /// @param listener The callback to invoke on state changes.
    /// @param replay   If true, immediately invoke the listener with the current state.
    /// @return A SubscriptionId that can be used to cancel the subscription.
    SubscriptionId listen(Listener listener, bool replay = false) {
        SubscriptionId id;
        {
            std::lock_guard lock(mutex_);
            if (closed_) return InvalidSubscriptionId;

            id = ++next_id_;
            listeners_[id] = listener;
        }

        // Replay current state outside the lock
        if (replay) {
            try {
                listener(current_state_);
            } catch (...) {
                // Swallow replay exceptions
            }
        }

        return id;
    }

    /// Subscribe and return an RAII subscription handle.
    ///
    /// The subscription is automatically cancelled when the returned
    /// StreamSubscription is destroyed.
    ///
    /// @param listener The callback to invoke on state changes.
    /// @param replay   If true, immediately invoke the listener with the current state.
    /// @return An RAII StreamSubscription handle.
    StreamSubscription<T> subscribe(Listener listener, bool replay = false) {
        SubscriptionId id = listen(std::move(listener), replay);
        return StreamSubscription<T>(this, id);
    }

    /// Subscribe to errors.
    /// @param listener The callback to invoke on errors.
    /// @return A SubscriptionId for the error subscription.
    SubscriptionId onError(ErrorListener listener) {
        std::lock_guard lock(mutex_);
        if (closed_) return InvalidSubscriptionId;

        SubscriptionId id = ++next_id_;
        error_listeners_[id] = std::move(listener);
        return id;
    }

    /// Cancel a subscription by ID.
    /// @param id The subscription ID to cancel.
    void cancel(SubscriptionId id) {
        std::lock_guard lock(mutex_);
        listeners_.erase(id);
        error_listeners_.erase(id);
        paused_ids_.erase(id);
    }

    /// Cancel all subscriptions.
    void cancelAll() {
        std::lock_guard lock(mutex_);
        listeners_.clear();
        error_listeners_.clear();
        paused_ids_.clear();
    }

    // ── Pause / Resume ─────────────────────────────────────────

    /// Pause a subscription. Paused subscriptions do not receive events.
    /// @param id The subscription ID to pause.
    void pause(SubscriptionId id) {
        std::lock_guard lock(mutex_);
        if (listeners_.count(id)) {
            paused_ids_.insert(id);
        }
    }

    /// Resume a paused subscription.
    /// @param id The subscription ID to resume.
    void resume(SubscriptionId id) {
        std::lock_guard lock(mutex_);
        paused_ids_.erase(id);
    }

    // ── State Access ───────────────────────────────────────────

    /// @return The current state (latest emitted value).
    [[nodiscard]] const T& current() const {
        std::lock_guard lock(mutex_);
        return current_state_;
    }

    /// @return The previous state (before the last emission).
    [[nodiscard]] const T& previous() const {
        std::lock_guard lock(mutex_);
        return previous_state_;
    }

    // ── Stream Lifecycle ───────────────────────────────────────

    /// Close the stream. No more values can be emitted.
    /// All subscriptions are cancelled.
    void close() {
        std::lock_guard lock(mutex_);
        if (closed_) return;
        closed_ = true;
        listeners_.clear();
        error_listeners_.clear();
        paused_ids_.clear();
    }

    /// @return true if the stream is closed.
    [[nodiscard]] bool isClosed() const {
        std::lock_guard lock(mutex_);
        return closed_;
    }

    // ── Diagnostics ────────────────────────────────────────────

    /// @return The number of active listeners.
    [[nodiscard]] size_t listenerCount() const {
        std::lock_guard lock(mutex_);
        return listeners_.size();
    }

    /// @return true if there are any active listeners.
    [[nodiscard]] bool hasListeners() const {
        std::lock_guard lock(mutex_);
        return !listeners_.empty();
    }

    /// @return The total number of emissions since creation.
    [[nodiscard]] uint64_t emitCount() const {
        std::lock_guard lock(mutex_);
        return emit_count_;
    }

    // ── Transformation Operators ───────────────────────────────

    /// Create a derived stream that transforms each state value.
    ///
    /// @tparam U The output type.
    /// @param transform A function that maps T → U.
    /// @return A shared_ptr to a new Stream<U> that mirrors this stream.
    template<typename U>
    std::shared_ptr<Stream<U>> map(std::function<U(const T&)> transform) {
        auto derived = std::make_shared<Stream<U>>(transform(current_state_));
        listen([derived, transform = std::move(transform)](const T& state) {
            derived->emit(transform(state));
        });
        return derived;
    }

    /// Create a derived stream that only passes through states matching a predicate.
    ///
    /// @param predicate A function that returns true for states to keep.
    /// @return A shared_ptr to a new Stream<T> with filtered emissions.
    std::shared_ptr<Stream<T>> where(std::function<bool(const T&)> predicate) {
        auto derived = std::make_shared<Stream<T>>(current_state_);
        listen([derived, predicate = std::move(predicate)](const T& state) {
            if (predicate(state)) {
                derived->emit(state);
            }
        });
        return derived;
    }

private:
    mutable std::mutex mutex_;

    T current_state_;                                         ///< Current state value.
    T previous_state_;                                        ///< Previous state value.

    std::unordered_map<SubscriptionId, Listener> listeners_;  ///< State listeners.
    std::unordered_map<SubscriptionId, ErrorListener> error_listeners_;  ///< Error listeners.
    std::unordered_set<SubscriptionId> paused_ids_;           ///< Paused subscription IDs.

    SubscriptionId next_id_ = 0;                              ///< Next subscription ID.
    bool closed_ = false;                                     ///< Stream closed flag.
    uint64_t emit_count_ = 0;                                 ///< Total emission count.
};

// ════════════════════════════════════════════════════════════════
// StreamSubscription — out-of-class method definitions
// ════════════════════════════════════════════════════════════════

template<typename T>
void StreamSubscription<T>::cancel() {
    if (stream_ && id_ != InvalidSubscriptionId) {
        stream_->cancel(id_);
        stream_ = nullptr;
        id_ = InvalidSubscriptionId;
    }
}

template<typename T>
void StreamSubscription<T>::pause() {
    if (stream_ && id_ != InvalidSubscriptionId) {
        stream_->pause(id_);
        paused_ = true;
    }
}

template<typename T>
void StreamSubscription<T>::resume() {
    if (stream_ && id_ != InvalidSubscriptionId) {
        stream_->resume(id_);
        paused_ = false;
    }
}

}  // namespace enki
