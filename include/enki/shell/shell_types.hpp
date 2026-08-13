#pragma once
/// @file shell_types.hpp
/// @brief Types and configuration options for Desktop Shell Surfaces and Popups.

#include "enki/core/types.hpp"
#include "enki/platform/layer_surface.hpp"
#include <functional>
#include <memory>
#include <string>

namespace enki {

class SurfaceHost;

/// Configuration options for spawning a native compositor popup
struct PopupOptions {
    SurfaceHost*    parent_host   = nullptr; ///< The surface this popup is attached to (required for Wayland xdg_popup).
    Point           position;             ///< Exact absolute screen coordinate (x, y) where the popup should appear
    int32_t         width         = 260;  ///< Desired popup width
    int32_t         height        = 320;  ///< Desired popup height
    KeyboardMode    keyboard_mode = KeyboardMode::OnDemand; ///< Focus mode for keyboard input
    bool            auto_dismiss  = true; ///< Close automatically when user clicks outside
    std::shared_ptr<Output> target_output = nullptr; ///< Output monitor (nullptr = auto)
    std::function<void()>   on_close;     ///< Optional callback invoked when popup is closed
};

} // namespace enki
