#pragma once
/// @file app.hpp
/// @brief ENKI Application entry point and lifecycle manager.
///
/// App encapsulates the entire lifecycle of a ENKI application:
///   1. Platform & window initialization
///   2. Skia GPU context setup
///   3. Widget tree bootstrap (BuildOwner + Element tree)
///   4. The main render/event loop (layout → build → paint)
///   5. Graceful teardown
///
/// ## Usage
/// @code
///   class MyApp : public StatelessWidget {
///       WidgetPtr build(BuildContext& ctx) override {
///           return text("Hello, ENKI!");
///       }
///       std::string_view typeName() const override { return "MyApp"; }
///   };
///
///   int main() {
///       return enki::runApp(std::make_shared<MyApp>());
///   }
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/window.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include <memory>
#include <string>
#include <functional>

namespace enki {

// Forward declarations
class SurfaceHost;
struct WindowConfig;
class BuildOwner;

// ════════════════════════════════════════════════════════════════
// AppConfig — Application configuration
// ════════════════════════════════════════════════════════════════

/// @brief Real-time performance and frame statistics.
struct FrameStats {
    double   fps            = 0.0;   ///< Instantaneous smoothed frames per second.
    double   frame_time_ms  = 0.0;   ///< Total frame duration in milliseconds.
    double   cpu_time_ms    = 0.0;   ///< CPU build, layout, and paint time in ms.
    double   gpu_render_ms  = 0.0;   ///< Pure Skia GPU raster command submission in ms.
    double   swap_time_ms   = 0.0;   ///< Wayland / EGL buffer presentation & compositor IPC in ms.
    double   gpu_time_ms    = 0.0;   ///< Combined GPU + swap time in ms (for backward compatibility).
    uint64_t total_frames   = 0;     ///< Monotonic frame counter.
};

/// @brief Configuration for the ENKI application.
struct AppConfig {
    std::string title                    = "ENKI App";   ///< Window title.
    int         width                    = 800;           ///< Initial window width.
    int         height                   = 600;           ///< Initial window height.
    bool        resizable                = true;          ///< Whether the window is resizable.
    bool        vsync                    = true;          ///< Enable vertical sync.
    bool        msaa                     = true;          ///< Enable MSAA antialiasing.
    Color       clear_color              = 0xFF0F172A;   ///< Default background color.
    int         target_fps               = 60;            ///< Target frames per second.
    WindowMode  window_mode              = WindowMode::Normal; ///< Standard window or Layer Shell.
    bool        show_performance_overlay = false;         ///< Render built-in FPS & latency HUD.
};

// ════════════════════════════════════════════════════════════════
// App — Application lifecycle manager
// ════════════════════════════════════════════════════════════════

/// @brief Manages the complete lifecycle of a ENKI application.
///
/// App is the top-level coordinator. It owns the platform, window,
/// Skia GPU context, and the widget/element/render-object trees.
/// Call run() to start the event loop; it returns when the user
/// closes the window.
class App {
public:
    /// Create and initialize the application.
    ///
    /// @param root_widget  The root widget of the application.
    /// @param config       Application configuration.
    /// @return Result<App> — the initialized app, or an error string.
    static Result<std::unique_ptr<App>> create(
        WidgetPtr root_widget,
        AppConfig config = {});

    ~App();

    // Non-copyable, non-movable
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /// Run the application event loop.
    /// Blocks until the window is closed.
    ///
    /// @return Exit code (0 = success).
    int run();

    /// Request the application to quit on the next frame.
    void quit();

    /// Get real-time frame statistics (FPS, latency, frame times).
    [[nodiscard]] FrameStats frameStats() const;

    /// Get current FPS.
    [[nodiscard]] double currentFps() const;

    /// Get current frame duration in milliseconds.
    [[nodiscard]] double currentFrameTimeMs() const;

    /// Get the window title.
    [[nodiscard]] const std::string& title() const;

    /// Change the window title at runtime.
    void setTitle(std::string_view title);

    /// Get the current window size.
    [[nodiscard]] Size windowSize() const;

    /// Get the DPI scale factor.
    [[nodiscard]] float dpiScale() const;

    /// Access the platform (for advanced use).
    [[nodiscard]] Platform& platform();

    /// Access the window (for advanced use).
    [[nodiscard]] Window& window();

    // ── Multi-Surface & Popup Support ──────────────────────────────

    /// Get active App singleton instance.
    static App* instance();

    /// Create and manage a new popup surface (xdg_popup / native popup window).
    SurfaceHost* addPopup(SurfaceHost* parent, WindowConfig config, WidgetPtr root_widget);

    /// Add an additional window surface to this application.
    SurfaceHost* addWindow(WindowConfig config, WidgetPtr root_widget);

    /// Remove and destroy an active surface.
    void removeSurface(SurfaceHost* host);

    /// Find the SurfaceHost that owns the given BuildOwner.
    [[nodiscard]] SurfaceHost* findSurfaceByOwner(const BuildOwner* owner) const;

private:
    App();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ════════════════════════════════════════════════════════════════
// runApp — Convenience entry point
// ════════════════════════════════════════════════════════════════

/// @brief The primary entry point for ENKI applications.
///
/// Creates an App with the given root widget and runs the event loop.
/// Returns the exit code to be returned from main().
///
/// @param root_widget  The root widget of the application.
/// @param config       Optional application configuration.
/// @return int         Exit code (0 = success, non-zero = error).
///
/// @code
///   int main() {
///       return enki::runApp(std::make_shared<MyApp>());
///   }
/// @endcode
int runApp(WidgetPtr root_widget, AppConfig config = {});

}  // namespace enki
