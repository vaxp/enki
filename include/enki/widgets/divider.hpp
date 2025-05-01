#pragma once
/// @file divider.hpp
/// @brief Divider widget for separating content visually.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"

namespace enki {

/// Options for configuring a Divider.
struct DividerOptions {
    float height = 16.0f;       ///< Total height (or width for vertical) of the divider box.
    float thickness = 1.0f;     ///< Thickness of the line drawn.
    float indent = 0.0f;        ///< Empty space leading the divider.
    float end_indent = 0.0f;    ///< Empty space trailing the divider.
    Color color = 0xFF363B42;   ///< Color of the divider line.
};

/// @brief A horizontal line, with padding on either side.
class Divider : public SingleChildRenderObjectWidget {
public:
    DividerOptions options;

    Divider(DividerOptions options = DividerOptions())
        : options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    
    [[nodiscard]] std::string_view typeName() const override { return "Divider"; }
};

/// @brief A vertical line, with padding on either side.
class VerticalDivider : public SingleChildRenderObjectWidget {
public:
    DividerOptions options;

    VerticalDivider(DividerOptions options = DividerOptions())
        : options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    
    [[nodiscard]] std::string_view typeName() const override { return "VerticalDivider"; }
};

inline WidgetPtr divider(DividerOptions options = DividerOptions()) {
    return std::make_shared<Divider>(std::move(options));
}

inline WidgetPtr verticalDivider(DividerOptions options = DividerOptions()) {
    return std::make_shared<VerticalDivider>(std::move(options));
}

} // namespace enki
