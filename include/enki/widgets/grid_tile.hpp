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

class GridTileBarWidget : public StatelessWidget {
public:
    GridTileBarProps props;

    GridTileBarWidget() = default;
    explicit GridTileBarWidget(GridTileBarProps p) : StatelessWidget(p.key), props(std::move(p)) {}
    GridTileBarWidget(Key k, GridTileBarProps p) : StatelessWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "GridTileBar"; }
};

// ════════════════════════════════════════════════════════════════
// GridTile
// ════════════════════════════════════════════════════════════════

struct GridTileProps {
    Key key = Key::none();
    WidgetPtr child;           ///< Main content (fills the tile).
    WidgetPtr header;          ///< Optional overlay at the top.
    WidgetPtr footer;          ///< Optional overlay at the bottom.
};

class GridTileWidget : public StatelessWidget {
public:
    GridTileProps props;

    GridTileWidget() = default;
    explicit GridTileWidget(GridTileProps p) : StatelessWidget(p.key), props(std::move(p)) {}
    GridTileWidget(Key k, GridTileProps p) : StatelessWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "GridTile"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct GridTileBar {
    Key key = Key::none();
    WidgetPtr leading = nullptr;
    WidgetPtr leading_widget = nullptr;
    WidgetPtr title = nullptr;
    WidgetPtr title_widget = nullptr;
    WidgetPtr subtitle = nullptr;
    WidgetPtr subtitle_widget = nullptr;
    WidgetPtr trailing = nullptr;
    WidgetPtr trailing_widget = nullptr;

    Color background_color = 0xCC000000;
    float padding_vertical   = 8.0f;
    float padding_horizontal = 8.0f;
    float leading_gap        = 8.0f;
    float trailing_gap       = 8.0f;

    operator WidgetPtr() const {
        GridTileBarProps p;
        p.key = key;
        p.leading_widget = leading ? leading : leading_widget;
        p.title_widget = title ? title : title_widget;
        p.subtitle_widget = subtitle ? subtitle : subtitle_widget;
        p.trailing_widget = trailing ? trailing : trailing_widget;
        p.background_color = background_color;
        p.padding_vertical = padding_vertical;
        p.padding_horizontal = padding_horizontal;
        p.leading_gap = leading_gap;
        p.trailing_gap = trailing_gap;
        return std::make_shared<GridTileBarWidget>(key, std::move(p));
    }
};

struct GridTile {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    WidgetPtr header = nullptr;
    WidgetPtr footer = nullptr;

    operator WidgetPtr() const {
        GridTileProps p;
        p.key = key;
        p.child = child;
        p.header = header;
        p.footer = footer;
        return std::make_shared<GridTileWidget>(key, std::move(p));
    }
};

} // namespace enki
