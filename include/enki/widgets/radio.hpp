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

/// Options for configuring a Radio button.
struct RadioOptions {
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
    RadioOptions options;

    Radio(int value, int group_value, std::function<void(int)> on_changed = nullptr, RadioOptions options = RadioOptions())
        : value(value), group_value(group_value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Radio"; }
};

inline WidgetPtr radio(int value, int group_value, std::function<void(int)> on_changed = nullptr, RadioOptions options = RadioOptions()) {
    return std::make_shared<Radio>(value, group_value, std::move(on_changed), std::move(options));
}

} // namespace enki
