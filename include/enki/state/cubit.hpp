#pragma once
/// @file cubit.hpp
/// @brief Cubit — lightweight reactive state management via direct method calls.
///
/// A Cubit is a simplified BLoC that exposes state changes through direct
/// function calls rather than an event-based approach. It is ideal for
/// simple state management scenarios where the ceremony of defining
/// event classes is unnecessary.
///
/// Architecture:
///   - Cubit<S> holds a Stream<S> internally.
///   - State changes are triggered by calling `emit(newState)`.
///   - Listeners are notified reactively.
///   - Lifecycle hooks (onChange, onError) provide observability.
///
/// Comparison with BLoC:
///   | Feature          | Cubit              | BLoC              |
///   |------------------|--------------------|--------------------|
///   | Input            | Method calls       | Events (add())     |
///   | Traceability     | Stack traces       | Event log          |
///   | Complexity       | Low                | Medium-High        |
///   | Best for         | Simple state       | Complex workflows  |
///
/// Usage:
/// @code
///   struct CounterState {
///       int count = 0;
///       bool operator==(const CounterState&) const = default;
///   };
///
///   class CounterCubit : public enki::Cubit<CounterState> {
///   public:
///       CounterCubit() : Cubit(CounterState{0}) {}
///
///       void increment() { emit({state().count + 1}); }
///       void decrement() { emit({state().count - 1}); }
///       void reset()     { emit({0}); }
///
///   protected:
///       void onChange(const Change<CounterState>& change) override {
///           // Optional: log state transitions
///       }
///   };
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/state/stream.hpp"
#include "enki/core/types.hpp"
#include <functional>
#include <string>
#include <type_traits>
#include <cassert>
#include <mutex>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Change<S> — represents a state transition
// ════════════════════════════════════════════════════════════════

/// @brief Represents a state transition from one value to another.
///
/// Change objects are passed to `onChange()` to provide full visibility
/// into state transitions, useful for logging, debugging, and analytics.
///
/// @tparam S The state type.
template<typename S>
struct Change {
    const S& current;   ///< The state before the transition.
    const S& next;      ///< The state after the transition.
};

// ════════════════════════════════════════════════════════════════
// BlocObserver — global observer for all BLoC/Cubit instances
// ════════════════════════════════════════════════════════════════

/// @brief Global observer for all Cubit and BLoC state changes.
///
/// BlocObserver provides a centralized place to monitor all state
/// management activity across the application. Useful for:
///   - Centralized logging
///   - Analytics
///   - Debugging
///   - State persistence
///
/// @code
///   class AppBlocObserver : public enki::BlocObserver {
///   public:
///       void onChangeAny(const std::string& cubit_name,
///                        const std::string& description) override {
///           std::cout << "[" << cubit_name << "] " << description << "\n";
///       }
///   };
///
///   // In main():
///   enki::BlocObserver::setGlobal(std::make_shared<AppBlocObserver>());
/// @endcode
class BlocObserver {
public:
    virtual ~BlocObserver() = default;

    /// Called when any Cubit/BLoC is created.
    /// @param name The type name of the Cubit/BLoC.
    virtual void onCreate(std::string_view name) {}

    /// Called when any Cubit/BLoC emits a state change.
    /// @param name        The type name of the Cubit/BLoC.
    /// @param description A human-readable description of the change.
    virtual void onChangeAny(std::string_view name, std::string_view description) {}

    /// Called when any Cubit/BLoC encounters an error.
    /// @param name  The type name of the Cubit/BLoC.
    /// @param error The error message.
    virtual void onErrorAny(std::string_view name, std::string_view error) {}

    /// Called when any Cubit/BLoC is closed.
    /// @param name The type name of the Cubit/BLoC.
    virtual void onClose(std::string_view name) {}

    /// Set the global BlocObserver instance.
    /// @param observer The observer to use globally. Pass nullptr to disable.
    static void setGlobal(std::shared_ptr<BlocObserver> observer) {
        global_observer_ = std::move(observer);
    }

    /// @return The current global BlocObserver, or nullptr if none is set.
    static BlocObserver* global() {
        return global_observer_.get();
    }

private:
    static inline std::shared_ptr<BlocObserver> global_observer_ = nullptr;
};

// ════════════════════════════════════════════════════════════════
// Cubit<S> — Lightweight Reactive State Manager
// ════════════════════════════════════════════════════════════════

