#pragma once
/// @file divider.hpp
/// @brief Advanced Divider & VerticalDivider widgets for ENKI Framework.
///
/// Supports solid, dashed, and gradient lines with optional center label,
/// rounded caps, and custom thickness — for both horizontal and vertical orientations.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// DividerStyle — line style enum
// ════════════════════════════════════════════════════════════════

enum class DividerStyle {
    Solid,      ///< Continuous solid line (default)
    Dashed,     ///< Dashed line pattern
    Dotted,     ///< Dotted (small square dots)
    Gradient,   ///< Gradient fade from color to transparent at edges
};

// ════════════════════════════════════════════════════════════════
// DividerProps
// ════════════════════════════════════════════════════════════════

struct DividerProps {
    Key   key        = Key::none();
    float height     = 16.0f;          ///< Total bounding box height (or width for VerticalDivider).
    float thickness  = 1.0f;           ///< Thickness of the drawn line.
    float indent     = 0.0f;           ///< Leading space (left for H, top for V).
    float end_indent = 0.0f;           ///< Trailing space (right for H, bottom for V).
    Color color      = 0xFF334155;     ///< Primary line color.

    DividerStyle style = DividerStyle::Solid; ///< Line style.

    // Dashed / dotted
    float dash_length = 6.0f;          ///< Dash segment length (Dashed/Dotted).
    float dash_gap    = 4.0f;          ///< Gap between segments.

    // Gradient (fades from transparent → color → transparent)
    bool  gradient_fade = false;       ///< Gradient fade at both ends.

    // Label in the middle (Horizontal only)
    std::string label = "";            ///< Optional label centered on the divider.
    float label_font_size  = 11.5f;   ///< Font size of the label.
    Color label_color      = 0xFF64748B; ///< Color of the label text.
    float label_padding    = 10.0f;   ///< Horizontal padding around label.
    Color label_bg_color   = 0xFF0F172A; ///< Background behind label (matches parent bg).

    // Rounded caps
    bool round_caps = false;           ///< Draw round end caps on the line.
};

// ════════════════════════════════════════════════════════════════
// DividerWidget & VerticalDividerWidget Implementations
// ════════════════════════════════════════════════════════════════

class DividerWidget : public SingleChildRenderObjectWidget {
public:
    DividerProps options;

    DividerWidget() = default;
    explicit DividerWidget(DividerProps opt)
        : SingleChildRenderObjectWidget(opt.key), options(std::move(opt)) {}
    DividerWidget(Key key, DividerProps opt)
        : SingleChildRenderObjectWidget(std::move(key)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Divider"; }
};

class VerticalDividerWidget : public SingleChildRenderObjectWidget {
public:
    DividerProps options;

    VerticalDividerWidget() = default;
    explicit VerticalDividerWidget(DividerProps opt)
        : SingleChildRenderObjectWidget(opt.key), options(std::move(opt)) {}
    VerticalDividerWidget(Key key, DividerProps opt)
        : SingleChildRenderObjectWidget(std::move(key)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "VerticalDivider"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Divider Structs (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Divider {
    Key   key        = Key::none();
    float height     = 16.0f;
    float thickness  = 1.0f;
    float indent     = 0.0f;
    float end_indent = 0.0f;
    Color color      = 0xFF334155;

    DividerStyle style = DividerStyle::Solid;

    // Dashed / dotted
    float dash_length = 6.0f;
    float dash_gap    = 4.0f;

    // Gradient
    bool  gradient_fade = false;

    // Label
    std::string label = "";
    float label_font_size  = 11.5f;
    Color label_color      = 0xFF64748B;
    float label_padding    = 10.0f;
    Color label_bg_color   = 0xFF0F172A;

    // Rounded caps
    bool round_caps = false;

    operator WidgetPtr() const {
        DividerProps p;
        p.key = key;
        p.height = height;
        p.thickness = thickness;
        p.indent = indent;
        p.end_indent = end_indent;
        p.color = color;
        p.style = style;
        p.dash_length = dash_length;
        p.dash_gap = dash_gap;
        p.gradient_fade = gradient_fade;
        p.label = label;
        p.label_font_size = label_font_size;
        p.label_color = label_color;
        p.label_padding = label_padding;
        p.label_bg_color = label_bg_color;
        p.round_caps = round_caps;
        return std::make_shared<DividerWidget>(key, std::move(p));
    }
};

struct VerticalDivider {
    Key   key        = Key::none();
    float width      = 16.0f;
    float height     = 16.0f;
    float thickness  = 1.0f;
    float indent     = 0.0f;
    float end_indent = 0.0f;
    Color color      = 0xFF334155;

    DividerStyle style = DividerStyle::Solid;

    float dash_length = 6.0f;
    float dash_gap    = 4.0f;
    bool  gradient_fade = false;
    bool  round_caps = false;

    operator WidgetPtr() const {
        DividerProps p;
        p.key = key;
        p.height = (width != 16.0f) ? width : height;
        p.thickness = thickness;
        p.indent = indent;
        p.end_indent = end_indent;
        p.color = color;
        p.style = style;
        p.dash_length = dash_length;
        p.dash_gap = dash_gap;
        p.gradient_fade = gradient_fade;
        p.round_caps = round_caps;
        return std::make_shared<VerticalDividerWidget>(key, std::move(p));
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Helpers
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<DividerWidget> divider(DividerProps props = {}) {
    return std::make_shared<DividerWidget>(std::move(props));
}

inline std::shared_ptr<VerticalDividerWidget> verticalDivider(DividerProps props = {}) {
    return std::make_shared<VerticalDividerWidget>(std::move(props));
}

} // namespace enki
