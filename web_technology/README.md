# web_technology — CEF WebView Subsystem

> **Isolation Level**: Complete. No file in this directory is included by Enki Core.
> The only bridge is `include/web_technology/IWebViewBackend.hpp`.

## Overview

This subsystem embeds **Chromium Embedded Framework (CEF)** inside Enki as a fully isolated, optionally-compiled component. It provides the `WebView` widget — a native Chromium browser surface rendered offscreen into Enki's Skia canvas.

## Architecture

```
Enki Core (Skia · Anu · Widget Tree)
    │
    │   #include <web_technology/IWebViewBackend.hpp>
    │   (the ONLY Enki→web_technology include)
    │
    ▼
IWebViewBackend  ←── pure abstract interface
    │
    ▼
CefBridge        ←── concrete CEF implementation
    │
    ├── EnkiCefApp         (CefApp + command-line switches)
    ├── EnkiCefClient      (CefClient aggregator)
    ├── EnkiRenderHandler  (Offscreen Rendering — OnPaint → SkImage)
    ├── EnkiLifeSpanHandler(browser lifecycle + popup blocking)
    ├── InputForwarder     (Enki events → CEF mouse/keyboard events)
    └── JSBridge           (eval_js + bind_function via V8 IPC)
```

## Directory Structure

```
web_technology/
├── include/web_technology/
│   ├── IWebViewBackend.hpp    ← The bridge interface (Enki Core sees this)
│   ├── WebViewFrame.hpp       ← Pixel frame carrier
│   └── WebViewEvent.hpp       ← Input event types
├── cef/
│   ├── CefBridge.{hpp,cpp}    ← IWebViewBackend implementation
│   ├── EnkiCefApp.{hpp,cpp}
│   ├── EnkiCefClient.{hpp,cpp}
│   ├── EnkiRenderHandler.{hpp,cpp}
│   ├── EnkiLifeSpanHandler.{hpp,cpp}
│   ├── InputForwarder.{hpp,cpp}
│   └── JSBridge.{hpp,cpp}
├── subprocess/
│   ├── cef_subprocess_main.cpp ← Helper process entry point
│   └── meson.build
├── meson.build
└── README.md
```

## Building

### Prerequisites

1. Download a **CEF prebuilt distribution** from [cef-builds.spotifycdn.com](https://cef-builds.spotifycdn.com/index.html)
   - Choose **Linux x86_64** → **Standard Distribution**
   - Build `libcef_dll_wrapper`:
     ```bash
     cd /path/to/cef
     mkdir build && cd build
     cmake -DCMAKE_BUILD_TYPE=Release ..
     make -j$(nproc) libcef_dll_wrapper
     ```

2. Enable the subsystem in Meson:
   ```bash
   meson setup build \
     -Denable_webview=true \
     -Dcef_root=/path/to/cef-prebuilt
   ```

### Without WebView

```bash
meson setup build   # enable_webview defaults to false — builds fine without CEF
```

## Isolation Rules

| Rule | Enforcement |
|---|---|
| No `#include <cef_*.h>` outside `web_technology/` | Meson compile boundary |
| Web Host subsystem is self-contained | `web_technology/meson.build` |
| CEF runs its own event loop in windowed mode | Direct native X11 integration |
| Subprocess is a separate binary | `subprocess/meson.build` |

## Usage

```cpp
#include <web_technology/EnkiWebHost.hpp>

int main(int argc, char* argv[]) {
    enki::web::EnkiWebHost host(argc, argv);
    
    // Load app configuration from enki.json manifest
    host.load_config("my_app/enki.json");
    
    // Start the desktop application
    return host.run();
}
```


## CEF Message Loop Integration

CEF requires `CefDoMessageLoopWork()` to be called from the main thread on each frame. In Enki's render loop, add:

```cpp
// In your app frame loop:
#ifdef ENKI_HAS_WEBVIEW
    enki::web::CefGlobal::do_message_loop_work();
#endif
```
