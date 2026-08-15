#pragma once
/// @file list_tile.hpp
/// @brief ListTile widget — a single row in a list with leading, title, subtitle, and trailing slots.
///
/// Features:
///   - Leading widget slot (icon, avatar, checkbox, etc.)
///   - Title and optional subtitle with configurable text styles
///   - Trailing widget slot (icon, badge, switch, etc.)
///   - Hover, pressed, selected visual states with smooth transitions
///   - Dense mode for compact display
///   - Configurable tile color, selected color, shape
///   - onTap, onLongPress, onSecondaryTap callbacks
///   - Enabled/disabled state
///   - Full Anu-driven layout — zero manual size calculations
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/animation/ticker.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Visual Density
// ════════════════════════════════════════════════════════════════

/// @brief Controls the visual compactness of a ListTile.
enum class VisualDensity {
    /// Standard spacing — comfortable touch targets.
    Standard,
    /// Compact spacing — denser information display for desktop.
    Compact,
    /// Comfortable spacing — slightly more relaxed than standard.
    Comfortable,
};

// ════════════════════════════════════════════════════════════════
// ListTileOptions
// ════════════════════════════════════════════════════════════════

/// @brief Visual and behavioral configuration for ListTile.
struct ListTileOptions {
    // ── Sizing & Density ───────────────────────────────────────
    VisualDensity visual_density = VisualDensity::Standard;

    /// Minimum height of the tile when there is one line of text.
    float min_height = 56.0f;
    /// Minimum height when subtitle is present (two-line tile).
    float min_height_two_line = 72.0f;
    /// Minimum height when the tile is dense.
    float dense_min_height = 48.0f;

    /// Padding inside the tile (content insets).
    EdgeInsets content_padding = EdgeInsets::symmetric(0.0f, 16.0f);

    /// Gap between the leading widget and the title/subtitle column.
    float leading_gap = 16.0f;
    /// Gap between the title/subtitle column and the trailing widget.
    float trailing_gap = 8.0f;

    // ── Colors ─────────────────────────────────────────────────
    Color tile_color          = Colors::Transparent;
    Color hover_color         = 0x0DFFFFFF;   // ~5% white overlay
    Color pressed_color       = 0x1AFFFFFF;   // ~10% white overlay
    Color selected_color      = 0x1A2563EB;   // ~10% primary blue
    Color focus_color         = 0x1A2563EB;
    Color disabled_color      = 0x40808080;   // muted overlay
    Color splash_color        = 0x33FFFFFF;

    // ── Shape ──────────────────────────────────────────────────
    BorderRadius shape = BorderRadius::zero();

    // ── Icon / Leading Constraints ─────────────────────────────
    /// Width reserved for the leading widget.
    float leading_width = 40.0f;
    /// Width reserved for the trailing widget.
    float trailing_width = 24.0f;

    // ── Interaction ────────────────────────────────────────────
    bool enable_feedback = true;   ///< Play haptic/audio feedback on tap.

