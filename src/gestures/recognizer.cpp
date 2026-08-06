/// @file recognizer.cpp
/// @brief Implementation of gesture recognizers for ENKI Framework.

#include "enki/gestures/recognizer.hpp"
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// TapGestureRecognizer Implementation
// ════════════════════════════════════════════════════════════════

void TapGestureRecognizer::handlePointerDown(const PointerEvent& e) {
    down_position_   = e.position;
    down_local_pos_  = e.localPosition;
    down_button_     = e.button;
    down_modifiers_  = e.modifiers;
    down_time_       = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());
    is_down_         = true;

    bool has_double_tap = (on_double_tap != nullptr || on_double_tap_down != nullptr);

    // Check double-tap initiation
    if (has_double_tap && consecutive_taps_ == 1 &&
        (down_time_ - last_tap_time_ <= double_tap_timeout) &&
        (distance(e.position, last_tap_pos_) <= slop_distance)) {
        if (on_double_tap_down) {
            TapDownDetails details{e.position, e.localPosition, e.button, e.modifiers, down_time_};
            on_double_tap_down(details);
        } else if (on_tap_down) {
            TapDownDetails details{e.position, e.localPosition, e.button, e.modifiers, down_time_};
            on_tap_down(details);
        }
    } else {
        if (down_button_ == MouseButton::Right) {
            if (on_secondary_tap_down) {
                TapDownDetails details{e.position, e.localPosition, e.button, e.modifiers, down_time_};
                on_secondary_tap_down(details);
            }
        } else if (down_button_ == MouseButton::Left) {
            if (on_tap_down) {
                TapDownDetails details{e.position, e.localPosition, e.button, e.modifiers, down_time_};
                on_tap_down(details);
            }
        }
    }
}

void TapGestureRecognizer::handlePointerMove(const PointerEvent& e) {
    if (!is_down_) return;

    if (distance(e.position, down_position_) > slop_distance) {
        is_down_          = false;
        consecutive_taps_ = 0;
        if (on_tap_cancel) on_tap_cancel();
        if (on_double_tap_cancel) on_double_tap_cancel();
    }
}

void TapGestureRecognizer::handlePointerUp(const PointerEvent& e) {
    if (!is_down_) return;
    is_down_ = false;

    double now = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());
    bool has_double_tap = (on_double_tap != nullptr || on_double_tap_down != nullptr);

    if (distance(e.position, down_position_) <= slop_distance) {
        if (down_button_ == MouseButton::Right) {
            if (on_secondary_tap_up) {
                TapUpDetails details{e.position, e.localPosition, e.button, e.modifiers, now};
                on_secondary_tap_up(details);
            }
            if (on_secondary_tap) {
                on_secondary_tap();
            }
            consecutive_taps_ = 0;
        } else if (down_button_ == MouseButton::Left) {
            if (has_double_tap && consecutive_taps_ == 1 &&
                (now - last_tap_time_ <= double_tap_timeout) &&
                (distance(e.position, last_tap_pos_) <= slop_distance)) {
                if (on_double_tap) {
                    on_double_tap();
                }
                consecutive_taps_ = 0;
            } else {
                if (has_double_tap) {
                    consecutive_taps_ = 1;
                    last_tap_pos_     = e.position;
                    last_tap_time_    = now;
                } else {
                    consecutive_taps_ = 0;
                }

                if (on_tap_up) {
                    TapUpDetails details{e.position, e.localPosition, e.button, e.modifiers, now};
                    on_tap_up(details);
                }
                if (on_tap) {
                    on_tap();
                }
            }
        }
    } else {
        consecutive_taps_ = 0;
        if (on_tap_cancel) on_tap_cancel();
    }
}

void TapGestureRecognizer::handlePointerCancel() {
    if (is_down_) {
        is_down_          = false;
        consecutive_taps_ = 0;
        if (on_tap_cancel) on_tap_cancel();
        if (on_double_tap_cancel) on_double_tap_cancel();
    }
}

void TapGestureRecognizer::reset() {
    is_down_          = false;
    consecutive_taps_ = 0;
}

// ════════════════════════════════════════════════════════════════
// LongPressGestureRecognizer Implementation
// ════════════════════════════════════════════════════════════════

