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

struct SwitchProps {
    Key key = Key::none();
    bool value = false;
    std::function<void(bool)> on_changed = nullptr;

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
    SwitchProps options;

    Switch(bool value, std::function<void(bool)> on_changed = nullptr, SwitchProps options = SwitchProps())
        : value(value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    Switch(Key key, bool value, std::function<void(bool)> on_changed, SwitchProps options)
        : StatefulWidget(std::move(key)), value(value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Switch"; }
};

inline WidgetPtr toggleSwitch(bool value, std::function<void(bool)> on_changed = nullptr, SwitchProps options = SwitchProps()) {
    return std::make_shared<Switch>(value, std::move(on_changed), std::move(options));
}

inline std::shared_ptr<Switch> toggleSwitch(SwitchProps props) {
    if (props.on_changed == nullptr) props.disabled = true;
    return std::make_shared<Switch>(std::move(props.key), props.value, std::move(props.on_changed), std::move(props));
}

} // namespace enki
