#pragma once
/// @file split_view.hpp
/// @brief Advanced SplitView widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports Horizontal/Vertical orientations, interactive drag-to-resize handles,
/// min/max pane constraints, snap-to-collapse, and SplitViewController.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace enki {

/// Split layout orientation
enum class SplitOrientation {
    Horizontal, ///< Left / Right panes
    Vertical    ///< Top / Bottom panes
};

/// ════════════════════════════════════════════════════════════════
/// SplitView Options
/// ════════════════════════════════════════════════════════════════

struct SplitViewOptions {
    SplitOrientation orientation = SplitOrientation::Horizontal;
    float initial_ratio = 0.50f;         ///< 0.0f to 1.0f (fraction for leading pane)

    float min_leading_size = 100.0f;     ///< Minimum size in pixels for leading pane
    float min_trailing_size = 100.0f;    ///< Minimum size in pixels for trailing pane

    bool allow_collapse = true;          ///< Allow snapping to 0 or 1.0
    float snap_threshold = 40.0f;        ///< Drag threshold below which pane collapses

    float handle_thickness = 6.0f;       ///< Width/height of the draggable divider bar
    Color handle_color = 0xFF334155;     ///< Slate 700
    Color handle_hover_color = 0xFF38BDF8; ///< Sky 400 highlight
    Color handle_active_color = 0xFF0284C7; ///< Active dragging blue
    Color background_color = 0xFF0F172A; ///< Slate 900

    bool show_handle_grip = true;        ///< Render grip dots (⋮⋮) on divider

    // Callbacks
    std::function<void(float ratio)> on_split_changed;
    std::function<void(bool is_leading)> on_pane_collapsed;
};

/// ════════════════════════════════════════════════════════════════
/// SplitView Controller
/// ════════════════════════════════════════════════════════════════

class SplitViewController {
public:
    std::function<void(float)> set_ratio_fn;
    std::function<float()> get_ratio_fn;
    std::function<void()> collapse_leading_fn;
    std::function<void()> collapse_trailing_fn;
    std::function<void()> reset_fn;

    void setRatio(float r) { if (set_ratio_fn) set_ratio_fn(r); }
    [[nodiscard]] float getRatio() const { return get_ratio_fn ? get_ratio_fn() : 0.5f; }
    void collapseLeading() { if (collapse_leading_fn) collapse_leading_fn(); }
    void collapseTrailing() { if (collapse_trailing_fn) collapse_trailing_fn(); }
    void reset() { if (reset_fn) reset_fn(); }
};

struct SplitViewProps {
    Key key = Key::none();
    WidgetPtr first_child;
    WidgetPtr second_child;
    SplitViewOptions options;
    std::shared_ptr<SplitViewController> controller;
};

/// ════════════════════════════════════════════════════════════════
/// SplitView Widget
/// ════════════════════════════════════════════════════════════════

class SplitView : public StatefulWidget {
public:
    WidgetPtr leading;
    WidgetPtr trailing;
    SplitViewOptions options;
    std::shared_ptr<SplitViewController> controller;

    SplitView() = default;
    SplitView(WidgetPtr leading_, WidgetPtr trailing_, SplitViewOptions opts = {},
              std::shared_ptr<SplitViewController> ctrl = nullptr)
        : leading(std::move(leading_)), trailing(std::move(trailing_)),
          options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "SplitView"; }
};

inline std::shared_ptr<SplitView> splitView(
    WidgetPtr leading,
    WidgetPtr trailing,
    SplitViewOptions options = {},
    std::shared_ptr<SplitViewController> controller = nullptr) {
    return std::make_shared<SplitView>(std::move(leading), std::move(trailing), std::move(options), std::move(controller));
}

inline std::shared_ptr<SplitView> splitView(SplitViewProps props) {
    auto sv = std::make_shared<SplitView>(std::move(props.first_child), std::move(props.second_child), std::move(props.options), std::move(props.controller));
    sv->key = props.key;
    return sv;
}

inline std::shared_ptr<SplitView> horizontalSplit(
    WidgetPtr leading,
    WidgetPtr trailing,
    float ratio = 0.50f,
    std::shared_ptr<SplitViewController> controller = nullptr) {
    SplitViewOptions opts;
    opts.orientation = SplitOrientation::Horizontal;
    opts.initial_ratio = ratio;
    return std::make_shared<SplitView>(std::move(leading), std::move(trailing), opts, std::move(controller));
}

inline std::shared_ptr<SplitView> verticalSplit(
    WidgetPtr leading,
    WidgetPtr trailing,
    float ratio = 0.50f,
    std::shared_ptr<SplitViewController> controller = nullptr) {
    SplitViewOptions opts;
    opts.orientation = SplitOrientation::Vertical;
    opts.initial_ratio = ratio;
    return std::make_shared<SplitView>(std::move(leading), std::move(trailing), opts, std::move(controller));
}

} // namespace enki
