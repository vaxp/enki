#pragma once
/// @file IWebViewBackend.hpp
/// @brief The ONLY bridge between Enki Core and the web_technology subsystem.
///
/// This pure-abstract interface is the sole point of contact.
/// Enki Core (widgets, render objects) includes ONLY this header.
/// CEF headers NEVER appear outside the web_technology/ directory.
///
/// Architecture:
///   Enki Core  →  #include <web_technology/IWebViewBackend.hpp>
///   web_technology/cef/CefBridge  →  implements IWebViewBackend
///
/// Threading model:
///   - All public methods must be called from the Enki main thread.
///   - Internally, the backend manages its own CEF UI/IO/Renderer threads.
///   - OnPaint callbacks arrive on the CEF Renderer thread; the backend
///     copies the frame and signals the Enki thread via the paint callback.
///
/// @copyright ENKI Framework — MIT License

#include "web_technology/WebViewFrame.hpp"
#include "web_technology/WebViewEvent.hpp"
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// Callbacks (registered by the WebView widget)
// ════════════════════════════════════════════════════════════════

/// Called on the Enki render thread when a new pixel frame is ready.
using OnPaintCallback   = std::function<void(WebViewFrame frame)>;

/// Called when the page title changes.
using OnTitleCallback   = std::function<void(std::string title)>;

/// Called when the page URL changes (navigation / redirect).
using OnURLCallback     = std::function<void(std::string url)>;

/// Called when a page load completes (success or failure).
/// @param success   true if HTTP status was 2xx.
/// @param http_code HTTP status code (0 if not HTTP).
using OnLoadCallback    = std::function<void(bool success, int http_code)>;

/// Called when the page starts loading (new navigation begun).
using OnLoadStartCallback = std::function<void(std::string url)>;

/// Called for browser console.log / console.error messages.
using OnConsoleCallback = std::function<void(std::string message, int line,
                                             std::string source)>;

/// Called when the browser wants to open a new popup window.
/// Return true to allow the popup, false to block it.
using OnNewWindowCallback = std::function<bool(std::string url)>;

// ════════════════════════════════════════════════════════════════
// BackendConfig — construction parameters
// ════════════════════════════════════════════════════════════════

struct BackendConfig {
    int   width        = 1280;
    int   height       = 800;
    float device_scale = 1.0f;

    bool  enable_js    = true;
    bool  enable_webgl = true;
    bool  enable_images = true;

    /// Open Chrome DevTools on this port (0 = disabled).
    int   devtools_port = 0;

    /// Background color (ARGB hex, e.g. 0xFFFFFFFF = white).
    uint32_t background_color = 0xFFFFFFFF;

    /// User-Agent override (empty = default Chromium UA).
    std::string user_agent;

    /// If true, create a real OS window with the browser embedded.
    /// If false, use Offscreen Rendering (OSR) — headless, for Enki Canvas embedding.
    bool windowed_mode = false;

    /// Window title shown in the OS title bar (windowed_mode only).
    std::string window_title = "Enki Web Host";

    /// Initial URL to load immediately upon browser creation (avoids about:blank roundtrip).
    std::string initial_url;
};

// ════════════════════════════════════════════════════════════════
// IWebViewBackend — pure abstract interface
// ════════════════════════════════════════════════════════════════

class IWebViewBackend {
public:
    virtual ~IWebViewBackend() = default;

    // ── Lifecycle ──────────────────────────────────────────────

    /// Initialize the backend and create an offscreen browser.
    /// Must be called once before any other method.
    /// @return true on success.
    virtual bool initialize(const BackendConfig& config) = 0;

    /// Shut down the browser and release all resources.
    /// After this call the object must not be used again.
    virtual void shutdown() = 0;

    // ── Navigation ─────────────────────────────────────────────

    /// Navigate to the given URL.
    virtual void load_url(std::string_view url) = 0;

