#pragma once
/// @file popup.hpp
/// @brief Universal Native Popup widget built on NativePopup.
///
/// Popup is the core generalized widget for spawning floating native compositor
/// surfaces with arbitrary content, 12-direction anchor placement, cursor tracking,
/// modal placement, and programmatic controller support.
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

/// Positioning mode for Popup relative to anchor target or screen
enum class PopupPlacement {
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    LeftTop,
    LeftCenter,
    LeftBottom,
    RightTop,
    RightCenter,
    RightBottom,
    FollowCursor,
    CenterScreen,
    Manual
};

/// Trigger mode for opening/closing Popup
enum class PopupTrigger {
    Click,
    Hover,
    LongPress,
    SecondaryClick,
    Manual
};

/// @brief Programmatic controller for Popup state.
class PopupController {
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

/// Configuration options for Popup styling, placement, and behavior
struct PopupWidgetOptions {
    PopupPlacement placement   = PopupPlacement::BottomCenter;
    PopupTrigger trigger       = PopupTrigger::Click;

    Point manual_position      = {0.0f, 0.0f}; ///< Used when placement == Manual
    Point offset               = {0.0f, 0.0f}; ///< Additional pixel offset (x, y)

    Color background_color     = 0xFA1F242C;   ///< ARGB background color
    Color border_color         = 0xFF363B42;   ///< Outer border stroke color
    float border_width         = 1.0f;
    float border_radius        = 10.0f;

    float elevation            = 12.0f;        ///< Skia drop shadow blur
    Color shadow_color         = 0x60000000;

    EdgeInsets padding         = EdgeInsets::all(12.0f);
    Size content_size          = Size{240.0f, 160.0f};
    bool auto_dismiss          = true;         ///< Auto close on click outside

    std::string custom_shader  = "";           ///< Optional SkSL runtime shader
};

using PopupConfig = PopupWidgetOptions;

struct PopupProps {
    Key key = Key::none();
    WidgetPtr child;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> builder;
    std::function<WidgetPtr(BuildContext&)> simple_builder;
    PopupWidgetOptions options;
    std::shared_ptr<PopupController> controller;
};

/// @brief Universal Popup widget implementation wrapping an anchor child and popup content.
class PopupWidget : public StatefulWidget {
public:
    WidgetPtr child;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder;
    PopupWidgetOptions options;
    std::shared_ptr<PopupController> controller;

    PopupWidget(WidgetPtr child,
                std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
                PopupWidgetOptions options = PopupWidgetOptions(),
                std::shared_ptr<PopupController> controller = nullptr)
        : child(std::move(child)),
          popup_builder(std::move(popup_builder)),
          options(std::move(options)),
          controller(std::move(controller)) {}

    PopupWidget(WidgetPtr child,
                std::function<WidgetPtr(BuildContext&)> simple_builder,
                PopupWidgetOptions options = PopupWidgetOptions(),
                std::shared_ptr<PopupController> controller = nullptr)
        : child(std::move(child)),
          popup_builder([b = std::move(simple_builder)](BuildContext& ctx, std::shared_ptr<NativePopup>) {
              return b ? b(ctx) : nullptr;
          }),
          options(std::move(options)),
          controller(std::move(controller)) {}

    PopupWidget(Key key,
                WidgetPtr child,
                std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
                PopupWidgetOptions options = PopupWidgetOptions(),
                std::shared_ptr<PopupController> controller = nullptr)
        : StatefulWidget(std::move(key)),
          child(std::move(child)),
          popup_builder(std::move(popup_builder)),
          options(std::move(options)),
          controller(std::move(controller)) {}

    PopupWidget(Key key,
                WidgetPtr child,
                std::function<WidgetPtr(BuildContext&)> simple_builder,
                PopupWidgetOptions options = PopupWidgetOptions(),
                std::shared_ptr<PopupController> controller = nullptr)
        : StatefulWidget(std::move(key)),
          child(std::move(child)),
          popup_builder([b = std::move(simple_builder)](BuildContext& ctx, std::shared_ptr<NativePopup>) {
              return b ? b(ctx) : nullptr;
          }),
          options(std::move(options)),
          controller(std::move(controller)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Popup"; }

    /// Static imperative launcher to open a Popup anywhere programmatically.
    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
        PopupWidgetOptions options = PopupWidgetOptions()
    );

    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&)> popup_builder,
        PopupWidgetOptions options = PopupWidgetOptions()
    ) {
        return show(context, [b = std::move(popup_builder)](BuildContext& ctx, std::shared_ptr<NativePopup>) {
            return b ? b(ctx) : nullptr;
        }, options);
    }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Popup {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> builder = nullptr;
    std::function<WidgetPtr(BuildContext&)> simple_builder = nullptr;
    PopupWidgetOptions options = {};
    std::shared_ptr<PopupController> controller = nullptr;

    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> popup_builder,
        PopupWidgetOptions options = PopupWidgetOptions()) {
        return PopupWidget::show(context, std::move(popup_builder), options);
    }

    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        std::function<WidgetPtr(BuildContext&)> popup_builder,
        PopupWidgetOptions options = PopupWidgetOptions()) {
        return PopupWidget::show(context, std::move(popup_builder), options);
    }

    operator WidgetPtr() const {
        if (builder) {
            return std::make_shared<PopupWidget>(key, child, builder, options, controller);
        } else if (simple_builder) {
            return std::make_shared<PopupWidget>(key, child, simple_builder, options, controller);
        } else {
            return std::make_shared<PopupWidget>(key, child, [](BuildContext&, std::shared_ptr<NativePopup>) -> WidgetPtr {
                return nullptr;
            }, options, controller);
        }
    }
};

} // namespace enki
