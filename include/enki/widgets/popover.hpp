#pragma once
/// @file popover.hpp
/// @brief Advanced Native Popover widget built on NativePopup.
///
/// Popover spawns a floating native compositor surface (NativePopup) containing
/// arbitrary interactive content (forms, buttons, controls, lists) attached
/// to a target anchor widget.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/shell/shell_types.hpp"

#include <string>
#include <memory>
#include <functional>

namespace enki {

/// Preferred opening direction for Popover relative to target
enum class PopoverDirection {
    Top,
    Bottom,
    Left,
    Right,
    Auto
};

/// Popover alignment relative to target edge
enum class PopoverAlignment {
    Start,
    Center,
    End
};

/// Trigger mode for opening/closing Popover
enum class PopoverTrigger {
    Click,
    Hover,
    Manual
};

/// @brief Programmatic controller for Popover state.
class PopoverController {
private:
    std::function<void(bool)> toggle_callback_;
    bool is_open_ = false;

public:
    void setToggleCallback(std::function<void(bool)> cb) {
        toggle_callback_ = std::move(cb);
    }

    void show() {
        is_open_ = true;
        if (toggle_callback_) toggle_callback_(true);
    }

    void hide() {
        is_open_ = false;
        if (toggle_callback_) toggle_callback_(false);
    }

    void toggle() {
        if (is_open_) hide();
        else show();
    }

    [[nodiscard]] bool isOpen() const { return is_open_; }
};

/// Configuration options for Popover styling and positioning
struct PopoverOptions {
    PopoverDirection direction = PopoverDirection::Top;
    PopoverAlignment alignment = PopoverAlignment::Center;
    PopoverTrigger trigger     = PopoverTrigger::Click;

    Color background_color     = 0xFA1F242C; ///< ARGB background color
    Color border_color         = 0xFF363B42; ///< Border stroke color
    float border_width         = 1.0f;
    float border_radius        = 10.0f;

    float arrow_size           = 10.0f;     ///< Pointer arrow size in pixels
    bool show_arrow            = true;

    float elevation            = 12.0f;     ///< Drop shadow distance
    Color shadow_color         = 0x60000000;

    EdgeInsets padding         = EdgeInsets::all(12.0f);
    Size content_size          = Size{260.0f, 180.0f};
    bool auto_dismiss          = true;      ///< Close on click outside

    std::string custom_shader  = "";        ///< Optional SkSL shader code
};

/// @brief Popover widget wrapping a target anchor child and popover content.
class Popover : public StatefulWidget {
public:
    WidgetPtr child;
    std::function<WidgetPtr(BuildContext&)> popover_builder;
    PopoverOptions options;
    std::shared_ptr<PopoverController> controller;

    Popover(WidgetPtr child,
            std::function<WidgetPtr(BuildContext&)> popover_builder,
            PopoverOptions options = PopoverOptions(),
            std::shared_ptr<PopoverController> controller = nullptr)
        : child(std::move(child)),
          popover_builder(std::move(popover_builder)),
          options(std::move(options)),
          controller(std::move(controller)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Popover"; }
};

// ── Factory Helpers ────────────────────────────────────────────────

inline WidgetPtr popover(
    WidgetPtr child,
    std::function<WidgetPtr(BuildContext&)> popover_builder,
    PopoverOptions options = PopoverOptions(),
    std::shared_ptr<PopoverController> controller = nullptr) {
    return std::make_shared<Popover>(
        std::move(child),
        std::move(popover_builder),
        std::move(options),
        std::move(controller)
    );
}

} // namespace enki
