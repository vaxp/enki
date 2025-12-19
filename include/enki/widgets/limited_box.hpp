#pragma once
/// @file limited_box.hpp
/// @brief ENKI Section 11: LimitedBox layout widget (C++20 Declarative API).
///
/// A box that limits its size only when it's unconstrained.
/// If this widget's incoming constraints are unbounded:
///   - If maxWidth is set and width is unconstrained, width is limited to maxWidth.
///   - If maxHeight is set and height is unconstrained, height is limited to maxHeight.
/// If incoming constraints are bounded, LimitedBox does not limit its child and
/// allows the parent's constraints to govern the layout.
///
/// Common use cases:
///   - Inside a vertical ScrollView or ListView where height is unconstrained,
///     preventing a child from collapsing or taking infinite height.
///   - Inside a horizontal ScrollView or Row where width is unconstrained.
///   - Composing flexible cards that size naturally in bounded layouts but have
///     a fallback maximum when embedded in scrolling feeds.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// LimitedBox Widget (RenderObject Representation)
// ════════════════════════════════════════════════════════════════

class LimitedBoxWidget : public SingleChildRenderObjectWidget {
public:
    std::optional<float> max_width  = std::nullopt;
    std::optional<float> max_height = std::nullopt;

    LimitedBoxWidget() = default;
    explicit LimitedBoxWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}

    LimitedBoxWidget(Key key, WidgetPtr child,
                     std::optional<float> max_w = std::nullopt,
                     std::optional<float> max_h = std::nullopt)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          max_width(max_w),
          max_height(max_h) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "LimitedBox"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Props & Factory Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct LimitedBoxProps {
    Key                  key        = Key::none();
    std::optional<float> max_width  = std::nullopt;
    std::optional<float> max_height = std::nullopt;
    WidgetPtr            child      = nullptr;

    operator WidgetPtr() const;
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// @brief Construct a declarative LimitedBox using designated initializers.
inline std::shared_ptr<LimitedBoxWidget> limitedBox(LimitedBoxProps props = {}) {
    return std::make_shared<LimitedBoxWidget>(
        props.key,
        std::move(props.child),
        props.max_width,
        props.max_height
    );
}

/// @brief Direct functional constructor for LimitedBox.
inline std::shared_ptr<LimitedBoxWidget> limitedBox(WidgetPtr child,
                                                   std::optional<float> max_w = std::nullopt,
                                                   std::optional<float> max_h = std::nullopt,
                                                   Key key = Key::none()) {
    return std::make_shared<LimitedBoxWidget>(
        std::move(key),
        std::move(child),
        max_w,
        max_h
    );
}

inline LimitedBoxProps::operator WidgetPtr() const {
    return limitedBox(*this);
}

using LimitedBox = LimitedBoxProps;

} // namespace enki
