#pragma once
/// @file InputForwarder.hpp
/// @brief Translates Enki input events into CEF input events.
///
/// Enki uses its own event types (enki::PointerEvent, enki::KeyEvent).
/// CEF uses CefMouseEvent, CefKeyEvent, etc.
/// InputForwarder converts between them and forwards to the browser.
///
/// All translation happens in the static helper methods; no state is needed.
///
/// @copyright ENKI Framework — MIT License

#include <include/cef_browser.h>
#include <include/cef_base.h>
#include <web_technology/WebViewEvent.hpp>

namespace enki::web {

class InputForwarder {
public:
    // ── Mouse ──────────────────────────────────────────────────

    static void sendMouseMove(CefRefPtr<CefBrowser>      browser,
                              const WebMouseMoveEvent&   event);

    static void sendMouseClick(CefRefPtr<CefBrowser>     browser,
                               const WebMouseClickEvent& event);

    static void sendMouseWheel(CefRefPtr<CefBrowser>      browser,
                               const WebMouseWheelEvent& event);

    // ── Keyboard ───────────────────────────────────────────────

    static void sendKey(CefRefPtr<CefBrowser>  browser,
                        const WebKeyEvent&     event);

    // ── Text / IME ─────────────────────────────────────────────

    static void sendText(CefRefPtr<CefBrowser>       browser,
                         const WebTextInputEvent&    event);

    // ── Focus ──────────────────────────────────────────────────

    static void sendFocus(CefRefPtr<CefBrowser> browser, bool focused);

    // ── Helpers: Enki → WebViewEvent translation ───────────────

    /// Convert an enki::KeyMod bitmask to a WebKeyMod bitmask.
    static uint32_t modifiersFromEnki(int enki_mods);

    /// Convert a WebMouseButton to CEF's MouseButtonType.
    static cef_mouse_button_type_t toCefButton(WebMouseButton btn);

    // ── X11 → Windows VK key code mapping ─────────────────────
    // CEF uses Windows virtual-key codes even on Linux.

    /// Map an X11 keysym to a Windows VK code for CEF.
    static int xKeysymToWindowsVK(unsigned int keysym);

private:
    InputForwarder() = delete;
};

} // namespace enki::web
