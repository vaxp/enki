#pragma once
/// @file checkbox.hpp
/// @brief Checkbox widget for boolean input.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

/// Options for configuring a Checkbox.
struct CheckboxOptions {
    float size = 18.0f;                             ///< Width and height of the checkbox.
    float border_width = 2.0f;                      ///< Thickness of the border.
    float border_radius = 4.0f;                     ///< Corner radius.
    
    Color active_color = 0xFF2563EB;                ///< Background color when checked.
    Color check_color = 0xFFFFFFFF;                 ///< Color of the checkmark.
    
    Color border_color = 0xFF363B42;                ///< Border color when unchecked.
    Color hover_border_color = 0xFF58A6FF;          ///< Border color when hovered and unchecked.
    Color inactive_bg_color = 0x00000000;           ///< Background color when unchecked.
    
    bool disabled = false;                          ///< If true, the checkbox is non-interactive.
    Color disabled_color = 0xFF475569;              ///< Color used when disabled.
};

/// @brief A material-style checkbox for boolean selection.
class Checkbox : public StatefulWidget {
public:
    bool value;
    std::function<void(bool)> on_changed;
    CheckboxOptions options;

    Checkbox(bool value, std::function<void(bool)> on_changed = nullptr, CheckboxOptions options = CheckboxOptions())
        : value(value), on_changed(std::move(on_changed)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Checkbox"; }
};

inline WidgetPtr checkbox(bool value, std::function<void(bool)> on_changed = nullptr, CheckboxOptions options = CheckboxOptions()) {
    return std::make_shared<Checkbox>(value, std::move(on_changed), std::move(options));
}

} // namespace enki
