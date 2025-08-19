#pragma once
/// @file badge.hpp
/// @brief Badge widget for notifications, counters, and status dots.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"

namespace enki {

/// @brief A material-style badge widget.
///
/// Draws a badge (notification counter or status dot) over a child widget.
class BadgeWidget : public StatelessWidget {
public:
    WidgetPtr child;
    WidgetPtr label;
    
    Color bg_color = 0xFFEF4444;                            ///< Background color (default Red).
    Alignment alignment = Alignment::TopRight;              ///< Placement of the badge relative to the child.
    Point offset = {0.0f, 0.0f};                            ///< X,Y offset for fine-tuning positioning.
    float size = 12.0f;                                     ///< Diameter if it's an empty dot.
    StyleInsets padding = StyleInsets::symmetric(2.0f, 6.0f); ///< Padding if it contains a label.
    BorderRadius border_radius = BorderRadius::circular(10.0f); ///< Corner radius.

    explicit BadgeWidget(Key key) : StatelessWidget(std::move(key)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Badge"; }
};

struct Badge {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    WidgetPtr label = nullptr;

    Color bg_color = 0xFFEF4444;
    Alignment alignment = Alignment::TopRight;
    Point offset = {0.0f, 0.0f};
    float size = 12.0f;
    StyleInsets padding = StyleInsets::symmetric(2.0f, 6.0f);
    BorderRadius border_radius = BorderRadius::circular(10.0f);

    operator WidgetPtr() const {
        auto w = std::make_shared<BadgeWidget>(key);
        w->child = child;
        w->label = label;
        w->bg_color = bg_color;
        w->alignment = alignment;
        w->offset = offset;
        w->size = size;
        w->padding = padding;
        w->border_radius = border_radius;
        return w;
    }
};

} // namespace enki
