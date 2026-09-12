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

### Linux System Packages

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

### Windows Prerequisites

- **Visual Studio 2022 (Build Tools or Community/Pro)**:
  - Workload: *Desktop development with C++* (MSVC v143+ compiler supporting C++20).
  - Windows 10/11 SDK.
  - CMake and Ninja (bundled with Visual Studio Build Tools under `Common7\IDE\CommonExtensions\Microsoft\CMake`).
- **Meson Build System**:
  - Available via MSYS2 (`C:\msys64\ucrt64\bin\meson.exe`) or Python `pip install meson`.
- **Git for Windows**:
  - Provides GNU `tar` and `bzip2` under `C:\Program Files\Git\usr\bin\` for extracting `.tar.bz2` CEF archives.

### Compilers

- **Linux**: GCC 13+ or Clang 16+ (C++20 required).
- **Windows**: MSVC 19.34+ (Visual Studio 2022 v17.4+ with `/std:c++20`).

---

## Step 1: Download CEF Binary Distribution

The Web Host engine is powered by Chromium Embedded Framework (CEF). The binary must be obtained separately (it is not committed to the repository due to its large size).

### Verified Compatible Versions

- **Linux (x64)**: `cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal`
- **Windows (x64)**: `cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal`

---

### Linux (x64) Setup

```bash
cd modules/web_technology
mkdir -p cef_binary && cd cef_binary

# Download Linux minimal build (~130 MB)
wget "https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal.tar.bz2"

# Extract
tar -xjf cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal.tar.bz2
```

---

### Windows (x64) Setup

On Windows, download the archive and extract it directly into `modules/web_technology/cef_binary/` using Git's GNU `tar`:

- **Official Windows Package URL**:
  `https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal.tar.bz2`
- **Target Extraction Directory**:
  `modules/web_technology/cef_binary/`

#### PowerShell Automated Download & Extraction:

```powershell
cd modules\web_technology
if (-not (Test-Path "cef_binary")) { New-Item -ItemType Directory -Name "cef_binary" }
cd cef_binary

$url = "https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal.tar.bz2"
curl.exe -L -o cef_win.tar.bz2 $url

# Use Git's GNU tar to decompress .tar.bz2 properly on Windows
$env:PATH = "C:\Program Files\Git\usr\bin;$env:PATH"
& "C:\Program Files\Git\usr\bin\tar.exe" -xjf cef_win.tar.bz2
Remove-Item cef_win.tar.bz2
```

Expected layout after extraction on Windows:

```
modules/web_technology/cef_binary/
└── cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal/
    ├── CMakeLists.txt
    ├── cmake/
    ├── include/             ← CEF C++ headers
    ├── libcef_dll/          ← C++ wrapper source
    ├── Release/             ← Binaries & Import Library
    │   ├── libcef.lib       ← Windows import library (65 KB)
    │   ├── libcef.dll       ← Main CEF engine (253 MB)
    │   ├── chrome_elf.dll
    │   ├── d3dcompiler_47.dll
    │   ├── dxcompiler.dll
    │   ├── libEGL.dll / libGLESv2.dll
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

The C++ wrapper library provides the high-level C++ API over the raw CEF C API. It must be compiled once per CEF distribution.

### Linux

```bash
CEF_DIR="modules/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal"
cd "$CEF_DIR"
mkdir -p build && cd build

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 ..
cmake --build . --target libcef_dll_wrapper -- -j$(nproc)
```

Expected artifact: `libcef_dll_wrapper/libcef_dll_wrapper.a` (~15 MB).

### Windows

> [!IMPORTANT]
> **Windows Considerations**:
> 1. **Avoid MAX_PATH Overflow**: CMake object paths for CEF test files can exceed Windows' default 260-character `MAX_PATH` limit. Build in a short directory path (e.g. `C:\cef_b` or `enki\cef_b`) and then copy the output library.
> 2. **CRT Flag Matching**: Enki is configured with `b_vscrt: mt` (static CRT). You **MUST** configure CEF wrapper with `-DCEF_RUNTIME_LIBRARY_FLAG=/MT` (the default) so both use `/MT` (`libcpmt.lib`), preventing `LNK2005` runtime library mismatch errors.

#### Windows Build Commands (Command Prompt with MSVC):