/// @brief Lightweight state management via direct method calls.
///
/// Cubit is the recommended starting point for state management in ENKI.
/// It provides a simple, intuitive API for managing reactive state with
/// minimal boilerplate.
///
/// @tparam S The state type. Must satisfy StateType (copy-constructible,
///           default-constructible, equality-comparable).
template<StateType S>
class Cubit {
public:
    /// The state type, accessible for BlocBuilder/BlocProvider.
    using state_type = S;

    /// Construct a Cubit with an initial state.
    /// @param initial_state The initial state value.
    explicit Cubit(S initial_state)
        : stream_(std::move(initial_state)) {
        if (auto* obs = BlocObserver::global()) {
            obs->onCreate(typeName());
        }
    }

    /// Destructor — closes the internal stream.
    virtual ~Cubit() {
        if (!closed_) {
            close();
        }
    }

    // Non-copyable, non-movable (identity-bound)
    Cubit(const Cubit&) = delete;
    Cubit& operator=(const Cubit&) = delete;
    Cubit(Cubit&&) = delete;
    Cubit& operator=(Cubit&&) = delete;

    // ── State Access ───────────────────────────────────────────

    /// @return The current state.
    [[nodiscard]] const S& state() const {
        return stream_.current();
    }

    /// @return The previous state (before the last emission).
    [[nodiscard]] const S& previousState() const {
        return stream_.previous();
    }

    // ── State Emission ─────────────────────────────────────────

    /// Emit a new state.
    ///
    /// Triggers onChange() and notifies all stream listeners.
    /// Duplicate states are suppressed (distinct-by-default).
    ///
    /// @param new_state The new state to emit.
    void emit(S new_state) {
        if (closed_) return;

        // Check for distinct
        if (state() == new_state) return;

        const auto old_state = state();
        Change<S> change{old_state, new_state};

        // Notify the subclass
        onChange(change);

        // Notify global observer
        if (auto* obs = BlocObserver::global()) {
            obs->onChangeAny(typeName(), "State changed");
        }

        // Emit through the stream
        stream_.emit(std::move(new_state));
    }

    // ── Subscription ───────────────────────────────────────────

    /// Subscribe to state changes.
    /// @param listener The callback to invoke on state changes.
    /// @param replay   If true, immediately replay the current state.
    /// @return A SubscriptionId for managing the subscription.
    SubscriptionId listen(typename Stream<S>::Listener listener, bool replay = false) {
        return stream_.listen(std::move(listener), replay);
    }

    /// Subscribe with an RAII handle.
    /// @param listener The callback to invoke on state changes.
    /// @param replay   If true, immediately replay the current state.
    /// @return An RAII StreamSubscription.
    StreamSubscription<S> subscribe(typename Stream<S>::Listener listener, bool replay = false) {
        return stream_.subscribe(std::move(listener), replay);
    }

    /// Cancel a subscription.
    /// @param id The subscription ID to cancel.
    void cancelSubscription(SubscriptionId id) {
        stream_.cancel(id);
    }

    // ── Lifecycle ──────────────────────────────────────────────

    /// Close this Cubit. No more state changes will be accepted.
    /// All subscriptions are cancelled.
    void close() {
        if (closed_) return;
        closed_ = true;

        if (auto* obs = BlocObserver::global()) {
            obs->onClose(typeName());
        }

        stream_.close();
    }

    /// @return true if this Cubit is closed.
    [[nodiscard]] bool isClosed() const { return closed_; }

    // ── Internal Stream Access ─────────────────────────────────

    /// @return A reference to the internal stream. Used by BlocBuilder.
    Stream<S>& stream() { return stream_; }

    /// @return A const reference to the internal stream.
    const Stream<S>& stream() const { return stream_; }

    // ── Observable Hooks ───────────────────────────────────────

    /// @return A stable type name for this Cubit subclass.
    /// Override in subclasses for better logging.
    [[nodiscard]] virtual std::string_view typeName() const {
        return "Cubit";
    }

protected:
    /// Called when a new state is about to be emitted.
    ///
    /// Override for logging, analytics, or side effects.
    /// Called BEFORE the stream emission, so `state()` still returns
    /// the old state. Use `change.next` for the new state.
    ///
    /// @param change The state transition (current → next).
    virtual void onChange(const Change<S>& change) {}

    /// Called when an error occurs during emission or in a listener.
    ///
    /// @param error    The error message.
    /// @param trace    Optional stack trace or context.
    virtual void onError(std::string_view error, std::string_view trace = "") {
        if (auto* obs = BlocObserver::global()) {
            obs->onErrorAny(typeName(), error);
        }
    }

private:
    Stream<S> stream_;       ///< Internal state stream.
    bool closed_ = false;    ///< Closed flag.
};

}  // namespace enki
