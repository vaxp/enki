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
// GestureDetectorProps
// ════════════════════════════════════════════════════════════════

struct GestureDetectorProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    
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

    explicit GestureDetector(const GestureDetectorProps& props = {})
        : SingleChildRenderObjectWidget(props.key, props.child),
          hit_test_behavior(props.hit_test_behavior),
          cursor_type(props.cursor_type),
          on_tap_down(props.on_tap_down),
          on_tap_up(props.on_tap_up),
          on_tap(props.on_tap),
          on_tap_cancel(props.on_tap_cancel),
          on_secondary_tap_down(props.on_secondary_tap_down),
          on_secondary_tap_up(props.on_secondary_tap_up),
          on_secondary_tap(props.on_secondary_tap),
          on_double_tap_down(props.on_double_tap_down),
          on_double_tap(props.on_double_tap),
          on_double_tap_cancel(props.on_double_tap_cancel),
          on_long_press_start(props.on_long_press_start),
          on_long_press_move(props.on_long_press_move),
          on_long_press_end(props.on_long_press_end),
          on_long_press(props.on_long_press),
          on_pan_start(props.on_pan_start),
          on_pan_update(props.on_pan_update),
          on_pan_end(props.on_pan_end),
          on_pan_cancel(props.on_pan_cancel),
          on_hover_enter(props.on_hover_enter),
          on_hover_exit(props.on_hover_exit),
          on_hover_move(props.on_hover_move),
          on_scroll(props.on_scroll) {}

    GestureDetector(Key key, const GestureDetectorProps& props)
        : GestureDetector([&]() {
            auto p = props;
            p.key = std::move(key);
            return p;
        }()) {}

    // ── Render Object Creation & Reconciliation ────────────────
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "GestureDetector"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<GestureDetector> gestureDetector(const GestureDetectorProps& props = {}) {
    return std::make_shared<GestureDetector>(props);
}

inline std::shared_ptr<GestureDetector> gestureDetector(Key key, GestureDetectorProps props) {
    props.key = std::move(key);
    return std::make_shared<GestureDetector>(std::move(props));
}

} // namespace enki