    constexpr bool operator==(const ListTileOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderListTile — The Render Object
// ════════════════════════════════════════════════════════════════

/// @brief Render object for ListTile — handles hover/press/select painting and
///        delegates all layout (row sizing, padding, flex gaps) to Anu.
class RenderListTile : public RenderBox {
public:
    explicit RenderListTile(ListTileOptions options);
    ~RenderListTile() override = default;

    void setOptions(const ListTileOptions& options);
    [[nodiscard]] const ListTileOptions& options() const { return options_; }

    void setSelected(bool selected);
    void setEnabled(bool enabled);
    void setFocused(bool focused);

    [[nodiscard]] bool selected() const { return selected_; }
    [[nodiscard]] bool enabled()  const { return enabled_; }

    // ── RenderObject interface ──────────────────────────────────
    void paint(PaintContext& context) override;

    // ── Pointer events — drives hover/press visuals ─────────────
    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerEnter(const PointerEvent& e) override;
    void handlePointerExit(const PointerEvent& e) override;

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override;
    [[nodiscard]] SystemCursor cursor() const override {
        return enabled_ ? SystemCursor::Pointer : SystemCursor::Default;
    }

    // ── Ripple / animation tick ────────────────────────────────
    void tick(double now) override;

    // ── Callbacks (set by widget) ──────────────────────────────
    std::function<void()> on_tap;
    std::function<void()> on_long_press;
    std::function<void()> on_secondary_tap;

private:
    ListTileOptions options_;
    bool            selected_   = false;
    bool            enabled_    = true;
    bool            hovered_    = false;
    bool            pressed_    = false;
    bool            focused_    = false;

    std::unique_ptr<Ticker> ticker_;

    // Ripple animation state
    bool            ripple_active_ = false;
    Point           ripple_origin_ = {};
    double          ripple_start_  = 0.0;
    float           ripple_radius_ = 0.0f;
    float           ripple_alpha_  = 0.0f;

    static constexpr double kRippleDuration = 0.35;
    static constexpr float  kRippleMaxRadius = 200.0f;

    void paintBackground(PaintContext& ctx, const Rect& bounds);
    void paintRipple(PaintContext& ctx, const Rect& bounds);
};

// ════════════════════════════════════════════════════════════════
// ListTile Widget
// ════════════════════════════════════════════════════════════════

/// @brief A single row for a list — the fundamental building block of ListView.
///
/// Layout (Anu-driven):
/// ┌──────────────────────────────────────────────────────┐
/// │ [leading]  [title         ]  [trailing]              │
/// │            [subtitle      ]                          │
/// └──────────────────────────────────────────────────────┘
///
/// Usage:
/// @code
///   listTile()
///       ->leading(icon(Icons::Folder))
///       ->title(text("Documents"))
///       ->subtitle(text("42 items"))
///       ->trailing(icon(Icons::ChevronRight))
///       ->onTap([](){ /* open folder */ })
///       ->selected(true);
/// @endcode
class ListTile : public StatefulWidget {
public:
    // ── Slots ──────────────────────────────────────────────────
    WidgetPtr leading_widget;   ///< Leading slot (icon, avatar, checkbox).
    WidgetPtr title_widget;     ///< Required: main label widget.
    WidgetPtr subtitle_widget;  ///< Optional: secondary description.
    WidgetPtr trailing_widget;  ///< Trailing slot (icon, switch, badge).

    // ── State ──────────────────────────────────────────────────
    bool selected = false;      ///< Whether this tile is in a selected state.
    bool enabled  = true;       ///< Whether this tile is interactive.
    bool autofocus = false;

    // ── Callbacks ──────────────────────────────────────────────
    std::function<void()> on_tap;
    std::function<void()> on_long_press;
    std::function<void()> on_secondary_tap;

    // ── Configuration ──────────────────────────────────────────
    ListTileOptions options;

    ListTile() = default;

    // ── Fluent Builder API ─────────────────────────────────────

    ListTile& leading(WidgetPtr w) { leading_widget = std::move(w); return *this; }
    ListTile& title(WidgetPtr w)   { title_widget = std::move(w); return *this; }
    ListTile& subtitle(WidgetPtr w){ subtitle_widget = std::move(w); return *this; }
    ListTile& trailing(WidgetPtr w){ trailing_widget = std::move(w); return *this; }

    ListTile& onTap(std::function<void()> cb)          { on_tap = std::move(cb); return *this; }
    ListTile& onLongPress(std::function<void()> cb)    { on_long_press = std::move(cb); return *this; }
    ListTile& onSecondaryTap(std::function<void()> cb) { on_secondary_tap = std::move(cb); return *this; }

    ListTile& select(bool s)  { selected = s; return *this; }
    ListTile& enable(bool e)  { enabled = e; return *this; }
    ListTile& dense(bool d = true) {
        options.visual_density = d ? VisualDensity::Compact : VisualDensity::Standard;
        return *this;
    }
    ListTile& tileColor(Color c)     { options.tile_color = c; return *this; }
    ListTile& hoverColor(Color c)    { options.hover_color = c; return *this; }
    ListTile& selectedColor(Color c) { options.selected_color = c; return *this; }
    ListTile& shape(BorderRadius r)  { options.shape = r; return *this; }
    ListTile& contentPadding(EdgeInsets p) { options.content_padding = p; return *this; }
    ListTile& leadingGap(float g)  { options.leading_gap = g; return *this; }
    ListTile& trailingGap(float g) { options.trailing_gap = g; return *this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ListTile"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

/// Create a ListTile with title only.
inline std::shared_ptr<ListTile> listTile(WidgetPtr title) {
    auto t = std::make_shared<ListTile>();
    t->title_widget = std::move(title);
    return t;
}

/// Create a ListTile with leading icon and title.
inline std::shared_ptr<ListTile> listTile(WidgetPtr leading, WidgetPtr title) {
    auto t = std::make_shared<ListTile>();
    t->leading_widget = std::move(leading);
    t->title_widget   = std::move(title);
    return t;
}

/// Create a fully specified ListTile.
inline std::shared_ptr<ListTile> listTile(WidgetPtr leading, WidgetPtr title,
                                          WidgetPtr subtitle, WidgetPtr trailing = nullptr) {
    auto t = std::make_shared<ListTile>();
    t->leading_widget  = std::move(leading);
    t->title_widget    = std::move(title);
    t->subtitle_widget = std::move(subtitle);
    t->trailing_widget = std::move(trailing);
    return t;
}

/// Create an empty ListTile for building via fluent API.
inline std::shared_ptr<ListTile> listTile() {
    return std::make_shared<ListTile>();
}

} // namespace enki
