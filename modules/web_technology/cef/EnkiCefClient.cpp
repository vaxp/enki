/// @file EnkiCefClient.cpp
/// @copyright ENKI Framework — MIT License

#include "EnkiCefClient.hpp"
#include <include/cef_frame.h>
#include <unordered_map>
#include <mutex>

namespace enki::web {

EnkiCefClient::EnkiCefClient(
    CefRefPtr<EnkiRenderHandler>   render_handler,
    CefRefPtr<EnkiLifeSpanHandler> life_span_handler,
    OnTitleCallback    on_title,
    OnURLCallback      on_url,
    OnLoadCallback     on_load,
    OnLoadStartCallback on_load_start,
    OnConsoleCallback  on_console)
    : render_handler_   (std::move(render_handler))
    , life_span_handler_(std::move(life_span_handler))
    , on_title_         (std::move(on_title))
    , on_url_           (std::move(on_url))
    , on_load_          (std::move(on_load))
    , on_load_start_    (std::move(on_load_start))
    , on_console_       (std::move(on_console))
{}

// ── CefDisplayHandler ──────────────────────────────────────────

void EnkiCefClient::OnTitleChange(CefRefPtr<CefBrowser> /*browser*/,
                                  const CefString& title)
{
    if (on_title_) on_title_(title.ToString());
}

void EnkiCefClient::OnAddressChange(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRefPtr<CefFrame>   frame,
                                    const CefString&      url)
{
    // Only report URL changes from the main frame.
    if (!frame->IsMain()) return;
    if (on_url_) on_url_(url.ToString());
}

bool EnkiCefClient::OnConsoleMessage(CefRefPtr<CefBrowser> /*browser*/,
                                     cef_log_severity_t    /*level*/,
                                     const CefString&      message,
                                     const CefString&      source,
                                     int                   line)
{
    if (on_console_) {
        on_console_(message.ToString(), line, source.ToString());
    }
    // Return false → CEF also logs it internally.
    return false;
}

// ── CefLoadHandler ─────────────────────────────────────────────

void EnkiCefClient::OnLoadStart(CefRefPtr<CefBrowser> /*browser*/,
                                CefRefPtr<CefFrame>   frame,
                                TransitionType        /*transition*/)
{
    if (!frame->IsMain()) return;
    if (on_load_start_) {
        on_load_start_(frame->GetURL().ToString());
    }
}

void EnkiCefClient::OnLoadEnd(CefRefPtr<CefBrowser> /*browser*/,
                               CefRefPtr<CefFrame>   frame,
                               int                   http_status)
{
    if (!frame->IsMain()) return;
    if (on_load_) {
        bool success = (http_status >= 200 && http_status < 300)
                       || http_status == 0;  // local file / data URL
        on_load_(success, http_status);
    }
}

void EnkiCefClient::OnLoadError(CefRefPtr<CefBrowser> /*browser*/,
                                CefRefPtr<CefFrame>   frame,
                                ErrorCode             error_code,
                                const CefString&      /*error_text*/,
                                const CefString&      /*failed_url*/)
{
    if (!frame->IsMain()) return;
    // Treat any error as a failed load with code 0.
    if (on_load_) on_load_(false, static_cast<int>(error_code));
}

// ── JS Bridge — incoming messages from renderer process ────────

void EnkiCefClient::register_bound_function(
    const std::string& name,
    std::function<std::string(std::string_view)> fn)
{
    std::lock_guard<std::mutex> lk(bound_mutex_);
    bound_functions_[name] = std::move(fn);
}

bool EnkiCefClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser>        /*browser*/,
    CefRefPtr<CefFrame>          /*frame*/,
    CefProcessId                 source_process,
    CefRefPtr<CefProcessMessage> message)
{
    if (source_process != PID_RENDERER) return false;

    const std::string name = message->GetName().ToString();

    // ── "enki_js_call" ──────────────────────────────────────────
    // Renderer sends this when a bound JS function is invoked.
    if (name == "enki_js_call") {
        auto args       = message->GetArgumentList();
        std::string fname     = args->GetString(0).ToString();
        std::string args_json = args->GetString(1).ToString();

        std::function<std::string(std::string_view)> fn;
        {
            std::lock_guard<std::mutex> lk(bound_mutex_);
            auto it = bound_functions_.find(fname);
            if (it == bound_functions_.end()) return false;
            fn = it->second;
        }

        // Call the C++ handler (on the CEF UI thread).
        // The result is currently fire-and-forget; extend to return
        // a Promise if two-way async results are needed.
        if (fn) {
            fn(args_json);
        }
        return true;
    }

    return false;
}

} // namespace enki::web
