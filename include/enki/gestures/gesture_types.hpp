#pragma once
/// @file gesture_types.hpp
/// @brief Gesture event types, details structures, and HitTestBehavior.
///
/// Provides detailed event metadata for taps, double-taps, long-presses,
/// drags/pans, scaling, scrolling, and hover events across ENKI widgets.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/platform/input.hpp"
#include <chrono>
#include <functional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Hit Test Behavior
// ════════════════════════════════════════════════════════════════

/// @brief Controls how a GestureDetector participates in hit testing.
enum class HitTestBehavior {
    /// Only target when the child widget is directly hit.
    /// Hits in empty space within bounds are ignored.
    DeferToChild,

    /// Targets can receive hits across their entire bounding box,
    /// and opaque targets prevent objects visually behind them from receiving hits.
    Opaque,

    /// Targets can receive hits across their entire bounding box,
    /// but also allow targets visually behind them to receive hits.
    Translucent
};

// ════════════════════════════════════════════════════════════════
// Tap Gesture Details
// ════════════════════════════════════════════════════════════════

/// @brief Details for pointer-down event in tap sequence.
struct TapDownDetails {
    Point       global_position;
    Point       local_position;
    MouseButton button    = MouseButton::Left;
    int         modifiers = 0;
    double      timestamp = 0.0;
};

/// @brief Details for pointer-up event completing a tap.
struct TapUpDetails {
    Point       global_position;
    Point       local_position;
    MouseButton button    = MouseButton::Left;
    int         modifiers = 0;
    double      timestamp = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Drag & Pan Gesture Details
// ════════════════════════════════════════════════════════════════

/// @brief Details when a drag/pan gesture initiates beyond touch slop.
struct DragStartDetails {
    Point       global_position;
    Point       local_position;
    double      timestamp = 0.0;
};

/// @brief Details for continuous motion updates during a drag/pan.
struct DragUpdateDetails {
    Point       global_position;
    Point       local_position;
    Point       delta;            ///< Offset change since last update
    double      timestamp = 0.0;
};

/// @brief Details when a drag/pan completes on pointer up.
struct DragEndDetails {
    Point       velocity;         ///< Estimated velocity (pixels/second)
    double      timestamp = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Long Press Gesture Details
// ════════════════════════════════════════════════════════════════

/// @brief Details when a long-press duration threshold is reached.
struct LongPressStartDetails {
    Point       global_position;
    Point       local_position;
    double      timestamp = 0.0;
};

/// @brief Details during movement while in long-press state.
struct LongPressMoveUpdateDetails {
    Point       global_position;
    Point       local_position;
    Point       delta;
    double      timestamp = 0.0;
};

/// @brief Details when pointer is released after a long-press.
struct LongPressEndDetails {
    Point       global_position;
    Point       local_position;
    double      timestamp = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Scale & Pinch Gesture Details
// ════════════════════════════════════════════════════════════════

/// @brief Details when a scale/pinch gesture begins.
struct ScaleStartDetails {
    Point       focal_point;
    Point       local_focal_point;
    double      timestamp = 0.0;
};

/// @brief Details during scale/pinch manipulation.
struct ScaleUpdateDetails {
    Point       focal_point;
    Point       local_focal_point;
    float       scale             = 1.0f;
    Point       focal_point_delta;
    double      timestamp         = 0.0;
};

/// @brief Details when scale/pinch ends.
struct ScaleEndDetails {
    Point       velocity;
    double      timestamp = 0.0;
};

// ════════════════════════════════════════════════════════════════
// Gesture Callback Types
// ════════════════════════════════════════════════════════════════

using GestureTapDownCallback         = std::function<void(const TapDownDetails&)>;
using GestureTapUpCallback           = std::function<void(const TapUpDetails&)>;
using GestureTapCallback             = std::function<void()>;
using GestureTapCancelCallback       = std::function<void()>;

using GestureLongPressStartCallback  = std::function<void(const LongPressStartDetails&)>;
using GestureLongPressMoveCallback   = std::function<void(const LongPressMoveUpdateDetails&)>;
using GestureLongPressEndCallback    = std::function<void(const LongPressEndDetails&)>;
using GestureLongPressCallback       = std::function<void()>;

using GestureDragStartCallback       = std::function<void(const DragStartDetails&)>;
using GestureDragUpdateCallback      = std::function<void(const DragUpdateDetails&)>;
using GestureDragEndCallback         = std::function<void(const DragEndDetails&)>;
using GestureDragCancelCallback      = std::function<void()>;

using GestureScaleStartCallback      = std::function<void(const ScaleStartDetails&)>;
using GestureScaleUpdateCallback     = std::function<void(const ScaleUpdateDetails&)>;
using GestureScaleEndCallback        = std::function<void(const ScaleEndDetails&)>;

using GestureHoverCallback           = std::function<void(const PointerEvent&)>;
using GestureScrollCallback          = std::function<void(float dx, float dy)>;

} // namespace enki
