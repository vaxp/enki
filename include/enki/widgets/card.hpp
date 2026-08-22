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

/// @brief A material-style card widget.
///
/// It acts as a specialized Container with default styling suited for cards, 
/// such as rounded corners, surface background color, and soft shadows.
class CardWidget : public StatelessWidget {
public:
    WidgetPtr child;

    Color color = 0xFF1E293B;                       ///< Background color of the card.
    Color shadow_color = 0x40000000;                ///< Color of the drop shadow.
    float elevation = 8.0f;                         ///< Elevation of the card (controls shadow size).
    BorderRadius border_radius = BorderRadius::circular(12.0f); ///< Corner radius.
    std::optional<Border> border = std::nullopt;    ///< Optional border around the card.
    StyleInsets margin = StyleInsets::all(4.0f);      ///< Margin around the card.
    StyleInsets padding = StyleInsets::all(0.0f);      ///< Padding inside the card.

    explicit CardWidget(Key key) : StatelessWidget(std::move(key)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Card"; }
};

struct Card {
    Color color = 0xFF1E293B;                       
    Color shadow_color = 0x40000000;                
    float elevation = 8.0f;                         
    BorderRadius border_radius = BorderRadius::circular(12.0f); 
    std::optional<Border> border = std::nullopt;    
    StyleInsets margin = StyleInsets::all(4.0f);      
    StyleInsets padding = StyleInsets::all(0.0f);      
    WidgetPtr child = nullptr;
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<CardWidget>(key);
        w->color = color;
        w->shadow_color = shadow_color;
        w->elevation = elevation;
        w->border_radius = border_radius;
        w->border = border;
        w->margin = margin;
        w->padding = padding;
        w->child = child;
        return w;
    }
};

} // namespace enki
