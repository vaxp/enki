#pragma once
/// @file EnkiLifeSpanHandler.hpp
/// @brief CefLifeSpanHandler — manages browser window lifecycle events.
///
/// Handles browser creation, new popup window requests, and browser
/// close events. Enki blocks all popup windows by default (since there
/// is no native OS window to host them); this can be overridden via
/// OnNewWindowCallback.
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_life_span_handler.h>
#include <web_technology/IWebViewBackend.hpp>
#include <functional>

namespace enki::web {

class EnkiLifeSpanHandler : public CefLifeSpanHandler {
public:
    using BrowserCreatedCallback = std::function<void(CefRefPtr<CefBrowser>)>;
    using BrowserClosedCallback  = std::function<void()>;

    EnkiLifeSpanHandler(BrowserCreatedCallback on_created,
                        BrowserClosedCallback  on_closed,
                        OnNewWindowCallback    on_new_window = nullptr);

    // ── CefLifeSpanHandler ─────────────────────────────────────

    /// Called when the browser is fully created and ready.
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    /// Called when a popup window is requested.
    /// Returns true to cancel (block) the popup — Enki's default.
    bool OnBeforePopup(
        CefRefPtr<CefBrowser>         browser,
        CefRefPtr<CefFrame>           frame,
        int                           popup_id,
        const CefString&              target_url,
        const CefString&              target_frame_name,
        WindowOpenDisposition         target_disposition,
        bool                          user_gesture,
        const CefPopupFeatures&       popup_features,
        CefWindowInfo&                window_info,
        CefRefPtr<CefClient>&         client,
        CefBrowserSettings&           settings,
        CefRefPtr<CefDictionaryValue>& extra_info,
        bool*                         no_javascript_access) override;

    /// Called when the browser begins its close sequence.
    bool DoClose(CefRefPtr<CefBrowser> browser) override;

    /// Called when the browser is fully closed and destroyed.
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

private:
    BrowserCreatedCallback on_created_;
    BrowserClosedCallback  on_closed_;
    OnNewWindowCallback    on_new_window_;

    IMPLEMENT_REFCOUNTING(EnkiLifeSpanHandler);
};

} // namespace enki::web
