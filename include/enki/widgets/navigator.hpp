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
///
/// Usage:
/// @code
///   auto nav = Navigator {
///       .initial_routes = {
///           RouteConfig{"home", []{ return make_shared<HomePage>(); }},
///       }
///   };
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
// NavigatorProps
// ════════════════════════════════════════════════════════════════

struct NavigatorProps {
    Key                      key                    = Key::none();
    std::vector<RouteConfig> initial_routes;
    Color                    background_color       = 0xFF0F172A;
    int                      transition_duration_ms = 300;
};

// ════════════════════════════════════════════════════════════════
// Navigator Widget Implementation
// ════════════════════════════════════════════════════════════════

class NavigatorWidget : public StatefulWidget {
public:
    std::vector<RouteConfig> initial_routes;
    NavigatorProps           options;

    NavigatorWidget() = default;
    explicit NavigatorWidget(std::vector<RouteConfig> routes, NavigatorProps opt = {})
        : initial_routes(std::move(routes)), options(std::move(opt)) {}
    
    NavigatorWidget(Key k, NavigatorProps opt)
        : StatefulWidget(std::move(k)), initial_routes(opt.initial_routes), options(std::move(opt)) {}

    // ── Static navigation helpers ──────────────────────────────
    static void push(BuildContext& ctx, RouteConfig route);
    static void pop(BuildContext& ctx);
    static bool canPop(BuildContext& ctx);

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Navigator"; }
};

// ════════════════════════════════════════════════════════════════
// NavigatorState
// ════════════════════════════════════════════════════════════════

class NavigatorState : public State {
public:
    void push(RouteConfig route);
    void pop();
    [[nodiscard]] bool canPop() const;

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;

    void initState() override;
    void dispose() override;

private:
    struct ActiveRoute {
        RouteConfig   config;
        WidgetPtr     widget_cache;
        float         anim_value = 1.0f;
        bool          entering   = false;
        bool          exiting    = false;
    };

    std::vector<ActiveRoute> stack_;
    std::unique_ptr<class Ticker> ticker_;
    bool animating_ = false;

    void tickAnimation();
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Navigator {
    Key                      key                    = Key::none();
    std::vector<RouteConfig> initial_routes;
    Color                    background_color       = 0xFF0F172A;
    int                      transition_duration_ms = 300;

    static void push(BuildContext& ctx, RouteConfig route) {
        NavigatorWidget::push(ctx, std::move(route));
    }

    static void pop(BuildContext& ctx) {
        NavigatorWidget::pop(ctx);
    }

    static bool canPop(BuildContext& ctx) {
        return NavigatorWidget::canPop(ctx);
    }

    operator WidgetPtr() const {
        NavigatorProps props;
        props.key = key;
        props.initial_routes = initial_routes;
        props.background_color = background_color;
        props.transition_duration_ms = transition_duration_ms;
        return std::make_shared<NavigatorWidget>(key, std::move(props));
    }
};

} // namespace enki
