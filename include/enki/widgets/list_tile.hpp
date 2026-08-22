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

enum class VisualDensity {
    Standard,
    Compact,
    Comfortable,
};

// ════════════════════════════════════════════════════════════════
// ListTileProps
// ════════════════════════════════════════════════════════════════

struct ListTileProps {
    Key key = Key::none();

    // ── Slots ──────────────────────────────────────────────────
    WidgetPtr leading_widget;
    WidgetPtr title_widget;
    WidgetPtr subtitle_widget;
    WidgetPtr trailing_widget;

    // ── State ──────────────────────────────────────────────────
    bool selected = false;
    bool enabled  = true;
    bool autofocus = false;

    // ── Callbacks ──────────────────────────────────────────────
    std::function<void()> on_tap;
    std::function<void()> on_long_press;
    std::function<void()> on_secondary_tap;

    // ── Sizing & Density ───────────────────────────────────────
    VisualDensity visual_density = VisualDensity::Standard;

    float min_height = 56.0f;
    float min_height_two_line = 72.0f;
    float dense_min_height = 48.0f;

    EdgeInsets content_padding = EdgeInsets::symmetric(0.0f, 16.0f);

    float leading_gap = 16.0f;
    float trailing_gap = 8.0f;

    // ── Colors ─────────────────────────────────────────────────
    Color tile_color          = Colors::Transparent;
    Color hover_color         = 0x0DFFFFFF;
    Color pressed_color       = 0x1AFFFFFF;
    Color selected_color      = 0x1A2563EB;
    Color focus_color         = 0x1A2563EB;
    Color disabled_color      = 0x40808080;
    Color splash_color        = 0x33FFFFFF;

    // ── Shape ──────────────────────────────────────────────────
    BorderRadius shape = BorderRadius::zero();

    // ── Icon / Leading Constraints ─────────────────────────────
    float leading_width = 40.0f;
    float trailing_width = 24.0f;

    // ── Interaction ────────────────────────────────────────────
    bool enable_feedback = true;
};

// ════════════════════════════════════════════════════════════════
// RenderListTile
// ════════════════════════════════════════════════════════════════

class RenderListTile : public RenderBox {
public:
    explicit RenderListTile(ListTileProps props);
    ~RenderListTile() override = default;

    void setProps(const ListTileProps& props);
    [[nodiscard]] const ListTileProps& props() const { return props_; }

    void setSelected(bool selected);
    void setEnabled(bool enabled);
    void setFocused(bool focused);

    [[nodiscard]] bool selected() const { return selected_; }
    [[nodiscard]] bool enabled()  const { return enabled_; }

    void paint(PaintContext& context) override;

    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerEnter(const PointerEvent& e) override;
    void handlePointerExit(const PointerEvent& e) override;

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override;
    [[nodiscard]] SystemCursor cursor() const override {
        return enabled_ ? SystemCursor::Pointer : SystemCursor::Default;
    }

    void tick(double now) override;

    std::function<void()> on_tap;
    std::function<void()> on_long_press;
    std::function<void()> on_secondary_tap;

private:
    ListTileProps props_;
    bool            selected_   = false;
    bool            enabled_    = true;
    bool            hovered_    = false;
    bool            pressed_    = false;
    bool            focused_    = false;

    std::unique_ptr<Ticker> ticker_;

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
// ListTile Widget Implementation
// ════════════════════════════════════════════════════════════════

class ListTileWidget : public StatefulWidget {
public:
    ListTileProps props;

    ListTileWidget() = default;
    explicit ListTileWidget(ListTileProps p) : StatefulWidget(p.key), props(std::move(p)) {}
    ListTileWidget(Key k, ListTileProps p) : StatefulWidget(std::move(k)), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ListTile"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Proxy Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct ListTile {
    Key key = Key::none();

    WidgetPtr leading = nullptr;
    WidgetPtr leading_widget = nullptr;
    WidgetPtr title = nullptr;
    WidgetPtr title_widget = nullptr;
    WidgetPtr subtitle = nullptr;
    WidgetPtr subtitle_widget = nullptr;
    WidgetPtr trailing = nullptr;
    WidgetPtr trailing_widget = nullptr;

    bool selected = false;
    bool enabled  = true;
    bool autofocus = false;

    std::function<void()> on_tap = nullptr;
    std::function<void()> on_long_press = nullptr;
    std::function<void()> on_secondary_tap = nullptr;

    VisualDensity visual_density = VisualDensity::Standard;

    float min_height = 56.0f;
    float min_height_two_line = 72.0f;
    float dense_min_height = 48.0f;

    EdgeInsets content_padding = EdgeInsets::symmetric(0.0f, 16.0f);

    float leading_gap = 16.0f;
    float trailing_gap = 8.0f;

    Color tile_color          = Colors::Transparent;
    Color hover_color         = 0x0DFFFFFF;
    Color pressed_color       = 0x1AFFFFFF;
    Color selected_color      = 0x1A2563EB;
    Color focus_color         = 0x1A2563EB;
    Color disabled_color      = 0x40808080;
    Color splash_color        = 0x33FFFFFF;

    BorderRadius shape = BorderRadius::zero();

    float leading_width = 40.0f;
    float trailing_width = 24.0f;

    bool enable_feedback = true;

    operator WidgetPtr() const {
        ListTileProps p;
        p.key = key;
        p.leading_widget = leading ? leading : leading_widget;
        p.title_widget = title ? title : title_widget;
        p.subtitle_widget = subtitle ? subtitle : subtitle_widget;
        p.trailing_widget = trailing ? trailing : trailing_widget;
        p.selected = selected;
        p.enabled = enabled;
        p.autofocus = autofocus;
        p.on_tap = on_tap;
        p.on_long_press = on_long_press;
        p.on_secondary_tap = on_secondary_tap;
        p.visual_density = visual_density;
        p.min_height = min_height;
        p.min_height_two_line = min_height_two_line;
        p.dense_min_height = dense_min_height;
        p.content_padding = content_padding;
        p.leading_gap = leading_gap;
        p.trailing_gap = trailing_gap;
        p.tile_color = tile_color;
        p.hover_color = hover_color;
        p.pressed_color = pressed_color;
        p.selected_color = selected_color;
        p.focus_color = focus_color;
        p.disabled_color = disabled_color;
        p.splash_color = splash_color;
        p.shape = shape;
        p.leading_width = leading_width;
        p.trailing_width = trailing_width;
        p.enable_feedback = enable_feedback;
        return std::make_shared<ListTileWidget>(key, std::move(p));
    }
};

} // namespace enki