```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

set "CEF_DIR=modules\web_technology\cef_binary\cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal"
set "BUILD_DIR=cef_b"

:: 1. Configure with short build path and /MT CRT
cmake -S "%CEF_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCEF_RUNTIME_LIBRARY_FLAG=/MT -DUSE_SANDBOX=OFF

:: 2. Build the wrapper library
ninja -C "%BUILD_DIR%" libcef_dll_wrapper

:: 3. Copy output library to CEF Release folder
copy /y "%BUILD_DIR%\libcef_dll_wrapper\libcef_dll_wrapper.lib" "%CEF_DIR%\Release\libcef_dll_wrapper.lib"

:: 4. Clean up temporary short build folder
rmdir /s /q "%BUILD_DIR%"
```

Expected artifact: `Release/libcef_dll_wrapper.lib` (~76 MB static library).

---

## Step 3: Deploy Runtime Resources & DLLs

### Linux

CEF on Linux requires runtime resources (`icudtl.dat`, `*.pak`, `locales/`) accessible alongside `libcef.so`:

```bash
CEF_ROOT="modules/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal"
cp -rn "$CEF_ROOT/Resources/"* "$CEF_ROOT/Release/"
```

### Windows

On Windows, CEF executables (`web_host_demo.exe` and `enki_cef_subprocess.exe`) load `libcef.dll`, `chrome_elf.dll`, V8 snapshots, and UI resources directly from their working directory. Deploy them with this PowerShell script:

```powershell
$cefDir = "modules\web_technology\cef_binary\cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal"
$releaseDir = "$cefDir\Release"
$resourcesDir = "$cefDir\Resources"

# Targets: examples folder and subprocess folder
$destinations = @("build-Win\examples", "build-Win\modules\web_technology\subprocess")

foreach ($dest in $destinations) {
    if (-not (Test-Path $dest)) { New-Item -ItemType Directory -Path $dest -Force }
    Copy-Item "$releaseDir\*.dll" $dest -Force
    Copy-Item "$releaseDir\*.bin" $dest -Force
    Copy-Item "$resourcesDir\icudtl.dat" $dest -Force
    Copy-Item "$resourcesDir\*.pak" $dest -Force
    Copy-Item "$resourcesDir\locales" "$dest\locales" -Recurse -Force
}
```

---

## Step 4: Configure and Build Enki

### Linux Build

From the repository root:

```bash
# Configure (auto-detects web_technology)
meson setup build_web -Denable_webview=true -Dbuild_examples=true

# Build all web-related targets
ninja -C build_web modules/web_technology/subprocess/enki_cef_subprocess examples/web_host_demo
```

### Windows Build (MSVC 2022)

From PowerShell or Developer Command Prompt:

```cmd
:: 1. Configure Meson build directory for Windows
meson setup --reconfigure build-Win -Denable_webview=true -Dbuild_examples=true

:: 2. Compile Web Host and CEF Helper Subprocess using MSVC x64
cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"" && ninja -C build-Win modules/web_technology/subprocess/enki_cef_subprocess.exe examples/web_host_demo.exe"
```

### Verify the Build Artifacts

#### Linux Artifacts
```bash
ls -lh build_web/modules/web_technology/subprocess/enki_cef_subprocess
ls -lh build_web/examples/web_host_demo
```

#### Windows Artifacts
```powershell
Get-Item build-Win\modules\web_technology\subprocess\enki_cef_subprocess.exe
Get-Item build-Win\examples\web_host_demo.exe
```

---

## Step 5: Run and Verify

### Linux Execution
```bash
# Run the built-in demo app
./build_web/examples/web_host_demo
```
A native X11 window appears rendering Chromium with full hardware acceleration.

### Windows Execution
```powershell
# Run the built-in demo app on Windows
.\build-Win\examples\web_host_demo.exe
```
A native Win32 window (`HWND`) appears running Chromium 144 with Direct3D 11 GPU acceleration, displaying the dynamic hardware metrics (CPU, RAM, storage) and 60 FPS WebGL test.

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

### Windows: `LNK2005: already defined in LIBCMT.lib / MSVCRT.lib`

Enki uses static C++ runtime (`/MT`, `b_vscrt: mt`). If `libcef_dll_wrapper.lib` was compiled with `/MD`, the linker throws duplicate symbol conflicts. **Fix**: Re-configure CMake with `-DCEF_RUNTIME_LIBRARY_FLAG=/MT` and re-build the wrapper.

