#pragma once
/// @file JSBridge.hpp
/// @brief C++ ↔ JavaScript bidirectional bridge.
///
/// JSBridge manages:
///   1. eval_js()      — fire-and-forget JS evaluation.
///   2. bind_function() — expose a C++ function to window.* in JS.
///
/// The binding mechanism uses CEF IPC:
///   Browser process  →  "enki_bind_function" IPC  →  Renderer process
///   Renderer process →  "enki_js_call" IPC         →  Browser process
///
/// JSBridge holds the browser reference and the registry of bound functions.
/// EnkiCefClient dispatches "enki_js_call" messages to JSBridge::dispatch().
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_browser.h>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace enki::web {

class JSBridge {
public:
    JSBridge() = default;

    // ── Setup ──────────────────────────────────────────────────

    /// Must be called once the browser is created.
    void set_browser(CefRefPtr<CefBrowser> browser);

    /// Clear the browser reference on shutdown.
    void clear_browser();

    // ── API ────────────────────────────────────────────────────

    /// Evaluate script in the main frame (fire-and-forget).
    void eval_js(std::string_view script);

    /// Evaluate script in a named frame.
    void eval_js_in_frame(std::string_view script, std::string_view frame_name);

    /// Bind a C++ function and inject it into the renderer process.
    /// @param name  JavaScript function name (no spaces/dots).
    /// @param fn    Handler invoked with JSON-encoded arguments.
    ///              Return value is a JSON string (ignored by default).
    void bind_function(std::string_view name,
                       std::function<std::string(std::string_view)> fn);

    /// Dispatch an incoming "enki_js_call" IPC message.
    /// Called by EnkiCefClient::OnProcessMessageReceived().
    bool dispatch(const std::string& func_name,
                  const std::string& args_json);

    // ── Queries ────────────────────────────────────────────────

    [[nodiscard]] bool has_browser() const;

private:
    mutable std::mutex         mutex_;
    CefRefPtr<CefBrowser>      browser_;

    std::unordered_map<std::string,
        std::function<std::string(std::string_view)>> bound_;

    /// Send "enki_bind_function" IPC to the renderer so it can
    /// register the function in the V8 context.
    void inject_function_into_renderer(const std::string& name);
};

} // namespace enki::web
