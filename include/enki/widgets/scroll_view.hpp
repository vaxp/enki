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
#include <optional>

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
// ScrollViewWidget Engine Implementation
// ════════════════════════════════════════════════════════════════

class ScrollViewWidget : public SingleChildRenderObjectWidget {
public:
    ScrollOptions options;

    ScrollViewWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    ScrollViewWidget(ScrollOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), options(std::move(opt)) {}
    ScrollViewWidget(Key k, ScrollOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(k), std::move(child)), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ScrollView"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative ScrollView Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct ScrollView {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    Axis direction = Axis::Vertical;
    bool clamp_overscroll = true;
    bool show_scrollbar = false;
    float scroll_speed = 50.0f;

    operator WidgetPtr() const {
        ScrollOptions opt;
        opt.direction = direction;
        opt.clamp_overscroll = clamp_overscroll;
        opt.show_scrollbar = show_scrollbar;
        opt.scroll_speed = scroll_speed;
        return std::make_shared<ScrollViewWidget>(key, opt, child);
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

struct ScrollProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    Axis direction = Axis::Vertical;
    bool clamp_overscroll = true;
    bool show_scrollbar = false;
    float scroll_speed = 50.0f; // Multiplier for mouse wheel

    operator WidgetPtr() const {
        ScrollOptions opt;
        opt.direction = direction;
        opt.clamp_overscroll = clamp_overscroll;
        opt.show_scrollbar = show_scrollbar;
        opt.scroll_speed = scroll_speed;
        return std::make_shared<ScrollViewWidget>(key, opt, child);
    }
};

inline std::shared_ptr<ScrollViewWidget> scrollView(WidgetPtr child) {
    return std::make_shared<ScrollViewWidget>(std::move(child));
}

inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollOptions opt, WidgetPtr child) {
    return std::make_shared<ScrollViewWidget>(std::move(opt), std::move(child));
}

inline std::shared_ptr<ScrollViewWidget> scrollView(Key k, ScrollOptions opt, WidgetPtr child) {
    return std::make_shared<ScrollViewWidget>(std::move(k), std::move(opt), std::move(child));
}

inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollProps props) {
    ScrollOptions opt;
    opt.direction = props.direction;
    opt.clamp_overscroll = props.clamp_overscroll;
    opt.show_scrollbar = props.show_scrollbar;
    opt.scroll_speed = props.scroll_speed;
    return std::make_shared<ScrollViewWidget>(std::move(props.key), opt, std::move(props.child));
}

inline std::shared_ptr<ScrollViewWidget> scrollView(ScrollProps props, WidgetPtr child) {
    props.child = std::move(child);
    return scrollView(std::move(props));
}

} // namespace enki
