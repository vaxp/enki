#pragma once
/// @file intrinsic_height.hpp
/// @brief ENKI Section 11: IntrinsicHeight layout widget (C++20 Declarative API).
///
/// Sizes its child to the child's maximum intrinsic/natural height.
/// Optionally snaps the computed height to multiples of step_height, and width to step_width.
///
/// Common use cases:
///   - Uniformly sizing all cards and vertical dividers in a horizontal Row to match the tallest item.
///   - Constraining a child's height to its natural content without stretching to parent height.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// IntrinsicHeight Widget
// ════════════════════════════════════════════════════════════════

class IntrinsicHeightWidget : public SingleChildRenderObjectWidget {
public:
    std::optional<float> step_height;
    std::optional<float> step_width;

    IntrinsicHeightWidget() = default;
    explicit IntrinsicHeightWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    IntrinsicHeightWidget(Key key, WidgetPtr child,
                          std::optional<float> step_h = std::nullopt,
                          std::optional<float> step_w = std::nullopt)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          step_height(step_h), step_width(step_w) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "IntrinsicHeight"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Props & Factory Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct IntrinsicHeightProps {
    Key key = Key::none();
    std::optional<float> step_height;
    std::optional<float> step_width;
    WidgetPtr child = nullptr;
};

struct IntrinsicHeight {
    Key key = Key::none();
    std::optional<float> step_height;
    std::optional<float> step_width;
    WidgetPtr child = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<IntrinsicHeightWidget>(key, child, step_height, step_width);
    }
};

inline std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(const IntrinsicHeightProps& props) {
    return std::make_shared<IntrinsicHeightWidget>(props.key, props.child, props.step_height, props.step_width);
}

inline std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(WidgetPtr child) {
    return std::make_shared<IntrinsicHeightWidget>(Key::none(), std::move(child));
}

inline std::shared_ptr<IntrinsicHeightWidget> intrinsicHeight(float step_height, WidgetPtr child) {
    return std::make_shared<IntrinsicHeightWidget>(Key::none(), std::move(child), step_height, std::nullopt);
}

} // namespace enki
