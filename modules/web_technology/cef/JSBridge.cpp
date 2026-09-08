/// @file JSBridge.cpp
/// @copyright ENKI Framework — MIT License

#include "JSBridge.hpp"
#include <include/cef_frame.h>
#include <include/cef_process_message.h>

namespace enki::web {

// ── Setup ──────────────────────────────────────────────────────

void JSBridge::set_browser(CefRefPtr<CefBrowser> browser)
{
    std::lock_guard<std::mutex> lk(mutex_);
    browser_ = std::move(browser);

    // Re-inject all already-registered functions into the new browser.
    for (auto& [name, _] : bound_) {
        inject_function_into_renderer(name);
    }
}

void JSBridge::clear_browser()
{
    std::lock_guard<std::mutex> lk(mutex_);
    browser_ = nullptr;
}

bool JSBridge::has_browser() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    return browser_ != nullptr;
}

// ── eval_js ────────────────────────────────────────────────────

void JSBridge::eval_js(std::string_view script)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (!browser_) return;

    auto frame = browser_->GetMainFrame();
    if (frame) {
        // Execute in the browser's renderer process.
        // The empty URL means "execute directly".
        frame->ExecuteJavaScript(
            CefString(std::string(script)),
            frame->GetURL(),
            0 /* start_line */);
    }
}

void JSBridge::eval_js_in_frame(std::string_view script,
                                std::string_view frame_name)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (!browser_) return;

    CefRefPtr<CefFrame> frame;
    if (frame_name.empty()) {
        frame = browser_->GetMainFrame();
    } else {
        frame = browser_->GetFrameByName(
            CefString(std::string(frame_name)));
    }
    if (!frame) return;

    frame->ExecuteJavaScript(
        CefString(std::string(script)),
        frame->GetURL(),
        0);
}

// ── bind_function ──────────────────────────────────────────────

void JSBridge::bind_function(
    std::string_view name,
    std::function<std::string(std::string_view)> fn)
{
    std::string sname(name);
    {
        std::lock_guard<std::mutex> lk(mutex_);
        bound_[sname] = std::move(fn);

        // If the browser exists already, inject immediately.
        // Otherwise inject_function_into_renderer will be called
        // from set_browser() when the browser is later created.
        if (browser_) {
            inject_function_into_renderer(sname);
        }
    }
}

void JSBridge::inject_function_into_renderer(const std::string& name)
{
    // Must be called with mutex_ held.
    if (!browser_) return;

    // Send IPC message to renderer process.
    // EnkiSubprocessApp::OnProcessMessageReceived() will receive this
    // and create a V8 function in window.<name>.
    auto msg  = CefProcessMessage::Create("enki_bind_function");
    auto args = msg->GetArgumentList();
    args->SetString(0, name);

    auto frame = browser_->GetMainFrame();
    if (frame) {
        frame->SendProcessMessage(PID_RENDERER, msg);
    }
}

// ── dispatch ───────────────────────────────────────────────────

bool JSBridge::dispatch(const std::string& func_name,
                        const std::string& args_json)
{
    std::function<std::string(std::string_view)> fn;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = bound_.find(func_name);
        if (it == bound_.end()) return false;
        fn = it->second;
    }
    // Call the handler outside the lock to avoid deadlock
    // if the handler itself calls back into JSBridge.
    fn(args_json);
    return true;
}

} // namespace enki::web
