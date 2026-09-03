#pragma once
/// @file knob.hpp
/// @brief Knob (Rotary Dial) widget for ENKI Framework.
/// Studio-grade circular rotary dial input for audio gear, synthesizers, and instruments.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <functional>
#include <memory>

namespace enki {

struct KnobProps {
    float                               value = 0.0f;
    float                               min_value = 0.0f;
    float                               max_value = 100.0f;
    float                               step = 1.0f;
    float                               size = 72.0f;
    bool                                is_bipolar = false; // center 0, e.g. -50 to +50
    bool                                show_value = true;

    std::string                         label = "";
    std::string                         unit = "%";

    Color                               active_color = 0xFF00E5FF;
    Color                               track_color = 0x3300E5FF;
    Color                               dial_color = 0xFF0F172A;
    Color                               pointer_color = 0xFFFFFFFF;
    Color                               text_color = 0xFF94A3B8;

    std::function<void(float)>          on_value_changed;

    operator WidgetPtr() const;
};

class KnobWidget : public SingleChildRenderObjectWidget {
public:
    KnobProps props;

    explicit KnobWidget(KnobProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "Knob"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr knob(KnobProps props) {
    return std::make_shared<KnobWidget>(std::move(props));
}

} // namespace enki
