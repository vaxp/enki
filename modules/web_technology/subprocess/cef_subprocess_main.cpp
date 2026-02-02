/// @file cef_subprocess_main.cpp
/// @brief Entry point for the CEF helper subprocess.
///
/// Chromium requires a separate helper process for rendering, GPU, and
/// plugin sandboxing.  This binary is launched automatically by CEF with
/// special command-line arguments.  It must call CefExecuteProcess() and exit.
///
/// Build: compiled by web_technology/subprocess/meson.build into
///        a standalone executable (enki_cef_subprocess).
///
/// Usage: specified via CefSettings::browser_subprocess_path in CefBridge.
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_app.h>
#include "../cef/EnkiCefApp.hpp"

int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);

    // Use EnkiSubprocessApp so the renderer process can handle
    // V8 context creation and IPC messages for JS bindings.
    CefRefPtr<enki::web::EnkiSubprocessApp> app(
        new enki::web::EnkiSubprocessApp());

    // CefExecuteProcess returns -1 if this is the main (browser) process.
    // It returns >= 0 if this is a subprocess, in which case we must exit.
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }

    // Should never reach here in subprocess mode.
    return 0;
}
