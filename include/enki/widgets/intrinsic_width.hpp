#pragma once
/// @file intrinsic_width.hpp
/// @brief ENKI Section 11: IntrinsicWidth layout widget (C++20 Declarative API).
///
/// Sizes its child to the child's maximum intrinsic/natural width.
/// Optionally snaps the computed width to multiples of step_width, and height to step_height.
///
/// Common use cases:
///   - Uniformly sizing all buttons in a vertical Column to match the widest button.
///   - Preventing a child from stretching to full container width when natural width is desired.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// IntrinsicWidth Widget
// ════════════════════════════════════════════════════════════════

class IntrinsicWidthWidget : public SingleChildRenderObjectWidget {
public:
    std::optional<float> step_width;
    std::optional<float> step_height;

    IntrinsicWidthWidget() = default;
    explicit IntrinsicWidthWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    IntrinsicWidthWidget(Key key, WidgetPtr child,
                         std::optional<float> step_w = std::nullopt,
                         std::optional<float> step_h = std::nullopt)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          step_width(step_w), step_height(step_h) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "IntrinsicWidth"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Props & Factory Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct IntrinsicWidthProps {
    Key key = Key::none();
    std::optional<float> step_width;
    std::optional<float> step_height;
    WidgetPtr child = nullptr;
};

struct IntrinsicWidth {
    Key key = Key::none();
    std::optional<float> step_width;
    std::optional<float> step_height;
    WidgetPtr child = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<IntrinsicWidthWidget>(key, child, step_width, step_height);
    }
};

inline std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(const IntrinsicWidthProps& props) {
    return std::make_shared<IntrinsicWidthWidget>(props.key, props.child, props.step_width, props.step_height);
}

inline std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(WidgetPtr child) {
    return std::make_shared<IntrinsicWidthWidget>(Key::none(), std::move(child));
}

inline std::shared_ptr<IntrinsicWidthWidget> intrinsicWidth(float step_width, WidgetPtr child) {
    return std::make_shared<IntrinsicWidthWidget>(Key::none(), std::move(child), step_width, std::nullopt);
}

} // namespace enki
