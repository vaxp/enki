# 05 — Framework Developer Guide: Building the Web Engine

This guide is intended for **Enki framework contributors** who need to build, modify, or extend the Web Host subsystem (`web_technology/`).

---

## Architecture Overview

```
enki/
├── web_technology/            ← Isolated Web Host subsystem
│   ├── cef/                   ← CEF backend (CefBridge, JSBridge, handlers)
│   ├── host/                  ← Application host (EnkiWebHost, NativeAPIs, AppConfig)
│   ├── include/web_technology/← Public headers (no CEF leakage)
│   ├── subprocess/            ← CEF helper subprocess binary
│   ├── cef_binary/            ← CEF binary distribution (not committed to git)
│   ├── template/              ← Starter app template
│   └── meson.build            ← Isolated build definition
├── examples/web_host_demo.cpp ← Standalone Web Host demo
└── meson.build                ← Root build integrates web_technology
```

### Key Design Principles

1. **Strict isolation** — No CEF headers appear outside `web_technology/`. The web technology subsystem remains completely modular.
2. **Native Windowed Engine** — Creates a native X11 window; CEF renders directly into it for maximum GPU performance with zero overhead.
3. **Subprocess isolation** — CEF utilizes a dedicated helper binary (`enki_cef_subprocess`) for GPU, renderer, and utility processes.

---

## Prerequisites

### System Packages

```bash
# Debian / Ubuntu
sudo apt install \
    build-essential cmake ninja-build meson python3 pkg-config \
    libx11-dev libxcomposite-dev libxrandr-dev libxdamage-dev \
    libxfixes-dev libxi-dev libxtst-dev libnss3-dev \
    libatk-bridge2.0-dev libcups2-dev libdrm-dev libgbm-dev \
    libasound2-dev

# Arch Linux
sudo pacman -S base-devel cmake ninja meson python \
    libx11 nss atk cups libdrm mesa alsa-lib
```

### Compiler

```bash
# GCC 13+ or Clang 16+ required for C++20
gcc --version   # must report 13.x or higher
```

---

## Step 1: Download CEF Binary Distribution

The Web Host engine is powered by CEF. The binary must be obtained separately (it is not committed to the repository due to its large size ~392 MB).

### Verified Compatible Version

```
cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal
```

### Download & Extract

```bash
cd enki/web_technology

# Create the target directory
mkdir -p cef_binary
cd cef_binary

# Download
wget "https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal.tar.bz2"

# Extract
tar -xjf cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal.tar.bz2
```

Expected layout after extraction:

```
cef_binary/
└── cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal/
    ├── CMakeLists.txt
    ├── cmake/
    ├── include/             ← CEF C++ headers
    ├── libcef_dll/          ← C++ wrapper source
    ├── Release/             ← Compiled shared libraries
    │   ├── libcef.so        ← Main CEF library (1.5 GB)
    │   ├── libEGL.so
    │   ├── libGLESv2.so
    │   ├── libvk_swiftshader.so
    │   └── v8_context_snapshot.bin
    └── Resources/           ← Runtime resources
        ├── icudtl.dat       ← ICU internationalization (10 MB)
        ├── resources.pak
        ├── chrome_100_percent.pak
        ├── chrome_200_percent.pak
        └── locales/
```

---

## Step 2: Build `libcef_dll_wrapper`

The C++ wrapper library provides the high-level C++ API over the raw CEF C API. It must be compiled once per CEF version.

```bash
CEF_DIR="enki/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal"

cd "$CEF_DIR"
mkdir -p build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 \
      ..

# Build only the wrapper (skips the sample apps)
cmake --build . --target libcef_dll_wrapper -- -j$(nproc)
```

Expected output:

```
libcef_dll_wrapper/
└── libcef_dll_wrapper.a    ← ~8-15 MB static library
```

> **Note:** This step only needs to be done once. Meson detects the `.a` file automatically.

---

## Step 3: Copy Resources to Release Directory

CEF on Linux requires runtime resources (`icudtl.dat`, `*.pak`, `locales/`) to be accessible alongside `libcef.so`. Copy them into the `Release/` directory:

```bash
CEF_ROOT="enki/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal"

cp -rn "$CEF_ROOT/Resources/"* "$CEF_ROOT/Release/"
```

---

## Step 4: Configure and Build Enki

From the repository root:

```bash
# Configure (auto-detects web_technology)
meson setup build_web

# Build all targets
ninja -C build_web

# Or build only the web-related targets
ninja -C build_web web_technology/libenki_web.a
ninja -C build_web web_technology/subprocess/enki_cef_subprocess
ninja -C build_web examples/web_host_demo
```

### Verify the Build Artifacts

