#pragma once
/// @file skia_canvas.hpp
/// @brief Declarative Skia 2D Canvas widget for custom painting and graphics (C++20 designated initializers).
///
/// Features:
///   - 100% C++20 Declarative Syntax (designated initializers)
///   - High-level Enki Canvas API painter: void(Canvas& canvas, Size size)
///   - Direct Skia SkCanvas* painter: void(SkCanvas* canvas, Size size)
///   - Foreground painter for overlay rendering in front of child widgets
///   - Seamless child widget composition (painter -> child -> foreground_painter)
///   - Automatic Anu Flexbox layout integration (width, height, min/max, percent/auto/point)
///   - Repaint controller integration (AnimationController drives frame updates)
///   - Geometric clipping (HardEdge, AntiAlias, BorderRadius)
///   - Custom shape hit-testing delegation
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/animation/animation_controller.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>

class SkCanvas;

namespace enki {

/// Callback signatures
using CanvasPainterCallback       = std::function<void(Canvas& canvas, Size size)>;
using SkiaDirectPainterCallback   = std::function<void(SkCanvas* canvas, Size size)>;
using CanvasHitTestCallback       = std::function<bool(Point localPoint, Size size)>;

// ════════════════════════════════════════════════════════════════
// SkiaCanvasStyle
// ════════════════════════════════════════════════════════════════

struct SkiaCanvasStyle {
    CanvasPainterCallback                painter                 = nullptr;
    SkiaDirectPainterCallback            skia_painter            = nullptr;
    CanvasPainterCallback                foreground_painter      = nullptr;
    SkiaDirectPainterCallback            skia_foreground_painter = nullptr;
    CanvasHitTestCallback                hit_test                = nullptr;
    std::shared_ptr<AnimationController> repaint                 = nullptr;

    std::optional<StyleValue>            width                   = std::nullopt;
    std::optional<StyleValue>            height                  = std::nullopt;
    std::optional<StyleValue>            min_width               = std::nullopt;
    std::optional<StyleValue>            min_height              = std::nullopt;
    std::optional<StyleValue>            max_width               = std::nullopt;
    std::optional<StyleValue>            max_height              = std::nullopt;

    Clip                                 clip_behavior           = Clip::None;
    BorderRadius                         clip_radius             = BorderRadius::zero();
    bool                                 is_complex              = false;
};

// ════════════════════════════════════════════════════════════════
// SkiaCanvasWidget
// ════════════════════════════════════════════════════════════════

class SkiaCanvasWidget : public SingleChildRenderObjectWidget {
public:
    SkiaCanvasStyle style;

    SkiaCanvasWidget() = default;
    explicit SkiaCanvasWidget(SkiaCanvasStyle s, WidgetPtr child = nullptr, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), style(std::move(s)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "SkiaCanvas"; }
};

// ════════════════════════════════════════════════════════════════
// SkiaCanvasProps & Declarative Factory
// ════════════════════════════════════════════════════════════════

struct SkiaCanvasProps {
    CanvasPainterCallback                painter                 = nullptr;
    SkiaDirectPainterCallback            skia_painter            = nullptr;
    CanvasPainterCallback                foreground_painter      = nullptr;
    SkiaDirectPainterCallback            skia_foreground_painter = nullptr;
    WidgetPtr                            child                   = nullptr;
    CanvasHitTestCallback                hit_test                = nullptr;
    std::shared_ptr<AnimationController> repaint                 = nullptr;

    std::optional<StyleValue>            width                   = std::nullopt;
    std::optional<StyleValue>            height                  = std::nullopt;
    std::optional<StyleValue>            min_width               = std::nullopt;
    std::optional<StyleValue>            min_height              = std::nullopt;
    std::optional<StyleValue>            max_width               = std::nullopt;
    std::optional<StyleValue>            max_height              = std::nullopt;

    Clip                                 clip_behavior           = Clip::None;
    BorderRadius                         clip_radius             = BorderRadius::zero();
    bool                                 is_complex              = false;
    Key                                  key                     = Key::none();

    operator WidgetPtr() const;
};

struct SkiaCanvas : public SkiaCanvasProps {
    using SkiaCanvasProps::SkiaCanvasProps;
};

inline WidgetPtr skiaCanvas(const SkiaCanvasProps& props = {}) {
    return static_cast<WidgetPtr>(props);
}

} // namespace enki
