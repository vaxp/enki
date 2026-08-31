#pragma once
/// @file EnkiCefClient.hpp
/// @brief CefClient — aggregates all CEF handler interfaces.
///
/// CefClient is the central handler object that CEF queries for optional
/// sub-handlers (render, load, display, life-span, request, etc.).
/// EnkiCefClient wires together all Enki-specific handlers.
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_client.h>
#include <include/cef_display_handler.h>
#include <include/cef_load_handler.h>
#include <include/cef_request_handler.h>

#include "EnkiRenderHandler.hpp"
#include "EnkiLifeSpanHandler.hpp"
#include <web_technology/IWebViewBackend.hpp>

namespace enki::web {

class EnkiCefClient : public CefClient,
                      public CefDisplayHandler,
                      public CefLoadHandler,
                      public CefRequestHandler {
public:
    EnkiCefClient(
        CefRefPtr<EnkiRenderHandler>   render_handler,
        CefRefPtr<EnkiLifeSpanHandler> life_span_handler,
        OnTitleCallback    on_title,
        OnURLCallback      on_url,
        OnLoadCallback     on_load,
        OnLoadStartCallback on_load_start,
        OnConsoleCallback  on_console);

    // ── CefClient ──────────────────────────────────────────────

    CefRefPtr<CefRenderHandler>   GetRenderHandler()   override { return render_handler_;    }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return life_span_handler_; }
    CefRefPtr<CefDisplayHandler>  GetDisplayHandler()  override { return this; }
    CefRefPtr<CefLoadHandler>     GetLoadHandler()     override { return this; }
    CefRefPtr<CefRequestHandler>  GetRequestHandler()  override { return this; }

    /// Called when a process message arrives from the renderer subprocess.
    /// Used by JSBridge to receive results of bound function calls.
    bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser>        browser,
        CefRefPtr<CefFrame>          frame,
        CefProcessId                 source_process,
        CefRefPtr<CefProcessMessage> message) override;

    // ── CefDisplayHandler ──────────────────────────────────────

    void OnTitleChange(CefRefPtr<CefBrowser> browser,
                       const CefString& title) override;

    void OnAddressChange(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame>   frame,
                         const CefString&      url) override;

    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                          cef_log_severity_t    level,
                          const CefString&      message,
                          const CefString&      source,
                          int                   line) override;

    // ── CefLoadHandler ─────────────────────────────────────────

    void OnLoadStart(CefRefPtr<CefBrowser>    browser,
                     CefRefPtr<CefFrame>      frame,
                     TransitionType           transition_type) override;

    void OnLoadEnd(CefRefPtr<CefBrowser>  browser,
                   CefRefPtr<CefFrame>    frame,
                   int                    http_status_code) override;

    void OnLoadError(CefRefPtr<CefBrowser>  browser,
                     CefRefPtr<CefFrame>    frame,
                     ErrorCode              error_code,
                     const CefString&       error_text,
                     const CefString&       failed_url) override;

    // ── JS call routing ────────────────────────────────────────

    /// Register a bound function so OnProcessMessageReceived can find it.
    void register_bound_function(
        const std::string& name,
        std::function<std::string(std::string_view)> fn);

private:
    CefRefPtr<EnkiRenderHandler>   render_handler_;
    CefRefPtr<EnkiLifeSpanHandler> life_span_handler_;

    OnTitleCallback      on_title_;
    OnURLCallback        on_url_;
    OnLoadCallback       on_load_;
    OnLoadStartCallback  on_load_start_;
    OnConsoleCallback    on_console_;

    std::mutex bound_mutex_;
    std::unordered_map<std::string,
        std::function<std::string(std::string_view)>> bound_functions_;

    IMPLEMENT_REFCOUNTING(EnkiCefClient);
};

} // namespace enki::web
