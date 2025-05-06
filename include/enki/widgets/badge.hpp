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

struct BadgeOptions {
    Color bg_color = 0xFFEF4444;                            ///< Background color (default Red).
    Alignment alignment = Alignment::TopRight;              ///< Placement of the badge relative to the child.
    Point offset = {0.0f, 0.0f};                            ///< X,Y offset for fine-tuning positioning.
    float size = 12.0f;                                     ///< Diameter if it's an empty dot.
    StyleInsets padding = StyleInsets::symmetric(2.0f, 6.0f); ///< Padding if it contains a label.
    BorderRadius border_radius = BorderRadius::circular(10.0f); ///< Corner radius.
};

/// @brief A material-style badge widget.
///
/// Draws a badge (notification counter or status dot) over a child widget.
class Badge : public StatelessWidget {
public:
    WidgetPtr child;
    WidgetPtr label;
    BadgeOptions options;

    Badge() = default;
    explicit Badge(WidgetPtr c) : child(std::move(c)) {}
    Badge(WidgetPtr c, WidgetPtr l) : child(std::move(c)), label(std::move(l)) {}
    Badge(WidgetPtr c, WidgetPtr l, BadgeOptions opt)
        : child(std::move(c)), label(std::move(l)), options(std::move(opt)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Badge"; }

    // ── Fluent Builder API ─────────────────────────────────────

    Badge& bgColor(Color c) { options.bg_color = c; return *this; }
    Badge& alignment(Alignment a) { options.alignment = a; return *this; }
    Badge& offset(float x, float y) { options.offset = {x, y}; return *this; }
    Badge& size(float s) { options.size = s; return *this; }
    Badge& padding(StyleInsets p) { options.padding = p; return *this; }
};

// ── Global Factory Helper ────────────────────────────────────

inline std::shared_ptr<Badge> badge(WidgetPtr child, WidgetPtr label = nullptr) {
    return std::make_shared<Badge>(std::move(child), std::move(label));
}

} // namespace enki
