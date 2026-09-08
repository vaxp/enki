/// @file InputForwarder.cpp
/// @brief Translates Enki input events into CEF input events and forwards them.
///
/// Key translation reference:
///   https://source.chromium.org/chromium/chromium/src/+/main:ui/events/keycodes/
///
/// @copyright ENKI Framework — MIT License

#include "InputForwarder.hpp"
#include <include/cef_browser.h>
#include <include/cef_base.h>
#include <X11/keysym.h>

namespace enki::web {

// ── Mouse ──────────────────────────────────────────────────────

void InputForwarder::sendMouseMove(CefRefPtr<CefBrowser>    browser,
                                   const WebMouseMoveEvent& evt)
{
    CefMouseEvent e;
    e.x         = evt.x;
    e.y         = evt.y;
    e.modifiers = static_cast<uint32_t>(evt.modifiers);

    auto host = browser->GetHost();
    host->SendMouseMoveEvent(e, evt.leaving);
}

void InputForwarder::sendMouseClick(CefRefPtr<CefBrowser>      browser,
                                    const WebMouseClickEvent& evt)
{
    CefMouseEvent e;
    e.x         = evt.x;
    e.y         = evt.y;
    e.modifiers = static_cast<uint32_t>(evt.modifiers);

    auto type = toCefButton(evt.button);
    auto host = browser->GetHost();
    host->SendMouseClickEvent(e, type, !evt.pressed, evt.click_count);
}

void InputForwarder::sendMouseWheel(CefRefPtr<CefBrowser>       browser,
                                    const WebMouseWheelEvent& evt)
{
    CefMouseEvent e;
    e.x         = evt.x;
    e.y         = evt.y;
    e.modifiers = static_cast<uint32_t>(evt.modifiers);

    // CEF uses pixels; most frameworks deliver lines.
    // Multiply by a typical line height (20px) if the caller sends line counts.
    browser->GetHost()->SendMouseWheelEvent(e, evt.delta_x, evt.delta_y);
}

// ── Keyboard ───────────────────────────────────────────────────

void InputForwarder::sendKey(CefRefPtr<CefBrowser> browser,
                             const WebKeyEvent&    evt)
{
    CefKeyEvent e;
    e.type                  = static_cast<cef_key_event_type_t>(evt.type);
    e.modifiers             = evt.modifiers;
    e.windows_key_code      = evt.windows_key_code;
    e.native_key_code       = evt.native_key_code;
    e.is_system_key         = evt.is_system_key;
    e.character             = evt.character;
    e.unmodified_character  = evt.unmodified_character;
    e.focus_on_editable_field = false;

    browser->GetHost()->SendKeyEvent(e);
}

// ── Text / IME ─────────────────────────────────────────────────

void InputForwarder::sendText(CefRefPtr<CefBrowser>     browser,
                              const WebTextInputEvent& evt)
{
    // Inject each Unicode code point as a KEYEVENT_CHAR.
    const std::string& utf8 = evt.text_utf8;

    // Simple approach: send one Char event per character.
    // For full IME support, use CefTextInputContext / ImeCommitText.
    for (unsigned char ch : utf8) {
        if (ch < 0x80) {   // ASCII fast-path
            CefKeyEvent ke;
            ke.type             = KEYEVENT_CHAR;
            ke.windows_key_code = ch;
            ke.character        = static_cast<char16_t>(ch);
            ke.unmodified_character = ke.character;
            browser->GetHost()->SendKeyEvent(ke);
        }
    }
    // TODO: full UTF-8 decoding + multi-byte char16_t support.
}

// ── Focus ──────────────────────────────────────────────────────

void InputForwarder::sendFocus(CefRefPtr<CefBrowser> browser, bool focused)
{
    browser->GetHost()->SetFocus(focused);
}

// ── Modifier conversion ────────────────────────────────────────

uint32_t InputForwarder::modifiersFromEnki(int enki_mods)
{
    uint32_t result = 0;
    // enki::KeyMod bit layout:
    //   Shift = 1<<0, Ctrl = 1<<1, Alt = 1<<2, Super = 1<<3
    if (enki_mods & (1 << 0)) result |= static_cast<uint32_t>(WebKeyMod::Shift);
    if (enki_mods & (1 << 1)) result |= static_cast<uint32_t>(WebKeyMod::Control);
    if (enki_mods & (1 << 2)) result |= static_cast<uint32_t>(WebKeyMod::Alt);
    if (enki_mods & (1 << 3)) result |= static_cast<uint32_t>(WebKeyMod::Super);
    return result;
}

cef_mouse_button_type_t InputForwarder::toCefButton(WebMouseButton btn)
{
    switch (btn) {
        case WebMouseButton::Left:   return MBT_LEFT;
        case WebMouseButton::Middle: return MBT_MIDDLE;
        case WebMouseButton::Right:  return MBT_RIGHT;
        default:                     return MBT_LEFT;
    }
}

// ── X11 keysym → Windows VK ────────────────────────────────────

int InputForwarder::xKeysymToWindowsVK(unsigned int ks)
{
    // Covers the most common keys; extend as needed.
    // Reference: ui/events/keycodes/keyboard_codes_posix.h (Chromium)
    switch (ks) {
        case XK_BackSpace:  return 0x08;  // VK_BACK
        case XK_Tab:        return 0x09;  // VK_TAB
        case XK_Return:     return 0x0D;  // VK_RETURN
        case XK_Escape:     return 0x1B;  // VK_ESCAPE
        case XK_space:      return 0x20;  // VK_SPACE
        case XK_Delete:     return 0x2E;  // VK_DELETE
        case XK_Home:       return 0x24;  // VK_HOME
        case XK_End:        return 0x23;  // VK_END
        case XK_Page_Up:    return 0x21;  // VK_PRIOR
        case XK_Page_Down:  return 0x22;  // VK_NEXT
        case XK_Left:       return 0x25;  // VK_LEFT
        case XK_Right:      return 0x27;  // VK_RIGHT
        case XK_Up:         return 0x26;  // VK_UP
        case XK_Down:       return 0x28;  // VK_DOWN
        case XK_Insert:     return 0x2D;  // VK_INSERT
        case XK_F1:  case XK_F2:  case XK_F3:  case XK_F4:
        case XK_F5:  case XK_F6:  case XK_F7:  case XK_F8:
        case XK_F9:  case XK_F10: case XK_F11: case XK_F12:
            return 0x70 + (ks - XK_F1);  // VK_F1..VK_F12
        case XK_Shift_L: case XK_Shift_R:   return 0x10;  // VK_SHIFT
        case XK_Control_L: case XK_Control_R: return 0x11; // VK_CONTROL
        case XK_Alt_L: case XK_Alt_R:       return 0x12;  // VK_MENU
        case XK_Super_L: case XK_Super_R:   return 0x5B;  // VK_LWIN
        case XK_Caps_Lock:                  return 0x14;  // VK_CAPITAL
        default:
            // ASCII range: keysyms 0x20–0x7E map directly to VK codes.
            if (ks >= 0x20 && ks <= 0x7E)
                return static_cast<int>(ks & 0xFF);
            return 0;
    }
}

} // namespace enki::web
