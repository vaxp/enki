#pragma once
/// @file gesture_detector.hpp
/// @brief Comprehensive GestureDetector widget for pointer gestures, hover, scroll, and cursor dispatching.
///
/// Features:
///   - Tap, DoubleTap, Secondary (Right-click) Tap, and LongPress detection.
///   - 2D Pan/Drag with real-time delta displacement and velocity calculation.
///   - HitTestBehavior configuration (DeferToChild, Opaque, Translucent).
///   - System cursor customization on hover.
///   - Full integration with Anu Flexbox and Element Reconciliation.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/gestures/recognizer.hpp"
#include <memory>
#include <functional>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderGestureDetector
// ════════════════════════════════════════════════════════════════

class RenderGestureDetector : public RenderBox {
public:
    HitTestBehavior hit_test_behavior = HitTestBehavior::DeferToChild;
    SystemCursor    cursor_type       = SystemCursor::Default;

    // Gesture Recognizers
    TapGestureRecognizer       tap_recognizer;
    LongPressGestureRecognizer long_press_recognizer;
    PanGestureRecognizer       pan_recognizer;

    // Hover & Scroll Callbacks
    GestureHoverCallback       on_hover_enter;
    GestureHoverCallback       on_hover_exit;
    GestureHoverCallback       on_hover_move;
    GestureScrollCallback      on_scroll;

    RenderGestureDetector();
    ~RenderGestureDetector() override = default;

    // ── Hit Testing & Dispatch ─────────────────────────────────
    bool hitTest(HitTestResult& result, Point localPoint) override;
    bool handlesScroll() const override { return static_cast<bool>(on_scroll); }
    SystemCursor cursor() const override { return cursor_type; }
    void tick(double now) override;
    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerEnter(const PointerEvent& e) override;
    void handlePointerExit(const PointerEvent& e) override;
    void handlePointerScroll(float dx, float dy) override;

    void paint(PaintContext& context) override;
};

// ════════════════════════════════════════════════════════════════
// GestureDetector Widget
// ════════════════════════════════════════════════════════════════

class GestureDetector : public SingleChildRenderObjectWidget {
public:
    HitTestBehavior hit_test_behavior = HitTestBehavior::DeferToChild;
    SystemCursor    cursor_type       = SystemCursor::Default;

    // ── Tap Callbacks ──────────────────────────────────────────
    GestureTapDownCallback   on_tap_down;
    GestureTapUpCallback     on_tap_up;
    GestureTapCallback       on_tap;
    GestureTapCancelCallback on_tap_cancel;

    GestureTapDownCallback   on_secondary_tap_down;
    GestureTapUpCallback     on_secondary_tap_up;
    GestureTapCallback       on_secondary_tap;

    GestureTapDownCallback   on_double_tap_down;
    GestureTapCallback       on_double_tap;
    GestureTapCancelCallback on_double_tap_cancel;

    // ── Long Press Callbacks ───────────────────────────────────
    GestureLongPressStartCallback on_long_press_start;
    GestureLongPressMoveCallback  on_long_press_move;
    GestureLongPressEndCallback   on_long_press_end;
    GestureLongPressCallback      on_long_press;

    // ── Pan / Drag Callbacks ───────────────────────────────────
    GestureDragStartCallback  on_pan_start;
    GestureDragUpdateCallback on_pan_update;
    GestureDragEndCallback    on_pan_end;
    GestureDragCancelCallback on_pan_cancel;

    // ── Hover & Scroll Callbacks ───────────────────────────────
    GestureHoverCallback      on_hover_enter;
    GestureHoverCallback      on_hover_exit;
    GestureHoverCallback      on_hover_move;
    GestureScrollCallback     on_scroll;

    explicit GestureDetector(WidgetPtr child = nullptr)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}

    GestureDetector(Key key, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)) {}

    // ── Fluent Builder API ─────────────────────────────────────

    GestureDetector& onTap(GestureTapCallback cb) { on_tap = std::move(cb); return *this; }
    GestureDetector& onTapDown(GestureTapDownCallback cb) { on_tap_down = std::move(cb); return *this; }
    GestureDetector& onTapUp(GestureTapUpCallback cb) { on_tap_up = std::move(cb); return *this; }
    GestureDetector& onTapCancel(GestureTapCancelCallback cb) { on_tap_cancel = std::move(cb); return *this; }

    GestureDetector& onSecondaryTap(GestureTapCallback cb) { on_secondary_tap = std::move(cb); return *this; }
    GestureDetector& onSecondaryTapDown(GestureTapDownCallback cb) { on_secondary_tap_down = std::move(cb); return *this; }
    GestureDetector& onSecondaryTapUp(GestureTapUpCallback cb) { on_secondary_tap_up = std::move(cb); return *this; }

    GestureDetector& onDoubleTap(GestureTapCallback cb) { on_double_tap = std::move(cb); return *this; }
    GestureDetector& onDoubleTapDown(GestureTapDownCallback cb) { on_double_tap_down = std::move(cb); return *this; }
    GestureDetector& onDoubleTapCancel(GestureTapCancelCallback cb) { on_double_tap_cancel = std::move(cb); return *this; }

    GestureDetector& onLongPress(GestureLongPressCallback cb) { on_long_press = std::move(cb); return *this; }
    GestureDetector& onLongPressStart(GestureLongPressStartCallback cb) { on_long_press_start = std::move(cb); return *this; }
    GestureDetector& onLongPressMove(GestureLongPressMoveCallback cb) { on_long_press_move = std::move(cb); return *this; }
    GestureDetector& onLongPressEnd(GestureLongPressEndCallback cb) { on_long_press_end = std::move(cb); return *this; }

    GestureDetector& onPanStart(GestureDragStartCallback cb) { on_pan_start = std::move(cb); return *this; }
    GestureDetector& onPanUpdate(GestureDragUpdateCallback cb) { on_pan_update = std::move(cb); return *this; }
    GestureDetector& onPanEnd(GestureDragEndCallback cb) { on_pan_end = std::move(cb); return *this; }
    GestureDetector& onPanCancel(GestureDragCancelCallback cb) { on_pan_cancel = std::move(cb); return *this; }

    GestureDetector& onHoverEnter(GestureHoverCallback cb) { on_hover_enter = std::move(cb); return *this; }
    GestureDetector& onHoverExit(GestureHoverCallback cb) { on_hover_exit = std::move(cb); return *this; }
    GestureDetector& onHoverMove(GestureHoverCallback cb) { on_hover_move = std::move(cb); return *this; }
    GestureDetector& onScroll(GestureScrollCallback cb) { on_scroll = std::move(cb); return *this; }

    GestureDetector& cursor(SystemCursor c) { cursor_type = c; return *this; }
    GestureDetector& behavior(HitTestBehavior b) { hit_test_behavior = b; return *this; }
    GestureDetector& hitTestBehavior(HitTestBehavior b) { hit_test_behavior = b; return *this; }

    // ── Render Object Creation & Reconciliation ────────────────
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "GestureDetector"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<GestureDetector> gestureDetector(WidgetPtr child = nullptr) {
    return std::make_shared<GestureDetector>(std::move(child));
}

inline std::shared_ptr<GestureDetector> gestureDetector(Key key, WidgetPtr child = nullptr) {
    return std::make_shared<GestureDetector>(std::move(key), std::move(child));
}

} // namespace enki