void LongPressGestureRecognizer::handlePointerDown(const PointerEvent& e) {
    is_down_        = true;
    triggered_      = false;
    down_pos_       = e.position;
    down_local_pos_ = e.localPosition;
    last_local_pos_ = e.localPosition;
    down_time_      = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());
}

void LongPressGestureRecognizer::handlePointerMove(const PointerEvent& e) {
    if (!is_down_) return;

    double now = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());

    if (!triggered_) {
        if (distance(e.position, down_pos_) > slop_distance) {
            is_down_ = false;
            return;
        }

        if (now - down_time_ >= duration_threshold) {
            triggered_ = true;
            if (on_long_press_start) {
                LongPressStartDetails details{e.position, e.localPosition, now};
                on_long_press_start(details);
            }
            if (on_long_press) {
                on_long_press();
            }
        }
    } else {
        Point delta = {e.localPosition.x - last_local_pos_.x, e.localPosition.y - last_local_pos_.y};
        last_local_pos_ = e.localPosition;

        if (on_long_press_move) {
            LongPressMoveUpdateDetails details{e.position, e.localPosition, delta, now};
            on_long_press_move(details);
        }
    }
}

void LongPressGestureRecognizer::handlePointerUp(const PointerEvent& e) {
    if (is_down_) {
        double now = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());

        if (!triggered_ &&
            (now - down_time_ >= duration_threshold) &&
            (distance(e.position, down_pos_) <= slop_distance)) {
            triggered_ = true;
            if (on_long_press_start) {
                LongPressStartDetails details{e.position, e.localPosition, now};
                on_long_press_start(details);
            }
            if (on_long_press) {
                on_long_press();
            }
        }

        if (triggered_) {
            if (on_long_press_end) {
                LongPressEndDetails details{e.position, e.localPosition, now};
                on_long_press_end(details);
            }
        }

        is_down_   = false;
        triggered_ = false;
    }
}

void LongPressGestureRecognizer::handlePointerCancel() {
    is_down_   = false;
    triggered_ = false;
}

void LongPressGestureRecognizer::reset() {
    is_down_   = false;
    triggered_ = false;
}

// ════════════════════════════════════════════════════════════════
// PanGestureRecognizer Implementation
// ════════════════════════════════════════════════════════════════

void PanGestureRecognizer::handlePointerDown(const PointerEvent& e) {
    is_down_          = true;
    is_dragging_      = false;
    start_position_   = e.position;
    start_local_pos_  = e.localPosition;
    last_position_    = e.position;
    last_local_pos_   = e.localPosition;
    last_update_time_ = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());
    recent_velocity_  = {0, 0};
}

void PanGestureRecognizer::handlePointerMove(const PointerEvent& e) {
    if (!is_down_) return;

    double now = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());
    double dt  = std::max(0.001, now - last_update_time_);

    if (!is_dragging_) {
        if (distance(e.position, start_position_) >= touch_slop) {
            is_dragging_ = true;
            if (on_pan_start) {
                DragStartDetails details{e.position, e.localPosition, now};
                on_pan_start(details);
            }
        }
    }

    if (is_dragging_) {
        Point delta = {e.localPosition.x - last_local_pos_.x, e.localPosition.y - last_local_pos_.y};
        recent_velocity_ = {
            static_cast<float>((e.position.x - last_position_.x) / dt),
            static_cast<float>((e.position.y - last_position_.y) / dt)
        };

        last_position_    = e.position;
        last_local_pos_   = e.localPosition;
        last_update_time_ = now;

        if (on_pan_update) {
            DragUpdateDetails details{e.position, e.localPosition, delta, now};
            on_pan_update(details);
        }
    }
}

void PanGestureRecognizer::handlePointerUp(const PointerEvent& e) {
    if (is_down_) {
        double now = (e.timestamp > 0 ? e.timestamp : getCurrentTimeSeconds());

        if (is_dragging_) {
            if (on_pan_end) {
                DragEndDetails details{recent_velocity_, now};
                on_pan_end(details);
            }
        }

        is_down_     = false;
        is_dragging_ = false;
    }
}

void PanGestureRecognizer::handlePointerCancel() {
    if (is_dragging_) {
        if (on_pan_cancel) {
            on_pan_cancel();
        }
    }
    is_down_     = false;
    is_dragging_ = false;
}

void PanGestureRecognizer::reset() {
    is_down_     = false;
    is_dragging_ = false;
}

} // namespace enki