```bash
ls -lh build_web/web_technology/libenki_web.a
#  -rw-rw-r-- 1 x x 83K ...  web_technology/libenki_web.a

ls -lh build_web/web_technology/subprocess/enki_cef_subprocess
#  -rwxrwxr-x 1 x x 1.7M ... web_technology/subprocess/enki_cef_subprocess

ls -lh build_web/examples/web_host_demo
#  -rwxrwxr-x 1 x x 8.4M ... examples/web_host_demo
```

---

## Step 5: Run and Verify

```bash
cd enki

# Run the built-in demo app
./build_web/examples/web_host_demo
```

A native X11 window should appear. Closing it should terminate the process cleanly.

---

## Subsystem Deep Dive

### CEF Initialisation Flow

```
EnkiWebHost::run()
  ├── register_default_apis()          ← Register all built-in native APIs
  ├── IWebViewBackend::create()        ← Instantiate CefBridge
  ├── CefGlobal::ensure_initialized()  ← CefInitialize() with CefSettings
  │     ├── Sets resources_dir_path    ← Points to CEF Resources/
  │     ├── Sets locales_dir_path      ← Points to Resources/locales/
  │     ├── Sets root_cache_path       ← ~/.cache/enki_web/
  │     └── Sets browser_subprocess_path ← enki_cef_subprocess binary
  ├── CefBridge::initialize()
  │     └── create_browser()
  │           ├── Creates X11 window (windowed mode)
  │           └── CefBrowserHost::CreateBrowser()
  └── CefRunMessageLoop()              ← Drives CEF until window close
        └── [On close] CefQuitMessageLoop() ← In OnBeforeClose handler
```

### Adding a New Native API

1. **Define the API class** in `host/NativeAPIs.hpp`:

```cpp
class MyAPI : public INativeAPI {
public:
    std::string name() const override { return "my_api"; }
    bool is_permitted(const std::vector<std::string>& perms) const override {
        return std::find(perms.begin(), perms.end(), "my_api") != perms.end();
    }
    void register_functions(IWebViewBackend& backend) override;
};
```

2. **Implement the functions** in `host/NativeAPIs.cpp`:

```cpp
void MyAPI::register_functions(IWebViewBackend& backend)
{
    // Binds window.__enki_my_api_hello to a C++ lambda
    backend.bind_function("__enki_my_api_hello", [](std::string_view args_json) {
        // args_json is a JSON array string: e.g. ["arg1", 42]
        return std::string("{\"result\": \"hello from C++\"}");
    });
}
```

3. **Register the API** in `EnkiWebHost::register_default_apis()`:

```cpp
add_api(std::make_unique<MyAPI>());
```

4. **Expose it to JavaScript** by adding a JS wrapper to the bootstrap script in `build_js_bootstrap()`:

```javascript
window.enki.my_api = {
    hello: function() {
        return window.enki.__call('__enki_my_api_hello', {});
    }
};
```

5. **Grant the permission** in `enki.json`:

```json
{ "permissions": ["my_api"] }
```

---

## CEF Version Compatibility Notes

| CEF 144 API difference | Impact |
|---|---|
| `OnBeforePopup` takes `int popup_id` as 3rd param | `EnkiLifeSpanHandler` signature updated |
| `GetFrameByName` is a browser method | Fixed in `JSBridge.cpp` |
| `CefV8Context::Enter/Exit()` replaces `CefV8ContextLock` | Fixed in `EnkiCefApp.cpp` |
| `CefString` accepts `std::string` directly | Constructor fixed throughout |

---

## Build System Reference (`web_technology/meson.build`)

```
web_technology/
  └── meson.build
        ├── Finds cef_binary/ directory automatically
        ├── Verifies libcef_dll_wrapper.a exists
        ├── Defines cef_inc (include paths)
        ├── Defines cef_lib (libcef.so + libcef_dll_wrapper.a + X11)
        ├── Builds libenki_web.a (all cef/ + host/ sources)
        ├── Builds enki_cef_subprocess executable
        └── Exports web_technology_dep (dep used by examples/meson.build)
```

The root `meson.build` includes `web_technology/` **before** defining `libenki.a` so that `web_technology_dep` is available for all downstream targets.

---

## Common Build Issues

### `libcef_dll_wrapper.a: No such file or directory`

You have not compiled the CEF C++ wrapper yet. Run Step 2 above.

### `error: 'CefQuitMessageLoop' was not declared`

Include `<include/cef_app.h>` in the file using it.

### `Invalid file descriptor to ICU data received`

Resources are not accessible to CEF at runtime. Run Step 3 (copy Resources to Release/).

### `Opening in existing browser session`

Stale CEF profile lock. Delete the cache:

```bash
rm -rf ~/.cache/enki_web
```

### Stack smashing in child processes

Usually caused by a dual `Display*` connection to X11 (one from the host, one from CEF). The fix: open a Display connection to create the window, then close it immediately with `XCloseDisplay()` before handing the window handle to CEF.
