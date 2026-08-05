#pragma once
/// @file bloc_builder.hpp
/// @brief BlocBuilder — reactive UI rebuilding based on Cubit/BLoC state.
///
/// BlocBuilder listens to state changes from a Cubit or BLoC and
/// rebuilds a portion of the widget tree whenever the state changes.
/// It is the primary bridge between state management and the UI layer.
///
/// Key features:
///   - **Automatic subscription**: Subscribes on mount, cancels on unmount.
///   - **Selective rebuilding**: Optional `buildWhen` predicate to skip
///     unnecessary rebuilds.
///   - **Type-safe**: Automatically finds the Cubit/BLoC via BlocProvider.
///   - **Efficient**: Only rebuilds the subtree produced by the builder.
///
/// Usage:
/// @code
///   // Basic usage — rebuilds on every state change
///   auto widget = bloc_builder<CounterCubit>(
///       [](BuildContext& ctx, const CounterState& state) {
///           return text(std::to_string(state.count));
///       }
///   );
///
///   // With buildWhen — only rebuild when count changes parity
///   auto widget = bloc_builder<CounterCubit>(
///       [](BuildContext& ctx, const CounterState& state) {
///           return text(state.count % 2 == 0 ? "Even" : "Odd");
///       },
///       [](const CounterState& prev, const CounterState& curr) {
///           return (prev.count % 2) != (curr.count % 2);
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

// Forward declarations
class State;

// ════════════════════════════════════════════════════════════════
// BlocBuilderWidget<B> — StatefulWidget that rebuilds on state change
// ════════════════════════════════════════════════════════════════

/// @brief A widget that rebuilds its subtree when a Cubit/BLoC's state changes.
///
/// BlocBuilderWidget is a StatefulWidget that:
///   1. On mount, finds the nearest BlocProvider<B> ancestor.
///   2. Subscribes to state changes.
///   3. On each state change, calls the builder function to produce a new widget.
///   4. On unmount, cancels the subscription.
///
/// @tparam B The Cubit or BLoC type. Must be provided via BlocProvider<B>.
template<typename B>
class BlocBuilderWidget : public StatefulWidget {
public:
    /// The state type of the Cubit/BLoC.
    using S = typename B::state_type;

    /// Builder function: (context, state) → widget.
    using BuilderFn = std::function<WidgetPtr(BuildContext&, const S&)>;

    /// Predicate: (previous, current) → should rebuild?
    using BuildWhenFn = std::function<bool(const S& previous, const S& current)>;

    /// Construct a BlocBuilder widget.
    ///
    /// @param builder    The function that builds the widget from the current state.
    /// @param build_when Optional predicate to control when rebuilds happen.
    ///                   If null, rebuilds on every state change.
    BlocBuilderWidget(BuilderFn builder, BuildWhenFn build_when = nullptr)
        : builder_(std::move(builder)),
          build_when_(std::move(build_when)) {
        assert(builder_ && "BlocBuilder requires a builder function");
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "BlocBuilder";
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;

    /// @return The builder function.
    [[nodiscard]] const BuilderFn& builder() const { return builder_; }

    /// @return The build_when predicate (may be null).
    [[nodiscard]] const BuildWhenFn& buildWhen() const { return build_when_; }

private:
    BuilderFn builder_;
    BuildWhenFn build_when_;
};

// ════════════════════════════════════════════════════════════════
// BlocBuilderState<B> — State object for BlocBuilderWidget
// ════════════════════════════════════════════════════════════════

/// @brief Internal State for BlocBuilderWidget.
///
/// Manages the subscription lifecycle and triggers rebuilds.
///
/// @tparam B The Cubit or BLoC type.
template<typename B>
class BlocBuilderState : public State {
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

            auto* widget_typed = dynamic_cast<const BlocBuilderWidget<B>*>(
                this->widget());
            if (!widget_typed) return;

            // Check buildWhen predicate
            const auto& build_when = widget_typed->buildWhen();
            if (build_when && !build_when(previous_state_, new_state)) {
                previous_state_ = new_state;
                return;
            }

            previous_state_ = new_state;

            // Trigger rebuild
            this->setState([]{});
        });
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* widget_typed = dynamic_cast<const BlocBuilderWidget<B>*>(this->widget());
        assert(widget_typed && "BlocBuilderState is not associated with a BlocBuilderWidget");

        // Get current state from BLoC
        const S& current = bloc_ ? bloc_->state() : previous_state_;

        return widget_typed->builder()(ctx, current);
    }

    void dispose() override {
        // Cancel subscription
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
std::unique_ptr<State> BlocBuilderWidget<B>::createState() {
    return std::make_unique<BlocBuilderState<B>>();
}

// ════════════════════════════════════════════════════════════════
// DSL Helper — bloc_builder<B>(...)
// ════════════════════════════════════════════════════════════════

/// Create a BlocBuilder widget (DSL helper).
///
/// @tparam B The Cubit/BLoC type.
/// @param builder    Function that builds the widget from (context, state).
/// @param build_when Optional predicate to control when rebuilds happen.
/// @return A WidgetPtr to the BlocBuilder.
///
/// @code
///   auto counter_display = bloc_builder<CounterCubit>(
///       [](auto& ctx, auto& state) {
///           return text(std::to_string(state.count), {
///               .font_size = 48,
///               .color = colors::White,
///           });
///       }
///   );
/// @endcode
template<typename B>
WidgetPtr bloc_builder(
    typename BlocBuilderWidget<B>::BuilderFn builder,
    typename BlocBuilderWidget<B>::BuildWhenFn build_when = nullptr) {
    return std::make_shared<BlocBuilderWidget<B>>(
        std::move(builder), std::move(build_when));
}

}  // namespace enki
