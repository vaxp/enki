/// @file EnkiCefApp.cpp
/// @brief Implementation of EnkiCefApp and EnkiSubprocessApp.
///
/// @copyright ENKI Framework — MIT License

#include "EnkiCefApp.hpp"
#include <include/cef_command_line.h>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// EnkiCefApp
// ════════════════════════════════════════════════════════════════

EnkiCefApp::EnkiCefApp(ContextInitCallback on_context_init)
    : on_context_init_(std::move(on_context_init))
{}

void EnkiCefApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> cmd)
{
    // Only modify switches for the browser (main) process.
    if (!process_type.empty()) return;

    // ── Offscreen rendering ─────────────────────────────────────
    // Required for headless / embedded rendering without a window.
    cmd->AppendSwitch("disable-extensions");
    cmd->AppendSwitch("disable-pdf-extension");

    // ── GPU / Rendering ─────────────────────────────────────────
    cmd->AppendSwitch("enable-begin-frame-scheduling");

    // ── Security ────────────────────────────────────────────────
    cmd->AppendSwitch("allow-file-access-from-files");

    // ── Audio ───────────────────────────────────────────────────
    // Audio is enabled by default. Autoplay permitted for media.
    cmd->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");

    // ── Disable Google Services (GCM / Push Notifications) ─────
    // These cause harmless but noisy error messages when run offline.
    cmd->AppendSwitch("disable-background-networking");
    cmd->AppendSwitch("disable-sync");
    cmd->AppendSwitch("disable-gcm");
    cmd->AppendSwitch("disable-push-messaging");
    cmd->AppendSwitch("disable-component-update");
    cmd->AppendSwitch("disable-default-apps");
    cmd->AppendSwitch("no-first-run");
    cmd->AppendSwitch("noerrdialogs");
}

void EnkiCefApp::OnContextInitialized()
{
    // CEF is fully initialised on the UI thread.
    // Notify the CefBridge that it can now create browsers.
    if (on_context_init_) {
        on_context_init_();
    }
}

// ════════════════════════════════════════════════════════════════
// EnkiSubprocessApp (renderer / GPU process)
// ════════════════════════════════════════════════════════════════

void EnkiSubprocessApp::OnContextCreated(
    CefRefPtr<CefBrowser>   /*browser*/,
    CefRefPtr<CefFrame>     /*frame*/,
    CefRefPtr<CefV8Context> context)
{
    // Called in the renderer subprocess when a new V8 JS context is created.
    // The JSBridge injects bound C++ functions here.
    // Message: "enki_inject_bindings" → renderer registers functions into window.*
    //
    // Nothing to do here in the main app unless we inject functions globally.
    // The JSBridge sends an IPC message to inject per-browser bindings.
}

void EnkiSubprocessApp::OnContextReleased(
    CefRefPtr<CefBrowser>   /*browser*/,
    CefRefPtr<CefFrame>     /*frame*/,
    CefRefPtr<CefV8Context> /*context*/)
{
    // Clean up any V8 references held for this context.
}

bool EnkiSubprocessApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser>        browser,
    CefRefPtr<CefFrame>          frame,
    CefProcessId                 source_process,
    CefRefPtr<CefProcessMessage> message)
{
    const std::string name = message->GetName().ToString();

    // ── "enki_bind_function" ────────────────────────────────────
    // The browser process sent us the name of a C++ function to expose in JS.
    // We create a V8 handler that sends an IPC message back when called.
    if (name == "enki_bind_function") {
        auto args        = message->GetArgumentList();
        std::string fname = args->GetString(0).ToString();

        auto context = frame->GetV8Context();
        struct V8Scope {
            CefRefPtr<CefV8Context> c;
            V8Scope(CefRefPtr<CefV8Context> ctx) : c(ctx) { if (c) c->Enter(); }
            ~V8Scope() { if (c) c->Exit(); }
        } v8_scope(context);

        // Create a native V8 function that posts a message to the browser.
        class EnkiCallHandler : public CefV8Handler {
        public:
            explicit EnkiCallHandler(std::string fname,
                                     CefRefPtr<CefFrame> frame)
                : fname_(std::move(fname)), frame_(frame) {}

            bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value>  /*object*/,
                         const CefV8ValueList&   args,
                         CefRefPtr<CefV8Value>&  /*retval*/,
                         CefString&              /*exception*/) override
            {
                // Serialise args to JSON and send IPC to browser process.
                std::string args_json = "[";
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) args_json += ",";
                    if (args[i]->IsString())
                        args_json += "\"" + args[i]->GetStringValue().ToString() + "\"";
                    else if (args[i]->IsInt())
                        args_json += std::to_string(args[i]->GetIntValue());
                    else if (args[i]->IsDouble())
                        args_json += std::to_string(args[i]->GetDoubleValue());
                    else if (args[i]->IsBool())
                        args_json += args[i]->GetBoolValue() ? "true" : "false";
                    else
                        args_json += "null";
                }
                args_json += "]";

                auto msg = CefProcessMessage::Create("enki_js_call");
                msg->GetArgumentList()->SetString(0, fname_);
                msg->GetArgumentList()->SetString(1, args_json);
                frame_->SendProcessMessage(PID_BROWSER, msg);
                return true;
            }
        private:
            std::string            fname_;
            CefRefPtr<CefFrame>    frame_;
            IMPLEMENT_REFCOUNTING(EnkiCallHandler);
        };

        auto global  = context->GetGlobal();
        auto handler = CefRefPtr<CefV8Handler>(new EnkiCallHandler(fname, frame));
        auto fn      = CefV8Value::CreateFunction(fname, handler);
        global->SetValue(fname, fn, V8_PROPERTY_ATTRIBUTE_NONE);
        return true;
    }

    return false;
}

} // namespace enki::web
