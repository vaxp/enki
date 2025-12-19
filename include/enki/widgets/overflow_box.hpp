#pragma once
/// @file overflow_box.hpp
/// @brief ENKI Section 11: OverflowBox layout widget (C++20 Declarative API).
///
/// A widget that imposes different constraints on its child than it gets from
/// its parent, possibly allowing the child to overflow the parent.
///
/// Common use cases:
///   - Badges, avatars, or indicators that need to hang outside the parent container border.
///   - Letting child content render at an arbitrary size without affecting parent's layout bounds.
///   - Controlled clipping or visible overflow of oversized artwork and interactive items.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// OverflowBox Widget (RenderObject Representation)
// ════════════════════════════════════════════════════════════════

class OverflowBoxWidget : public SingleChildRenderObjectWidget {
public:
    Alignment            alignment     = Alignment::Center;
    std::optional<float> min_width;
    std::optional<float> max_width;
    std::optional<float> min_height;
    std::optional<float> max_height;
    Clip                 clip_behavior = Clip::None;

    OverflowBoxWidget() = default;
    explicit OverflowBoxWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}

    OverflowBoxWidget(Key key, WidgetPtr child,
                      Alignment align = Alignment::Center,
                      std::optional<float> min_w = std::nullopt,
                      std::optional<float> max_w = std::nullopt,
                      std::optional<float> min_h = std::nullopt,
                      std::optional<float> max_h = std::nullopt,
                      Clip clip = Clip::None)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          alignment(align),
          min_width(min_w), max_width(max_w),
          min_height(min_h), max_height(max_h),
          clip_behavior(clip) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "OverflowBox"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Props & Factory Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct OverflowBoxProps {
    Key                  key           = Key::none();
    Alignment            alignment     = Alignment::Center;
    std::optional<float> min_width;
    std::optional<float> max_width;
    std::optional<float> min_height;
    std::optional<float> max_height;
    Clip                 clip_behavior = Clip::None;
    WidgetPtr            child         = nullptr;
};

struct OverflowBox {
    Key                  key           = Key::none();
    Alignment            alignment     = Alignment::Center;
    std::optional<float> min_width;
    std::optional<float> max_width;
    std::optional<float> min_height;
    std::optional<float> max_height;
    Clip                 clip_behavior = Clip::None;
    WidgetPtr            child         = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<OverflowBoxWidget>(
            key, child, alignment, min_width, max_width, min_height, max_height, clip_behavior
        );
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<OverflowBoxWidget> overflowBox(const OverflowBoxProps& props = {}) {
    return std::make_shared<OverflowBoxWidget>(
        props.key, props.child, props.alignment,
        props.min_width, props.max_width, props.min_height, props.max_height,
        props.clip_behavior
    );
}

inline std::shared_ptr<OverflowBoxWidget> overflowBox(OverflowBoxProps&& props) {
    return std::make_shared<OverflowBoxWidget>(
        std::move(props.key), std::move(props.child), props.alignment,
        props.min_width, props.max_width, props.min_height, props.max_height,
        props.clip_behavior
    );
}

inline std::shared_ptr<OverflowBoxWidget> overflowBox(Key key, OverflowBoxProps props) {
    return std::make_shared<OverflowBoxWidget>(
        std::move(key), std::move(props.child), props.alignment,
        props.min_width, props.max_width, props.min_height, props.max_height,
        props.clip_behavior
    );
}

inline std::shared_ptr<OverflowBoxWidget> overflowBox(WidgetPtr child) {
    return std::make_shared<OverflowBoxWidget>(Key::none(), std::move(child));
}

} // namespace enki
