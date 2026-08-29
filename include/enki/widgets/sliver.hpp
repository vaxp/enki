#pragma once
/// @file sliver.hpp
/// @brief Extended Scrolling & Sliver widgets for ENKI Framework (Section 14).
///
/// Widgets:
///   1. CustomScrollView  — Scroll viewport accepting an ordered sequence of slivers
///   2. SliverAppBar     — Collapsible, pinned, or floating header sliver with flexible space
///   3. SliverList       — Linear list sliver with static items, lazy builder, and separators
///   4. SliverGrid       — 2D grid sliver supporting fixed count or responsive max extent
///   5. SliverToBoxAdapter — Wraps any standard box widget into a sliver
///   6. SliverPadding    — Applies padding around a sliver
///
/// 100% C++20 Declarative Syntax with designated initializers.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/list_view.hpp"
#include "enki/widgets/grid_view.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. CustomScrollView Widget
// ════════════════════════════════════════════════════════════════

struct CustomScrollViewProps {
    Key key = Key::none();
    std::vector<WidgetPtr> slivers = {};
    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;
    bool show_scrollbar = true;
    bool shrink_wrap = false;
    bool reverse = false;
    float cache_extent = 0.0f;
    std::function<void(float offset)> on_scroll = nullptr;
};

class CustomScrollViewWidget : public StatefulWidget {
public:
    CustomScrollViewProps props;

    CustomScrollViewWidget() = default;
    explicit CustomScrollViewWidget(CustomScrollViewProps p)
        : StatefulWidget(p.key), props(std::move(p)) {}
    CustomScrollViewWidget(Key k, CustomScrollViewProps p)
        : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CustomScrollView"; }
};

struct CustomScrollView {
    Key key = Key::none();
    std::vector<WidgetPtr> slivers = {};
    std::vector<WidgetPtr> children = {}; // alias
    Axis direction = Axis::Vertical;
    ScrollPhysics scroll_physics = ScrollPhysics::Clamped;
    float scroll_speed = 50.0f;
    bool show_scrollbar = true;
    bool shrink_wrap = false;
    bool reverse = false;
    float cache_extent = 0.0f;
    std::function<void(float offset)> on_scroll = nullptr;

    operator WidgetPtr() const {
        CustomScrollViewProps p;
        p.key = key;
        p.slivers = !slivers.empty() ? slivers : children;
        p.direction = direction;
        p.scroll_physics = scroll_physics;
        p.scroll_speed = scroll_speed;
        p.show_scrollbar = show_scrollbar;
        p.shrink_wrap = shrink_wrap;
        p.reverse = reverse;
        p.cache_extent = cache_extent;
        p.on_scroll = on_scroll;
        return std::make_shared<CustomScrollViewWidget>(key, std::move(p));
    }
};

inline std::shared_ptr<CustomScrollViewWidget> customScrollView(CustomScrollViewProps props = {}) {
    return std::make_shared<CustomScrollViewWidget>(std::move(props));
}

inline std::shared_ptr<CustomScrollViewWidget> customScrollView(std::vector<WidgetPtr> slivers) {
    CustomScrollViewProps p;
    p.slivers = std::move(slivers);
    return std::make_shared<CustomScrollViewWidget>(std::move(p));
}

inline std::shared_ptr<CustomScrollViewWidget> customScrollView(std::initializer_list<WidgetPtr> slivers) {
    CustomScrollViewProps p;
    p.slivers = std::vector<WidgetPtr>(slivers);
    return std::make_shared<CustomScrollViewWidget>(std::move(p));
}

// ════════════════════════════════════════════════════════════════
// 2. SliverAppBar Widget
// ════════════════════════════════════════════════════════════════

struct SliverAppBarProps {
    Key key = Key::none();
    WidgetPtr title = nullptr;
    WidgetPtr leading = nullptr;
    std::vector<WidgetPtr> actions = {};
    WidgetPtr flexible_space = nullptr;
    WidgetPtr bottom = nullptr;
    float expanded_height = 200.0f;
    float collapsed_height = 56.0f;
    bool pinned = true;
    bool floating = false;
    bool snap = false;
    bool center_title = false;
    Color background_color = 0xFF1E293B;
    Color foreground_color = 0xFFFFFFFF;
    float elevation = 4.0f;
};

class SliverAppBarWidget : public StatefulWidget {
public:
    SliverAppBarProps props;

    SliverAppBarWidget() = default;
    explicit SliverAppBarWidget(SliverAppBarProps p)
        : StatefulWidget(p.key), props(std::move(p)) {}
    SliverAppBarWidget(Key k, SliverAppBarProps p)
        : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "SliverAppBar"; }
};

