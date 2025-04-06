#pragma once
/// @file panel_window.hpp
/// @brief Declarative Panel, Bar, and Dock definitions for Desktop Shells.

#include "enki/core/types.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/shell/shell_app.hpp"
#include "enki/tree/widget.hpp"

namespace enki {

/// Helper to configure a desktop bar / panel
inline LayerSurfaceConfig topBarConfig(int32_t height = 36, int32_t exclusive_zone = 36) {
    LayerSurfaceConfig cfg;
    cfg.namespace_id   = "enki-bar";
    cfg.layer          = ShellLayer::Top;
    cfg.anchor         = ShellAnchor::TopAll;
    cfg.width          = 0; // Span full monitor width
    cfg.height         = height;
    cfg.exclusive_zone = exclusive_zone;
    cfg.keyboard_mode  = KeyboardMode::None;
    cfg.transparent    = true;
    return cfg;
}

/// Helper to configure a bottom dock
inline LayerSurfaceConfig bottomDockConfig(int32_t height = 64, int32_t width = 600) {
    LayerSurfaceConfig cfg;
    cfg.namespace_id   = "enki-dock";
    cfg.layer          = ShellLayer::Top;
    cfg.anchor         = ShellAnchor::Bottom;
    cfg.width          = width;
    cfg.height         = height;
    cfg.exclusive_zone = height + 10;
    cfg.keyboard_mode  = KeyboardMode::None;
    cfg.transparent    = true;
    return cfg;
}

/// Helper to configure an OSD / notification surface
inline LayerSurfaceConfig osdConfig(int32_t width = 320, int32_t height = 80) {
    LayerSurfaceConfig cfg;
    cfg.namespace_id   = "enki-osd";
    cfg.layer          = ShellLayer::Overlay;
    cfg.anchor         = ShellAnchor::TopRight;
    cfg.width          = width;
    cfg.height         = height;
    cfg.exclusive_zone = -1; // Never reserve space
    cfg.margin.top     = 48;
    cfg.margin.right   = 24;
    cfg.keyboard_mode  = KeyboardMode::None;
    cfg.transparent    = true;
    return cfg;
}

} // namespace enki
