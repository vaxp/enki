#pragma once
/// @file bloc_listener.hpp
/// @brief BlocListener — listens to Cubit/BLoC state changes for side effects.
///
/// BlocListener is similar to BlocBuilder but does NOT rebuild the UI.
/// Instead, it invokes a callback for side effects such as:
///   - Showing snackbars or dialogs
///   - Navigation
///   - Playing sounds
///   - Logging analytics events
///   - Triggering other BLoC events
///
/// Key differences from BlocBuilder:
///   | Feature        | BlocBuilder              | BlocListener            |
///   |----------------|--------------------------|--------------------------|
///   | Purpose        | Rebuild UI               | Side effects            |
///   | Output         | WidgetPtr                | void (callback)         |
///   | Frequency      | Every qualifying change  | Every qualifying change |
///   | Widget tree    | Replaces subtree         | No change               |
///
/// Usage:
/// @code
///   auto widget = bloc_listener<AuthBloc>(
///       [](BuildContext& ctx, const AuthState& state) {
///           if (state.error) {
///               // Show error dialog
///               Navigator::push(ctx, error_page);
///           }
///       },
///       my_child_widget,
///       [](const AuthState& prev, const AuthState& curr) {
///           return prev.error != curr.error;  // Only when error changes
///       }
///   );
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/state/stream.hpp"
#include "enki/state/state.hpp"
#include "enki/state/bloc_provider.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include <functional>
#include <memory>
#include <cassert>

namespace enki {

// ════════════════════════════════════════════════════════════════
// BlocListenerWidget<B> — listens for side effects
// ════════════════════════════════════════════════════════════════

/// @brief A widget that invokes a callback on Cubit/BLoC state changes.
///
/// Unlike BlocBuilder, BlocListener does not rebuild the widget tree.
/// It simply calls the listener function and renders the child unchanged.
///
/// @tparam B The Cubit or BLoC type.
template<typename B>
class BlocListenerWidget : public StatefulWidget {
public:
    /// The state type of the Cubit/BLoC.
    using S = typename B::state_type;

    /// Listener function: (context, state) → void.
    using ListenerFn = std::function<void(BuildContext&, const S&)>;

    /// Predicate: (previous, current) → should listen?
    using ListenWhenFn = std::function<bool(const S& previous, const S& current)>;

    /// Construct a BlocListener widget.
    ///
    /// @param listener    The callback for side effects.
    /// @param child       The child widget (passed through unchanged).
    /// @param listen_when Optional predicate to control when the listener fires.
    BlocListenerWidget(ListenerFn listener, WidgetPtr child,
                       ListenWhenFn listen_when = nullptr)
        : listener_(std::move(listener)),
          child_(std::move(child)),
          listen_when_(std::move(listen_when)) {
        assert(listener_ && "BlocListener requires a listener function");
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "BlocListener";
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;

    /// @return The listener function.
    [[nodiscard]] const ListenerFn& listener() const { return listener_; }

    /// @return The child widget.
    [[nodiscard]] WidgetPtr childWidget() const { return child_; }

    /// @return The listen_when predicate (may be null).
    [[nodiscard]] const ListenWhenFn& listenWhen() const { return listen_when_; }

private:
    ListenerFn listener_;
    WidgetPtr child_;
    ListenWhenFn listen_when_;
};

// ════════════════════════════════════════════════════════════════
// BlocListenerState<B> — State for BlocListenerWidget
// ════════════════════════════════════════════════════════════════

/// @brief Internal State for BlocListenerWidget.
///
/// @tparam B The Cubit or BLoC type.
template<typename B>
class BlocListenerState : public State {
public:
    using S = typename B::state_type;

    void initState() override {
        State::initState();

        auto* bloc = BlocProvider<B>::tryOf(this->context());
        if (!bloc) return;

        bloc_ = bloc;
        previous_state_ = bloc_->state();

        // Subscribe to state changes
        subscription_id_ = bloc_->listen([this](const S& new_state) {
            if (!this->mounted()) return;

            auto* widget_typed = dynamic_cast<const BlocListenerWidget<B>*>(
                this->widget());
            if (!widget_typed) return;

            // Check listenWhen predicate
            const auto& listen_when = widget_typed->listenWhen();
            if (listen_when && !listen_when(previous_state_, new_state)) {
                previous_state_ = new_state;
                return;
            }

            previous_state_ = new_state;

            // Invoke the listener for side effects
            try {
                widget_typed->listener()(this->context(), new_state);
            } catch (const std::exception& e) {
                // Log error but don't crash — side effects should be resilient
                (void)e;
            } catch (...) {
                // Swallow unknown exceptions from listener
            }
        });
    }

    WidgetPtr build(BuildContext& ctx) override {
        // BlocListener does NOT rebuild — just passes through the child
        auto* widget_typed = dynamic_cast<const BlocListenerWidget<B>*>(this->widget());
        assert(widget_typed);
        return widget_typed->childWidget();
    }

