#pragma once
/// @file rating_bar.hpp
/// @brief RatingBar widget for ENKI Framework.
/// An interactive star/icon rating input widget with fractional support and hover feedback.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>
#include <memory>

namespace enki {

struct RatingBarProps {
    float                               rating = 0.0f;
    int                                 max_rating = 5;
    float                               item_size = 24.0f;
    float                               item_spacing = 6.0f;
    bool                                allow_half = true;
    bool                                is_read_only = false;

    Color                               active_color = 0xFFF59E0B;   // Amber Gold
    Color                               inactive_color = 0x33FFFFFF; // Subtle translucent
    Color                               glow_color = 0x66F59E0B;

    std::function<void(float)>          on_rating_changed;
    std::function<void(float)>          on_hover;

    operator WidgetPtr() const;
};

class RatingBarWidget : public SingleChildRenderObjectWidget {
public:
    RatingBarProps props;

    explicit RatingBarWidget(RatingBarProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "RatingBar"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr ratingBar(RatingBarProps props) {
    return std::make_shared<RatingBarWidget>(std::move(props));
}

} // namespace enki
