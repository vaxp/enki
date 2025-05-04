#pragma once
/// @file switch.hpp
/// @brief Switch widget for boolean toggles.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

/// Options for configuring a Switch.
struct SwitchOptions {
    float width = 44.0f;                            ///< Width of the switch track.
    float height = 24.0f;                           ///< Height of the switch track.
    float thumb_padding = 2.0f;                     ///< Padding between the track and the thumb.
    
    Color active_color = 0xFF34C759;                ///< Background color when on (default green).
    Color active_thumb_color = 0xFFFFFFFF;          ///< Thumb color when on.
    
    Color inactive_color = 0xFFE5E5EA;              ///< Background color when off (default light gray).
    Color inactive_thumb_color = 0xFFFFFFFF;        ///< Thumb color when off.
    
    Color hover_color = 0xFF28A745;                 ///< Background color when on and hovered.
    Color hover_inactive_color = 0xFFD1D1D6;        ///< Background color when off and hovered.
    
    bool disabled = false;                          ///< If true, the switch is non-interactive.
    Color disabled_color = 0xFFF2F2F7;              ///< Background color when disabled.
    Color disabled_thumb_color = 0xFFE5E5EA;        ///< Thumb color when disabled.
};

/// @brief A material/iOS-style switch for boolean selection.
class Switch : public StatefulWidget {
public:
    bool value;
    std::function<void(bool)> on_changed;
    SwitchOptions options;

    Switch(bool value, std::function<void(bool)> on_changed = nullptr, SwitchOptions options = SwitchOptions())
        : value(value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Switch"; }
};

inline WidgetPtr toggleSwitch(bool value, std::function<void(bool)> on_changed = nullptr, SwitchOptions options = SwitchOptions()) {
    return std::make_shared<Switch>(value, std::move(on_changed), std::move(options));
}

} // namespace enki