struct SliverAppBar {
    Key key = Key::none();
    WidgetPtr title = nullptr;
    WidgetPtr leading = nullptr;
    std::vector<WidgetPtr> actions = {};
    WidgetPtr flexible_space = nullptr;
    WidgetPtr bottom = nullptr;
    float expanded_height = 200.0f;
    float collapsed_height = 56.0f;
    bool pinned = true;
    bool floating = false;
    bool snap = false;
    bool center_title = false;
    Color background_color = 0xFF1E293B;
    Color foreground_color = 0xFFFFFFFF;
    float elevation = 4.0f;

    operator WidgetPtr() const {
        return std::make_shared<SliverAppBarWidget>(key, SliverAppBarProps{
            .key = key,
            .title = title,
            .leading = leading,
            .actions = actions,
            .flexible_space = flexible_space,
            .bottom = bottom,
            .expanded_height = expanded_height,
            .collapsed_height = collapsed_height,
            .pinned = pinned,
            .floating = floating,
            .snap = snap,
            .center_title = center_title,
            .background_color = background_color,
            .foreground_color = foreground_color,
            .elevation = elevation,
        });
    }
};

inline std::shared_ptr<SliverAppBarWidget> sliverAppBar(SliverAppBarProps props = {}) {
    return std::make_shared<SliverAppBarWidget>(std::move(props));
}

// ════════════════════════════════════════════════════════════════
// 3. SliverList Widget
// ════════════════════════════════════════════════════════════════

struct SliverListProps {
    Key key = Key::none();
    std::vector<WidgetPtr> items = {};
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    std::function<WidgetPtr(int index)> separator_builder = nullptr;
    EdgeInsets padding = EdgeInsets{};
};

class SliverListWidget : public StatelessWidget {
public:
    SliverListProps props;

    SliverListWidget() = default;
    explicit SliverListWidget(SliverListProps p)
        : StatelessWidget(p.key), props(std::move(p)) {}
    SliverListWidget(Key k, SliverListProps p)
        : StatelessWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "SliverList"; }
};

struct SliverList {
    Key key = Key::none();
    std::vector<WidgetPtr> items = {};
    std::vector<WidgetPtr> children = {}; // alias
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    std::function<WidgetPtr(int index)> separator_builder = nullptr;
    EdgeInsets padding = EdgeInsets{};

    operator WidgetPtr() const {
        SliverListProps p;
        p.key = key;
        p.items = !children.empty() ? children : items;
        p.item_count = item_count;
        p.item_builder = item_builder;
        p.separator_builder = separator_builder;
        p.padding = padding;
        return std::make_shared<SliverListWidget>(key, std::move(p));
    }
};

inline std::shared_ptr<SliverListWidget> sliverList(SliverListProps props = {}) {
    return std::make_shared<SliverListWidget>(std::move(props));
}

inline std::shared_ptr<SliverListWidget> sliverList(std::vector<WidgetPtr> items) {
    SliverListProps p;
    p.items = std::move(items);
    return std::make_shared<SliverListWidget>(std::move(p));
}

inline std::shared_ptr<SliverListWidget> sliverList(std::initializer_list<WidgetPtr> items) {
    SliverListProps p;
    p.items = std::vector<WidgetPtr>(items);
    return std::make_shared<SliverListWidget>(std::move(p));
}

inline std::shared_ptr<SliverListWidget> sliverListCount(int count, std::function<WidgetPtr(int index)> builder) {
    SliverListProps p;
    p.item_count = count;
    p.item_builder = std::move(builder);
    return std::make_shared<SliverListWidget>(std::move(p));
}

inline std::shared_ptr<SliverListWidget> sliverListSeparated(
    int count,
    std::function<WidgetPtr(int index)> builder,
    std::function<WidgetPtr(int index)> separator)
{
    SliverListProps p;
    p.item_count = count;
    p.item_builder = std::move(builder);
    p.separator_builder = std::move(separator);
    return std::make_shared<SliverListWidget>(std::move(p));
}

// ════════════════════════════════════════════════════════════════
// 4. SliverGrid Widget
// ════════════════════════════════════════════════════════════════

struct SliverGridProps {
    Key key = Key::none();
    std::vector<WidgetPtr> items = {};
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    SliverGridDelegateFixedCount fixed_delegate = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent max_delegate;
    bool use_max_extent_delegate = false;
    EdgeInsets padding = EdgeInsets{};
};

class SliverGridWidget : public StatelessWidget {
public:
    SliverGridProps props;

