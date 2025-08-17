#pragma once
/// @file tooltip.hpp
/// @brief Advanced Native Tooltip widget built on NativePopup.
///
/// Tooltips are spawned as native compositor popup surfaces (NativePopup)
/// allowing them to float over window boundaries and desktop shell panels.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/shell/shell_types.hpp"

#include <string>
#include <memory>
#include <chrono>
#include <functional>

namespace enki {

/// Position options for positioning the tooltip relative to target widget
enum class TooltipPosition {
    Auto,   ///< Automatically pick best position based on screen boundaries
    Top,    ///< Above the target widget
    Bottom, ///< Below the target widget
    Left,   ///< To the left of the target widget
    Right   ///< To the right of the target widget
};

/// Trigger events that activate the tooltip
enum class TooltipTrigger {
    Hover,     ///< Show on mouse hover
    LongPress, ///< Show on touch long-press
    Tap,       ///< Toggle on tap/click
    Manual     ///< Programmatically triggered only
};

/// Configuration options for styling and behavior of Tooltip
struct TooltipOptions {
    Color background_color = 0xEE0F172A; ///< Dark translucent slate (0xAARRGGBB)
    Color text_color       = 0xFFF8FAFC; ///< Light off-white
    Color border_color     = 0x3394A3B8; ///< Subtle slate border
    float border_width     = 1.0f;
    float border_radius    = 8.0f;
    EdgeInsets padding     = EdgeInsets::symmetric(6.0f, 12.0f);
    float elevation        = 6.0f;
    Color shadow_color     = 0x40000000;

    float arrow_size       = 6.0f;               ///< Height/width of arrow pointer tail
    TooltipPosition position = TooltipPosition::Auto;
    TooltipTrigger trigger   = TooltipTrigger::Hover;

    std::chrono::milliseconds show_delay{400};   ///< Delay before tooltip appears on hover
    std::chrono::milliseconds hide_delay{150};   ///< Delay before tooltip hides on hover exit
    bool interactive       = false;              ///< Keep open when pointer moves onto tooltip body
    std::string custom_shader = "";              ///< SkSL custom shader code for glassmorphism/effects

    float font_size        = 13.0f;
};

struct TooltipProps {
    Key key = Key::none();
    WidgetPtr child;
    std::string message;
    WidgetPtr rich_message;
    TooltipOptions options;
};

/// @brief Tooltip widget wrapping a target child widget.
class Tooltip : public StatefulWidget {
public:
    WidgetPtr child;
    std::string message;
    WidgetPtr rich_message;
    TooltipOptions options;

    /// Constructor for text message tooltip
    Tooltip(WidgetPtr child, std::string message, TooltipOptions options = TooltipOptions())
        : child(std::move(child)), message(std::move(message)), rich_message(nullptr), options(std::move(options)) {}

    /// Constructor for rich custom widget content tooltip
    Tooltip(WidgetPtr child, WidgetPtr rich_message, TooltipOptions options = TooltipOptions())
        : child(std::move(child)), message(""), rich_message(std::move(rich_message)), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Tooltip"; }
};

/// Helper function to construct a text Tooltip
inline WidgetPtr tooltip(WidgetPtr child, std::string message, TooltipOptions options = TooltipOptions()) {
    return std::make_shared<Tooltip>(std::move(child), std::move(message), std::move(options));
}

/// Helper function to construct a rich widget Tooltip
inline WidgetPtr tooltip(WidgetPtr child, WidgetPtr rich_message, TooltipOptions options = TooltipOptions()) {
    return std::make_shared<Tooltip>(std::move(child), std::move(rich_message), std::move(options));
}

inline WidgetPtr tooltip(TooltipProps props) {
    auto tt = props.rich_message 
        ? std::make_shared<Tooltip>(std::move(props.child), std::move(props.rich_message), std::move(props.options))
        : std::make_shared<Tooltip>(std::move(props.child), std::move(props.message), std::move(props.options));
    tt->key = props.key;
    return tt;
}

} // namespace enki
