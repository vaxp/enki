#pragma once
/// @file recognizer.hpp
/// @brief Gesture recognizer classes for parsing low-level pointer events into semantic gestures.
///
/// Implements TapGestureRecognizer, LongPressGestureRecognizer, and PanGestureRecognizer.
///
/// @copyright ENKI Framework — MIT License

#include "enki/gestures/gesture_types.hpp"
#include <cmath>
#include <chrono>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Gesture Recognizer Base Class
// ════════════════════════════════════════════════════════════════

class GestureRecognizer {
public:
    virtual ~GestureRecognizer() = default;

    virtual void handlePointerDown(const PointerEvent& e) = 0;
    virtual void handlePointerMove(const PointerEvent& e) = 0;
    virtual void handlePointerUp(const PointerEvent& e)   = 0;
    virtual void handlePointerCancel()                    = 0;
    virtual void reset()                                  = 0;

protected:
    static double getCurrentTimeSeconds() {
        using namespace std::chrono;
        auto now = steady_clock::now().time_since_epoch();
        return duration_cast<duration<double>>(now).count();
    }

    static float distance(Point a, Point b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// ════════════════════════════════════════════════════════════════
// Tap Gesture Recognizer
// ════════════════════════════════════════════════════════════════

class TapGestureRecognizer : public GestureRecognizer {
public:
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

    float  slop_distance       = 18.0f;  ///< Maximum pixels pointer can move during tap
    double double_tap_timeout  = 0.300;  ///< Maximum seconds between two taps for double-tap

    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerCancel() override;
    void reset() override;

private:
    bool        is_down_           = false;
    Point       down_position_     = {0, 0};
    Point       down_local_pos_    = {0, 0};
    MouseButton down_button_       = MouseButton::Left;
    int         down_modifiers_    = 0;
    double      down_time_         = 0.0;

    // Double-tap tracking
    int         consecutive_taps_  = 0;
    Point       last_tap_pos_      = {0, 0};
    double      last_tap_time_     = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Long Press Gesture Recognizer
// ════════════════════════════════════════════════════════════════

class LongPressGestureRecognizer : public GestureRecognizer {
public:
    GestureLongPressStartCallback on_long_press_start;
    GestureLongPressMoveCallback  on_long_press_move;
    GestureLongPressEndCallback   on_long_press_end;
    GestureLongPressCallback      on_long_press;

    float  slop_distance       = 14.0f;  ///< Maximum displacement before gesture is rejected
    double duration_threshold  = 0.400;  ///< Seconds to trigger long-press (~400ms)

    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerCancel() override;
    void reset() override;

private:
    bool   is_down_        = false;
    bool   triggered_      = false;
    Point  down_pos_       = {0, 0};
    Point  down_local_pos_ = {0, 0};
    Point  last_local_pos_ = {0, 0};
    double down_time_      = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Pan / Drag Gesture Recognizer
// ════════════════════════════════════════════════════════════════

class PanGestureRecognizer : public GestureRecognizer {
public:
    GestureDragStartCallback  on_pan_start;
    GestureDragUpdateCallback on_pan_update;
    GestureDragEndCallback    on_pan_end;
    GestureDragCancelCallback on_pan_cancel;

    float touch_slop = 6.0f; ///< Movement needed to claim drag gesture

    void handlePointerDown(const PointerEvent& e) override;
    void handlePointerMove(const PointerEvent& e) override;
    void handlePointerUp(const PointerEvent& e) override;
    void handlePointerCancel() override;
    void reset() override;

private:
    bool   is_down_          = false;
    bool   is_dragging_      = false;
    Point  start_position_   = {0, 0};
    Point  start_local_pos_  = {0, 0};
    Point  last_position_    = {0, 0};
    Point  last_local_pos_   = {0, 0};
    double last_update_time_ = 0.0;
    Point  recent_velocity_  = {0, 0};
};

} // namespace enki
