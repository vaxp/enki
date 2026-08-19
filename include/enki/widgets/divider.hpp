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
// DividerOptions
// ════════════════════════════════════════════════════════════════

struct DividerOptions {
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
    std::string label;                 ///< Optional label centered on the divider.
    float label_font_size  = 11.5f;   ///< Font size of the label.
    Color label_color      = 0xFF64748B; ///< Color of the label text.
    float label_padding    = 10.0f;   ///< Horizontal padding around label.
    Color label_bg_color   = 0xFF0F172A; ///< Background behind label (matches parent bg).

    // Rounded caps
    bool round_caps = false;           ///< Draw round end caps on the line.
};

// ════════════════════════════════════════════════════════════════
// Divider Widget (Horizontal)
// ════════════════════════════════════════════════════════════════

class Divider : public SingleChildRenderObjectWidget {
public:
    DividerOptions options;

    explicit Divider(DividerOptions options = {})
        : options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Divider"; }
};

// ════════════════════════════════════════════════════════════════
// VerticalDivider Widget
// ════════════════════════════════════════════════════════════════

class VerticalDivider : public SingleChildRenderObjectWidget {
public:
    DividerOptions options;

    explicit VerticalDivider(DividerOptions options = {})
        : options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "VerticalDivider"; }
};

// ════════════════════════════════════════════════════════════════
// Factory helpers
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Divider> divider(DividerOptions options = {}) {
    return std::make_shared<Divider>(std::move(options));
}

inline std::shared_ptr<VerticalDivider> verticalDivider(DividerOptions options = {}) {
    return std::make_shared<VerticalDivider>(std::move(options));
}

} // namespace enki
