#pragma once
/// @file dismissible.hpp
/// @brief Dismissible widget for swipe-to-dismiss actions in ENKI Framework (Category 9. Gestures / Interaction).
/// Supports horizontal/vertical swipe gestures, dual action backgrounds, and dismissal animation.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>

namespace enki {

/// Dismiss direction
enum class DismissDirection {
    Horizontal,     ///< Can be dismissed by swiping left or right
    EndToStart,     ///< Can only be dismissed by swiping left
    StartToEnd,     ///< Can only be dismissed by swiping right
    Vertical        ///< Can be dismissed by swiping up or down
};

/// ════════════════════════════════════════════════════════════════
/// Dismissible Options
/// ════════════════════════════════════════════════════════════════

struct DismissibleProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::string id = "";

    DismissDirection direction = DismissDirection::Horizontal;
    float dismiss_threshold = 0.35f;   ///< Fraction of width needed to trigger dismissal

    WidgetPtr background = nullptr;              ///< Revealed when swiping right (StartToEnd)
    WidgetPtr secondary_background = nullptr;    ///< Revealed when swiping left (EndToStart)

    std::function<void(DismissDirection dir)> on_dismissed = nullptr;
    std::function<bool(DismissDirection dir)> confirm_dismiss = nullptr;
    std::function<void()> on_resize = nullptr;
};

/// ════════════════════════════════════════════════════════════════
/// Dismissible Implementation Widget
/// ════════════════════════════════════════════════════════════════

class DismissibleWidget : public StatefulWidget {
public:
    std::string id = "";
    WidgetPtr child;
    DismissibleProps options;

    DismissibleWidget() = default;
    DismissibleWidget(std::string id_, WidgetPtr child_, DismissibleProps opts = {})
        : id(std::move(id_)), child(std::move(child_)), options(std::move(opts)) {}
    DismissibleWidget(Key key, std::string id_, WidgetPtr child_, DismissibleProps opts = {})
        : StatefulWidget(std::move(key)), id(std::move(id_)), child(std::move(child_)), options(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Dismissible"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Dismissible {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::string id = "";

    DismissDirection direction = DismissDirection::Horizontal;
    float dismiss_threshold = 0.35f;

    WidgetPtr background = nullptr;
    WidgetPtr secondary_background = nullptr;

    std::function<void(DismissDirection dir)> on_dismissed = nullptr;
    std::function<bool(DismissDirection dir)> confirm_dismiss = nullptr;
    std::function<void()> on_resize = nullptr;

    operator WidgetPtr() const {
        DismissibleProps opts;
        opts.key = key;
        opts.child = child;
        opts.id = id;
        opts.direction = direction;
        opts.dismiss_threshold = dismiss_threshold;
        opts.background = background;
        opts.secondary_background = secondary_background;
        opts.on_dismissed = on_dismissed;
        opts.confirm_dismiss = confirm_dismiss;
        opts.on_resize = on_resize;
        return std::make_shared<DismissibleWidget>(key, id, child, std::move(opts));
    }
};

} // namespace enki
