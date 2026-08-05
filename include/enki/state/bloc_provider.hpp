#pragma once
/// @file bloc_provider.hpp
/// @brief BlocProvider — dependency injection of Cubit/BLoC into the widget tree.
///
/// BlocProvider is an InheritedWidget-style provider that makes a Cubit or
/// BLoC instance available to all descendant widgets via the BuildContext.
///
/// Design:
///   - Uses ProxyWidget to inject data into the element tree.
///   - Supports both "create" mode (creates and owns the Cubit) and
///     "value" mode (wraps an existing, externally-owned Cubit).
///   - Lazy creation: in "create" mode, the Cubit is created on first access.
///   - Type-safe lookup via `BlocProvider<B>::of(ctx)`.
///
/// Usage:
/// @code
///   // Provide a CounterCubit to the subtree
///   auto app = bloc_provider<CounterCubit>(
///       /* create */ [] { return std::make_shared<CounterCubit>(); },
///       /* child  */ my_page_widget
///   );
///
///   // Access from a descendant widget
///   WidgetPtr build(BuildContext& ctx) override {
///       auto& counter = BlocProvider<CounterCubit>::of(ctx);
///       return text(std::to_string(counter.state().count));
///   }
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include <memory>
#include <functional>
#include <cassert>
#include <typeinfo>

namespace enki {

// ════════════════════════════════════════════════════════════════
// BlocProvider<B> — Widget that provides a Cubit/BLoC to descendants
// ════════════════════════════════════════════════════════════════

/// @brief Provides a Cubit/BLoC instance to all descendant widgets.
///
/// BlocProvider uses the ProxyWidget/ProxyElement mechanism to inject
/// a Cubit or BLoC into the widget tree. Descendant widgets can retrieve
/// the instance via `BlocProvider<B>::of(ctx)`.
///
/// @tparam B The Cubit or BLoC type to provide.
template<typename B>
class BlocProvider : public ProxyWidget {
public:
    /// Factory function type for creating the Cubit/BLoC.
    using CreateFn = std::function<std::shared_ptr<B>()>;

    /// Create a BlocProvider that creates and owns a Cubit/BLoC.
    ///
    /// The Cubit is created lazily on first access. When the provider
    /// is removed from the tree, the Cubit is closed and destroyed
    /// (if this is the last shared_ptr reference).
    ///
    /// @param create A factory function that creates the Cubit/BLoC.
    /// @param child  The child widget subtree.
    BlocProvider(CreateFn create, WidgetPtr child)
        : ProxyWidget(Key::none(), std::move(child)),
          create_(std::move(create)),
          owns_bloc_(true) {
        assert(create_ && "BlocProvider create function must not be null");
    }

    /// Create a BlocProvider that wraps an existing Cubit/BLoC.
    ///
    /// The caller retains ownership of the Cubit. This is useful when
    /// the Cubit is created externally (e.g., in a parent scope) and
    /// shared across multiple providers.
    ///
    /// @param bloc  The existing Cubit/BLoC instance.
    /// @param child The child widget subtree.
    BlocProvider(std::shared_ptr<B> bloc, WidgetPtr child)
        : ProxyWidget(Key::none(), std::move(child)),
          bloc_(std::move(bloc)),
          owns_bloc_(false) {
        assert(bloc_ && "BlocProvider bloc must not be null");
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "BlocProvider";
    }

    // ── Static Lookup ──────────────────────────────────────────

    /// Look up the nearest BlocProvider<B> in the ancestor tree and
    /// return a reference to the provided Cubit/BLoC.
    ///
    /// @param ctx The build context to search from.
    /// @return A reference to the Cubit/BLoC instance.
    /// @throws std::runtime_error if no BlocProvider<B> is found.
    ///
    /// @code
    ///   auto& counter = BlocProvider<CounterCubit>::of(ctx);
    ///   counter.increment();
    /// @endcode
    static B& of(BuildContext& ctx) {
        auto* provider = findProvider(ctx);
        assert(provider && "BlocProvider::of() called but no BlocProvider<B> "
                          "found in ancestor tree. Did you forget to wrap your "
                          "widget tree with a BlocProvider?");
        return provider->getBloc();
    }

