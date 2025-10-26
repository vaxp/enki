#pragma once
/// @file CefBridge.hpp
/// @brief Concrete implementation of IWebViewBackend using CEF.
///
/// CefBridge is the "glue layer" that:
///   1. Initialises the CEF library (once per process, via CefInitialize).
///   2. Creates an offscreen CefBrowser.
///   3. Implements all IWebViewBackend methods by delegating to the browser host.
///   4. Routes CEF callbacks to the Enki callbacks registered by WebView widget.
///
/// Only one CEF instance exists per process. Multiple CefBridge objects share
/// the same CEF message loop (via CefDoMessageLoopWork() called from Enki's
/// frame loop).
///
/// @copyright ENKI Framework — MIT License

#include <web_technology/IWebViewBackend.hpp>
#include "EnkiCefApp.hpp"
#include "EnkiCefClient.hpp"
#include "JSBridge.hpp"
#include <include/cef_browser.h>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// CefGlobal — singleton managing the CEF process-wide state
// ════════════════════════════════════════════════════════════════

/// Manages the single CEF process initialisation / shutdown.
/// All CefBridge instances share one global CEF context.
class CefGlobal {
public:
    /// Returns the process-wide singleton.
    static CefGlobal& instance();

    /// Initialise CEF on first call; subsequent calls are no-ops.
    /// @param argc             Command line argument count.
    /// @param argv             Command line arguments array.
    /// @param subprocess_path  Absolute path to the helper subprocess binary.
    /// @param windowed_mode    If true, disable windowless_rendering (for real windows).
    /// @return false if initialisation failed.
    bool ensure_initialized(int argc = 0, char* argv[] = nullptr,
                            const std::string& subprocess_path = "",
                            bool windowed_mode = false);

    /// Shut down CEF.  Call once, when the last browser is destroyed.
    void shutdown();

    /// Drive the CEF message loop (call from Enki's frame loop).
    /// Must be called on the thread that called ensure_initialized().
    static void do_message_loop_work();

    [[nodiscard]] bool is_initialized() const { return initialized_; }

private:
    CefGlobal() = default;
    std::atomic<bool>        initialized_{false};
    std::atomic<bool>        shutdown_called_{false};
};

// ════════════════════════════════════════════════════════════════
// CefBridge — IWebViewBackend implementation
// ════════════════════════════════════════════════════════════════

class CefBridge : public IWebViewBackend {
public:
    CefBridge();
    ~CefBridge() override;

    // ── IWebViewBackend ────────────────────────────────────────

    bool initialize(const BackendConfig& config)          override;
    void shutdown()                                        override;

    void load_url(std::string_view url)                   override;
    void load_html(std::string_view html,
                   std::string_view base_url)             override;
    void reload(bool ignore_cache)                        override;
    void stop_loading()                                   override;
    void go_back()                                        override;
    void go_forward()                                     override;

    void resize(int width, int height)                    override;
    void set_device_scale(float scale)                    override;
    void set_focus(bool focused)                          override;

    void send_mouse_move (const WebMouseMoveEvent&  e)    override;
    void send_mouse_click(const WebMouseClickEvent& e)    override;
    void send_mouse_wheel(const WebMouseWheelEvent& e)    override;
    void send_key        (const WebKeyEvent& e)           override;
    void send_text       (const WebTextInputEvent& e)     override;

    void eval_js         (std::string_view script)        override;
    void eval_js_in_frame(std::string_view script,
                          std::string_view frame_name)    override;
    void bind_function(
        std::string_view name,
        std::function<std::string(std::string_view)> fn)  override;

    bool        is_loading()     const override;
    bool        can_go_back()    const override;
    bool        can_go_forward() const override;
    std::string get_url()        const override;
    std::string get_title()      const override;

    void set_on_paint      (OnPaintCallback    cb)        override;
    void set_on_title      (OnTitleCallback    cb)        override;
    void set_on_url        (OnURLCallback      cb)        override;
    void set_on_load       (OnLoadCallback     cb)        override;
    void set_on_load_start (OnLoadStartCallback cb)       override;
    void set_on_console    (OnConsoleCallback  cb)        override;
    void set_on_new_window (OnNewWindowCallback cb)       override;

private:
    // ── Internal helpers ───────────────────────────────────────

    void create_browser(const BackendConfig& config);
    void on_browser_created(CefRefPtr<CefBrowser> browser);
    void on_browser_closed();

    // ── State ──────────────────────────────────────────────────

    BackendConfig              config_;
    CefRefPtr<CefBrowser>      browser_;
    CefRefPtr<EnkiCefClient>   client_;
    CefRefPtr<EnkiRenderHandler> render_handler_;

    std::unique_ptr<JSBridge>  js_bridge_;

    mutable std::mutex         mutex_;
    std::condition_variable    browser_ready_cv_;
    bool                       browser_ready_  = false;
    bool                       shutdown_called_ = false;

    // Pending URL to load once browser is ready.
    std::string                pending_url_;
    std::string                pending_html_;
    std::string                pending_html_base_url_;

    // Cached queries (updated on CEF callbacks, read from Enki thread).
    mutable std::mutex         state_mutex_;
    std::atomic<bool>          is_loading_{false};
    std::atomic<bool>          can_go_back_{false};
    std::atomic<bool>          can_go_forward_{false};
    std::string                current_url_;
    std::string                current_title_;

    // User callbacks
    OnPaintCallback     on_paint_;
    OnTitleCallback     on_title_;
    OnURLCallback       on_url_;
    OnLoadCallback      on_load_;
    OnLoadStartCallback on_load_start_;
    OnConsoleCallback   on_console_;
    OnNewWindowCallback on_new_window_;
};

} // namespace enki::web
