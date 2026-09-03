#pragma once
/// @file otp_field.hpp
/// @brief OTPField widget for ENKI Framework.
/// Segmented One-Time Password input with auto-focus advance, paste distribution, and glowing boxes.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace enki {

struct OTPFieldProps {
    int                                 length = 6;
    float                               box_size = 48.0f;
    float                               gap = 10.0f;
    float                               border_radius = 10.0f;
    bool                                is_obscured = false;
    bool                                has_error = false;
    bool                                auto_focus = false;
    std::string                         initial_value = "";

    Color                               active_border_color = 0xFF00E5FF;
    Color                               inactive_border_color = 0x33FFFFFF;
    Color                               box_background = 0x4D000000;
    Color                               text_color = 0xFFFFFFFF;
    Color                               error_color = 0xFFEF4444;

    std::function<void(const std::string&)> on_changed;
    std::function<void(const std::string&)> on_completed;

    operator WidgetPtr() const;
};

class OTPFieldWidget : public StatefulWidget {
public:
    OTPFieldProps props;

    explicit OTPFieldWidget(OTPFieldProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "OTPField"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr otpField(OTPFieldProps props) {
    return std::make_shared<OTPFieldWidget>(std::move(props));
}

} // namespace enki