### Windows: `Fatal error C1083: Cannot open compiler intermediate file ... path too long`

The default Windows path limit (`MAX_PATH` = 260 characters) is exceeded by deep CEF object trees if built inside `modules\web_technology\cef_binary\...`. **Fix**: Build inside a short path such as `-B cef_b` or `-B C:\cef_b`, then copy `libcef_dll_wrapper.lib` to the CEF `Release/` directory.

### Windows: Subprocess Crash or Blank White Window

If `web_host_demo.exe` opens but remains blank white or crashes on startup:
1. Ensure `chrome_elf.dll` and `v8_context_snapshot.bin` are in the same folder as the `.exe`.
2. Ensure `icudtl.dat`, `resources.pak`, `chrome_100_percent.pak`, and the `locales/` directory are copied to the executable directory.
3. Verify that `enki_cef_subprocess.exe` exists in `build-Win\modules\web_technology\subprocess\enki_cef_subprocess.exe`.

---

## Windows Platform Support & Deep Dive (دعم منصة Windows)

### 1. CEF Binary Package for Windows (حزمة CEF المعتمدة)

* **Direct Download Link (رابط التحميل المباشر)**:
  ```
  https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal.tar.bz2
  ```
* **Extraction Directory (مسار فك الضغط الإلزامي)**:
  `modules/web_technology/cef_binary/`

### 2. Extraction Procedure (طريقة فك الضغط)

Windows standard `tar.exe` lacks native bzip2 decompression filters in older updates. It is recommended to use Git's GNU tar:

```powershell
cd modules\web_technology
if (-not (Test-Path "cef_binary")) { New-Item -ItemType Directory -Name "cef_binary" }
cd cef_binary

# Download archive (~144 MB)
curl.exe -L -o cef_win.tar.bz2 "https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal.tar.bz2"

# Decompress using Git GNU tar
& "C:\Program Files\Git\usr\bin\tar.exe" -xjf cef_win.tar.bz2
Remove-Item cef_win.tar.bz2
```

The resulting folder must be:
`modules/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal/`

### 3. Preparation & Compilation of `libcef_dll_wrapper.lib` (تهيئة وبناء المكتبة)

The C++ wrapper must be compiled using MSVC (`v143+`) with static CRT (`/MT`) matching Enki's build configuration:

```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

set "CEF_DIR=modules\web_technology\cef_binary\cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_windows64_minimal"
set "BUILD_DIR=cef_b"

:: Configure using Ninja and short path to prevent MAX_PATH overflow
cmake -S "%CEF_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCEF_RUNTIME_LIBRARY_FLAG=/MT -DUSE_SANDBOX=OFF

:: Compile wrapper
ninja -C "%BUILD_DIR%" libcef_dll_wrapper

:: Deploy static library to Release directory
copy /y "%BUILD_DIR%\libcef_dll_wrapper\libcef_dll_wrapper.lib" "%CEF_DIR%\Release\libcef_dll_wrapper.lib"

:: Clean temporary build directory
rmdir /s /q "%BUILD_DIR%"
```

### 4. Windows Architecture & Integration Highlights

* **Native Win32 Implementation (`NativeAPIs_win.cpp`)**:
  * Unlike Linux which reads `/proc/meminfo`, `/proc/stat`, and `/proc/version`, Windows uses direct Win32 API functions:
    * **Memory**: `GlobalMemoryStatusEx()` providing total, available, and used physical RAM.
    * **CPU**: `GetSystemInfo()` providing architecture and core count; high-resolution frequency via `QueryPerformanceCounter()`.
    * **Storage**: `GetDiskFreeSpaceExW()` querying drive quotas and free disk space.
    * **Shell**: `ShellExecuteW()` to launch external links in the default Windows browser.
    * **Mouse & Screen**: `GetCursorPos()` and `GetSystemMetrics(SM_CXSCREEN / SM_CYSCREEN)` for coordinates.
* **Direct3D 11 Hardware Acceleration**:
  * CEF on Windows integrates with ANGLE and native Direct3D 11 (`d3dcompiler_47.dll`, `dxcompiler.dll`), offering Zero-Copy rendering directly to the Win32 window (`HWND`) at maximum frame rates.
* **Alloy Window Mode**:
  * `windowless_rendering_enabled = false` embeds the Chromium rendering surface directly into the parent Win32 window, handling WM_PAINT, resizing, and input routing with minimal latency.