    /// Load raw HTML content.
    /// @param base_url  Base URL for resolving relative links.
    virtual void load_html(std::string_view html,
                           std::string_view base_url = "about:blank") = 0;

    /// Reload the current page.
    /// @param ignore_cache  If true, bypass the HTTP cache (hard reload).
    virtual void reload(bool ignore_cache = false) = 0;

    /// Stop the current page load.
    virtual void stop_loading() = 0;

    /// Navigate backwards in history.
    virtual void go_back() = 0;

    /// Navigate forwards in history.
    virtual void go_forward() = 0;

    // ── Geometry ───────────────────────────────────────────────

    /// Resize the offscreen rendering surface.
    /// Triggers a new OnPaint with the updated dimensions.
    virtual void resize(int width, int height) = 0;

    /// Update the device pixel ratio (HiDPI scaling).
    virtual void set_device_scale(float scale) = 0;

    // ── Focus ──────────────────────────────────────────────────

    /// Notify the browser that it gained or lost keyboard focus.
    virtual void set_focus(bool focused) = 0;

    // ── Input Forwarding ───────────────────────────────────────

    /// Forward a mouse move / enter / leave event.
    virtual void send_mouse_move(const WebMouseMoveEvent& event) = 0;

    /// Forward a mouse button press or release.
    virtual void send_mouse_click(const WebMouseClickEvent& event) = 0;

    /// Forward a mouse wheel scroll.
    virtual void send_mouse_wheel(const WebMouseWheelEvent& event) = 0;

    /// Forward a keyboard key-down or key-up.
    virtual void send_key(const WebKeyEvent& event) = 0;

    /// Forward composed text (IME output).
    virtual void send_text(const WebTextInputEvent& event) = 0;

    // ── JavaScript Bridge ──────────────────────────────────────

    /// Evaluate JavaScript in the main frame.
    /// The result is delivered asynchronously (fire-and-forget).
    virtual void eval_js(std::string_view script) = 0;

    /// Evaluate JavaScript in the named frame (empty = main frame).
    virtual void eval_js_in_frame(std::string_view script,
                                  std::string_view frame_name) = 0;

    /// Bind a C++ function as a global JavaScript function.
    ///
    /// The function will be available as `window.<name>(arg0, arg1, ...)`
    /// in the browser. Arguments are JSON-encoded; the return value is
    /// a JSON string that becomes the JS return value.
    ///
    /// @param name  JavaScript function name (no dots/spaces).
    /// @param fn    C++ handler. Called on the CEF renderer thread.
    virtual void bind_function(
        std::string_view name,
        std::function<std::string(std::string_view args_json)> fn) = 0;

    // ── Queries ────────────────────────────────────────────────

    [[nodiscard]] virtual bool        is_loading()     const = 0;
    [[nodiscard]] virtual bool        can_go_back()    const = 0;
    [[nodiscard]] virtual bool        can_go_forward() const = 0;
    [[nodiscard]] virtual std::string get_url()        const = 0;
    [[nodiscard]] virtual std::string get_title()      const = 0;

    // ── Callbacks ──────────────────────────────────────────────

    virtual void set_on_paint      (OnPaintCallback    cb) = 0;
    virtual void set_on_title      (OnTitleCallback    cb) = 0;
    virtual void set_on_url        (OnURLCallback      cb) = 0;
    virtual void set_on_load       (OnLoadCallback     cb) = 0;
    virtual void set_on_load_start (OnLoadStartCallback cb) = 0;
    virtual void set_on_console    (OnConsoleCallback  cb) = 0;
    virtual void set_on_new_window (OnNewWindowCallback cb) = 0;

    // ── Factory ────────────────────────────────────────────────

    /// Create the concrete backend.
    ///
    /// Returns nullptr if the WebView subsystem was not compiled in
    /// (i.e., ENKI_HAS_WEBVIEW was not defined during build).
    ///
    /// Defined in web_technology/cef/CefBridge.cpp.
    static std::unique_ptr<IWebViewBackend> create();
};

} // namespace enki::web
