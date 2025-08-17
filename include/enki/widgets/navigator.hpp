#pragma once
/// @file navigator.hpp
/// @brief Navigator — stack-based page/route manager.
///
/// Navigator manages a stack of Routes. Each Route has a widget content
/// and an entrance/exit animation. Supports push, pop, and replace.
///
/// Features:
///   - Stack of pages with push/pop operations.
///   - Per-route slide or fade transition animation.
///   - Back navigation (pop).
///   - Static access helpers: Navigator::push(ctx, route), Navigator::pop(ctx).
///   - Route configuration with builder function.
///   - Fluent API.
///
/// Usage:
/// @code
///   auto nav = navigator({
///       RouteConfig{"home", []{ return make_shared<HomePage>(); }},
///   });
///
///   // Inside a widget:
///   Navigator::push(ctx, RouteConfig{"detail", [](){ return make_shared<DetailPage>(); }});
///   Navigator::pop(ctx);
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/state/state.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RouteTransition
// ════════════════════════════════════════════════════════════════

enum class RouteTransition {
    None,   ///< Instant swap
    Slide,  ///< Slide from right (push) / left (pop)
    Fade,   ///< Opacity fade
    Scale,  ///< Scale + fade
};

// ════════════════════════════════════════════════════════════════
// RouteConfig
// ════════════════════════════════════════════════════════════════

struct RouteConfig {
    std::string                  name;
    std::function<WidgetPtr()>   builder;
    RouteTransition              transition = RouteTransition::Slide;

    RouteConfig() = default;
    RouteConfig(std::string name, std::function<WidgetPtr()> builder,
                RouteTransition trans = RouteTransition::Slide)
        : name(std::move(name)), builder(std::move(builder)), transition(trans) {}
};

// ════════════════════════════════════════════════════════════════
// NavigatorOptions
// ════════════════════════════════════════════════════════════════

struct NavigatorProps {
    Key      key                  = Key::none();
    std::vector<RouteConfig> initial_routes;
    Color    background_color     = 0xFF0F172A;
    int      transition_duration_ms = 300;
};

// ════════════════════════════════════════════════════════════════
// Navigator Widget
// ════════════════════════════════════════════════════════════════

class Navigator : public StatefulWidget {
public:
    std::vector<RouteConfig>   initial_routes; ///< Initial route stack (bottom = first)
    NavigatorProps           options;

    Navigator() = default;
    explicit Navigator(std::vector<RouteConfig> routes, NavigatorProps opt = {})
        : initial_routes(std::move(routes)), options(std::move(opt)) {}
    
    Navigator(Key k, NavigatorProps opt) : StatefulWidget(std::move(k)), initial_routes(std::move(opt.initial_routes)), options(std::move(opt)) {}

    // ── Static navigation helpers ──────────────────────────────

    /// Push a new route onto the navigator closest to this context.
    static void push(BuildContext& ctx, RouteConfig route);

    /// Pop the topmost route. No-op if only one route remains.
    static void pop(BuildContext& ctx);

    /// Returns true if there is more than one route (can pop).
    static bool canPop(BuildContext& ctx);

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Navigator"; }
};

// ════════════════════════════════════════════════════════════════
// NavigatorState — publicly accessible for static helpers
// ════════════════════════════════════════════════════════════════

class NavigatorState : public State {
public:
    /// Push a new route onto this navigator.
    void push(RouteConfig route);

    /// Pop the topmost route.
    void pop();

    /// Returns true if more than one route exists.
    [[nodiscard]] bool canPop() const;

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;

    void initState() override;
    void dispose() override;

private:
    struct ActiveRoute {
        RouteConfig   config;
        WidgetPtr     widget_cache;
        float         anim_value = 1.0f; ///< 0=entering 1=fully visible
        bool          entering   = false;
        bool          exiting    = false;
    };

    std::vector<ActiveRoute> stack_;
    std::unique_ptr<class Ticker> ticker_;
    bool animating_ = false;

    void tickAnimation();
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Navigator> navigator(std::vector<RouteConfig> routes,
                                            NavigatorProps options = {}) {
    return std::make_shared<Navigator>(std::move(routes), std::move(options));
}

inline std::shared_ptr<Navigator> navigator(NavigatorProps props) {
    return std::make_shared<Navigator>(std::move(props.key), std::move(props));
}

} // namespace enki
