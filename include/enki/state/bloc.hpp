#pragma once
/// @file bloc.hpp
/// @brief BLoC — Business Logic Component for event-driven state management.
///
/// BLoC extends Cubit with a formal event-driven architecture. Instead of
/// calling methods directly, state changes are triggered by adding events
/// via `add()`. Each event type has a registered handler that processes it
/// and emits new states.
///
/// This pattern provides:
///   - **Full traceability**: Every state change is associated with an event.
///   - **Separation of concerns**: Events describe WHAT happened, handlers
///     decide HOW to respond.
///   - **Testability**: Events can be replayed, recorded, and verified.
///   - **Transition tracking**: Every state change produces a Transition
///     record (event + old state → new state).
///
/// When to use BLoC vs Cubit:
///   - Use **Cubit** for simple state (counters, toggles, form fields).
///   - Use **BLoC** for complex workflows (auth flows, multi-step forms,
///     real-time data, undo/redo).
///
/// Usage:
/// @code
///   // 1. Define events
///   struct IncrementEvent : enki::BlocEvent {
///       std::string_view typeName() const override { return "IncrementEvent"; }
///   };
///
///   struct DecrementEvent : enki::BlocEvent {
///       std::string_view typeName() const override { return "DecrementEvent"; }
///   };
///
///   // 2. Define state
///   struct CounterState {
///       int value = 0;
///       bool operator==(const CounterState&) const = default;
///   };
///
///   // 3. Define BLoC
///   class CounterBloc : public enki::Bloc<CounterState> {
///   public:
///       CounterBloc() : Bloc(CounterState{0}) {
///           on<IncrementEvent>([this](const IncrementEvent&, auto emit) {
///               emit({state().value + 1});
///           });
///           on<DecrementEvent>([this](const DecrementEvent&, auto emit) {
///               emit({state().value - 1});
///           });
///       }
///       std::string_view typeName() const override { return "CounterBloc"; }
///   };
///
///   // 4. Use
///   CounterBloc bloc;
///   bloc.add(std::make_unique<IncrementEvent>());
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/state/cubit.hpp"
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <queue>
#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <cassert>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// BlocEvent — base class for all events
// ════════════════════════════════════════════════════════════════

/// @brief Base class for all BLoC events.
///
/// Events represent user actions, system events, or any input that
/// triggers a state change. Each event type must provide a unique
/// `typeName()` for handler dispatch and debugging.
///
/// Events are immutable value objects. They describe WHAT happened,
/// not HOW to respond.
struct BlocEvent {
    virtual ~BlocEvent() = default;

    /// @return A stable, unique type name for this event class.
    /// Used for handler dispatch and logging.
    [[nodiscard]] virtual std::string_view typeName() const = 0;

    /// @return A human-readable debug description of this event.
    [[nodiscard]] virtual std::string debugDescription() const {
        return std::string(typeName());
    }
};

// ════════════════════════════════════════════════════════════════
// Transition<S> — records a state transition caused by an event
// ════════════════════════════════════════════════════════════════

/// @brief Records a complete state transition: event + old state → new state.
///
/// Transitions provide full audit trail for state changes. They are
/// passed to `onTransition()` for logging and debugging.
///
/// @tparam S The state type.
template<typename S>
struct Transition {
    const S& currentState;       ///< State before the event.
    const BlocEvent& event;      ///< The event that caused the transition.
    const S& nextState;          ///< State after the event.
};

// ════════════════════════════════════════════════════════════════
// Bloc<S> — Event-driven State Manager
// ════════════════════════════════════════════════════════════════

/// @brief Event-driven state management built on top of Cubit.
///
/// Bloc adds an event layer on top of Cubit. Instead of calling
/// methods directly, you `add()` events which are dispatched to
/// registered handlers. Handlers can emit one or more states.
///
/// @tparam S The state type. Must satisfy StateType.
template<StateType S>
class Bloc : public Cubit<S> {
public:
    using Cubit<S>::Cubit;

    /// Callback type for event handlers.
    /// @param event The event to handle.
    /// @param emit  A function to call to emit new states.
    template<typename E>
    using EventHandler = std::function<void(const E& event, std::function<void(S)> emit)>;

    /// @return A stable type name. Override for better logging.
    [[nodiscard]] std::string_view typeName() const override {
        return "Bloc";
    }

    // ── Event Registration ─────────────────────────────────────

