#pragma once
/// @file WebViewEvent.hpp
/// @brief Input event types forwarded from Enki to the WebView backend.
///
/// These types mirror Enki's native input events but are defined independently
/// so that the web_technology subsystem has zero dependency on Enki Core headers.
/// The InputForwarder translates enki::PointerEvent / enki::KeyEvent into these.
///
/// @copyright ENKI Framework — MIT License

#include <cstdint>
#include <string>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// Mouse Button
// ════════════════════════════════════════════════════════════════

enum class WebMouseButton : int {
    Left   = 0,
    Middle = 1,
    Right  = 2,
    None   = -1,
};

// ════════════════════════════════════════════════════════════════
// Keyboard Modifier Flags (matches CEF EVENTFLAG_*)
// ════════════════════════════════════════════════════════════════

enum class WebKeyMod : uint32_t {
    None        = 0,
    CapsLock    = 1 << 0,
    Shift       = 1 << 1,
    Control     = 1 << 2,
    Alt         = 1 << 3,
    LeftButton  = 1 << 6,
    MiddleButton= 1 << 7,
    RightButton = 1 << 8,
    Super       = 1 << 9,
    IsKeyPad    = 1 << 10,
    IsLeft      = 1 << 11,
    IsRight     = 1 << 12,
};

inline WebKeyMod operator|(WebKeyMod a, WebKeyMod b) {
    return static_cast<WebKeyMod>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool operator&(WebKeyMod a, WebKeyMod b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

// ════════════════════════════════════════════════════════════════
// Mouse Move / Enter / Leave Event
// ════════════════════════════════════════════════════════════════

struct WebMouseMoveEvent {
    int        x         = 0;
    int        y         = 0;
    WebKeyMod  modifiers = WebKeyMod::None;
    bool       leaving   = false;   // true → mouse left the view
};

// ════════════════════════════════════════════════════════════════
// Mouse Button Event
// ════════════════════════════════════════════════════════════════

struct WebMouseClickEvent {
    int           x           = 0;
    int           y           = 0;
    WebMouseButton button     = WebMouseButton::Left;
    bool           pressed    = true;
    int            click_count = 1;   // 2 = double-click
    WebKeyMod      modifiers   = WebKeyMod::None;
};

// ════════════════════════════════════════════════════════════════
// Mouse Wheel Event
// ════════════════════════════════════════════════════════════════

struct WebMouseWheelEvent {
    int       x        = 0;
    int       y        = 0;
    int       delta_x  = 0;
    int       delta_y  = 0;
    WebKeyMod modifiers = WebKeyMod::None;
};

// ════════════════════════════════════════════════════════════════
// Key Event
// ════════════════════════════════════════════════════════════════

enum class WebKeyEventType : int {
    RawKeyDown = 0,
    KeyDown    = 1,
    KeyUp      = 2,
    Char       = 3,
};

struct WebKeyEvent {
    WebKeyEventType type       = WebKeyEventType::RawKeyDown;
    uint32_t        modifiers  = 0;
    int             windows_key_code = 0;   // VK_* code (CEF uses Windows key codes)
    int             native_key_code  = 0;   // X11 keycode
    bool            is_system_key    = false;
    char16_t        character        = 0;
    char16_t        unmodified_character = 0;
};

// ════════════════════════════════════════════════════════════════
// Text Input Event (IME / composed text)
// ════════════════════════════════════════════════════════════════

struct WebTextInputEvent {
    std::string text_utf8;
};

// ════════════════════════════════════════════════════════════════
// Touch Event (future-proof placeholder)
// ════════════════════════════════════════════════════════════════

struct WebTouchPoint {
    int   id           = 0;
    float x            = 0.0f;
    float y            = 0.0f;
    float radius_x     = 1.0f;
    float radius_y     = 1.0f;
    float rotation     = 0.0f;
    float pressure     = 1.0f;
};

} // namespace enki::web
