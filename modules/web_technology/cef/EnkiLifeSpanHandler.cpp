/// @file EnkiLifeSpanHandler.cpp
/// @copyright ENKI Framework — MIT License

#include "EnkiLifeSpanHandler.hpp"
#include <include/cef_app.h>

namespace enki::web {

EnkiLifeSpanHandler::EnkiLifeSpanHandler(
    BrowserCreatedCallback on_created,
    BrowserClosedCallback  on_closed,
    OnNewWindowCallback    on_new_window)
    : on_created_   (std::move(on_created))
    , on_closed_    (std::move(on_closed))
    , on_new_window_(std::move(on_new_window))
{}

void EnkiLifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    // Browser is ready — notify CefBridge so it can store the handle
    // and complete any pending load_url calls.
    if (on_created_) on_created_(browser);
}

bool EnkiLifeSpanHandler::OnBeforePopup(
    CefRefPtr<CefBrowser>          /*browser*/,
    CefRefPtr<CefFrame>            /*frame*/,
    int                            /*popup_id*/,
    const CefString&               target_url,
    const CefString&               /*target_frame_name*/,
    WindowOpenDisposition          /*target_disposition*/,
    bool                           /*user_gesture*/,
    const CefPopupFeatures&        /*popup_features*/,
    CefWindowInfo&                 /*window_info*/,
    CefRefPtr<CefClient>&          /*client*/,
    CefBrowserSettings&            /*settings*/,
    CefRefPtr<CefDictionaryValue>& /*extra_info*/,
    bool*                          /*no_javascript_access*/)
{
    // Ask the application whether to allow this popup.
    if (on_new_window_) {
        std::string url = target_url.ToString();
        bool allow = on_new_window_(url);
        // Return true = cancel/block the popup.
        return !allow;
    }
    // Block all popups by default — no native window to host them.
    return true;
}

bool EnkiLifeSpanHandler::DoClose(CefRefPtr<CefBrowser> /*browser*/)
{
    // Return false to allow the default close behaviour.
    return false;
}

void EnkiLifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> /*browser*/)
{
    if (on_closed_) on_closed_();

    // Quit the CEF message loop — this causes CefRunMessageLoop() to return,
    // which unwinds EnkiWebHost::run() and shuts down the process cleanly.
    CefQuitMessageLoop();
}

} // namespace enki::web
