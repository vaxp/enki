#pragma once
/// @file scroll_view.hpp
/// @brief ScrollView widget for ENKI Framework
///
/// Features:
///   - 100% Anu-driven Layout using Overflow::Scroll.
///   - Context coordinate translations for hit-testing and painting.
///   - Mouse wheel and touch pan scrolling.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ScrollOptions
// ════════════════════════════════════════════════════════════════

struct ScrollOptions {
    Axis direction = Axis::Vertical;
    bool clamp_overscroll = true;
    bool show_scrollbar = false;
    float scroll_speed = 50.0f; // Multiplier for mouse wheel

    constexpr bool operator==(const ScrollOptions&) const = default;
};

// ════════════════════════════════════════════════════════════════
// RenderScrollView
// ════════════════════════════════════════════════════════════════

class RenderScrollView : public RenderBox {
public:
    ScrollOptions options;

    RenderScrollView(ScrollOptions opt);
    ~RenderScrollView() override = default;

    void setOptions(const ScrollOptions& opt);

    // Coordinate state
    float scroll_offset_x = 0.0f;
    float scroll_offset_y = 0.0f;
    float max_scroll_x = 0.0f;
    float max_scroll_y = 0.0f;

    // Gesture Recognizer for touch scrolling
    PanGestureRecognizer pan_recognizer;

    // Hit Testing & Painting overrides
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;
    void paint(PaintContext& context) override;
    void syncLayout() override;

    // Event overrides
    bool handlesScroll() const override { return true; }
    void handlePointerScroll(float dx, float dy) override;
    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;

    void updateScrollOffsets(float dx, float dy);
};

// ════════════════════════════════════════════════════════════════
// ScrollView Widget
// ════════════════════════════════════════════════════════════════

class ScrollView : public SingleChildRenderObjectWidget {
public:
    ScrollOptions options;

    ScrollView(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    ScrollView(ScrollOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), options(std::move(opt)) {}
    ScrollView(Key k, ScrollOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(k), std::move(child)), options(std::move(opt)) {}

    // Fluent API
    ScrollView& direction(Axis dir) { options.direction = dir; return *this; }
    ScrollView& showScrollbar(bool show) { options.show_scrollbar = show; return *this; }
    ScrollView& scrollSpeed(float speed) { options.scroll_speed = speed; return *this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ScrollView"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<ScrollView> scrollView(WidgetPtr child) {
    return std::make_shared<ScrollView>(std::move(child));
}

inline std::shared_ptr<ScrollView> scrollView(ScrollOptions opt, WidgetPtr child) {
    return std::make_shared<ScrollView>(std::move(opt), std::move(child));
}

inline std::shared_ptr<ScrollView> scrollView(Key k, ScrollOptions opt, WidgetPtr child) {
    return std::make_shared<ScrollView>(std::move(k), std::move(opt), std::move(child));
}

} // namespace enki