    SliverGridWidget() = default;
    explicit SliverGridWidget(SliverGridProps p)
        : StatelessWidget(p.key), props(std::move(p)) {}
    SliverGridWidget(Key k, SliverGridProps p)
        : StatelessWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "SliverGrid"; }
};

struct SliverGrid {
    Key key = Key::none();
    std::vector<WidgetPtr> items = {};
    std::vector<WidgetPtr> children = {}; // alias
    int item_count = 0;
    std::function<WidgetPtr(int index)> item_builder = nullptr;
    SliverGridDelegateFixedCount fixed_delegate = SliverGridDelegateFixedCount(2);
    SliverGridDelegateMaxExtent max_delegate;
    bool use_max_extent_delegate = false;
    EdgeInsets padding = EdgeInsets{};

    operator WidgetPtr() const {
        SliverGridProps p;
        p.key = key;
        p.items = !children.empty() ? children : items;
        p.item_count = item_count;
        p.item_builder = item_builder;
        p.fixed_delegate = fixed_delegate;
        p.max_delegate = max_delegate;
        p.use_max_extent_delegate = use_max_extent_delegate;
        p.padding = padding;
        return std::make_shared<SliverGridWidget>(key, std::move(p));
    }
};

inline std::shared_ptr<SliverGridWidget> sliverGrid(SliverGridProps props = {}) {
    return std::make_shared<SliverGridWidget>(std::move(props));
}

inline std::shared_ptr<SliverGridWidget> sliverGrid(
    int count,
    std::function<WidgetPtr(int index)> builder,
    SliverGridDelegateFixedCount delegate = SliverGridDelegateFixedCount(2))
{
    SliverGridProps p;
    p.item_count = count;
    p.item_builder = std::move(builder);
    p.fixed_delegate = delegate;
    p.use_max_extent_delegate = false;
    return std::make_shared<SliverGridWidget>(std::move(p));
}

inline std::shared_ptr<SliverGridWidget> sliverGridExtent(
    int count,
    std::function<WidgetPtr(int index)> builder,
    SliverGridDelegateMaxExtent delegate)
{
    SliverGridProps p;
    p.item_count = count;
    p.item_builder = std::move(builder);
    p.max_delegate = delegate;
    p.use_max_extent_delegate = true;
    return std::make_shared<SliverGridWidget>(std::move(p));
}

// ════════════════════════════════════════════════════════════════
// 5. SliverToBoxAdapter Widget
// ════════════════════════════════════════════════════════════════

struct SliverToBoxAdapterProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;
};

class SliverToBoxAdapterWidget : public StatelessWidget {
public:
    WidgetPtr child;

    SliverToBoxAdapterWidget() = default;
    explicit SliverToBoxAdapterWidget(WidgetPtr c) : child(std::move(c)) {}
    SliverToBoxAdapterWidget(Key k, WidgetPtr c)
        : StatelessWidget(std::move(k)), child(std::move(c)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "SliverToBoxAdapter"; }
};

struct SliverToBoxAdapter {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<SliverToBoxAdapterWidget>(key, child);
    }
};

inline std::shared_ptr<SliverToBoxAdapterWidget> sliverToBoxAdapter(WidgetPtr child) {
    return std::make_shared<SliverToBoxAdapterWidget>(std::move(child));
}

// ════════════════════════════════════════════════════════════════
// 6. SliverPadding Widget
// ════════════════════════════════════════════════════════════════

struct SliverPaddingProps {
    Key key = Key::none();
    EdgeInsets padding = EdgeInsets{};
    WidgetPtr sliver = nullptr;
    WidgetPtr child = nullptr; // alias
};

class SliverPaddingWidget : public StatelessWidget {
public:
    EdgeInsets padding;
    WidgetPtr sliver;

    SliverPaddingWidget() = default;
    SliverPaddingWidget(EdgeInsets pad, WidgetPtr slv)
        : padding(pad), sliver(std::move(slv)) {}
    SliverPaddingWidget(Key k, EdgeInsets pad, WidgetPtr slv)
        : StatelessWidget(std::move(k)), padding(pad), sliver(std::move(slv)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "SliverPadding"; }
};

struct SliverPadding {
    Key key = Key::none();
    EdgeInsets padding = EdgeInsets{};
    WidgetPtr sliver = nullptr;
    WidgetPtr child = nullptr; // alias

    operator WidgetPtr() const {
        return std::make_shared<SliverPaddingWidget>(key, padding, sliver ? sliver : child);
    }
};

inline std::shared_ptr<SliverPaddingWidget> sliverPadding(EdgeInsets padding, WidgetPtr sliver) {
    return std::make_shared<SliverPaddingWidget>(padding, std::move(sliver));
}

} // namespace enki
