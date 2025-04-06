#pragma once
/// @file shell_app.hpp
/// @brief Multi-surface Desktop Shell Application Coordinator.

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/platform/window.hpp"
#include "enki/shell/surface_host.hpp"
#include "enki/shell/shell_types.hpp"
#include "enki/app/app.hpp" // For AppConfig & FrameStats

#include <include/gpu/GrDirectContext.h>

#include <memory>
#include <vector>
#include <string>

namespace enki {

/// @brief Multi-surface Desktop Shell coordinator. Manages multiple concurrent
/// LayerSurfaces (Bars, Docks, OSDs) and native Popups across multiple monitors.
class ShellApp {
public:
    static Result<std::unique_ptr<ShellApp>> create(AppConfig config = {});
    static ShellApp* instance();

    ~ShellApp();

    // Non-copyable, non-movable
    ShellApp(const ShellApp&) = delete;
    ShellApp& operator=(const ShellApp&) = delete;

    /// Add a Layer Shell surface (Bar, Dock, Wallpaper, OSD, Launcher)
    SurfaceHost* addLayerSurface(LayerSurfaceConfig config, WidgetPtr root_widget);

    /// Add a standard window surface
    SurfaceHost* addWindow(WindowConfig config, WidgetPtr root_widget);

    /// Remove an existing surface host
    void removeSurface(SurfaceHost* host);

    /// Run the unified multi-surface event and render loop
    int run();

    /// Request application quit
    void quit();

    /// Accessors
    [[nodiscard]] Platform&        platform();
    [[nodiscard]] GrDirectContext* grContext();
    [[nodiscard]] size_t           surfaceCount() const;

private:
    ShellApp();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace enki