    void dispose() override {
        if (bloc_ && subscription_id_ != InvalidSubscriptionId) {
            bloc_->cancelSubscription(subscription_id_);
            subscription_id_ = InvalidSubscriptionId;
        }
        bloc_ = nullptr;
        State::dispose();
    }

private:
    B* bloc_ = nullptr;
    SubscriptionId subscription_id_ = InvalidSubscriptionId;
    S previous_state_{};
};

// ── createState implementation ──────────────────────────────────

template<typename B>
std::unique_ptr<State> BlocListenerWidget<B>::createState() {
    return std::make_unique<BlocListenerState<B>>();
}

// ════════════════════════════════════════════════════════════════
// BlocConsumerWidget<B> — combines Builder + Listener
// ════════════════════════════════════════════════════════════════

/// @brief Combines BlocBuilder and BlocListener into a single widget.
///
/// BlocConsumer both rebuilds the UI AND fires side effects on state change.
/// Use when you need both behaviors (e.g., update a counter display AND
/// show a congratulations dialog at milestones).
///
/// @tparam B The Cubit or BLoC type.
template<typename B>
class BlocConsumerWidget : public StatefulWidget {
public:
    using S = typename B::state_type;
    using BuilderFn = std::function<WidgetPtr(BuildContext&, const S&)>;
    using ListenerFn = std::function<void(BuildContext&, const S&)>;
    using BuildWhenFn = std::function<bool(const S& previous, const S& current)>;
    using ListenWhenFn = std::function<bool(const S& previous, const S& current)>;

    BlocConsumerWidget(BuilderFn builder, ListenerFn listener,
                       BuildWhenFn build_when = nullptr,
                       ListenWhenFn listen_when = nullptr)
        : builder_(std::move(builder)),
          listener_(std::move(listener)),
          build_when_(std::move(build_when)),
          listen_when_(std::move(listen_when)) {
        assert(builder_ && "BlocConsumer requires a builder function");
        assert(listener_ && "BlocConsumer requires a listener function");
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "BlocConsumer";
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;

    [[nodiscard]] const BuilderFn& builder() const { return builder_; }
    [[nodiscard]] const ListenerFn& listener() const { return listener_; }
    [[nodiscard]] const BuildWhenFn& buildWhen() const { return build_when_; }
    [[nodiscard]] const ListenWhenFn& listenWhen() const { return listen_when_; }

private:
    BuilderFn builder_;
    ListenerFn listener_;
    BuildWhenFn build_when_;
    ListenWhenFn listen_when_;
};

/// @brief Internal State for BlocConsumerWidget.
template<typename B>
class BlocConsumerState : public State {
public:
    using S = typename B::state_type;

    void initState() override {
        State::initState();

        auto* bloc = BlocProvider<B>::tryOf(this->context());
        if (!bloc) return;

        bloc_ = bloc;
        previous_state_ = bloc_->state();

        subscription_id_ = bloc_->listen([this](const S& new_state) {
            if (!this->mounted()) return;

            auto* w = dynamic_cast<const BlocConsumerWidget<B>*>(this->widget());
            if (!w) return;

            // Side effects (listener)
            const auto& listen_when = w->listenWhen();
            if (!listen_when || listen_when(previous_state_, new_state)) {
                try {
                    w->listener()(this->context(), new_state);
                } catch (...) {}
            }

            // UI rebuild (builder)
            const auto& build_when = w->buildWhen();
            bool should_rebuild = !build_when || build_when(previous_state_, new_state);

            previous_state_ = new_state;

            if (should_rebuild) {
                this->setState([]{});
            }
        });
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* w = dynamic_cast<const BlocConsumerWidget<B>*>(this->widget());
        assert(w);
        const S& current = bloc_ ? bloc_->state() : previous_state_;
        return w->builder()(ctx, current);
    }

    void dispose() override {
        if (bloc_ && subscription_id_ != InvalidSubscriptionId) {
            bloc_->cancelSubscription(subscription_id_);
        }
        bloc_ = nullptr;
        State::dispose();
    }

private:
    B* bloc_ = nullptr;
    SubscriptionId subscription_id_ = InvalidSubscriptionId;
    S previous_state_{};
};

template<typename B>
std::unique_ptr<State> BlocConsumerWidget<B>::createState() {
    return std::make_unique<BlocConsumerState<B>>();
}

// ════════════════════════════════════════════════════════════════
// DSL Helpers
// ════════════════════════════════════════════════════════════════

/// Create a BlocListener widget (DSL helper).
///
/// @tparam B The Cubit/BLoC type.
/// @param listener    Callback for side effects.
/// @param child       The child widget (passed through).
/// @param listen_when Optional predicate to control when the listener fires.
/// @return A WidgetPtr to the BlocListener.
template<typename B>
WidgetPtr bloc_listener(
    typename BlocListenerWidget<B>::ListenerFn listener,
    WidgetPtr child,
    typename BlocListenerWidget<B>::ListenWhenFn listen_when = nullptr) {
    return std::make_shared<BlocListenerWidget<B>>(
        std::move(listener), std::move(child), std::move(listen_when));
}

/// Create a BlocConsumer widget (DSL helper).
///
/// @tparam B The Cubit/BLoC type.
/// @param builder     Function that builds the widget from (context, state).
/// @param listener    Callback for side effects.
/// @param build_when  Optional predicate for selective rebuilds.
/// @param listen_when Optional predicate for selective listening.
/// @return A WidgetPtr to the BlocConsumer.
template<typename B>
WidgetPtr bloc_consumer(
    typename BlocConsumerWidget<B>::BuilderFn builder,
    typename BlocConsumerWidget<B>::ListenerFn listener,
    typename BlocConsumerWidget<B>::BuildWhenFn build_when = nullptr,
    typename BlocConsumerWidget<B>::ListenWhenFn listen_when = nullptr) {
    return std::make_shared<BlocConsumerWidget<B>>(
        std::move(builder), std::move(listener),
        std::move(build_when), std::move(listen_when));
}

}  // namespace enki
