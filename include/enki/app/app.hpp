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

// ════════════════════════════════════════════════════════════════
// AppConfig — Application configuration
// ════════════════════════════════════════════════════════════════

/// @brief Configuration for the ENKI application.
struct AppConfig {
    std::string title      = "ENKI App";   ///< Window title.
    int         width      = 800;           ///< Initial window width.
    int         height     = 600;           ///< Initial window height.
    bool        resizable  = true;          ///< Whether the window is resizable.
    bool        vsync      = true;          ///< Enable vertical sync.
    bool        msaa       = true;          ///< Enable MSAA antialiasing.
    Color       clear_color = 0xFF0F172A;   ///< Default background color.
    int         target_fps = 60;            ///< Target frames per second.
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
