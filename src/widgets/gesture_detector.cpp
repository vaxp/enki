/// @file gesture_detector.cpp
/// @brief Implementation of GestureDetector widget and RenderGestureDetector.

#include "enki/widgets/gesture_detector.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderGestureDetector Implementation
// ════════════════════════════════════════════════════════════════

RenderGestureDetector::RenderGestureDetector() {
    ANUNodeSetContext(anuNode(), this);
}

bool RenderGestureDetector::hitTest(HitTestResult& result, Point localPoint) {
    if (!hitTestSelf(localPoint)) {
        return false;
    }

    // First, test children (innermost first)
    bool childHit = hitTestChildren(result, localPoint);

    if (childHit) {
        result.add({this, localPoint});
        return true;
    }

    // If no child was hit, check behavior
    if (hit_test_behavior == HitTestBehavior::Opaque || hit_test_behavior == HitTestBehavior::Translucent) {
        result.add({this, localPoint});
        return true;
    }

    // HitTestBehavior::DeferToChild only consumes hit if a child was hit
    return false;
}

void RenderGestureDetector::handlePointerDown(const PointerEvent& e) {
    tap_recognizer.handlePointerDown(e);
    long_press_recognizer.handlePointerDown(e);
    pan_recognizer.handlePointerDown(e);
}

void RenderGestureDetector::handlePointerMove(const PointerEvent& e) {
    tap_recognizer.handlePointerMove(e);
    long_press_recognizer.handlePointerMove(e);
    pan_recognizer.handlePointerMove(e);

    if (on_hover_move) {
        on_hover_move(e);
    }
}

void RenderGestureDetector::handlePointerUp(const PointerEvent& e) {
    tap_recognizer.handlePointerUp(e);
    long_press_recognizer.handlePointerUp(e);
    pan_recognizer.handlePointerUp(e);
}

void RenderGestureDetector::handlePointerEnter(const PointerEvent& e) {
    if (on_hover_enter) {
        on_hover_enter(e);
    }
}

void RenderGestureDetector::handlePointerExit(const PointerEvent& e) {
    tap_recognizer.handlePointerCancel();
    long_press_recognizer.handlePointerCancel();
    pan_recognizer.handlePointerCancel();

    if (on_hover_exit) {
        on_hover_exit(e);
    }
}

void RenderGestureDetector::handlePointerScroll(float dx, float dy) {
    if (on_scroll) {
        on_scroll(dx, dy);
    }
}

void RenderGestureDetector::paint(PaintContext& context) {
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// GestureDetector Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> GestureDetector::createRenderObject(BuildContext&) {
    auto r = std::make_unique<RenderGestureDetector>();
    r->hit_test_behavior = hit_test_behavior;
    r->cursor_type       = cursor_type;

    // Tap Recognizer Callbacks
    r->tap_recognizer.on_tap_down           = on_tap_down;
    r->tap_recognizer.on_tap_up             = on_tap_up;
    r->tap_recognizer.on_tap                = on_tap;
    r->tap_recognizer.on_tap_cancel         = on_tap_cancel;
    r->tap_recognizer.on_secondary_tap_down = on_secondary_tap_down;
    r->tap_recognizer.on_secondary_tap_up   = on_secondary_tap_up;
    r->tap_recognizer.on_secondary_tap      = on_secondary_tap;
    r->tap_recognizer.on_double_tap_down    = on_double_tap_down;
    r->tap_recognizer.on_double_tap         = on_double_tap;
    r->tap_recognizer.on_double_tap_cancel  = on_double_tap_cancel;

    // Long Press Recognizer Callbacks
    r->long_press_recognizer.on_long_press_start = on_long_press_start;
    r->long_press_recognizer.on_long_press_move  = on_long_press_move;
    r->long_press_recognizer.on_long_press_end   = on_long_press_end;
    r->long_press_recognizer.on_long_press       = on_long_press;

    // Pan Recognizer Callbacks
    r->pan_recognizer.on_pan_start  = on_pan_start;
    r->pan_recognizer.on_pan_update = on_pan_update;
    r->pan_recognizer.on_pan_end    = on_pan_end;
    r->pan_recognizer.on_pan_cancel = on_pan_cancel;

    // Hover & Scroll Callbacks
    r->on_hover_enter = on_hover_enter;
    r->on_hover_exit  = on_hover_exit;
    r->on_hover_move  = on_hover_move;
    r->on_scroll      = on_scroll;

    return r;
}

void GestureDetector::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* r = dynamic_cast<RenderGestureDetector*>(&renderObject)) {
        r->hit_test_behavior = hit_test_behavior;
        r->cursor_type       = cursor_type;

        // Tap Recognizer Callbacks
        r->tap_recognizer.on_tap_down           = on_tap_down;
        r->tap_recognizer.on_tap_up             = on_tap_up;
        r->tap_recognizer.on_tap                = on_tap;
        r->tap_recognizer.on_tap_cancel         = on_tap_cancel;
        r->tap_recognizer.on_secondary_tap_down = on_secondary_tap_down;
        r->tap_recognizer.on_secondary_tap_up   = on_secondary_tap_up;
        r->tap_recognizer.on_secondary_tap      = on_secondary_tap;
        r->tap_recognizer.on_double_tap_down    = on_double_tap_down;
        r->tap_recognizer.on_double_tap         = on_double_tap;
        r->tap_recognizer.on_double_tap_cancel  = on_double_tap_cancel;

        // Long Press Recognizer Callbacks
        r->long_press_recognizer.on_long_press_start = on_long_press_start;
        r->long_press_recognizer.on_long_press_move  = on_long_press_move;
        r->long_press_recognizer.on_long_press_end   = on_long_press_end;
        r->long_press_recognizer.on_long_press       = on_long_press;

        // Pan Recognizer Callbacks
        r->pan_recognizer.on_pan_start  = on_pan_start;
        r->pan_recognizer.on_pan_update = on_pan_update;
        r->pan_recognizer.on_pan_end    = on_pan_end;
        r->pan_recognizer.on_pan_cancel = on_pan_cancel;

        // Hover & Scroll Callbacks
        r->on_hover_enter = on_hover_enter;
        r->on_hover_exit  = on_hover_exit;
        r->on_hover_move  = on_hover_move;
        r->on_scroll      = on_scroll;
    }
}

} // namespace enki