    /// Register a handler for a specific event type.
    ///
    /// Only one handler per event type is allowed. Registering a second
    /// handler for the same type will overwrite the first.
    ///
    /// The handler receives the event and an `emit` function. Call `emit`
    /// with a new state to trigger a state change. The handler may call
    /// `emit` zero, one, or multiple times.
    ///
    /// @tparam E The event type. Must derive from BlocEvent.
    /// @param handler The function to handle events of type E.
    ///
    /// @code
    ///   on<LoginRequestedEvent>([this](const auto& event, auto emit) {
    ///       emit(LoadingState{});
    ///       // ... perform async login ...
    ///       emit(LoggedInState{user});
    ///   });
    /// @endcode
    template<typename E>
    void on(EventHandler<E> handler) {
        static_assert(std::derived_from<E, BlocEvent>,
                      "Event type must derive from BlocEvent");

        auto type_idx = std::type_index(typeid(E));

        handlers_[type_idx] = [this, handler = std::move(handler)](
                                  const BlocEvent& event,
                                  std::function<void(S)> emitter) {
            handler(static_cast<const E&>(event), std::move(emitter));
        };
    }

    // ── Event Dispatch ─────────────────────────────────────────

    /// Add an event to be processed.
    ///
    /// The event is dispatched to the registered handler for its type.
    /// If no handler is registered, the event is silently dropped
    /// (in debug builds, an assertion fires).
    ///
    /// Events are processed synchronously in the order they are added.
    ///
    /// @param event The event to process.
    void add(std::unique_ptr<BlocEvent> event) {
        if (this->isClosed()) return;

        assert(event && "Cannot add a null event");

        // Notify onEvent hook
        onEvent(*event);

        // Notify global observer
        if (auto* obs = BlocObserver::global()) {
            obs->onChangeAny(typeName(),
                std::string("Event: ") + std::string(event->typeName()));
        }

        // Find handler
        auto type_idx = std::type_index(typeid(*event));
        auto it = handlers_.find(type_idx);

        if (it == handlers_.end()) {
            // No handler registered for this event type
            assert(false && "No handler registered for this event type. "
                           "Did you forget to call on<EventType>() in the constructor?");
            return;
        }

        // Create the emit function that tracks transitions
        auto emitter = [this, &event](S new_state) {
            if (this->isClosed()) return;

            const auto old_state = this->state();

            // Skip if same state (distinct check)
            if (old_state == new_state) return;

            // Create transition record
            Transition<S> transition{old_state, *event, new_state};
            onTransition(transition);

            // Notify global observer
            if (auto* obs = BlocObserver::global()) {
                obs->onChangeAny(this->typeName(), "Transition via " +
                    std::string(event->typeName()));
            }

            // Emit through Cubit's stream (bypasses Cubit::onChange since
            // we already tracked via onTransition)
            this->stream().emit(std::move(new_state));
        };

        // Dispatch to handler
        try {
            it->second(*event, emitter);
        } catch (const std::exception& e) {
            this->onError(e.what(), event->typeName());
        } catch (...) {
            this->onError("Unknown error in event handler", event->typeName());
        }
    }

    /// Convenience: add an event constructed in-place.
    ///
    /// @tparam E    The event type.
    /// @tparam Args Constructor argument types.
    /// @param args  Arguments to forward to the event constructor.
    ///
    /// @code
    ///   bloc.add<IncrementEvent>();
    ///   bloc.add<SetValueEvent>(42);
    /// @endcode
    template<typename E, typename... Args>
    void add(Args&&... args) {
        static_assert(std::derived_from<E, BlocEvent>,
                      "Event type must derive from BlocEvent");
        add(std::make_unique<E>(std::forward<Args>(args)...));
    }

    // ── Observable Hooks ───────────────────────────────────────

protected:
    /// Called when an event is added (before dispatching to handler).
    ///
    /// Override for event logging, analytics, or validation.
    ///
    /// @param event The event that was added.
    virtual void onEvent(const BlocEvent& event) {}

    /// Called when a state transition occurs.
    ///
    /// Override for transition logging, undo/redo tracking, or analytics.
    /// Called after the handler emits a new state but before listeners
    /// are notified.
    ///
    /// @param transition The complete transition record.
    virtual void onTransition(const Transition<S>& transition) {}

private:
    /// Type-erased event handler.
    using HandlerFn = std::function<void(const BlocEvent&, std::function<void(S)>)>;

    /// Map from event type to handler.
    std::unordered_map<std::type_index, HandlerFn> handlers_;
};

}  // namespace enki
