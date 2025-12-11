#pragma once
/// @file pin_field.hpp
/// @brief PinField widget for ENKI Framework.
/// Secure PIN input with delayed masking, masked bullets, and glowing state indicators.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace enki {

struct PinFieldProps {
    int                                 length = 4;
    float                               box_size = 46.0f;
    float                               gap = 12.0f;
    float                               border_radius = 12.0f;
    int                                 obscure_delay_ms = 400; // 0 = immediate
    std::string                         mask_char = "●";
    bool                                numeric_only = true;
    bool                                has_error = false;
    bool                                auto_focus = false;
    std::string                         initial_value = "";

    Color                               active_border_color = 0xFF00E5FF;
    Color                               inactive_border_color = 0x33FFFFFF;
    Color                               filled_dot_color = 0xFF38BDF8;
    Color                               box_background = 0x59000000;
    Color                               error_color = 0xFFEF4444;

    std::function<void(const std::string&)> on_changed;
    std::function<void(const std::string&)> on_completed;

    operator WidgetPtr() const;
};

class PinFieldWidget : public StatefulWidget {
public:
    PinFieldProps props;

    explicit PinFieldWidget(PinFieldProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "PinField"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr pinField(PinFieldProps props) {
    return std::make_shared<PinFieldWidget>(std::move(props));
}

} // namespace enki
