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

struct DismissibleOptions {
    DismissDirection direction = DismissDirection::Horizontal;
    float dismiss_threshold = 0.35f;   ///< Fraction of width needed to trigger dismissal

    WidgetPtr background;              ///< Revealed when swiping right (StartToEnd)
    WidgetPtr secondary_background;    ///< Revealed when swiping left (EndToStart)

    std::function<void(DismissDirection dir)> on_dismissed;
    std::function<bool(DismissDirection dir)> confirm_dismiss;
    std::function<void()> on_resize;
};

/// ════════════════════════════════════════════════════════════════
/// Dismissible Widget
/// ════════════════════════════════════════════════════════════════

class Dismissible : public StatefulWidget {
public:
    std::string id = "";
    WidgetPtr child;
    DismissibleOptions options;

    Dismissible() = default;
    Dismissible(std::string id_, WidgetPtr child_, DismissibleOptions opts = {})
        : id(std::move(id_)), child(std::move(child_)), options(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Dismissible"; }
};

inline std::shared_ptr<Dismissible> dismissible(
    std::string id,
    WidgetPtr child,
    DismissibleOptions options = {}) {
    return std::make_shared<Dismissible>(std::move(id), std::move(child), std::move(options));
}

} // namespace enki
