#pragma once
/// @file grid_tile.hpp
/// @brief GridTile and GridTileBar widgets — cells for GridView with layered decoration.
///
/// Features:
///   - GridTile:    A Stack-based cell with optional header and footer overlays.
///   - GridTileBar: A ready-made overlay bar with title, subtitle, and trailing action.
///   - Layered overlay with configurable gradient scrim for readability over images.
///   - Full Anu-driven layout — children sized by Anu within the tile's Anu node.
///
/// Typical usage with GridView:
/// @code
///   gridView(3, [](int i) {
///       return gridTile(
///           image("photo.jpg"),
///           GridTileBar()
///               .title(text("Photo " + std::to_string(i)))
///               .trailing(iconButton(Icons::FavoriteOutline, []{}))
///       );
///   });
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// GridTileBar
// ════════════════════════════════════════════════════════════════

/// @brief A bar widget designed to be placed as a header or footer overlay
///        on a GridTile, typically over an image or media content.
///
/// Renders a translucent background scrim to maintain text legibility
/// regardless of the underlying content color.
///
/// Layout: Row → [leading?] [title/subtitle Column (flex=1)] [trailing?]
struct GridTileBarProps {
    Key key = Key::none();
    WidgetPtr leading_widget;
    WidgetPtr title_widget;
    WidgetPtr subtitle_widget;
    WidgetPtr trailing_widget;

    // ── Visual ─────────────────────────────────────────────────
    Color    background_color = 0xCC000000; ///< Background scrim (default: 80% opaque black).
    float    padding_vertical   = 8.0f;
    float    padding_horizontal = 8.0f;
    float    leading_gap        = 8.0f;
    float    trailing_gap       = 8.0f;
};

class GridTileBar : public StatelessWidget {
public:
    GridTileBarProps props;

    GridTileBar() = default;
    explicit GridTileBar(GridTileBarProps p) : props(std::move(p)) {}

    // ── Fluent Builder ─────────────────────────────────────────
    GridTileBar& leading(WidgetPtr w)   { props.leading_widget = std::move(w); return *this; }
    GridTileBar& title(WidgetPtr w)     { props.title_widget = std::move(w); return *this; }
    GridTileBar& subtitle(WidgetPtr w)  { props.subtitle_widget = std::move(w); return *this; }
    GridTileBar& trailing(WidgetPtr w)  { props.trailing_widget = std::move(w); return *this; }
    GridTileBar& backgroundColor(Color c) { props.background_color = c; return *this; }
    GridTileBar& padding(float v, float h) {
        props.padding_vertical = v;
        props.padding_horizontal = h;
        return *this;
    }

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "GridTileBar"; }
};

inline std::shared_ptr<GridTileBar> gridTileBar(GridTileBarProps props = {}) {
    return std::make_shared<GridTileBar>(std::move(props));
}

// ════════════════════════════════════════════════════════════════
// GridTile
// ════════════════════════════════════════════════════════════════

/// @brief A single cell in a GridView, with optional header and footer overlays.
///
/// Architecture (Stack-based, Anu-driven):
/// ┌──────────────────────────────────┐
/// │ [header — top overlay]           │
/// │                                  │
/// │      child (content)             │
/// │                                  │
/// │ [footer — bottom overlay]        │
/// └──────────────────────────────────┘
///
/// The child fills the tile completely. The header and footer
/// are absolutely positioned at the top and bottom of the Stack.
///
/// Usage:
/// @code
///   gridTile(
///       image("thumb.jpg")->fit(BoxFit::Cover),
///       nullptr,  // no header
///       GridTileBar()
///           .title(text("Vacation 2024"))
///           .trailing(iconButton(Icons::Share, []{}))
///   );
/// @endcode
struct GridTileProps {
    Key key = Key::none();
    WidgetPtr child;           ///< Main content (fills the tile).
    WidgetPtr header;          ///< Optional overlay at the top.
    WidgetPtr footer;          ///< Optional overlay at the bottom.
};

class GridTile : public StatelessWidget {
public:
    GridTileProps props;

    GridTile() = default;
    explicit GridTile(GridTileProps p) : props(std::move(p)) {}

    // ── Fluent Builder ─────────────────────────────────────────
    GridTile& setChild(WidgetPtr w)  { props.child = std::move(w); return *this; }
    GridTile& setHeader(WidgetPtr w) { props.header = std::move(w); return *this; }
    GridTile& setFooter(WidgetPtr w) { props.footer = std::move(w); return *this; }

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "GridTile"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<GridTile> gridTile(GridTileProps props = {}) {
    return std::make_shared<GridTile>(std::move(props));
}

inline std::shared_ptr<GridTile> gridTile(WidgetPtr child) {
    GridTileProps props;
    props.child = std::move(child);
    return std::make_shared<GridTile>(std::move(props));
}

inline std::shared_ptr<GridTile> gridTile(WidgetPtr child, WidgetPtr footer) {
    GridTileProps props;
    props.child = std::move(child);
    props.footer = std::move(footer);
    return std::make_shared<GridTile>(std::move(props));
}

inline std::shared_ptr<GridTile> gridTile(WidgetPtr child, WidgetPtr header, WidgetPtr footer) {
    GridTileProps props;
    props.child = std::move(child);
    props.header = std::move(header);
    props.footer = std::move(footer);
    return std::make_shared<GridTile>(std::move(props));
}

} // namespace enki
