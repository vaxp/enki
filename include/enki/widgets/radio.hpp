#pragma once
/// @file radio.hpp
/// @brief Radio widget for single-choice selection.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

struct RadioProps {
    Key key = Key::none();
    int value = 0;
    int group_value = 0;
    std::function<void(int)> on_changed = nullptr;

    float size = 20.0f;                             ///< Diameter of the outer circle.
    float inner_size = 10.0f;                       ///< Diameter of the inner selected circle.
    float border_width = 2.0f;                      ///< Thickness of the unselected border.
    
    Color active_color = 0xFF2563EB;                ///< Color of border and inner circle when selected.
    Color inactive_color = 0xFF64748B;              ///< Color of border when unselected.
    Color hover_color = 0xFF3B82F6;                 ///< Color of border when hovered and unselected.
    Color bg_color = 0x00000000;                    ///< Background color inside the outer circle.
    
    bool disabled = false;                          ///< If true, the radio is non-interactive.
    Color disabled_color = 0xFF94A3B8;              ///< Color used when disabled.
};

/// @brief A material-style radio button for single selection from a group.
///
/// Uses `int` for the value and group_value types for simplicity and performance.
class Radio : public StatefulWidget {
public:
    int value;
    int group_value;
    std::function<void(int)> on_changed;
    RadioProps options;

    Radio(int value, int group_value, std::function<void(int)> on_changed = nullptr, RadioProps options = RadioProps())
        : value(value), group_value(group_value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    Radio(Key key, int value, int group_value, std::function<void(int)> on_changed, RadioProps options)
        : StatefulWidget(std::move(key)), value(value), group_value(group_value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Radio"; }
};

inline std::shared_ptr<Radio> radio(int value, int group_value, std::function<void(int)> on_changed = nullptr, RadioProps options = RadioProps()) {
    return std::make_shared<Radio>(value, group_value, std::move(on_changed), std::move(options));
}

inline std::shared_ptr<Radio> radio(RadioProps props) {
    if (props.on_changed == nullptr) props.disabled = true;
    return std::make_shared<Radio>(std::move(props.key), props.value, props.group_value, std::move(props.on_changed), std::move(props));
}

} // namespace enki
