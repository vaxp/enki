#pragma once
/// @file segmented_control.hpp
/// @brief SegmentedControl widget for ENKI Framework.
/// Horizontally grouped mutually-exclusive option buttons with smooth animated sliding indicator.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace enki {

struct SegmentItem {
    std::string label;
    std::string icon = "";
    bool        enabled = true;

    SegmentItem(std::string l = "", std::string i = "")
        : label(std::move(l)), icon(std::move(i)) {}
};

struct SegmentedControlProps {
    std::vector<SegmentItem>            items;
    int                                 selected_index = 0;
    std::function<void(int)>            on_change;

    Color                               track_color = 0x59000000;
    Color                               thumb_color = 0xFF0C3559;
    Color                               thumb_border_color = 0x6600E5FF;
    Color                               active_text_color = 0xFF38BDF8;
    Color                               inactive_text_color = 0xFF94A3B8;
    Color                               border_color = 0x3300E5FF;
    float                               border_width = 1.0f;
    float                               border_radius = 10.0f;
    float                               thumb_radius = 8.0f;
    float                               height = 38.0f;
    float                               width = 0.0f; // 0 = fit content or fill
    float                               padding = 4.0f;

    operator WidgetPtr() const;
};

class SegmentedControlWidget : public SingleChildRenderObjectWidget {
public:
    SegmentedControlProps props;

    explicit SegmentedControlWidget(SegmentedControlProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "SegmentedControl"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr segmentedControl(SegmentedControlProps props) {
    return std::make_shared<SegmentedControlWidget>(std::move(props));
}

} // namespace enki