    /// Try to look up the nearest BlocProvider<B>. Returns nullptr if not found.
    ///
    /// @param ctx The build context to search from.
    /// @return Pointer to the Cubit/BLoC, or nullptr if not found.
    static B* tryOf(BuildContext& ctx) {
        auto* provider = findProvider(ctx);
        if (!provider) return nullptr;
        return &provider->getBloc();
    }

    // ── Internal ───────────────────────────────────────────────

    /// Get or create the Cubit/BLoC instance.
    B& getBloc() {
        if (!bloc_ && create_) {
            bloc_ = create_();
            assert(bloc_ && "BlocProvider create function returned null");
        }
        return *bloc_;
    }

    /// @return The shared_ptr to the Cubit/BLoC.
    [[nodiscard]] std::shared_ptr<B> blocPtr() {
        if (!bloc_ && create_) {
            bloc_ = create_();
        }
        return bloc_;
    }

    /// @return true if this provider owns (created) the Cubit/BLoC.
    [[nodiscard]] bool ownsBloc() const { return owns_bloc_; }

private:
    /// Find the nearest BlocProvider<B> in the ancestor tree.
    static BlocProvider<B>* findProvider(BuildContext& ctx) {
        return ctx.findAncestorWidgetOfType<BlocProvider<B>>();
    }

    CreateFn create_;                  ///< Factory (for "create" mode).
    std::shared_ptr<B> bloc_;          ///< The Cubit/BLoC instance.
    bool owns_bloc_ = false;           ///< true if we created and own the bloc.
};

// ════════════════════════════════════════════════════════════════
// MultiBlocProvider — provide multiple BLoCs at once
// ════════════════════════════════════════════════════════════════

/// @brief Helper to provide multiple BLoCs without deep nesting.
///
/// Usage:
/// @code
///   auto app = multi_bloc_provider({
///       [](WidgetPtr child) {
///           return std::make_shared<BlocProvider<AuthBloc>>(
///               [] { return std::make_shared<AuthBloc>(); }, child);
///       },
///       [](WidgetPtr child) {
///           return std::make_shared<BlocProvider<ThemeBloc>>(
///               [] { return std::make_shared<ThemeBloc>(); }, child);
///       },
///   }, root_widget);
/// @endcode
using ProviderFactory = std::function<WidgetPtr(WidgetPtr child)>;

/// Create a chain of providers wrapping a child widget.
/// Providers are applied from last to first (innermost to outermost).
///
/// @param providers The list of provider factories.
/// @param child     The innermost child widget.
/// @return The outermost provider widget.
inline WidgetPtr multi_bloc_provider(
    std::vector<ProviderFactory> providers,
    WidgetPtr child) {

    WidgetPtr result = std::move(child);
    for (auto it = providers.rbegin(); it != providers.rend(); ++it) {
        result = (*it)(std::move(result));
    }
    return result;
}

// ════════════════════════════════════════════════════════════════
// DSL Helper — bloc_provider<B>(...)
// ════════════════════════════════════════════════════════════════

/// DSL helper to create a BlocProvider with create function.
///
/// @tparam B The Cubit/BLoC type.
/// @param create Factory function.
/// @param child  The child widget.
/// @return A shared_ptr to the BlocProvider widget.
template<typename B>
WidgetPtr bloc_provider(
    typename BlocProvider<B>::CreateFn create,
    WidgetPtr child) {
    return std::make_shared<BlocProvider<B>>(std::move(create), std::move(child));
}

/// DSL helper to create a BlocProvider with an existing instance.
///
/// @tparam B The Cubit/BLoC type.
/// @param bloc  The existing Cubit/BLoC.
/// @param child The child widget.
/// @return A shared_ptr to the BlocProvider widget.
template<typename B>
WidgetPtr bloc_provider_value(
    std::shared_ptr<B> bloc,
    WidgetPtr child) {
    return std::make_shared<BlocProvider<B>>(std::move(bloc), std::move(child));
}

}  // namespace enki
