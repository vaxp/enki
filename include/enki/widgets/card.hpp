#pragma once
/// @file card.hpp
/// @brief Card widget for grouping content with elevation, rounded corners, and shadow.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/container.hpp"
#include <optional>

namespace enki {

struct CardProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    Color color = 0xFF1E293B;                       ///< Background color of the card.
    Color shadow_color = 0x40000000;                ///< Color of the drop shadow.
    float elevation = 8.0f;                         ///< Elevation of the card (controls shadow size).
    BorderRadius border_radius = BorderRadius::circular(12.0f); ///< Corner radius.
    std::optional<Border> border = std::nullopt;    ///< Optional border around the card.
    EdgeInsets margin = EdgeInsets::all(4.0f);      ///< Margin around the card.
    StyleInsets padding = StyleInsets::all(0.0f);      ///< Padding inside the card.
};

/// @brief A material-style card widget.
///
/// It acts as a specialized Container with default styling suited for cards, 
/// such as rounded corners, surface background color, and soft shadows.
class Card : public StatelessWidget {
public:
    WidgetPtr child;
    CardProps options;

    Card() = default;
    explicit Card(WidgetPtr c) : child(std::move(c)) {}
    Card(WidgetPtr c, CardProps opt)
        : child(std::move(c)), options(std::move(opt)) {}
    Card(Key key, WidgetPtr c, CardProps opt)
        : StatelessWidget(std::move(key)), child(std::move(c)), options(std::move(opt)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Card"; }

    // ── Fluent Builder API ─────────────────────────────────────

    Card& color(Color c) { options.color = c; return *this; }
    Card& shadowColor(Color c) { options.shadow_color = c; return *this; }
    Card& elevation(float e) { options.elevation = e; return *this; }
    Card& borderRadius(BorderRadius r) { options.border_radius = r; return *this; }
    Card& borderRadius(float r) { options.border_radius = BorderRadius::circular(r); return *this; }
    Card& border(Border b) { options.border = b; return *this; }
    Card& border(Color c, float w) { options.border = Border(c, w); return *this; }
    Card& margin(EdgeInsets m) { options.margin = m; return *this; }
    Card& marginAll(float m) { options.margin = EdgeInsets::all(m); return *this; }
    Card& padding(StyleInsets p) { options.padding = p; return *this; }
    Card& paddingAll(float p) { options.padding = StyleInsets::all(p); return *this; }
};

// ── Global Factory Helper ────────────────────────────────────

inline std::shared_ptr<Card> card(WidgetPtr child) {
    return std::make_shared<Card>(std::move(child));
}

inline std::shared_ptr<Card> card(CardProps props) {
    return std::make_shared<Card>(std::move(props.key), std::move(props.child), std::move(props));
}

inline std::shared_ptr<Card> card(CardProps props, WidgetPtr child) {
    props.child = std::move(child);
    return card(std::move(props));
}

} // namespace enki
