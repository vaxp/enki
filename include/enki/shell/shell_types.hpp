#pragma once
/// @file shell_types.hpp
/// @brief Types and configuration options for Desktop Shell Surfaces and Popups.

#include "enki/core/types.hpp"
#include "enki/platform/layer_surface.hpp"
#include <functional>
#include <memory>
#include <string>

namespace enki {

/// Placement relative to the anchor widget for native popups
enum class PopupPlacement {
    BottomStart,  ///< Below anchor, aligned to left edge
    BottomCenter, ///< Below anchor, centered horizontally
    BottomEnd,    ///< Below anchor, aligned to right edge
    TopStart,     ///< Above anchor, aligned to left edge
    TopCenter,    ///< Above anchor, centered horizontally
    TopEnd,       ///< Above anchor, aligned to right edge
    LeftStart,    ///< Left of anchor, aligned to top edge
    LeftCenter,   ///< Left of anchor, centered vertically
    LeftEnd,      ///< Left of anchor, aligned to bottom edge
    RightStart,   ///< Right of anchor, aligned to top edge
    RightCenter,  ///< Right of anchor, centered vertically
    RightEnd      ///< Right of anchor, aligned to bottom edge
};

/// Configuration options for spawning a native compositor popup
struct PopupOptions {
    Rect            anchor_rect;          ///< Global screen bounding box of the triggering widget
    PopupPlacement  placement     = PopupPlacement::BottomStart;
    int32_t         width         = 260;  ///< Desired popup width
    int32_t         height        = 320;  ///< Desired popup height
    int32_t         offset_gap    = 6;    ///< Gap between anchor widget and popup in pixels
    KeyboardMode    keyboard_mode = KeyboardMode::OnDemand; ///< Focus mode for keyboard input
    bool            auto_dismiss  = true; ///< Close automatically when user clicks outside
    std::shared_ptr<Output> target_output = nullptr; ///< Output monitor (nullptr = auto)
    std::function<void()>   on_close;     ///< Optional callback invoked when popup is closed
};

} // namespace enki
