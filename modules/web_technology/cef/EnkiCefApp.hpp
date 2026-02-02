#pragma once
/// @file EnkiCefApp.hpp
/// @brief CefApp + CefBrowserProcessHandler implementation for Enki.
///
/// EnkiCefApp is instantiated once per process (main process and
/// renderer/GPU subprocess).  In the main (browser) process it:
///   - Configures CEF command-line switches (WebGL, GPU, etc.)
///   - Provides the BrowserProcessHandler for OnContextInitialized
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_app.h>
#include <include/cef_browser_process_handler.h>
#include <include/cef_render_process_handler.h>
#include <functional>
#include <vector>
#include <string>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// EnkiCefApp — main process CefApp
// ════════════════════════════════════════════════════════════════

class EnkiCefApp : public CefApp,
                   public CefBrowserProcessHandler {
public:
    using ContextInitCallback = std::function<void()>;

    explicit EnkiCefApp(ContextInitCallback on_context_init = nullptr);

    // ── CefApp ─────────────────────────────────────────────────
    CefRefPtr<CefBrowserProcessHandler>
        GetBrowserProcessHandler() override { return this; }

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override;

    // ── CefBrowserProcessHandler ────────────────────────────────
    void OnContextInitialized() override;

private:
    ContextInitCallback on_context_init_;

    IMPLEMENT_REFCOUNTING(EnkiCefApp);
};

// ════════════════════════════════════════════════════════════════
// EnkiSubprocessApp — render/gpu subprocess CefApp
// ════════════════════════════════════════════════════════════════

/// Used in the helper subprocess binary only.
class EnkiSubprocessApp : public CefApp,
                          public CefRenderProcessHandler {
public:
    EnkiSubprocessApp() = default;

    CefRefPtr<CefRenderProcessHandler>
        GetRenderProcessHandler() override { return this; }

    // Called in renderer subprocess to expose bound C++ functions to JS.
    void OnContextCreated(CefRefPtr<CefBrowser>  browser,
                          CefRefPtr<CefFrame>    frame,
                          CefRefPtr<CefV8Context> context) override;

    void OnContextReleased(CefRefPtr<CefBrowser>   browser,
                           CefRefPtr<CefFrame>     frame,
                           CefRefPtr<CefV8Context> context) override;

    bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser>        browser,
        CefRefPtr<CefFrame>          frame,
        CefProcessId                 source_process,
        CefRefPtr<CefProcessMessage> message) override;

private:
    IMPLEMENT_REFCOUNTING(EnkiSubprocessApp);
};

} // namespace enki::web
