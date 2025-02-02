#pragma once
/// @file state.hpp
/// @brief State base class — manages mutable state for StatefulWidgets.
///
/// State<T> is the C++20 equivalent of  's State class. It holds
/// the mutable data for a StatefulWidget and provides lifecycle hooks
/// for initialization, updates, and cleanup.
///
/// Key differences from  :
///   - State is not parameterized on Widget type (uses runtime polymorphism).
///   - setState() accepts a lambda that modifies state atomically.
///   - Lifecycle hooks follow C++ RAII patterns.
///
/// Lifecycle:
///   1. `createState()` — called by StatefulWidget, creates the State object.
///   2. `initState()` — called after mount, before the first build.
///   3. `build()` — called to produce the widget subtree.
///   4. `didUpdateWidget()` — called when the parent widget is replaced (same type).
///   5. `didChangeDependencies()` — called when inherited data changes.
///   6. `deactivate()` — called when temporarily removed from the tree.
///   7. `dispose()` — called when permanently removed. Release resources here.
///
/// Usage:
/// @code
///   class CounterState : public enki::State {
///       int count_ = 0;
///   public:
///       void initState() override {
///           State::initState();
///           // Initialize resources, start animations, etc.
///       }
///
///       WidgetPtr build(BuildContext& ctx) override {
///           return text(std::to_string(count_));
///       }
///
///       void incrementCounter() {
///           setState([this] { count_++; });
///       }
///
///       void dispose() override {
///           // Release resources
///           State::dispose();
///       }
///   };
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include <functional>
#include <cassert>
#include <string>

namespace enki {

// Forward declarations
class StatefulElement;
class StatefulWidget;
class BuildContext;

// ════════════════════════════════════════════════════════════════
// State — Base class for widget state
// ════════════════════════════════════════════════════════════════

/// @brief Base class for mutable state associated with a StatefulWidget.
///
/// State objects are long-lived — they persist across widget rebuilds as
/// long as the element remains in the tree. This is where you store data
/// that changes over time and affects the UI.
///
/// Important rules:
///   - Always call the superclass lifecycle methods when overriding.
///   - Only call `setState()` when the state actually changes.
///   - Never call `setState()` during `build()`.
///   - Release all resources in `dispose()`.
///   - After `dispose()`, the State is defunct — do not access widget/context.
class State {
public:
    virtual ~State() = default;

    // Non-copyable, non-movable (identity-bound to an element)
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State(State&&) = delete;
    State& operator=(State&&) = delete;

    // ── Lifecycle Hooks ────────────────────────────────────────

    /// Called once after the State is created and mounted.
    /// Override to perform one-time initialization (start animations,
    /// subscribe to streams, load data, etc.).
    ///
    /// @note Always call `State::initState()` first in your override.
    virtual void initState();

    /// Called when the StatefulWidget is replaced with a new widget of the
    /// same type (during reconciliation). The old widget is passed so you
    /// can compare configurations.
    ///
    /// @param oldWidget The previous widget configuration.
    virtual void didUpdateWidget(const Widget& oldWidget);

    /// Called when an InheritedWidget that this State depends on changes.
    /// Override to react to changes in inherited data (theme, locale, etc.).
    virtual void didChangeDependencies();

    /// Called when this State is temporarily removed from the tree.
    /// The State may be reactivated if the element is reinserted.
    virtual void deactivate();

    /// Called when this State is permanently removed from the tree.
    /// Release all resources here (cancel subscriptions, close streams,
    /// dispose animation controllers, etc.).
    ///
    /// @note Always call `State::dispose()` last in your override.
    /// @note After dispose(), `mounted()` returns false. Do NOT call
    ///       setState() or access context() after this point.
    virtual void dispose();

    // ── Build ──────────────────────────────────────────────────

    /// Build the widget subtree for this state.
    /// Called whenever the framework determines that this State needs
    /// to be rebuilt (e.g., after setState(), didUpdateWidget(), etc.).
    ///
    /// @param ctx The build context for ancestor lookup and media queries.
    /// @return The root widget of this state's subtree.
    [[nodiscard]] virtual WidgetPtr build(BuildContext& ctx) = 0;

    // ── State Update ───────────────────────────────────────────

    /// Schedule a state update and rebuild.
    ///
    /// The provided function `fn` is called synchronously to modify the
    /// state. After `fn` returns, the element is marked dirty and will
    /// be rebuilt before the next frame.
    ///
    /// @param fn A function that modifies the state. Called immediately.
    ///
    /// @code
    ///   setState([this] {
    ///       count_++;
    ///       label_ = "Count: " + std::to_string(count_);
    ///   });
    /// @endcode
    ///
    /// @throws std::logic_error if called after dispose() or during build().
    void setState(std::function<void()> fn);

    // ── Accessors ──────────────────────────────────────────────

    /// @return The BuildContext for this state.
    /// @throws std::logic_error if not yet mounted or already disposed.
    [[nodiscard]] BuildContext& context();

    /// @return The BuildContext for this state (const version).
    [[nodiscard]] const BuildContext& context() const;

    /// @return true if this State is currently mounted in the element tree.
    [[nodiscard]] bool mounted() const { return mounted_; }

    /// @return The widget associated with this state.
    /// @note This returns a raw pointer to the current widget. The widget
    ///       may change across rebuilds (but its type stays the same).
    [[nodiscard]] const StatefulWidget* widget() const;

    /// @return The owning StatefulElement (for render object access).
    /// @note Use with care — only valid while mounted.
    [[nodiscard]] StatefulElement* element() const { return element_; }


protected:
    /// Default constructor. Only accessible to subclasses.
    State() = default;

private:
    friend class StatefulElement;  // Element manages State lifecycle

    /// Set the element that owns this state.
    /// Called by StatefulElement during mount.
    void setElement(StatefulElement* element);

    StatefulElement* element_ = nullptr;  ///< Owning element (set by StatefulElement).
    bool mounted_ = false;                ///< true between initState and dispose.
    bool disposed_ = false;               ///< true after dispose() has been called.
};

}  // namespace enki
